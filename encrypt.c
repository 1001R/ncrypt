#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include "util.h"

#if 0
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
#endif

void
nc_encrypt(const char *pubkey) {
  unsigned char pk[crypto_box_PUBLICKEYBYTES];
  unsigned char esk[crypto_box_SECRETKEYBYTES];
  unsigned char sbuf[4096], *ct;
  char *obuf, *p;
  unsigned char nonce[crypto_box_NONCEBYTES] = {0};
  int r, ofd = STDOUT_FILENO;
  size_t n, olen, clen, sbufsz = sizeof(sbuf);
  int xc = EXIT_FAILURE;

  if (sodium_hex2bin(pk, 32, pubkey, strlen(pubkey), ": ", &n, NULL) == -1 || n != 32) {
      warnx("Invalid public key");
      goto c0;
  }

  if (readsecret(sbuf, &sbufsz) == -1) {
    warnx("Failed to read secret");
    goto c0;
  }
  
  clen = crypto_box_PUBLICKEYBYTES + crypto_box_MACBYTES + sbufsz;
  ct = malloc(clen);
  if (!ct) {
    warn("Out of memory");
    goto c2;
  }
  olen = sodium_base64_ENCODED_LEN(clen, sodium_base64_VARIANT_ORIGINAL_NO_PADDING) + 6;
  obuf = malloc(olen);
  if (!obuf) {
    warnx("Out of memory");
    goto c3;
  }
  strcpy(obuf, "ENC[");

  if (crypto_box_keypair(ct, esk) == -1) {
    warnx("Cannot create ephemeral key pair");
    goto c4;
  }

  if (crypto_box_easy(ct + crypto_box_PUBLICKEYBYTES, sbuf, sbufsz, nonce, pk, esk) != 0) {
    warnx("Cannot encrypt secret");
    goto c5;
  }
  if (!sodium_bin2base64(obuf + 4, olen - 6, ct, clen, sodium_base64_VARIANT_ORIGINAL_NO_PADDING)) {
    warnx("Cannot BASE64 encode ciphertext");
    goto c5;
  }
  strcat(obuf, "]\n");
  p = obuf;
  while (olen && (r = write(ofd, p, olen)) > 0) {
    olen -= r;
    p += r;
  }
  if (r == -1) {
    warn("Cannot write to output file");
    goto c5;
  }
  xc = 0;
c5:
  sodium_memzero(esk, crypto_box_SECRETKEYBYTES);
c4:
  free(obuf);
c3:
  free(ct);
c2:
  sodium_memzero(sbuf, sbufsz);
c0:
  exit(xc);
}

#if 0
int encrypt_main(int argc, char **argv) {
  const char *infile = NULL, *outfile = NULL, *pubkey = NULL;
  int opt;

  struct option options[] = {
    {
      .name = "key",
      .has_arg = required_argument,
      .flag = NULL,
      .val = 'k'
    },
    {
      .name = "input",
      .has_arg = required_argument,
      .flag = NULL,
      .val = 'i'
    },
    {
      .name = "output",
      .has_arg = required_argument,
      .flag = NULL,
      .val = 'o'
    },
    {NULL}
  };

  while ((opt = getopt_long(argc, argv, "i:k:", options, NULL)) != -1) {
    switch (opt) {
    case 'i':
      infile = optarg;
      break;
    case 'o':
      outfile = optarg;
      break;
    case 'k':
      pubkey = optarg;
      break;
    default:
      return -1;
    }
  }
  if (pubkey == NULL) {
    return -1;
  }
  encrypt(pubkey, infile, outfile);
  return 0;
}
#endif