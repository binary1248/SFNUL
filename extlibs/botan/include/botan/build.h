#ifndef BOTAN_BUILD_CONFIG_H_
#define BOTAN_BUILD_CONFIG_H_

#define BOTAN_VERSION_MAJOR 2
#define BOTAN_VERSION_MINOR 6
#define BOTAN_VERSION_PATCH 0
#define BOTAN_VERSION_DATESTAMP 0

#define BOTAN_VERSION_RELEASE_TYPE "unreleased"

#define BOTAN_VERSION_VC_REVISION "unknown"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 32

#define BOTAN_DLL

/* Target identification and feature test macros */

#if defined(__clang__)
    #define BOTAN_BUILD_COMPILER_IS_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define BOTAN_BUILD_COMPILER_IS_GCC
#elif defined(__INTEL_COMPILER)
    #define BOTAN_BUILD_COMPILER_IS_INTEL
#elif defined(_MSC_VER)
    #define BOTAN_BUILD_COMPILER_IS_MSVC
#else
    #error Unsupported compiler
#endif


#if defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define BOTAN_TARGET_CPU_IS_X86_FAMILY
    #define BOTAN_TARGET_ARCH_IS_X86_64
    #define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
    #define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT

    #define BOTAN_HAS_ENTROPY_SRC_RDRAND 20131128
    #define BOTAN_HAS_ENTROPY_SRC_RDSEED 20151218
    #define BOTAN_HAS_SHA1_SSE2 20160803
    #define BOTAN_HAS_SHA1_X86_SHA_NI 20160803
    #define BOTAN_HAS_SHA2_32_X86 20160803
    #define BOTAN_HAS_GCM_CLMUL 20131227
    #define BOTAN_HAS_GCM_CLMUL_SSSE3 20171020
    #define BOTAN_HAS_CHACHA_SSE2 20160831
#elif defined(__i386__) || defined(_X86_) || defined(_M_I86) || defined(_M_IX86)
    #define BOTAN_TARGET_CPU_IS_X86_FAMILY
    #define BOTAN_TARGET_ARCH_IS_X86_32
    #define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN

    #define BOTAN_HAS_ENTROPY_SRC_RDRAND 20131128
    #define BOTAN_HAS_ENTROPY_SRC_RDSEED 20151218
    #define BOTAN_HAS_SHA1_SSE2 20160803
    #define BOTAN_HAS_SHA1_X86_SHA_NI 20160803
    #define BOTAN_HAS_SHA2_32_X86 20160803
    #define BOTAN_HAS_GCM_CLMUL 20131227
    #define BOTAN_HAS_GCM_CLMUL_SSSE3 20171020
    #define BOTAN_HAS_CHACHA_SSE2 20160831
#elif defined(__aarch64__)
    #define BOTAN_TARGET_CPU_IS_ARM_FAMILY
    #define BOTAN_TARGET_ARCH_IS_ARM64
    #define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
    #define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT

    #define BOTAN_HAS_SHA1_ARMV8 20160803
    #define BOTAN_HAS_SHA2_32_ARMV8 20160803

    #define BOTAN_TARGET_CPU_HAS_GCM_PMULL 20170903
#elif defined(__arm__) ||  defined(__thumb__) ||  defined(_M_ARM) ||  defined(_M_ARMT)
    #define BOTAN_TARGET_CPU_IS_ARM_FAMILY
    #define BOTAN_TARGET_ARCH_IS_ARM32
    #define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN

    #define BOTAN_TARGET_CPU_HAS_GCM_PMULL 20170903
#elif defined(__powerpc__) || defined(_M_PPC)
    #define BOTAN_TARGET_CPU_IS_PPC_FAMILY
#else
    #error Unsupported CPU architecture
#endif

#if defined(WIN32)
    #define BOTAN_TARGET_OS_IS_WINDOWS

    #define BOTAN_TARGET_OS_HAS_WIN32
    #define BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK
    #define BOTAN_TARGET_OS_HAS_RTLGENRANDOM
    #define BOTAN_TARGET_OS_HAS_WINSOCK2

    #define BOTAN_HAS_ENTROPY_SRC_WIN32 20131128
