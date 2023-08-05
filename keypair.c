#include <sodium.h>

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

void nc_keypair() {
  unsigned char pk[crypto_box_PUBLICKEYBYTES];
  unsigned char sk[crypto_box_SECRETKEYBYTES];
  char buf[MAX(crypto_box_PUBLICKEYBYTES, crypto_box_SECRETKEYBYTES) * 2 + 1];
  crypto_box_keypair(pk, sk);
  printf("private: %s\n", sodium_bin2hex(buf, sizeof(buf), sk, crypto_box_SECRETKEYBYTES));
  printf("public:  %s\n", sodium_bin2hex(buf, sizeof(buf), pk, crypto_box_PUBLICKEYBYTES));
  sodium_memzero(sk, crypto_box_SECRETKEYBYTES);
  exit(EXIT_SUCCESS);
}