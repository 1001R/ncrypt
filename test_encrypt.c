#include <string.h>
#include <assert.h>
#include <errno.h>

#include "test.c"
#include "encrypt.c"


void test_readhexkey() {
  unsigned char key[32];

  unsigned char key0[32] = {
    0x6a, 0x41, 0x18, 0x32, 0xa1, 0xe1, 0x84, 0x92, 
    0x2a, 0xfb, 0xe5, 0x4f, 0xfc, 0xf5, 0x4b, 0x56, 
    0x30, 0x54, 0x4e, 0xbb, 0xd6, 0x71, 0xf5, 0xe7, 
    0x2f, 0x0f, 0x04, 0x83, 0xbe, 0x27, 0x07, 0x97, 
  };
  unsigned char key1[32] = {
    0xd3, 0xc0, 0x46, 0x62, 0x03, 0x24, 0x17, 0xd8,
    0xf2, 0x08, 0xab, 0x09, 0xbb, 0x7b, 0x33, 0x6c,
    0x0d, 0x98, 0xd0, 0x20, 0x5e, 0x76, 0x9a, 0x70,
    0x36, 0xb6, 0x1b, 0x81, 0x71, 0x57, 0x4b, 0x5e, 
  };

  memset(key, 0, 32);
  ASSERT(readhexkey("tests/readhexkey/key0", key) == 0);
  ASSERT(memcmp(key, key0, 32) == 0);

  memset(key, 0, 32);
  ASSERT(readhexkey("tests/readhexkey/key1", key) == 0);
  ASSERT(memcmp(key, key1, 32) == 0);

  memset(key, 0, 32);
  ASSERT(readhexkey("tests/readhexkey/key2", key) == 0);
  ASSERT(memcmp(key, key1, 32) == 0);

  memset(key, 0, 32);
  ASSERT(readhexkey("tests/readhexkey/key3", key) == -2);
  ASSERT(memcmp(key, key1, 32) == 0);

  ASSERT(readhexkey("tests/readhexkey/key4", key) == -2);

  ASSERT(readhexkey("tests/readhexkey/doesnotexist", key) == -1);
  ASSERT(errno == ENOENT);
}

void test_readfile() {
    char buf[10];

    memset(buf, 0, sizeof(buf));
    ASSERT(readsecret("tests/readfile/file0", buf, sizeof(buf)) == 10);
    ASSERT(strncmp(buf, "0123456789", 10) == 0);

    ASSERT(readsecret("tests/readfile/file1", buf, sizeof(buf)) == -1);

    ASSERT(readsecret("tests/readfile/doesnotexist", buf, sizeof(buf)) == -1);
    ASSERT(errno == ENOENT);
}

extern randombytes_implementation rb_well512;

void main() {
  TEST(readhexkey)
  TEST(readfile)
  // test_readhexkey();
  // test_readfile();

  // randombytes_set_implementation(&rb_well512);
  // (void) sodium_init();
  // create_keypair();

}
