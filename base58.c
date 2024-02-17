// 31 db 7f 4e b4 98 b9 c5 04a051f5bef326c93d33cb037dc5da62fc44069f2735957e
// 4Md6u9vtZ1AZdeFN53zEPS2XejftPBzeHsHWQp7ahbG1
// 

#include <stdint.h>
#include <string.h>

#include <stdio.h>


static const char ALPHABET[58] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static uint64_t 
div58(uint64_t *w, size_t n) {
    uint64_t r = 0;
    for (int i = 0; i < n; i++) {
        w[i] |= r << 32;
        r = w[i] % 58;
        w[i] /= 58;
    }
    return r;
}

static void
mult58(uint64_t w[8], int x) {
    for (int i = 0; i < 8; i++) {
        w[i] *= 58;
    }
    w[7] += x;
    for (int i = 7; i > 0; i--) {
        w[i - 1] += (w[i] >> 32);
        w[i] &= 0xffffffff;
    }
}


int
b58_encode(unsigned char key[32], char out[45]) {
    uint64_t words[8] = {0}, *w = NULL, r = 0;
    size_t i, wlen = 0;
    unsigned char *p = key;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            words[i] = (words[i] << 8) + *p++;
        }
        if (!w && words[i]) {
            w = words + i;
            wlen = 8 - i;
        }
    }
    if (!w) {
        strcpy(out, "1");
        return 0;
    }
    char *o = out + 44;
    *o-- = '\0';
    for (i = 0; i < 44 && wlen > 0; i++) {
        r = div58(w, wlen);
        *o-- = ALPHABET[r];
        if (*w == 0) {
            wlen--;
            w++;
        }
    }
    if (wlen) {
        return -1;
    }
    if (i < 44) {
        memmove(out, out + 44 - i, i + 1);
    }
    return 0;
}

int
b58_decode(const char *b58, unsigned char out[32]) {
    size_t n = strlen(b58);
    unsigned char *o = out;
    if (n > 44) {
        return -1;
    }
    uint64_t words[8] = {0};
    for (size_t i = 0; i < n; i++) {
        const char *s = memchr(ALPHABET, b58[i], 58);
        if (!s) {
            return -1;
        }
        mult58(words, s - ALPHABET);
    }
    if (words[0] > 0xffffffff) {
        return -1;
    }
    for (int i = 0; i < 8; i++) {
        uint64_t w = words[i];
        *o++ = (w >> 24) & 0xff;
        *o++ = (w >> 16) & 0xff;
        *o++ = (w >>  8) & 0xff;
        *o++ =  w        & 0xff;
    }
    return 0;
}
