#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

// headerfiles
#include "endian.h"
#include "code.h"
#include "io.h"

#define BITS     8
#define BIT_CALC (bit_index + 1) % (BITS * BLOCK)

uint64_t total_syms = 0;
uint64_t total_bits = 0;
static int end = -1;

static uint8_t write_buf[BLOCK] = { 0 }; // syms
static int sym_index = 0;

static uint8_t read_buf[BLOCK] = { 0 }; // bits
static int bit_index = 0;
// returns the value of the bit at the given index
// byte at the index are shifted right by the bit position, which places the target bit LSP
// div by 8 gives us a byte -- use % 8 to determine the bit position
// then mask the bits to give us 0 or 1
uint8_t get_bit(uint8_t buffer[], uint64_t bits) {
    return (buffer[bits / 8] >> (bits % 8)) & 0x01;
}

void set_bit(uint8_t buffer[], uint64_t bits) {
    buffer[bits / 8] |= (0x01 << bits % 8);
}

void clear_bit(uint8_t buffer[], uint32_t bits) {
    buffer[bits / 8] &= ~(0x01 << bits % 8);
}

// converts the number of bits to bytes
uint64_t bits_to_bytes(uint64_t bits) {
    // if(bits % 8){ return (bits / 8) + 1; } else { return (bits / 8 ); }
    return (bits % 8) ? (bits / 8) + 0x01 : (bits / 8);
}

// read is a syscall() and doesn't always read the right amount
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total = 0;

    while (true) {
        int writes = read(infile, buf, to_read);

        if (writes == end) {
            fprintf(stderr, "failed to write bytes to file.\n");
            exit(1);
        }

        total += writes;
        buf += writes;

        if (total == to_read || writes == 0) {
            break;
        }
    }

    return total;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total = 0;

    while (true) {
        int writes = write(outfile, buf, to_write);

        if (writes == end) {
            fprintf(stderr, "failed to write bytes to file.\n");
            exit(1);
        }

        total += writes;
        buf += writes;

        if (total == to_write || writes == 0) {
            break;
        }
    }

    return total;
}
// reads header to the infile
void read_header(int infile, FileHeader *header) {

    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    // assert(magic == MAGIC){ printf("Failed to match magic number.\n"); }
    if (big_endian()) {
        header->magic = swap32(header->magic); //magic is uint32_t
        header->protection = swap16(header->protection); //protection is uint16_t
    }
    // assert(header->magic == MAGIC);
    if (header->magic != MAGIC) {
        fprintf(stderr, "failed to read header to file.\n");
        exit(1);
    }

    total_bits += sizeof(FileHeader) * BITS;
}
// writes the header to the outfile
void write_header(int outfile, FileHeader *header) {

    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    // assert(magic == MAGIC){ printf("Failed to match magic number.\n"); }

    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    //assert(header->magic == MAGIC);
    if (header->magic != MAGIC) {
        fprintf(stderr, "failed to read header to file.\n");
        exit(1);
    }

    total_bits += sizeof(FileHeader) * BITS;
}

