// #define _GNU_SOURCE
// #define _POSIX_SOURCE

#include "util.h"
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define B64_DECODED_LEN(n) ((n) / 4 * 3 + (((n)&2) >> 1) + ((n)&1))

static int 
decrypt_secret(const char *ciphertext,
               size_t ctlen,
               char *plaintext,
               size_t ptlen,
               unsigned char *sk) {
  if ((ctlen & 3) == 1) {
    // illegal base64 encoding
    return -1;
  }
  size_t blen = B64_DECODED_LEN(ctlen);
  unsigned char ctbin[blen];
  int r;

  r = sodium_base642bin(ctbin, blen, ciphertext, ctlen, NULL, &blen, NULL,
                        sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  if (r == -1) {
    return -1;
  }
  if (blen <= crypto_box_PUBLICKEYBYTES + crypto_box_MACBYTES) {
    return -1;
  }
  if (blen - crypto_box_PUBLICKEYBYTES - crypto_box_MACBYTES > ptlen) {
    return -1;
  }
  unsigned char nonce[crypto_box_NONCEBYTES] = {0};
  r = crypto_box_open_easy(
    (unsigned char *)plaintext,
    ctbin + crypto_box_PUBLICKEYBYTES,
    blen - crypto_box_PUBLICKEYBYTES,
    nonce,
    ctbin,
    sk
  );
  return r == 0 ? blen - crypto_box_PUBLICKEYBYTES - crypto_box_MACBYTES : -1;
}

static int decrypt_stream(FILE *input, FILE *output, unsigned char *sk) {
  ssize_t r;
  char *line = NULL;
  size_t len;
  char sbuf[4096];
  int retval = -1;

  while ((r = getline(&line, &len, input)) > 0) {
    char *s = line, *t, *u;
    while ((t = strstr(s, "ENC["))) {
      if ((u = strstr(t + 4, "]"))) {
        *t = '\0';
        fputs(s, output);
        int slen = decrypt_secret(t + 4, u - t - 4, sbuf, sizeof(sbuf), sk);
        if (slen == -1) {
          goto c0;
        }
        fprintf(output, "%.*s", slen, sbuf);
        s = u + 1;
      }
    }
    if (*s) {
      fputs(s, output);
    }
  }
  retval = feof(input) ? 0 : -1;
c0:
  free(line);
  sodium_memzero(sbuf, sizeof(sbuf));
  return retval;
}

static int readsecretkey(int fd, unsigned char sk[crypto_box_SECRETKEYBYTES]) {
  char buf[128];
  int hexlen, rv = -1;
  size_t klen;

  if ((hexlen = readall(fd, buf, sizeof(buf))) <= 0) {
    goto c0;
  }
  if (sodium_hex2bin(sk, crypto_box_SECRETKEYBYTES, buf, hexlen, ": \t\r\n",
                     &klen, NULL) == -1 ||
      klen != crypto_box_SECRETKEYBYTES) {
    rv = -2;
  }
  rv = 0;
c0:
  sodium_memzero(buf, sizeof(buf));
  return rv;
}

void nc_decrypt(int keyfd) {
  unsigned char sk[crypto_box_SECRETKEYBYTES];
  int r, exitcode = EXIT_FAILURE;

  r = readsecretkey(keyfd, sk);
  if (r == -1) {
    warn("Can't read secret key");
    goto c0;
  }
  if (r == -2) {
    warnx("Invalid secret key");
    goto c0;
  }
  if (decrypt_stream(stdin, stdout, sk) == -1) {
    warnx("Illegal input or wrong key");
    goto c0;
  }
  exitcode = EXIT_SUCCESS;
c0:
  sodium_memzero(sk, sizeof(sk));
  exit(exitcode);
}

#if 0
int decrypt_main(int argc, char **argv) {
  const char *infile = NULL, *outfile = NULL;
  char *s;
  keysource_t keysource;
  int opt;
  uintmax_t fd;

  struct option options[] = {
    {
      .name = "keyfile",
      .has_arg = required_argument,
      .flag = NULL,
      .val = 'k'
    },
    {
      .name = "keyfd",
      .has_arg = required_argument,
      .flag = NULL,
      .val = 'K'
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

  while ((opt = getopt_long(argc, argv, "i:k:K:", options, NULL)) != -1) {
    switch (opt) {
    case 'i':
      infile = optarg;
      break;
    case 'o':
      outfile = optarg;
      break;
    case 'k':
      keysource.type = PATH;
      keysource.path = optarg;
      break;
    case 'K':
      fd = strtoumax(optarg, &s, 10);
      if (s == optarg || *s != '\0') {
        return -1;
      }
      keysource.type = FD;
      keysource.fd = fd;
      break;
    default:
      return -1;
    }
  }
  decrypt(&keysource, infile, outfile);
  return 0;
}
#endif
