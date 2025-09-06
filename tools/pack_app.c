#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint32_t ram_size;
    uint32_t entry_pc;
    uint32_t initial_sp;
    uint32_t flags;
} proc_img_hdr_t;

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s input_ram.bin output.proc\n", argv[0]);
        return 1;
    }
    const char *in = argv[1], *out = argv[2];

    FILE *fi = fopen(in, "rb");
    if (!fi) { perror("open input"); return 1; }

    fseek(fi, 0, SEEK_END);
    long sz = ftell(fi);
    fseek(fi, 0, SEEK_SET);

    if (sz <= 0) {
        fprintf(stderr, "bad input size\n");
        fclose(fi);
        return 1;
    }

    /* Read first 8 bytes (vector table: SP, PC) */
    uint32_t vt[2];
    if (fread(vt, sizeof(uint32_t), 2, fi) != 2) {
        fprintf(stderr, "failed to read vector table\n");
        fclose(fi);
        return 1;
    }
    rewind(fi);

    proc_img_hdr_t hdr = {0};
    hdr.ram_size   = (uint32_t)sz;
    hdr.initial_sp = vt[0];   /* first word = stack pointer */
    hdr.entry_pc   = vt[1];   /* second word = reset handler (with Thumb bit) */
    hdr.flags      = 0;

    FILE *fo = fopen(out, "wb");
    if (!fo) { perror("open output"); fclose(fi); return 1; }

    if (fwrite(&hdr, sizeof hdr, 1, fo) != 1) {
        perror("write hdr");
        fclose(fi); fclose(fo);
        return 1;
    }

    /* Copy app binary */
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, fi)) > 0) {
        if (fwrite(buf, 1, n, fo) != n) {
            perror("write body");
            fclose(fi); fclose(fo);
            return 1;
        }
    }

    fclose(fi);
    fclose(fo);

    printf("Packed %s -> %s\n", in, out);
    printf("  ram_size=%lu entry=0x%08x sp=0x%08x\n",
           sz, hdr.entry_pc, hdr.initial_sp);

    return 0;
}
