/*
* (C) 2011,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/nist_keywrap.h>
#include <botan/block_cipher.h>
#include <botan/loadstor.h>
#include <botan/exceptn.h>

namespace Botan {

namespace {

std::vector<uint8_t>
raw_nist_key_wrap(const uint8_t input[],
                  size_t input_len,
                  const BlockCipher& bc,
                  uint64_t ICV)
   {
   const size_t n = (input_len + 7) / 8;

   secure_vector<uint8_t> R((n + 1) * 8);
   secure_vector<uint8_t> A(16);

   store_be(ICV, A.data());

   copy_mem(&R[8], input, input_len);

   for(size_t j = 0; j <= 5; ++j)
      {
      for(size_t i = 1; i <= n; ++i)
         {
         const uint32_t t = static_cast<uint32_t>((n * j) + i);

         copy_mem(&A[8], &R[8*i], 8);

         bc.encrypt(A.data());
         copy_mem(&R[8*i], &A[8], 8);

         uint8_t t_buf[4] = { 0 };
         store_be(t, t_buf);
         xor_buf(&A[4], t_buf, 4);
         }
      }

   copy_mem(R.data(), A.data(), 8);

   return std::vector<uint8_t>(R.begin(), R.end());
   }

secure_vector<uint8_t>
raw_nist_key_unwrap(const uint8_t input[],
                    size_t input_len,
                    const BlockCipher& bc,
                    uint64_t& ICV_out)
   {
   if(input_len < 16 || input_len % 8 != 0)
      throw Invalid_Argument("Bad input size for NIST key unwrap");

   const size_t n = (input_len - 8) / 8;

   secure_vector<uint8_t> R(n * 8);
   secure_vector<uint8_t> A(16);

   for(size_t i = 0; i != 8; ++i)
      A[i] = input[i];

   copy_mem(R.data(), input + 8, input_len - 8);

   for(size_t j = 0; j <= 5; ++j)
      {
      for(size_t i = n; i != 0; --i)
         {
         const uint32_t t = static_cast<uint32_t>((5 - j) * n + i);

         uint8_t t_buf[4] = { 0 };
         store_be(t, t_buf);

         xor_buf(&A[4], t_buf, 4);

         copy_mem(&A[8], &R[8*(i-1)], 8);

         bc.decrypt(A.data());

         copy_mem(&R[8*(i-1)], &A[8], 8);
         }
      }

   ICV_out = load_be<uint64_t>(A.data(), 0);

   return R;
   }

}

std::vector<uint8_t>
nist_key_wrap(const uint8_t input[],
              size_t input_len,
              const BlockCipher& bc)
   {
   if(bc.block_size() != 16)
      throw Invalid_Argument("NIST key wrap algorithm requires a 128-bit cipher");

   if(input_len % 8 != 0)
      throw Invalid_Argument("Bad input size for NIST key wrap");

   return raw_nist_key_wrap(input, input_len, bc, 0xA6A6A6A6A6A6A6A6);
   }

secure_vector<uint8_t>
nist_key_unwrap(const uint8_t input[],
                size_t input_len,
                const BlockCipher& bc)
   {
   if(bc.block_size() != 16)
      throw Invalid_Argument("NIST key wrap algorithm requires a 128-bit cipher");

   if(input_len < 16 || input_len % 8 != 0)
      throw Invalid_Argument("Bad input size for NIST key unwrap");

   uint64_t ICV_out = 0;

   secure_vector<uint8_t> R = raw_nist_key_unwrap(input, input_len, bc, ICV_out);

   if(ICV_out != 0xA6A6A6A6A6A6A6A6)
      throw Integrity_Failure("NIST key unwrap failed");

   return R;
   }

std::vector<uint8_t>
nist_key_wrap_padded(const uint8_t input[],
                     size_t input_len,
                     const BlockCipher& bc)
   {
   if(bc.block_size() != 16)
      throw Invalid_Argument("NIST key wrap algorithm requires a 128-bit cipher");

   const uint64_t ICV = 0xA65959A600000000 | static_cast<uint32_t>(input_len);

   if(input_len <= 8)
      {
      /*
      * Special case for small inputs: if input <= 8 bytes just use ECB
      */
      std::vector<uint8_t> block(16);
      store_be(ICV, block.data());
      copy_mem(block.data() + 8, input, input_len);
      bc.encrypt(block);
      return block;
      }
   else
      {
      return raw_nist_key_wrap(input, input_len, bc, ICV);
      }
   }

secure_vector<uint8_t>
nist_key_unwrap_padded(const uint8_t input[],
                       size_t input_len,
                       const BlockCipher& bc)
   {
   if(bc.block_size() != 16)
      throw Invalid_Argument("NIST key wrap algorithm requires a 128-bit cipher");

   if(input_len < 16 || input_len % 8 != 0)
      throw Invalid_Argument("Bad input size for NIST key unwrap");

   uint64_t ICV_out = 0;
   secure_vector<uint8_t> R;

   if(input_len == 16)
      {
      secure_vector<uint8_t> block(input, input + input_len);
      bc.decrypt(block);

      ICV_out = load_be<uint64_t>(block.data(), 0);
      R.resize(8);
      copy_mem(R.data(), block.data() + 8, 8);
      }
   else
      {
      R = raw_nist_key_unwrap(input, input_len, bc, ICV_out);
      }

   if((ICV_out >> 32) != 0xA65959A6)
      throw Integrity_Failure("NIST key unwrap failed");

   const size_t len = (ICV_out & 0xFFFFFFFF);

   if(len > R.size() || len < R.size() - 8)
      throw Integrity_Failure("NIST key unwrap failed");

   const size_t padding = R.size() - len;

   for(size_t i = 0; i != padding; ++i)
      {
      if(R[R.size() - i - 1] != 0)
         throw Integrity_Failure("NIST key unwrap failed");
      }

   R.resize(R.size() - padding);

   return R;
   }

}
