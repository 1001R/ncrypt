#include <stdlib.h>
#include <unistd.h>
#include <stdnoreturn.h>
#include <string.h>
#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <limits.h>
#include <sodium.h>
#include <fcntl.h>

#include "command.h"
#include "decrypt.h"
#include "encrypt.h"
#include "keypair.h"

enum command {
  ENCRYPT = 1,
  DECRYPT,
  KEYPAIR,
  PUBKEY,
};

#define CMD(var, cmd) \
  if (var) errx(EXIT_FAILURE, "Options -e, -d, -g, -p are mutually exclusive."); \
  else var = (cmd)

static int
openkey(int keyfd, const char *keyarg) {
  char *s;
  int fd;
  if (keyfd) {
    uintmax_t u = strtoumax(keyarg, &s, 10);
    if (s == keyarg || *s != '\0' || u > INT_MAX) {
      errx(EXIT_FAILURE, "Invalid file descriptor: %s", keyarg);
    }
    fd = (int) u;
  } else if ((fd = open(keyarg, O_RDONLY)) == -1) {
    err(EXIT_FAILURE, "Can't open file `%s' for reading", keyarg);
  }
  return fd;
}

static void noreturn 
usage(const char *prog) {
  printf(
    "                                                                                \n"
    "%1$s -e PUBKEY                                                                  \n"
    "                                                                                \n"
    "  Read data from stdin, encrypt it using PUBKEY, write result to stdout. If     \n"
    "  stdin is a terminal, you will prompted to enter the secret to be              \n"
    "  encrypted.                                                                    \n"
    "                                                                                \n"
    "                                                                                \n"
    "%1$s -d [-K] KEY                                                                \n"
    "                                                                                \n"
    "  Read encrypted data from stdin, decrypt it, and write to stdout. With         \n"
    "  option -K, KEY is expected to be a decimal integer representing a file        \n"
    "  descriptor opened for reading, otherwise it should be the path to a file to   \n"
    "  read the private key from.                                                    \n"
    "                                                                                \n"
    "                                                                                \n"
    "%1$s -g                                                                         \n"
    "                                                                                \n"
    "  Generate a new keypair.                                                       \n"
    "                                                                                \n"
    "                                                                                \n"
    "%1$s -p [-K] KEY                                                                \n"
    "                                                                                \n"
    "  Print public key for the private key KEY to stdout. See above for details on  \n"
    "  how to pass the key)                                                          \n"
    "                                                                                \n"
    "                                                                                \n"
    "%1$s -h                                                                         \n"
    "                                                                                \n"
    "  Print this help message.                                                      \n"
    "                                                                                \n",
    prog
  );
  exit(0);
}

int main(int argc, char **argv) {
  int opt, keyfd = 0, nargs = 0;
  int command = 0;
  const char *prog;

  prog = strrchr(argv[0], '/');
  if (!prog) {
    prog = argv[0];
  } else {
    prog++;
  }

  while ((opt = getopt(argc, argv, "edgpkh")) != -1) {
    switch (opt) {
    case 'e':
      CMD(command, ENCRYPT);
      nargs = 1;
      break;
    case 'd':
      CMD(command, DECRYPT);
      nargs = 1;
      break;
    case 'g':
      CMD(command, KEYPAIR);
      nargs = 0;
      break;
    case 'p':
      CMD(command, PUBKEY);
      nargs = 1;
      break;
    case 'k':
      keyfd = 1;
      break;
    case 'h':
      usage(prog);
      break;
    default:
      errx(EXIT_FAILURE, "Invalid option: %c", opt);
    }
  }
  if (!command) {
    errx(EXIT_FAILURE, "Mandatory option missing, please provide excactly one of -e, -d, -g, or -p.");
  }
  if (keyfd && command != DECRYPT && command != PUBKEY) {
    errx(EXIT_FAILURE, "Option -k is only valid in combination with -d or -p.");
  }
  if (argc - optind != nargs) {
    errx(EXIT_FAILURE, "Invalid number of arguments.");
  }

  if (sodium_init() == -1) {
    errx(EXIT_FAILURE, "Cannot initialize libsodium");
  }

  switch (command) {
  case ENCRYPT:
    nc_encrypt(argv[optind]);
    break;
  case DECRYPT:
    nc_decrypt(openkey(keyfd, argv[optind]));
    break;
  case KEYPAIR:
    nc_keypair();
    break;
  case PUBKEY:
    errx(EXIT_FAILURE, "Not yet implemented");
    // nc_pubkey(openkey(keyfd, argv[optind]));
    break;
  }
  return EXIT_SUCCESS;
}