#else
    #if defined(__FreeBSD__)
        #define BOTAN_TARGET_OS_IS_FREEBSD
    #elif defined(__OpenBSD__)
        #define BOTAN_TARGET_OS_IS_OPENBSD
    #elif defined(__linux__)
        #define BOTAN_TARGET_OS_IS_LINUX
    #endif

    #define BOTAN_TARGET_OS_HAS_POSIX1
    #define BOTAN_TARGET_OS_HAS_POSIX_MLOCK
    #define BOTAN_TARGET_OS_HAS_CLOCK_GETTIME
    #define BOTAN_TARGET_OS_HAS_PROC_FS
    #define BOTAN_TARGET_OS_HAS_DEV_RANDOM
    #define BOTAN_TARGET_OS_HAS_SOCKETS

    #define BOTAN_HAS_ENTROPY_SRC_PROC_WALKER 20131128
#endif

#define BOTAN_TARGET_OS_HAS_FILESYSTEM
#define BOTAN_TARGET_OS_HAS_THREADS

#define BOTAN_TARGET_SUPPORTS_AESNI
#define BOTAN_TARGET_SUPPORTS_AVX2
#define BOTAN_TARGET_SUPPORTS_BMI2
#define BOTAN_TARGET_SUPPORTS_RDRAND
#define BOTAN_TARGET_SUPPORTS_RDSEED
#define BOTAN_TARGET_SUPPORTS_SHA
#define BOTAN_TARGET_SUPPORTS_SSE2
#define BOTAN_TARGET_SUPPORTS_SSE41
#define BOTAN_TARGET_SUPPORTS_SSE42
#define BOTAN_TARGET_SUPPORTS_SSSE3
#define BOTAN_TARGET_SUPPORTS_ARMV8CRYPTO
#define BOTAN_TARGET_SUPPORTS_NEON

