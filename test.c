#include <assert.h>
#include <stdlib.h>

static char _errors[4096];
static char *_err = _errors;
static size_t _errsz = 4096;

#define ASSERT(stmt) \
  if (!(stmt)) { \
    int n = snprintf(_err, _errsz, "  L%d: %s\n", __LINE__, #stmt); \
    assert(n > 0); \
    _errsz -= n; \
    _err += n; \
  }


#define TEST(name) \
  _err = _errors; \
  *_err = '\0'; \
  _errsz = sizeof(_errors); \
  test_##name(); \
  if (!_errors[0]) { \
      printf("%s\tOK\n", #name); \
  } else { \
      printf("%s\tERR\n", #name); \
      puts(_errors); \
  }
