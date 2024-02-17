#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include "util.h"

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