/*
* Module availability definitions
*/
#define BOTAN_HAS_ADLER32 20131128
#define BOTAN_HAS_AEAD_CCM 20131128
#define BOTAN_HAS_AEAD_CHACHA20_POLY1305 20141228
#define BOTAN_HAS_AEAD_EAX 20131128
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AEAD_OCB 20131128
#define BOTAN_HAS_AEAD_SIV 20131202
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_AES_NI 20131128
#define BOTAN_HAS_AES_SSSE3 20131128
#define BOTAN_HAS_ANSI_X919_MAC 20131128
#define BOTAN_HAS_ARIA 20170415
#define BOTAN_HAS_ASN1 20171109
#define BOTAN_HAS_AUTO_RNG 20161126
#define BOTAN_HAS_AUTO_SEEDING_RNG 20160821
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BCRYPT 20131128
#define BOTAN_HAS_BIGINT 20131128
#define BOTAN_HAS_BIGINT_MP 20151225
#define BOTAN_HAS_BLAKE2B 20130131
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_BLOWFISH 20131128
#define BOTAN_HAS_CAMELLIA 20150922
#define BOTAN_HAS_CASCADE 20131128
#define BOTAN_HAS_CAST 20131128
#define BOTAN_HAS_CAST_128 20171203
#define BOTAN_HAS_CAST_256 20171203
#define BOTAN_HAS_CBC_MAC 20131128
#define BOTAN_HAS_CECPQ1 20161116
#define BOTAN_HAS_CERTSTOR_SQL 20160818
#define BOTAN_HAS_CHACHA 20140103
#define BOTAN_HAS_CHACHA_RNG 20170728
#define BOTAN_HAS_CIPHER_MODES 20180124
#define BOTAN_HAS_CIPHER_MODE_PADDING 20131128
#define BOTAN_HAS_CMAC 20131128
#define BOTAN_HAS_CODEC_FILTERS 20131128
#define BOTAN_HAS_COMB4P 20131128
#define BOTAN_HAS_CPUID 20170917
#define BOTAN_HAS_CRC24 20131128
#define BOTAN_HAS_CRC32 20131128
#define BOTAN_HAS_CRYPTO_BOX 20131128
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_CURVE_25519 20170621
#define BOTAN_HAS_DES 20131128
#define BOTAN_HAS_DIFFIE_HELLMAN 20131128
#define BOTAN_HAS_DLIES 20160713
#define BOTAN_HAS_DL_GROUP 20131128
#define BOTAN_HAS_DL_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_DSA 20131128
#define BOTAN_HAS_DYNAMIC_LOADER 20160310
#define BOTAN_HAS_ECC_GROUP 20170225
#define BOTAN_HAS_ECC_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_ECDH 20131128
#define BOTAN_HAS_ECDSA 20131128
#define BOTAN_HAS_ECGDSA 20160301
#define BOTAN_HAS_ECIES 20160128
#define BOTAN_HAS_ECKCDSA 20160413
#define BOTAN_HAS_EC_CURVE_GFP 20131128
#define BOTAN_HAS_ED25519 20170607
#define BOTAN_HAS_ELGAMAL 20131128
#define BOTAN_HAS_EME_OAEP 20180305
#define BOTAN_HAS_EME_PKCS1v15 20131128
#define BOTAN_HAS_EME_RAW 20150313
#define BOTAN_HAS_EMSA1 20131128
#define BOTAN_HAS_EMSA_PKCS1 20140118
#define BOTAN_HAS_EMSA_PSSR 20131128
#define BOTAN_HAS_EMSA_RAW 20131128
#define BOTAN_HAS_EMSA_X931 20140118
#define BOTAN_HAS_ENTROPY_SOURCE 20151120
#define BOTAN_HAS_FFI 20170815
#define BOTAN_HAS_FILTERS 20160415
#define BOTAN_HAS_FPE_FE1 20131128
#define BOTAN_HAS_GMAC 20160207
#define BOTAN_HAS_GOST_28147_89 20131128
#define BOTAN_HAS_GOST_34_10_2001 20131128
#define BOTAN_HAS_GOST_34_11 20131128
#define BOTAN_HAS_HASH 20180112
#define BOTAN_HAS_HASH_ID 20131128
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HKDF 20170927
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_DRBG 20140319
#define BOTAN_HAS_HOTP 20170513
#define BOTAN_HAS_HTTP_UTIL 20171003
#define BOTAN_HAS_IDEA 20131128
#define BOTAN_HAS_IDEA_SSE2 20131128
#define BOTAN_HAS_ISO_9796 20161121
#define BOTAN_HAS_KASUMI 20131128
#define BOTAN_HAS_KDF1 20131128
#define BOTAN_HAS_KDF1_18033 20160128
#define BOTAN_HAS_KDF2 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KECCAK 20131128
#define BOTAN_HAS_KEYPAIR_TESTING 20131128
#define BOTAN_HAS_LION 20131128
#define BOTAN_HAS_LOCKING_ALLOCATOR 20131128
#define BOTAN_HAS_MAC 20150626
#define BOTAN_HAS_MCEIES 20150706
#define BOTAN_HAS_MCELIECE 20150922
#define BOTAN_HAS_MD4 20131128
#define BOTAN_HAS_MD5 20131128
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MEM_POOL 20180309
#define BOTAN_HAS_MGF1 20140118
#define BOTAN_HAS_MISTY1 20131128
#define BOTAN_HAS_MODES 20150626
#define BOTAN_HAS_MODE_CBC 20131128
#define BOTAN_HAS_MODE_CFB 20131128
#define BOTAN_HAS_MODE_XTS 20131128
#define BOTAN_HAS_NEWHOPE 20161018
#define BOTAN_HAS_NIST_KEYWRAP 20171119
#define BOTAN_HAS_NOEKEON 20131128
#define BOTAN_HAS_NOEKEON_SIMD 20160903
#define BOTAN_HAS_NUMBERTHEORY 20131128
#define BOTAN_HAS_OCSP 20161118
#define BOTAN_HAS_OFB 20131128
#define BOTAN_HAS_PACKAGE_TRANSFORM 20131128
#define BOTAN_HAS_PARALLEL_HASH 20131128
#define BOTAN_HAS_PASSHASH9 20131128
#define BOTAN_HAS_PBKDF 20150626
#define BOTAN_HAS_PBKDF1 20131128
#define BOTAN_HAS_PBKDF2 20131128
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PGP_S2K 20170527
#define BOTAN_HAS_PKCS11 20160219
#define BOTAN_HAS_PKCS5_PBES2 20141119
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_POLY1305 20141227
#define BOTAN_HAS_POLY_DBL 20170927
#define BOTAN_HAS_PSK_DB 20171119
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RC4 20131128
#define BOTAN_HAS_RDRAND_RNG 20160619
#define BOTAN_HAS_RFC3394_KEYWRAP 20131128
#define BOTAN_HAS_RFC6979_GENERATOR 20140321
#define BOTAN_HAS_RIPEMD_160 20131128
#define BOTAN_HAS_RSA 20160730
#define BOTAN_HAS_SALSA20 20171114
#define BOTAN_HAS_SEED 20131128
#define BOTAN_HAS_SERPENT 20131128
#define BOTAN_HAS_SERPENT_SIMD 20160903
#define BOTAN_HAS_SHA1 20131128
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SHA3 20161018
#define BOTAN_HAS_SHACAL2 20170813
#define BOTAN_HAS_SHACAL2_SIMD 20170813
#define BOTAN_HAS_SHAKE 20161009
#define BOTAN_HAS_SHAKE_CIPHER 20161018
#define BOTAN_HAS_SIMD_32 20131128
#define BOTAN_HAS_SIPHASH 20150110
#define BOTAN_HAS_SKEIN_512 20131128
#define BOTAN_HAS_SM2 20170907
#define BOTAN_HAS_SM3 20170402
#define BOTAN_HAS_SM4 20170716
#define BOTAN_HAS_SOCKETS 20171216
#define BOTAN_HAS_SP800_108 20160128
#define BOTAN_HAS_SP800_56A 20170501
#define BOTAN_HAS_SP800_56C 20160211
#define BOTAN_HAS_SRP6 20161017
#define BOTAN_HAS_STATEFUL_RNG 20160819
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_STREEBOG 20170623
#define BOTAN_HAS_SYSTEM_RNG 20141202
#define BOTAN_HAS_THREAD_UTILS 20180112
#define BOTAN_HAS_THREEFISH_512 20131224
#define BOTAN_HAS_THREEFISH_512_AVX2 20160903
#define BOTAN_HAS_THRESHOLD_SECRET_SHARING 20131128
#define BOTAN_HAS_TIGER 20131128
#define BOTAN_HAS_TLS 20150319
#define BOTAN_HAS_TLS_CBC 20161008
#define BOTAN_HAS_TLS_SESSION_MANAGER_SQL_DB 20141219
#define BOTAN_HAS_TLS_V10_PRF 20131128
#define BOTAN_HAS_TLS_V12_PRF 20131128
#define BOTAN_HAS_TOTP 20170519
#define BOTAN_HAS_TWOFISH 20131128
#define BOTAN_HAS_UTIL_FUNCTIONS 20171003
#define BOTAN_HAS_WHIRLPOOL 20131128
#define BOTAN_HAS_X509_CERTIFICATES 20151023
#define BOTAN_HAS_X942_PRF 20131128
#define BOTAN_HAS_XMSS 20161008
#define BOTAN_HAS_XTEA 20131128


