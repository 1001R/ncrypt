#include <stdlib.h>
#include <err.h>
#include <limits.h>

enum error {
    ERR_FOO = INT_MIN,
    ERR_BAR,
    ERR_XYZ,
};

extern const char *errmsg[];

/*
noreturn void
errexit(int err, const char *fmt, ...) {
    // errmsg[err - INT_MIN];
    if (err == -1) {
        err(EXIT_FAILURE, fmt);
    }
    errx(EXIT_FAILURE, fmt)
}
*/

#define errexit(e, fmt, ...) \
    if ((e) == -1) err(EXIT_FAILURE, fmt __VA_OPT__(,) __VA_ARGS__); \
    else errx(EXIT_FAILURE, fmt ": %s", __VA_ARGS__ __VA_OPT__(,) errmsg[e - INT_MIN])
