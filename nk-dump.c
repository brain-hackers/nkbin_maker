#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

unsigned char magic[7] = "B000FF\n";

#define READ(buf, len)                                                                             \
    if (!fread(buf, len, 1, f))                                                                    \
        err(1, "read error");

int main(int argc, char *argv[]) {
    FILE *f;
    unsigned char marker[7];
    unsigned int i32, addr, cksum, len, i;
    int n, ch, c;

    if (argc != 2) {
        fprintf(stderr, "usage: nk-dump filename\n");
        return 1;
    }

    if ((f = fopen(argv[1], "rb")) == NULL)
        err(1, "can't open %s", argv[1]);

    READ(marker, sizeof(marker));
    if (memcmp(magic, marker, sizeof(magic)))
        errx(1, "signature does not match");
    READ(&i32, sizeof(i32));
    printf("addr      %08x\n", i32);
    READ(&i32, sizeof(i32));
    printf("length    %08x\n", i32);
    n = 0;
    while (!feof(f)) {
        READ(&addr, sizeof(addr));
        READ(&len, sizeof(len));
        READ(&cksum, sizeof(cksum));

        printf("image %u\n", n);
        c = 0;
        for (i = 0; i < len; i++) {
            ch = getc(f);
            c += ch;
        }
        printf("  addr      %08x\n", addr);
        printf("  len       %08x\n", len);
        printf("  cksum     %08x", cksum);
        if (cksum == c) {
            printf(" [ok]\n");
        } else {
            printf(" != calculated cksum %08x\n", c);
        }

        n++;
    }
    fclose(f);

    return 0;
}

/* vim:set ts=2 sw=2: */