/*
* Local/misc configuration options (if any) follow
*/


/*
* Things you can edit (but probably shouldn't)
*/

#if !defined(BOTAN_DEPRECATED_PUBLIC_MEMBER_VARIABLES)

  #if defined(BOTAN_NO_DEPRECATED)
     #define BOTAN_DEPRECATED_PUBLIC_MEMBER_VARIABLES private
  #else
     #define BOTAN_DEPRECATED_PUBLIC_MEMBER_VARIABLES public
  #endif

#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Minimum and maximum sizes to allocate out of the mlock pool (bytes)
   Default min is 16 as smaller values are easily bruteforceable and thus
   likely not cryptographic keys.
*/
#define BOTAN_MLOCK_ALLOCATOR_MIN_ALLOCATION 16
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/*
* Total maximum amount of RAM (in KiB) we will lock into memory, even
* if the OS would let us lock more
*/
#define BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB 512

/*
* If enabled uses memset via volatile function pointer to zero memory,
* otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/*
* Set number of bits used to generate mask for blinding the
* representation of an ECC point. Set to zero to disable this
* side-channel countermeasure.
*/
#define BOTAN_POINTGFP_RANDOMIZE_BLINDING_BITS 80

/*
* Normally blinding is performed by choosing a random starting point (plus
* its inverse, of a form appropriate to the algorithm being blinded), and
* then choosing new blinding operands by successive squaring of both
* values. This is much faster than computing a new starting point but
* introduces some possible corelation
*
* To avoid possible leakage problems in long-running processes, the blinder
* periodically reinitializes the sequence. This value specifies how often
* a new sequence should be started.
*/
#define BOTAN_BLINDING_REINIT_INTERVAL 32

