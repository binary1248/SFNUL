
#ifndef BOTAN_BUILD_CONFIG_H__
#define BOTAN_BUILD_CONFIG_H__

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 11
#define BOTAN_VERSION_PATCH 8
#define BOTAN_VERSION_DATESTAMP 0

#define BOTAN_VERSION_RELEASE_TYPE "unreleased"

#define BOTAN_VERSION_VC_REVISION "git:19cf90aa74efdf6c03eca29717d6d5ac91c8d221"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#ifndef BOTAN_DLL
  #define BOTAN_DLL
#endif

#ifdef _MSC_VER
  #define BOTAN_NOEXCEPT
  #define __func__ __FUNCTION__
  #pragma warning(disable : 4800)
#else
  #define BOTAN_NOEXCEPT noexcept
#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Maximum size to allocate out of the mlock pool */
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 4096

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 64

/* PK key consistency checking toggles */
#define BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD 1
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD 0
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE 1

/*
* RNGs will automatically poll the system for additional
* seed material after producing this many bytes of output.
*/
#define BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED 512
#define BOTAN_RNG_RESEED_POLL_BITS 128

/* Should we use GCC-style inline assembler? */
#define BOTAN_USE_GCC_INLINE_ASM 0

#ifdef __GNUC__
  #define BOTAN_GCC_VERSION \
     (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/* Target identification and feature test macros */
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 0

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

#define BOTAN_BUILD_COMPILER_IS_GCC

#if defined(_MSC_VER)
  // 4250: inherits via dominance (diamond inheritence issue)
  // 4251: needs DLL interface (STL DLL exports)
  #pragma warning(disable: 4250 4251)
#endif

/*
* Compile-time deprecatation warnings
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)

    #if BOTAN_GCC_VERSION >= 450
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
    #else
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))
    #endif

  #endif

#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_ALGORITHM_FACTORY 20131128
#define BOTAN_HAS_ASN1 20131128
#define BOTAN_HAS_AUTO_SEEDING_RNG 20131128
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BIGINT 20131128
#define BOTAN_HAS_BIGINT_MATH 20131128
#define BOTAN_HAS_BIGINT_MP 20131128
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_CIPHER_MODE_PADDING 20131128
#define BOTAN_HAS_CODEC_FILTERS 20131128
#define BOTAN_HAS_CORE_ENGINE 20131128
#define BOTAN_HAS_CREDENTIALS_MANAGER 20131128
#define BOTAN_HAS_CRYPTOBOX_PSK 20131128
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_DIFFIE_HELLMAN 20131128
#define BOTAN_HAS_DL_GROUP 20131128
#define BOTAN_HAS_DL_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_ECC_GROUP 20131128
#define BOTAN_HAS_ECC_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_ECDH 20131128
#define BOTAN_HAS_ECDSA 20131128
#define BOTAN_HAS_EC_CURVE_GFP 20131128
#define BOTAN_HAS_EME_PKCS1v15 20131128
#define BOTAN_HAS_EMSA_PKCS1 20140118
#define BOTAN_HAS_ENGINES 20131128
#define BOTAN_HAS_FILTERS 20131128
#define BOTAN_HAS_HASH_ID 20131128
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_RNG 20131128
#define BOTAN_HAS_HTTP_UTIL 20131128
#define BOTAN_HAS_IF_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_KDF2 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KEYPAIR_TESTING 20131128
#define BOTAN_HAS_MD5 20131128
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MODE_CBC 20131128
#define BOTAN_HAS_OCSP 20131128
#define BOTAN_HAS_OID_LOOKUP 20131128
#define BOTAN_HAS_PASSWORD_BASED_ENCRYPTION 20131128
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RSA 20131128
#define BOTAN_HAS_SHA1 20131128
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SRP6 20131128
#define BOTAN_HAS_SSL3_MAC 20131128
#define BOTAN_HAS_SSL_V3_PRF 20131128
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_TLS 20131128
#define BOTAN_HAS_TLS_V10_PRF 20131128
#define BOTAN_HAS_TLS_V12_PRF 20131128
#define BOTAN_HAS_TRANSFORM 20131209
#define BOTAN_HAS_UTIL_FUNCTIONS 20131128
#define BOTAN_HAS_X509_CERTIFICATES 20131128
#define BOTAN_HAS_PARALLEL_HASH 20131128

/*
* Local configuration options (if any) follow
*/


#endif
