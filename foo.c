#include <stdio.h>
// enum keysource_type { FD, PATH };

struct keysource {
    // enum keysource_type type;
    enum { FD, PATH } type;
    union {
        int fd;
        const char *path;
    };
};

void func(const struct keysource *ks) {
    if (ks->type == FD) {
        printf("FD: %d\n", ks->fd);
    } else {
        printf("PATH: %s\n", ks->path);
    }
}

int main() {
    struct keysource ks = {FD, 10};
    func(&ks);
    ks.type = PATH;
    ks.path = "/dev/null";
    func(&ks);
    return 0;
}