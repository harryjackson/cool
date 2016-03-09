//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the
// public domain. The author hereby disclaims copyright to this source
// code.

#ifndef COOL_MURMURHASH3_H_
#define COOL_MURMURHASH3_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

void cool_MurmurHash3_x86_32 (const void *key, int len, uint32_t seed, void *out);

void cool_MurmurHash3_x86_128(const void *key, int len, uint32_t seed, void *out);

void cool_MurmurHash3_x64_128(const void *key, int len, uint32_t seed, void *out);

//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // COOL_MURMURHASH3_H_
