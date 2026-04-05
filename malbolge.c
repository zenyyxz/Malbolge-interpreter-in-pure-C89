#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define VM_SIZE 59049
#define HALF    243

/* Registers and Memory */
static unsigned short mem[VM_SIZE];
static unsigned short A = 0, C = 0, D = 0;
static unsigned short lookup[HALF][HALF];
static int verbose = 0;

/* The "Encryption" table - Malbolge's instruction scrambler */
static const char *xl = "5z]&gqtyfr$(we4{WP)H-Zn,[%\\3dL+Q;>U!pJS72FhOA1CB6v^=I_0/8|jsb9m<.TVac`uY*MK'X~xDl}REokN:#?G\"i@";

/* Mnemonic map for -v mode */
static const char *ops[94];

/* Precompute the 5-trit crazy operation for speed.
   Malbolge is slow enough as it is. */
static void build_lookup(void) {
    int i, j, k;
    static const int tbl[3][3] = { {1,0,0}, {1,0,2}, {2,2,1} };

    for (i = 0; i < HALF; i++) {
        for (j = 0; j < HALF; j++) {
            unsigned short res = 0, p3 = 1;
            int ti = i, tj = j;
            for (k = 0; k < 5; k++) {
                res += (unsigned short)(tbl[tj % 3][ti % 3] * p3);
                ti /= 3; tj /= 3; p3 *= 3;
            }
            lookup[i][j] = res;
        }
    }
}

static unsigned short crz(unsigned short a, unsigned short d) {
    return (unsigned short)(lookup[a % HALF][d % HALF] + lookup[a / HALF][d / HALF] * HALF);
}

static unsigned short rotr(unsigned short v) {
    return (unsigned short)((v / 3) + (v % 3) * 19683);
}

static void setup_trace(void) {
    int i;
    for (i = 0; i < 94; i++) ops[i] = "nop";
    ops[4]  = "jmp"; ops[5]  = "out"; ops[23] = "in ";
    ops[39] = "rot"; ops[40] = "mov"; ops[62] = "crz";
    ops[68] = "nop"; ops[81] = "hlt";
}

int main(int argc, char **argv) {
    FILE *fp = stdin;
    int i, ch, op;
    const char *src = NULL;

    build_lookup();
    setup_trace();

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) verbose = 1;
        else if (!src) src = argv[i];
    }

    if (src && !(fp = fopen(src, "r"))) {
        perror(src);
        return 1;
    }

    /* Load and validate. Malbolge ignores whitespace. */
    for (i = 0; (ch = fgetc(fp)) != EOF; ) {
        if (isspace(ch)) continue;
        if (ch < 33 || ch > 126) continue;

        /* Check if the initial op is actually valid */
        op = (ch + i) % 94;
        if (op != 4 && op != 5 && op != 23 && op != 39 && op != 40 && op != 62 && op != 68 && op != 81) {
            fprintf(stderr, "invalid op at %d\n", i);
            if (fp != stdin) fclose(fp);
            return 1;
        }
        if (i >= VM_SIZE) break;
        mem[i++] = (unsigned short)ch;
    }
    if (fp != stdin) fclose(fp);

    /* Initialize the rest of memory using the crazy op */
    for (; i < VM_SIZE; i++) mem[i] = crz(mem[i - 1], mem[i - 2]);

    while (mem[C] >= 33 && mem[C] <= 126) {
        op = (mem[C] + C) % 94;

        if (verbose) {
            fprintf(stderr, "C:%05u D:%05u A:%05u OP:%s\n", C, D, A, ops[op]);
        }

        switch (op) {
            case 4:  C = mem[D]; break;
            case 5:  putchar(A % 256); fflush(stdout); break;
            case 23: ch = getchar(); A = (unsigned short)((ch == EOF) ? 59048 : ch); break;
            case 39: A = mem[D] = rotr(mem[D]); break;
            case 40: D = mem[D]; break;
            case 62: A = mem[D] = crz(A, mem[D]); break;
            case 81: return 0;
            default: break; /* nop */
        }

        /* Encryption & Pointer Step */
        mem[C] = (unsigned short)xl[mem[C] - 33];
        C = (unsigned short)((C + 1) % VM_SIZE);
        D = (unsigned short)((D + 1) % VM_SIZE);
    }

    return 0;
}
