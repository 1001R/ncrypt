#ifndef _TEST_H_
#define _TEST_H_

#define ASSERT(expr) if (!(expr)) { return -1; }
#define TEST(name) int test_##name()

#endif