#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

// header files
#include "word.h"
#include "code.h"
#include "endian.h"
#include "trie.h"
#include "io.h"

#define OPTIONS "hvi:o:"

static uint64_t bits_to_bytes(uint64_t bits) {
    // if(bits % 8){ return (bits / 8) + 1; } else { return (bits / 8 ); }
    return (bits % 8) ? (bits / 8) + 1 : (bits / 8);
}

int bit_len(uint16_t code) {
    int bitlen = (int) log2(code) + 1;
    return bitlen;
}
// print the help message
void print_help_message(void) {
    fprintf(stderr,

        "SYNOPSIS\n"
        "   Decompresses files with the LZ78 decompression algorithm.\n"
        "   Used with files compressed with the corresponding encoder.\n"
        "\n"
        "USAGE\n"
        "   ./decode [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v          Display decompression statistics\n"
        "   -i input    Specify input to decompress (stdin by default)\n"
        "   -o output   Specify output of decompressed input (stdout by default)\n"
        "   -h          Display program usage\n");
    return;
}

uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

// main function to encode the string/input  for the input file
int main(int argc, char **argv) {
    int opt;

    // default i/o and verbose options
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    bool stat_flag = false;
    int fail = -1;

    while ((opt = getopt(argc, argv, "vi:o:")) != -1) {
        switch (opt) {
        case 'v':
            stat_flag = true; // will print stats
            break;
        case 'i':
            if ((infile = open(optarg, O_RDONLY)) == fail) { // O_RDONLY is read only
                fprintf(stderr, "Failed to open %s.\n", optarg);
                close(infile);
                exit(1);
            }
            break;
        case 'o':
            if ((outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == fail) {
                fprintf(stderr, "Failed to open %s.\n", optarg);
                close(outfile);
                exit(1);
            }
            break;
        default: print_help_message(); return -1;
        }
    }

    FileHeader header;
    read_header(infile, &header);

    // check if the program is decodable and if the magic number matcehs
    if (header.magic != MAGIC) {
        fprintf(stderr, "Bad magic number!\n");
        close(infile);
        close(outfile);
        exit(1);
    }

    fchmod(outfile, header.protection);

    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    //printf("before while\n");
    while ((read_pair(infile, &curr_code, &curr_sym, bit_len(next_code)))) {
        //printf("in while\n");
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        //printf("after while\n");
        write_word(outfile, table[next_code]);
        next_code += 1;

        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }

    // flush any remianing words out to output file
    flush_words(outfile);

    // take out the trash
    wt_delete(table);
    //close(infile);
    //close(outfile);

    //print stats if verbose enabled
    if (stat_flag) {
        double savings = 100.0 * (1.0 - ((float) bits_to_bytes(total_bits) / (float) total_syms));
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes \n", bits_to_bytes(total_bits));
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes \n", total_syms);
        //fprintf(stderr, "Space saving: %.2f%% \n", savings);
        fprintf(stderr, "Compression ratio: %.2f%%\n", savings);
    }

    // close files and return
    close(infile);
    close(outfile);
    return 0;
}

