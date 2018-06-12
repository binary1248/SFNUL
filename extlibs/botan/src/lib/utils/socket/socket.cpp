/*
* (C) 2015,2016,2017 Jack Lloyd
* (C) 2016 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/socket.h>
#include <botan/exceptn.h>
#include <botan/mem_ops.h>
#include <chrono>

#if defined(BOTAN_HAS_BOOST_ASIO)
  /*
  * We don't need serial port support anyway, and asking for it
  * causes macro conflicts with Darwin's termios.h when this
  * file is included in the amalgamation. GH #350
  */
  #define BOOST_ASIO_DISABLE_SERIAL_PORT
  #include <boost/asio.hpp>
  #include <boost/asio/system_timer.hpp>

#elif defined(BOTAN_TARGET_OS_HAS_SOCKETS)
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>
  #include <fcntl.h>

#elif defined(BOTAN_TARGET_OS_HAS_WINSOCK2)
  #define NOMINMAX 1
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_HAS_BOOST_ASIO)

class Asio_Socket final : public OS::Socket
   {
   public:
      Asio_Socket(const std::string& hostname,
                  const std::string& service,
                  std::chrono::milliseconds timeout) :
         m_timeout(timeout), m_timer(m_io), m_tcp(m_io)
         {
         m_timer.expires_from_now(m_timeout);
         check_timeout();

         boost::asio::ip::tcp::resolver resolver(m_io);
         boost::asio::ip::tcp::resolver::query query(hostname, service);
         boost::asio::ip::tcp::resolver::iterator dns_iter = resolver.resolve(query);

         boost::system::error_code ec = boost::asio::error::would_block;

         auto connect_cb = [&ec](const boost::system::error_code& e,
                                 boost::asio::ip::tcp::resolver::iterator) { ec = e; };

         boost::asio::async_connect(m_tcp, dns_iter, connect_cb);

         while(ec == boost::asio::error::would_block)
            {
            m_io.run_one();
            }

         if(ec)
            throw boost::system::system_error(ec);
         if(ec || m_tcp.is_open() == false)
            throw Exception("Connection to host " + hostname + " failed");
         }

      void write(const uint8_t buf[], size_t len) override
         {
         m_timer.expires_from_now(m_timeout);

         boost::system::error_code ec = boost::asio::error::would_block;

         boost::asio::async_write(m_tcp, boost::asio::buffer(buf, len),
                                  [&ec](boost::system::error_code e, size_t got) { printf("wrote %d\n", got); ec = e; });

         while(ec == boost::asio::error::would_block) { m_io.run_one(); }

         if(ec)
            {
            throw boost::system::system_error(ec);
            }
         }

      size_t read(uint8_t buf[], size_t len) override
         {
         m_timer.expires_from_now(m_timeout);

         boost::system::error_code ec = boost::asio::error::would_block;
         size_t got = 0;

         auto read_cb = [&](const boost::system::error_code cb_ec, size_t cb_got) {
            ec = cb_ec; got = cb_got;
         };

         m_tcp.async_read_some(boost::asio::buffer(buf, len), read_cb);

         while(ec == boost::asio::error::would_block) { m_io.run_one(); }

         if(ec)
            {
            if(ec == boost::asio::error::eof)
               return 0;
            throw boost::system::system_error(ec); // Some other error.
            }

         return got;
         }

   private:
      void check_timeout()
         {
         if(m_tcp.is_open() && m_timer.expires_at() < std::chrono::system_clock::now())
            {
            boost::system::error_code err;
            m_tcp.close(err);
            }

         m_timer.async_wait(std::bind(&Asio_Socket::check_timeout, this));
         }

      const std::chrono::milliseconds m_timeout;
      boost::asio::io_context m_io;
      boost::asio::system_timer m_timer;
      boost::asio::ip::tcp::socket m_tcp;
   };

#elif defined(BOTAN_TARGET_OS_HAS_SOCKETS) || defined(BOTAN_TARGET_OS_HAS_WINSOCK2)

