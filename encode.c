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

#define OPTIONS "vi:o:h"

int bit_len(uint16_t bits) {
    int bitlen = (int) log2(bits) + 1;
    return bitlen;
}

static uint64_t bits_to_bytes(uint64_t bits) {
    // if(bits % 8){ return (bits / 8) + 1; } else { return (bits / 8 ); }
    return (bits % 8) ? (bits / 8) + 1 : (bits / 8);
}

// print the help message
void print_help_message(void) {
    fprintf(stderr, "SYNOPSIS\n"
                    "   Compresses files using the LZ78 compression algorithm.\n"
                    "   Compressed files are decompressed with the corresponding decoder.\n"
                    "\n"
                    "USAGE\n"
                    "   ./encode [-vh] [-i sinput] [-o output]\n"
                    "\n"
                    "OPTIONS\n"
                    "   -v          Display compression statistics\n"
                    "   -i input    Specify input to compress (stdin by default)\n"
                    "   -o output   Specify output of compressed input (stdout by default)\n"
                    "   -h          Display program help and usage\n");
    return;
}

uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

// main function to encode the string/input  for the input file
int main(int argc, char **argv) {

    // keep track and save the input and outfile files names
    int fail = -1;

    // var for the output and input files, stdin and stdout for the default values
    int infile = STDIN_FILENO; // 0
    int outfile = STDOUT_FILENO; // 1

    bool stat_flag = false;
    int opt = 0;

    //struct stat symbol;

    // parse through the command line
    while ((opt = getopt(argc, argv, "vi:o:h")) != -1) {
        switch (opt) {
        case 'h': // helper message
            print_help_message();
            return 0;
        case 'i':

            if ((infile = open(optarg, O_RDONLY)) == fail) { // O_RDONLY is read only
                fprintf(stderr, "Failed to open %s.\n", optarg);
                close(outfile);
                exit(1);
            }
            break;
        case 'o':
            /*
              * O_WRONLY = write only 
              * O_CREAT = create if it doesn't exist 
              * O_TRUNC = truncated (shorted) if it exists
              */

            if ((outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == fail) {
                fprintf(stderr, "Failed to open %s.\n", optarg);
                close(infile);
                exit(1);
            }

            // grab the stats for the change mod
            //fchmod(outfile, symbol.st_mode);

            break;
        case 'v': stat_flag = true; break;
        default: print_help_message(); return -1;
        }
    }

    struct stat symbol;
    fstat(infile, &symbol);

    FileHeader header = { 0, 0 };
    header.magic = MAGIC;
    header.protection = symbol.st_mode;

    fchmod(outfile, header.protection);
    write_header(outfile, &header);

    //encode the beggining and keep track fo the current and preivous nodes in the trie
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;

    // curr_sym & prev_sym
    uint8_t curr_sym = 0;
    uint16_t prev_sym = 0;
    int next_code = START_CODE;

    while (read_sym(infile, &curr_sym)) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);

        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_len(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }

        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }

        prev_sym = curr_sym;
    }

    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_len(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }

    // write the pair
    write_pair(outfile, STOP_CODE, 0, bit_len(next_code));
    flush_pairs(outfile);

    // stats information
    if (stat_flag) {
        double savings = 100.0 * (1.0 - ((float) bits_to_bytes(total_bits) / (float) total_syms));
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes \n", bits_to_bytes(total_bits));
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes \n", total_syms);
        //fprintf(stderr, "Space saving: %.2f%% \n", savings);
        fprintf(stderr, "Compression ratio: %.2f%%\n", savings);
    }

    // free all the used memory
    trie_delete(root);
    //fclose(in_filename);
    //fclose(out_filename);
    // var for the output and input files, stdin and stdout for the default values
    close(infile);
    close(outfile);

    return 0;
}
