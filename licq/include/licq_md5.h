/*
 * md5.h:  Header file for Colin Plumb's MD5 implementation.
 *         Modified by Ian Jackson so as not to use Colin Plumb's
 *         'usuals.h'.
 * 
 *         This file is in the public domain.
 */

#ifndef MD5_H
#define MD5_H

#include <stddef.h>
#include <stdint.h>

#define MD5_DIGEST_LENGTH 16

void md5(const uint8_t* buf, size_t len, uint8_t* digest);

struct MD5Context
{
  uint32_t buf[4];
  uint32_t bytes[2];
  uint32_t in[16];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context* context, uint8_t const* buf, size_t len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(uint32_t buf[4], const uint32_t in[16]);

void byteSwap(uint32_t* buf, unsigned words);

#endif				/* !MD5_H */