class BSD_Socket final : public OS::Socket
   {
   private:
#if defined(BOTAN_TARGET_OS_HAS_WINSOCK2)
      typedef SOCKET socket_type;
      typedef int socket_op_ret_type;
      static socket_type invalid_socket() { return INVALID_SOCKET; }
      static void close_socket(socket_type s) { ::closesocket(s); }
      static std::string get_last_socket_error() { return std::to_string(::WSAGetLastError()); }

      static bool nonblocking_connect_in_progress()
         {
         return (::WSAGetLastError() == WSAEWOULDBLOCK);
         }

      static void set_nonblocking(socket_type s)
         {
         u_long nonblocking = 1;
         ::ioctlsocket(s, FIONBIO, &nonblocking);
         }

      static void socket_init()
         {
         WSAData wsa_data;
         WORD wsa_version = MAKEWORD(2, 2);

         if (::WSAStartup(wsa_version, &wsa_data) != 0)
            {
            throw Exception("WSAStartup() failed: " + std::to_string(WSAGetLastError()));
            }

         if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
            {
            ::WSACleanup();
            throw Exception("Could not find a usable version of Winsock.dll");
            }
         }

      static void socket_fini()
         {
         ::WSACleanup();
         }
#else
      typedef int socket_type;
      typedef ssize_t socket_op_ret_type;
      static socket_type invalid_socket() { return -1; }
      static void close_socket(socket_type s) { ::close(s); }
      static std::string get_last_socket_error() { return ::strerror(errno); }
      static bool nonblocking_connect_in_progress() { return (errno == EINPROGRESS); }
      static void set_nonblocking(socket_type s)
         {
         if(::fcntl(s, F_SETFL, O_NONBLOCK) < 0)
            throw Exception("Setting socket to non-blocking state failed");
         }

      static void socket_init() {}
      static void socket_fini() {}
#endif

   public:
      BSD_Socket(const std::string& hostname,
                 const std::string& service,
                 std::chrono::microseconds timeout) : m_timeout(timeout)
         {
         socket_init();

         m_socket = invalid_socket();

         addrinfo hints;
         ::memset(&hints, 0, sizeof(addrinfo));
         hints.ai_family = AF_UNSPEC;
         hints.ai_socktype = SOCK_STREAM;
         addrinfo* res;

         if(::getaddrinfo(hostname.c_str(), service.c_str(), &hints, &res) != 0)
            {
            throw Exception("Name resolution failed for " + hostname);
            }

         for(addrinfo* rp = res; (m_socket == invalid_socket()) && (rp != nullptr); rp = rp->ai_next)
            {
            if(rp->ai_family != AF_INET && rp->ai_family != AF_INET6)
               continue;

            m_socket = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

            if(m_socket == invalid_socket())
               {
               // unsupported socket type?
               continue;
               }

            set_nonblocking(m_socket);

            int err = ::connect(m_socket, rp->ai_addr, rp->ai_addrlen);

            if(err == -1)
               {
               int active = 0;
               if(nonblocking_connect_in_progress())
                  {
                  struct timeval timeout_tv = make_timeout_tv();
                  fd_set write_set;
                  FD_ZERO(&write_set);
                  FD_SET(m_socket, &write_set);

                  active = ::select(m_socket + 1, nullptr, &write_set, nullptr, &timeout_tv);

                  if(active)
                     {
                     int socket_error = 0;
                     socklen_t len = sizeof(socket_error);

                     if(::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socket_error), &len) < 0)
                        throw Exception("Error calling getsockopt");

                     if(socket_error != 0)
                        {
                        active = 0;
                        }
                     }
                  }

               if(active == 0)
                  {
                  close_socket(m_socket);
                  m_socket = invalid_socket();
                  continue;
                  }
               }
            }

         ::freeaddrinfo(res);

         if(m_socket == invalid_socket())
            {
            throw Exception("Connecting to " + hostname +
                            " for service " + service + " failed");
            }
         }

      ~BSD_Socket()
         {
         close_socket(m_socket);
         m_socket = invalid_socket();
         socket_fini();
         }

      void write(const uint8_t buf[], size_t len) override
         {
         fd_set write_set;
         FD_ZERO(&write_set);
         FD_SET(m_socket, &write_set);

         size_t sent_so_far = 0;
         while(sent_so_far != len)
            {
            struct timeval timeout = make_timeout_tv();
            int active = ::select(m_socket + 1, nullptr, &write_set, nullptr, &timeout);

            if(active == 0)
               throw Exception("Timeout during socket write");

            const size_t left = len - sent_so_far;
            socket_op_ret_type sent = ::send(m_socket, cast_uint8_ptr_to_char(&buf[sent_so_far]), left, 0);
            if(sent < 0)
               throw Exception("Socket write failed with error '" +
                               std::string(::strerror(errno)) + "'");
            else
               sent_so_far += static_cast<size_t>(sent);
            }
         }

      size_t read(uint8_t buf[], size_t len) override
         {
         fd_set read_set;
         FD_ZERO(&read_set);
         FD_SET(m_socket, &read_set);

         struct timeval timeout = make_timeout_tv();
         int active = ::select(m_socket + 1, &read_set, nullptr, nullptr, &timeout);

         if(active == 0)
            throw Exception("Timeout during socket read");

         socket_op_ret_type got = ::recv(m_socket, cast_uint8_ptr_to_char(buf), len, 0);

         if(got < 0)
            throw Exception("Socket read failed with error '" +
                            std::string(::strerror(errno)) + "'");
         return static_cast<size_t>(got);
         }

   private:
      struct timeval make_timeout_tv() const
         {
         struct timeval tv;
         tv.tv_sec = m_timeout.count() / 1000000;
         tv.tv_usec = m_timeout.count() % 1000000;
         return tv;
         }

      const std::chrono::microseconds m_timeout;
      socket_type m_socket;
   };

#endif

}

std::unique_ptr<OS::Socket>
OS::open_socket(const std::string& hostname,
                const std::string& service,
                std::chrono::milliseconds timeout)
   {
#if defined(BOTAN_HAS_BOOST_ASIO)
   return std::unique_ptr<OS::Socket>(new Asio_Socket(hostname, service, timeout));

#elif defined(BOTAN_TARGET_OS_HAS_SOCKETS) || defined(BOTAN_TARGET_OS_HAS_WINSOCK2)
   return std::unique_ptr<OS::Socket>(new BSD_Socket(hostname, service, timeout));

#else
   // No sockets for you
   return std::unique_ptr<Socket>();
#endif
   }

}
