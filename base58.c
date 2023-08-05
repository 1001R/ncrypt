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
b58_encode(unsigned char key[32], char out[44]) {
    uint64_t words[8] = {0}, *w = NULL, r = 0;
    size_t i, wlen = 0;
    unsigned char *p = key;
    char *o = out + 43;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            words[i] = (words[i] << 8) + *p++;
        }
        if (!w && words[i]) {
            w = words + i;
            wlen = 8 - i;
        }
    }
    for (i = 0; i < 44 && wlen > 0; i++) {
        r = div58(w, wlen);
        *o-- = ALPHABET[r];
        if (*w == 0) {
            wlen--;
            w++;
        }
    }
    for (; i < 44; i++) {
        *o-- = ALPHABET[0];
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
}


int main() {
    unsigned char key[32] = {
        // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x31, 0xdb, 0x7f, 0x4e, 0xb4, 0x98, 0xb9, 0xc5,
        0x04, 0xa0, 0x51, 0xf5, 0xbe, 0xf3, 0x26, 0xc9, 
        0x3d, 0x33, 0xcb, 0x03, 0x7d, 0xc5, 0xda, 0x62,
        0xfc, 0x44, 0x06, 0x9f, 0x27, 0x35, 0x95, 0x7e, 
    };
    char buf[45];
    buf[44] = 0;

    for (int i = 0; i < 32; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");
    b58_encode(key, buf);
    printf("%.44s\n", buf);
    b58_decode(buf, key);
    for (int i = 0; i < 32; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");
    return 0;
}