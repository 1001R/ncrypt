#!/bin/sh

scriptdir=$(dirname "$0")

tmpdir=$(mktemp -d)
trap "rm -rf '$tmpfile'" EXIT

echo "#include <stdio.h>" >"$tmpdir/main.c"
echo "int r, nerr = 0, nok = 0;" >"$tmpdir/body.c"

for f in "$scriptdir"/t_*.c; do
	grep -Po '(?<=TEST\().+(?=\))' "$f" |\
	while read t; do
		echo "extern int test_$t();" >>"$tmpdir/main.c"
		cat <<-EOF >>"$tmpdir/body.c"
		r = test_$t();
		printf("%-50s%c\n", "$t", r == 0 ? '+' : '-');
		if (r == 0) nok++; else nerr++;
		EOF
	done
done
cat "$tmpdir/main.c"
cat <<EOF
int main() {
printf("Test\n");
printf("===================================================\n");
EOF
cat "$tmpdir/body.c"
cat <<EOF
printf("===================================================\n");
printf("Success:                                        %3d\n", nok);
printf("Errors:                                         %3d\n", nerr);
return nerr ? 1 : 0;
}
EOF