#include <stdlib.h>
#include <fcntl.h>
#include <sodium.h>
#include <err.h>
#include <unistd.h>
#include <string.h>

static int
readhexkey(const char *path, unsigned char key[32]) {
  char buf[128], *p = buf;
  size_t bufsz = sizeof(buf), klen;
  int fd, n;

  if (strcmp(path, "-") == 0) {
    fd = STDIN_FILENO;
  } else if ((fd = open(path, O_RDONLY)) == -1) {
    return -1;
  }

  while (bufsz && (n = read(fd, p, bufsz)) > 0) {
    bufsz -= n;
    p += n;
  }
  if (fd != STDIN_FILENO) {
    close(fd);
  }
  if (n == -1) {
    return -1;
    // err(EXIT_FAILURE, "cannot read public key");
  }
  if (n > 0) {
    return -2;
    // errx(EXIT_FAILURE, "invalid public key");
  }
  bufsz = sizeof(buf) - bufsz;
  if (sodium_hex2bin(key, 32, buf, bufsz, ": \t\r\n", &klen, NULL) == -1 || klen != 32) {
    return -2;
    // errx(EXIT_FAILURE, "invalid public key");
  }
  return 0;
}

static int
readfile(const char *path, unsigned char *buf, size_t bufsz) {
  size_t r;
  unsigned char *p = buf;
  int fd;

  if (strcmp(path, "-") == 0) {
    fd = STDIN_FILENO;
  } else if ((fd = open(path, O_RDONLY)) == -1) {
    return -1;
  }
  while (bufsz && (r = read(fd, p, bufsz)) > 0) {
    bufsz -= r;
    p += r;
  }
  // if the buffer is full, but the last read operation didn't return 0 (EOF)
  // try to read another byte. If we succeed there is more data available and 
  // the buffer is too small.
  if (r > 0 && bufsz == 0) {
    char c;
    r = read(fd, &c, 1);
  }
  close(fd);
  if (r > 0) {
    return -2;
  }
  return r == -1 ? -1 : p - buf;
}

#define ERR(rv, ...) \
    if ((rv) == -1) err(EXIT_FAILURE, __VA_ARGS__); \
    else if ((rv) == -2) errx(EXIT_FAILURE, __VA_ARGS__)

void
encrypt(const char *pkx, const char *secret, const char *output) {
  unsigned char pk[crypto_box_PUBLICKEYBYTES], 
                esk[crypto_box_SECRETKEYBYTES];
  unsigned char sec[4096], *ct;
  char *ct64, *p;
  unsigned char nonce[crypto_box_NONCEBYTES] = {0};
  int r, secsz, ofd;
  size_t n, n64;
  BEGIN

  if (strcmp(output, "-")) {
    ofd = STDOUT_FILENO;
  } else if ((ofd = open(output, O_CREAT | O_TRUNC | O_WRONLY, 0644)) == -1) {
    ERR(`"Cannot open `%s' for writing", output')
  }
  DEFER([
    if (ofd != STDOUT_FILENO) close(ofd);
  ])
  

  if (sodium_hex2bin(pk, 32, pkx, strlen(pkx), ": ", &n, NULL) == -1 || n != 32) {
    ERR(["invalid public key"])
  }
  secsz = readfile(secret, sec, sizeof(sec));
  ERR(["Cannot read secret"])
  DEFER([sodium_memzero(sec, secsz);])
  
  n = crypto_box_PUBLICKEYBYTES + crypto_box_MACBYTES + secsz;
  ct = malloc(n);
  if (!ct) {
    ERR(["Out of memory"]);
  }
  DEFER([free(ct);])

  n64 = sodium_base64_ENCODED_LEN(n, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  ct64 = malloc(n64);
  if (!ct64) {
    ERR(["Out of memory"])
  }
  DEFER([free(ct64);])

  if (sodium_init() == -1) {
    ERR(["Cannot initialize libsodium"])
  }

  if (crypto_box_keypair(ct, esk) == -1) {
    ERR(["Cannot create ephemeral key pair"])
  }

  if (crypto_box_easy(ct + crypto_box_PUBLICKEYBYTES, sec, secsz, nonce, pk, esk) != 0) {
    ERR(["Cannot encrypt secret"])
  }
  if (!sodium_bin2base64(ct64, n64, ct, n, sodium_base64_VARIANT_ORIGINAL_NO_PADDING)) {
    ERR(["Cannot BASE64 encode ciphertext"]);
  }
  p = ct64;
  while (n64 && (r = write(ofd, p, n64)) > 0) {
    n64 -= r;
    p += r;
  }
  if (r == -1) {
    ERR(["Cannot write to output file"])
  }
  END
}
