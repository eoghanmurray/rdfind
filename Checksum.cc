/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/
#include "Checksum.hh"
#include <array>
#include <cassert>
#include <cstdio>

#include <cstring> //for memcpy
// all this rubbish to include sha.h from nettle in the right way
#ifdef __cplusplus
extern "C" {
#include <nettle/md5.h>
#include <nettle/sha.h>
}
#else
#include <nettle/md5.h>
#include <nettle/sha.h>
#endif

// this is a small function to print the checksum to stdout
void
Checksum::display_hex(unsigned length, const void* data_)
{
  const char* data = static_cast<const char*>(data_);
  for (unsigned i = 0; i < length; i++)
    std::printf("%02x", data[i]);
  std::printf("\n");
}

// destructor
Checksum::~Checksum()
{
  release();
}

// deletes allocated memory
int
Checksum::release()
{
  switch (m_checksumtype) {
    case NOTSET:
      return 0;
      break;
    case SHA1:
      delete (struct sha1_ctx*)m_state;
      break;
    case MD5:
      delete (struct md5_ctx*)m_state;
      break;
    default:
      // this is never reached
      return -1;
  }
  m_state = 0;
  return 0;
}

// init
int
Checksum::init(int checksumtype)
{

  // first release allocated memory if it exists
  if (release())
    return -1;

  m_checksumtype = checksumtype;

  // one may not init to something stupid
  if (m_checksumtype == NOTSET)
    return -2;

  switch (m_checksumtype) {
    case SHA1: {
      m_state = new struct sha1_ctx;
      sha1_init((sha1_ctx*)m_state);
    } break;
    case MD5: {
      m_state = new struct md5_ctx;
      // fixme assert
      md5_init((md5_ctx*)m_state);
    } break;
    default:
      // not allowed to have something that is not recognized.
      return -1;
  }
  return 0;
}

// update
int
Checksum::update(unsigned int length, unsigned char* buffer)
{
  switch (m_checksumtype) {
    case SHA1:
      sha1_update((sha1_ctx*)m_state, length, buffer);
      break;
    case MD5:
      md5_update((md5_ctx*)m_state, length, buffer);
      break;
    default:
      return -1;
  }
  return 0;
}

int
Checksum::print()
{
  // the result is stored in this variable
  switch (m_checksumtype) {
    case SHA1: {
      std::array<unsigned char, SHA1_DIGEST_SIZE> digest;
      sha1_digest((sha1_ctx*)m_state, SHA1_DIGEST_SIZE, digest.data());
      display_hex(SHA1_DIGEST_SIZE, digest.data());
    } break;
    case MD5: {
      std::array<unsigned char, MD5_DIGEST_SIZE> digest;
      md5_digest((md5_ctx*)m_state, MD5_DIGEST_SIZE, digest.data());
      display_hex(MD5_DIGEST_SIZE, digest.data());
    } break;
    default:
      return -1;
  }
  return 0;
}

// returns the number of bytes that the buffer needs to be
int
Checksum::getDigestLength() const
{
  switch (m_checksumtype) {
    case SHA1:
      return SHA1_DIGEST_SIZE;
    case MD5:
      return MD5_DIGEST_SIZE;
    default:
      return -1;
  }
  return -1;
}

// writes the checksum to buffer
int
Checksum::printToBuffer(void* buffer, std::size_t N)
{

  assert(buffer);

  switch (m_checksumtype) {
    case SHA1:
      if (N >= SHA1_DIGEST_SIZE) {
        sha1_digest((sha1_ctx*)m_state,
                    SHA1_DIGEST_SIZE,
                    static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
    case MD5:
      if (N >= MD5_DIGEST_SIZE) {
        md5_digest((md5_ctx*)m_state,
                   MD5_DIGEST_SIZE,
                   static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
    default:
      return -1;
  }
  return 0;
}