/*
* Userspace RNGs like HMAC_DRBG will reseed after a specified number
* of outputs are generated. Set to zero to disable automatic reseeding.
*/
#define BOTAN_RNG_DEFAULT_RESEED_INTERVAL 1024
#define BOTAN_RNG_RESEED_POLL_BITS 256

#define BOTAN_RNG_AUTO_RESEED_TIMEOUT std::chrono::milliseconds(10)
#define BOTAN_RNG_RESEED_DEFAULT_TIMEOUT std::chrono::milliseconds(50)

/*
* Specifies (in order) the list of entropy sources that will be used
* to seed an in-memory RNG. The first in the default list: "rdseed"
* and "rdrand" do not count as contributing any entropy but are
* included as they are fast and help protect against a seriously
* broken system RNG.
*/
#define BOTAN_ENTROPY_DEFAULT_SOURCES \
   { "rdseed", "rdrand", "darwin_secrandom", "getentropy", \
     "dev_random", "system_rng", "proc_walk", "system_stats" }

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/*
* These control the RNG used by the system RNG interface
*/
#define BOTAN_SYSTEM_RNG_DEVICE "/dev/urandom"
#define BOTAN_SYSTEM_RNG_POLL_DEVICES { "/dev/urandom", "/dev/random", "/dev/srandom" }

/*
* This directory will be monitored by ProcWalking_EntropySource and
* the contents provided as entropy inputs to the RNG. May also be
* usefully set to something like "/sys", depending on the system being
* deployed to. Set to an empty string to disable.
*/
#define BOTAN_ENTROPY_PROC_FS_PATH "/proc"

/*
* These paramaters control how many bytes to read from the system
* PRNG, and how long to block if applicable. The timeout only applies
* to reading /dev/urandom and company.
*/
#define BOTAN_SYSTEM_RNG_POLL_REQUEST 64
#define BOTAN_SYSTEM_RNG_POLL_TIMEOUT_MS 20

/*
How many times to read from the RDRAND/RDSEED RNGs.
Each read generates 32 bits of output
*/
#define BOTAN_ENTROPY_INTEL_RNG_POLLS 32

// According to Intel, RDRAND is guaranteed to generate a random number within 10 retries on a working CPU
#define BOTAN_ENTROPY_RDRAND_RETRIES 10

/*
* RdSeed is not guaranteed to generate a random number within a specific number of retries
* Define the number of retries here
*/
#define BOTAN_ENTROPY_RDSEED_RETRIES 20

/*
* If no way of dynamically determining the cache line size for the
* system exists, this value is used as the default. Used by the side
* channel countermeasures rather than for alignment purposes, so it is
* better to be on the smaller side if the exact value cannot be
* determined. Typically 32 or 64 bytes on modern CPUs.
*/
#if !defined(BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE)
  #define BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE 32
#endif

/**
* Controls how AutoSeeded_RNG is instantiated
*/
#if !defined(BOTAN_AUTO_RNG_HMAC)

  #if defined(BOTAN_HAS_SHA2_64)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-384)"
  #elif defined(BOTAN_HAS_SHA2_32)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-256)"
  #elif defined(BOTAN_HAS_SHA3)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-3(256))"
  #elif defined(BOTAN_HAS_SHA1)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-1)"
  #endif
  // Otherwise, no hash found: leave BOTAN_AUTO_RNG_HMAC undefined

#endif

// Check for a common build problem:

#if defined(BOTAN_TARGET_ARCH_IS_X86_64) && ((defined(_MSC_VER) && !defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__x86_64__)) || \
                                             (defined(__GNUG__) && !defined(__x86_64__)))
    #error "Trying to compile Botan configured as x86_64 with non-x86_64 compiler."
#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_32) && ((defined(_MSC_VER) && defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__i386__)) || \
                                             (defined(__GNUG__) && !defined(__i386__)))

    #error "Trying to compile Botan configured as x86_32 with non-x86_32 compiler."
#endif

#include <botan/compiler.h>

#endif

