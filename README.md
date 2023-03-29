# **Assignment 6**
The goal of this assignment is to look at LWZ compression algorithm

### Build
---
```
$ make
```

### Running Encode
---
```
 $ ./encode -i <input file> -o <output file>
```
+ `-v`: Print compression statistics to stderr.
+ `-i <input>`: Specify input to compress (stdin by default)
+ `-o <output>`: Specify output of compressed input (stdout by default)

### Running Decode
---
```
 $ ./decode -i <input file> -o <output file>
```
+ `-v`: Print decompression statistics to stderr.
+ `-i <input>`: Specify input to decompress (stdin by default)
+ `-o <output>`: Specify output of decompressed input (stdout by default)

### Cleaning
---
```
$ make clean
```

### Include Files:
---
+ `encode.c`: This contains the implementation and main() function for the encode program.
+ `decode.c`: This contains the implementation and main() function for the decrypt program.
+ `trie.c`: This contains the implementation of the Trie library.
+ `trie.h`: This specifies the interface for the Trie library.
+ `word.c`: This contains the implementation of the word library.
+ `word.h`: This specifies the interface for the word library.
+ `io.c`: This contains the implementation of the i/o library.
+ `io.h`: This specifies the interface for the i/o library.
+ `endian.h`: This specifies the interface for the endian library.
+ `code.h`: This specifies the interface for the code library.
+ `Makefile` - has all the command to compile and clean the files
+ `README.md` - Describes how to use the script
+ `DESIGN.pdf` - Describes the design process 
+ `WRITEUP.pdf` - Includes the output of the program 




