/*
 * md5.h:  Header file for Colin Plumb's MD5 implementation.
 *         Modified by Ian Jackson so as not to use Colin Plumb's
 *         'usuals.h'.
 *
 *         This file is in the public domain.
 */

#ifndef LICQDAEMON_MD5_H
#define LICQDAEMON_MD5_H

#include <stddef.h>
#include <stdint.h>

namespace LicqDaemon
{

struct Md5Context
{
  uint32_t buf[4];
  uint64_t bytes;
  uint32_t in[16];
};

void md5Init(struct Md5Context* context);
void md5Update(struct Md5Context* context, uint8_t const* buf, size_t len);
void md5Final(struct Md5Context* context, unsigned char digest[16]);

} // namespace LicqDaemon

#endif