// read the file char by char
bool read_sym(int infile, uint8_t *sym) {

    uint16_t reads = 0;
    // if it isn't the symbol index
    if (!sym_index) {
        // read in a block symbol
        reads = read_bytes(infile, write_buf, BLOCK);
        if (reads < BLOCK) {
            end = reads + 1;
        }
    }

    // retrieves the next symbol from the buffer (sym) and increments the value of symindex
    *sym = write_buf[sym_index];
    sym_index = (sym_index + 1) % BLOCK;

    if (sym_index != end) { // check to see if the end of the file had been reached
        total_syms += 1;
    }

    // returns true if the symbol is successfully read and stored, and false otherwise.
    if (sym_index == end) {
        return false;
    } else {
        return true;
    }

    return sym_index;
}
// variable size code, fixed side sym
// move x number of time to write the code
// code x number of times to write the pair
// | set a bit
// & is get a bit
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    int i = 0;

    // Loop through each bit of the code and write it to the file.
    // If the bit is set (1), set the corresponding bit in the buffer.
    // The buffer is written to the file when it is full.
    do {
        // Return the i-th bit of the code.
        // If the bit is set, set the corresponding bit in the buffer.

        if ((code >> (i % 16)) & 0x01) {
            set_bit(read_buf, bit_index);
        } else {
            clear_bit(read_buf, bit_index);
        };

        // Increment the bit index and write the buffer to the file if it is full.
        bit_index += 1;
        if (bit_index == BITS * BLOCK) { // all bits are filled so write to buffer
            write_bytes(outfile, read_buf, BLOCK);
            // memset(read_buf, 0, BLOCK);
            bit_index = 0;
        }

        // Increment the bit index and the loop counter.
        i++;
    } while (i < bitlen);

    // Loop through each bit of the symbol and write it to the file.
    // If the bit is set (1), set the corresponding bit in the buffer.
    // The buffer is written to the file when it is full.
    int j = 0;
    do {
        // Return the j-th bit of the symbol.
        // If the bit is set, set the corresponding bit in the buffer.
        if ((sym >> (j % 8)) & 0x01) {
            set_bit(read_buf, bit_index);
        } else {
            clear_bit(read_buf, bit_index);
        };

        // Increment the bit index and write the buffer to the file if it is full.
        bit_index += 1;
        if (bit_index == BITS * BLOCK) {
            write_bytes(outfile, read_buf, BLOCK);
            // (read_buf, 0, BLOCK);
            bit_index = 0;
        }

        // Increment the bit index and the loop counter.
        j++;
    } while (j < BITS);

    // Calculate the total number of bits written to the file and update the total bits counter.
    uint64_t sum = (bitlen + BITS);
    total_bits += sum;
}

void flush_pairs(int outfile) {
    int flush = bits_to_bytes(bit_index);
    write_bytes(outfile, read_buf, flush);
}

// loop through code - to get one bit at a time use | (or)
// loop through sym - to get one bit at a time use | (or)
// used in decode

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    // Check if the machine is big endian and swap the code if necessary.
    if (big_endian()) {
        *code = swap16(*code);
    }

    // Initialize the code and symbol to 0.
    *code = 0, *sym = 0;

    // Loop through each bit of the code and read it from the file.
    // If the buffer is empty, read BLOCK bytes from the file.
    // The bit index is reset to the beginning of the buffer when it reaches the end.
    int i = 0;
    do {
        if (!bit_index) {
            read_bytes(infile, read_buf, BLOCK);
        }

        // Get the next bit from the buffer and set the corresponding bit in the code.
        // set the value of a specific bit "sym" variable to a 1 if the  bit in "read_buf" is also a 1.
        *code |= get_bit(read_buf, bit_index) << i;
        bit_index = BIT_CALC;
        i++;
    } while (i < bitlen);

    // Loop through each bit of the symbol and read it from the file.
    // If the buffer is empty, read BLOCK bytes from the file.
    // The bit index is reset to the beginning of the buffer when it reaches the end.
    int j = 0;
    do {
        if (!bit_index) {
            read_bytes(infile, read_buf, BLOCK);
        }

        // Get the next bit from the buffer and set the corresponding bit in the symbol.
        *sym |= get_bit(read_buf, bit_index) << j;
        bit_index = BIT_CALC;
        j++;
    } while (j < BITS);

    // Calculate the total number of bits read from the file and update the total bits counter.
    uint64_t sum = (bitlen + BITS);
    total_bits += sum;

    // Check if the code is not equal to the STOP_CODE.
    // If so, return true to indicate that a valid pair was read.
    // Otherwise, return false to indicate that the end of the file has been reached.
    if (*code != STOP_CODE) {
        return true;
    } else {
        return false;
    }
}
// each word has the number of syms and len
// write adds the sym + len into the buffer
void write_word(int outfile, Word *w) {

    uint32_t i = 0;
    do {
        // Check if the buffer is full and needs to be flushed to the file
        if (sym_index == BLOCK) {
            write_bytes(outfile, write_buf, BLOCK);
            sym_index = 0;
        }

        // stores the current symbol from the word (w->syms[i]) in the next available position in write
        write_buf[sym_index] = w->syms[i];
        sym_index += 1;
        i++;
    } while (i < w->len);

    // Update the total number of symbols written to the file
    total_syms += w->len;
}

// clear the remaining bit in the buffer
void flush_words(int outfile) {
    write_bytes(outfile, write_buf, sym_index);
}

