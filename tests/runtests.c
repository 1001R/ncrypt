#include <stdio.h>
extern int test_b58_encode();
int main() {
printf("Test\n");
printf("===================================================\n");
int r, nerr = 0, nok = 0;
r = test_b58_encode();
printf("%-50s%c\n", "b58_encode", r == 0 ? '+' : '-');
if (r == 0) nok++; else nerr++;
printf("===================================================\n");
printf("Success:                                        %3d\n", nok);
printf("Errors:                                         %3d\n", nerr);
return nerr ? 1 : 0;
}
