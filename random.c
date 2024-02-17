#include <sodium.h>

static unsigned long state[16] = {
  0xe33a40e9a1b19bd9,
  0xb47cfadfb7b155f9,
  0x6e1e70a35df816d3,
  0xa1f48c74cd12345b,
  0x127d59d2a5f54725,
  0x833d6d9787cc80e5,
  0xc2efb5a87c74c652,
  0x6f4978e695058ee0,
  0x88f4863810febbed,
  0x42e5acbe447960e3,
  0x16fe1539db2042bf,
  0x47046d9d80c1024c,
  0x7fb7103167b463fd,
  0x10939f5030ce6a1b,
  0xd37e7356d7490318,
  0xfa75b326edeef485,
};
/* init should also reset this to 0 */
static unsigned int index = 0;
/* return 32 bit random number */
static unsigned long WELLRNG512(void)
{
  unsigned long a, b, c, d;
  a = state[index];
  c = state[(index+13)&15];
  b = a^c^(a<<16)^(c<<15);
  c = state[(index+9)&15];
  c ^= (c>>11);
  a = state[index] = b^c; 
  d = a^((a<<5)&0xDA442D24UL);
  index = (index + 15)&15;
  a = state[index];
  state[index] = a^b^d^(a<<2)^(b<<18)^(c<<28);
  return state[index];
}

static const char *w512_name() {
  return "WELL512";
}

static uint32_t w512_random() {
  return (WELLRNG512() & 0xffffffff);
}

static void w512_stir() {
  
}

static void w512_buf(void * const buf, const size_t bufsz) {
  unsigned long *lp = buf;
  size_t n = bufsz;
  while (n >= sizeof(unsigned long)) {
    *lp++ = WELLRNG512();
    n -= sizeof(unsigned long);
  }
  unsigned char *cp = (void *)lp;
  unsigned long r = WELLRNG512();
  while (n) {
    *cp++ = r & 0xff;
    r >>= 8;
    --n;
  }
}

randombytes_implementation rb_well512 = {
  .implementation_name = w512_name,
  .random = w512_random,
  .stir = w512_stir,
  .uniform = NULL,
  .buf = w512_buf,
};
