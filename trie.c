#include <stdio.h>
#include <stdlib.h>

// headerfiles
#include "trie.h"
#include "code.h"

// Definition of function to create a new TrieNode
TrieNode *trie_node_create(uint16_t code) {
    // Allocate memory for a new TrieNode
    //TrieNode *t = calloc(1, sizeof(TrieNode));
    TrieNode *t = malloc(sizeof(TrieNode));

    for (int i = 0; i < ALPHABET; i++) {
        t->children[i] = NULL;
    }

    // Check if memory allocation was successful
    // if it wasn't return null
    if (t == NULL) {
        return NULL;
    }

    // Set the code of the TrieNode to the specified index
    t->code = code;

    // Return the new TrieNode
    return t;
}

// Definition of function to delete a TrieNode
void trie_node_delete(TrieNode *n) {
    // Check if the TrieNode is not null
    if (n) {
        // Free memory allocated for the TrieNode
        free(n);
        // Set the pointer to null
        (n) = NULL;
    }
}

// Definition of function to create a new Trie
TrieNode *trie_create(void) {
    // Create a new root node for the Trie
    TrieNode *root = trie_node_create(EMPTY_CODE);

    // Check if memory allocation was successful
    // if it wasn't return NULL
    if (root == NULL) {
        return NULL;
    }

    // Set all children of the root node to null
    /*
    for (int i = 0; i < ALPHABET; i++) {
        root->children[i] = NULL;
    }*/

    // Return the root node
    return root;
}

// Definition of function to reset a Trie
void trie_reset(TrieNode *root) {
    // Loop through all children of the root node
    for (int i = 0; i < ALPHABET; i++) {
        // Delete the child node
        trie_delete(root->children[i]);
        // Set the child node to null
        root->children[i] = NULL;
    }
}

// Definition of function to delete a Trie
void trie_delete(TrieNode *n) {
    // Check if the node is not null
    if (n) {
        // Loop through all children of the node
        for (int i = 0; i < ALPHABET; i++) {
            // If the child node is not null, delete it
            if (n->children[i] != NULL) {
                trie_delete(n->children[i]);
                // Set the child node to null
                n->children[i] = NULL;
            }
        }
        // Delete the node itself
        trie_node_delete(n);
    }
}

// Definition of function to traverse the Trie based on a given symbol
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    // Check if the node is null
    if (n == NULL) {
        return NULL;
    }
    // Check if the child node for the given symbol is null
    if (n->children[sym] == NULL) {
        return NULL;
    }
    // Return the child node for the given symbol
    return n->children[sym];
}
