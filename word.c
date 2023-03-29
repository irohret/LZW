#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// headerfiles
#include "word.h"
#include "code.h"
#include "trie.h"

// constructor for a Word ADT
// word_create takes an array of symbols (syms) and its length (len)
// as input arguments and returns a pointer to a newly created Word object
// on successful execution or NULL otherwise.

Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = malloc(sizeof(Word));

    // check memory allocation
    if (w == NULL) {
        return NULL;
    }

    w->len = len;

    // allocate for the array of symbols
    w->syms = malloc(len * sizeof(uint8_t));

    // do we remove symbols or do we make a new array of symbols
    // when you pass in the array for character, do we modify it or do we create a new one

    // copy symbols to the newly allocated memory
    memcpy(w->syms, syms, len * sizeof(uint8_t));

    return w;
}

// word_append_sym takes a Word pointer (w) and a symbol (sym) as input arguments
// and returns a pointer to a newly created Word object that represents the result of
// appending the symbol to the specified Word
/*
Word *word_append_sym(Word *w, uint8_t sym) {
    // calculate the new length of the Word object
    uint32_t new_len = w->len + 1; // add 1 to the len cause were appending to the length
    uint8_t *new_syms
        = malloc(new_len * sizeof(uint8_t)); // allocate memory for the new symbols array

    // memory allocation failed
    if (new_syms == NULL) {
        return NULL;
    }

    memcpy(
        new_syms, w->syms, w->len * sizeof(uint8_t)); // copy symbols from the original Word object

    //for (uint32_t i = 0; i < w->len; i++) {
    //  new_syms[i] = w->syms[i];
    //}

    // append the new symbol to the end of the new symbols array
    new_syms[new_len - 1] = sym; // array starts at 0 to gotta sub 1

    Word *new_word = malloc(sizeof(Word)); // allocate memory for the new Word object

    if (new_word == NULL) {
        free(new_syms); // free memory allocated for the new symbols array
        return NULL; // memory allocation failed
    }

    new_word->syms = new_syms; // set the symbols array of the new Word object
    new_word->len = new_len; // set the length of the new Word object

    return new_word; // return the newly created Word object
}*/

Word *word_append_sym(Word *w, uint8_t sym) {
	if (w == NULL) {
		return NULL;
	}


	uint8_t *new_syms = (uint8_t *) malloc((w->len + 1) * sizeof(uint8_t));

	for (uint8_t loc = 0; loc < w->len; loc += 1) {
		new_syms[loc] = w->syms[loc];
	}

	// CONSTRUCTS a new Word from the specificed Word, w, appended with a symbol, sym
	Word *new_word = word_create(new_syms, w->len + 1);

	if (new_word == NULL) {
		return NULL;
	}

	// APPEND 'sym' to end of array.
	new_word->syms[w->len] = sym;

	return new_word;
}

// a destructor for the Word object pointed to by w. it frees the memory allocated
// for the syms array and the Word object itself
void word_delete(Word *w) {
    // do nothing if w is NULL
    if (w == NULL) {
        return;
    }

    free(w->syms); // free memory for the symbols array
    free(w); // free memory for the Word object
}

// a destructor for the Word object pointed to by w. it frees the memory allocated
// for the syms array and the Word object itself
WordTable *wt_create(void) {
    // the WordTable object
    //WordTable *wt = (WordTable *) malloc(sizeof(WordTable));
    WordTable *wt = calloc(MAX_CODE, sizeof(WordTable));

    // check to see if the wt is NULL iff return NULL
    if (wt == NULL) {
        return NULL;
    }

    // create the code counter and the dictionary to store the values
    wt[EMPTY_CODE] = calloc(1, sizeof(Word));

    // check to see if the dict is NULL iff return NULL
    if (wt[EMPTY_CODE] == NULL) {
        wt_delete(wt);
        return NULL;
    }

    wt[EMPTY_CODE]->syms = NULL;
    wt[EMPTY_CODE]->len = 0;

    return wt;
}

void wt_reset(WordTable *wt) {
    for (int i = START_CODE; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]); // free memory allocated for the Word
            wt[i] = NULL; // set the pointer to NULL
        }
    }
}

// free the memory used by a WordTable object
void wt_delete(WordTable *wt) {
    if (wt) {
        for (int i = STOP_CODE; i < MAX_CODE; i++) {
            word_delete(wt[i]); // delete memory for the symbols table
            wt[i] = NULL;
        }

        free(wt); // free memory for the Word object
    }
}
