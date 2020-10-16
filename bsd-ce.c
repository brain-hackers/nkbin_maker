#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

char *outfile = "nk.bin";
int verbose = 0;
FILE *fout = NULL;
FILE *kfile = NULL;
FILE *lfile = NULL;
char *loader = NULL;
unsigned int loader_size = 0;

#define WRITE(arg, size)                                                                           \
    if (!fwrite(arg, size, 1, fout))                                                               \
        errx(1, "fwrite");

unsigned char magic[7] = "B000FF\n";
unsigned int start = 0xa0200000;
unsigned int len = 0;

void usage() {
    fprintf(stderr, "usage: bsd-ce [-h] [-o outfile] u-boot.bin\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    -h                 this help\n");
    fprintf(stderr, "    -o outfile         write output to [outfile]\n");
    fprintf(stderr, "                       default: %s\n", outfile);
}

int main(int argc, char *argv[]) {
    int n, i;
    unsigned int i32, cksum, ch;
    long len;

    while ((i = getopt(argc, argv, "ho:")) != -1) {
        switch (i) {
        case 'o':
            outfile = optarg;
            break;
        case 'h':
        default:
            usage();
            return 1;
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        fprintf(stderr, "missing kernel name\n");
        usage();
        return 1;
    }

    /* load the loader (err...) */
    if ((lfile = fopen(argv[0], "rb")) == NULL)
        errx(1, "unable to open '%s'", argv[0]);
    fseek(lfile, 0, SEEK_END);
    loader_size = ftell(lfile);
    fseek(lfile, 0, SEEK_SET);
    if ((loader = (char *)malloc(loader_size)) == NULL)
        errx(1, "can't allocate %u bytes for loader", loader_size);
    if (!fread(loader, loader_size, 1, lfile))
        errx(1, "can't read loader file");
    fclose(lfile);

    // next, try to create the output file
    if ((fout = fopen(outfile, "wb")) == NULL)
        errx(1, "unable to create '%s'", outfile);

    /*
     * Write NK.BIN header - this is a magic identifier, the first address
     * data is stored, and the total length
     */
    WRITE(magic, sizeof(magic));
    WRITE(&start, sizeof(start));
    WRITE(&loader_size, sizeof(loader_size));

    /* Calculate the loader checksum */
    cksum = 0;
    for (len = 0; len < loader_size; len++) {
        ch = (unsigned char)loader[len];
        cksum += ch;
    }
    i32 = start;
    WRITE(&i32, sizeof(unsigned int));

    i32 = loader_size;
    WRITE(&i32, sizeof(unsigned int));

    i32 = cksum;
    WRITE(&i32, sizeof(unsigned int));

    /* Put the loader in place */
    for (len = 0; len < loader_size; len++) {
        putc(loader[len], fout);
    }

    /*
     * Write NK.BIN footer - this is just a series of 3x uint32_t indicating
     * a file of zero length, starting at our init code.
     */
    i32 = 0;
    WRITE(&i32, sizeof(i32));
    i32 = start;
    WRITE(&i32, sizeof(i32));
    i32 = 0;
    WRITE(&i32, sizeof(i32));

    free(loader);
    fclose(fout);
    return 0;
}
