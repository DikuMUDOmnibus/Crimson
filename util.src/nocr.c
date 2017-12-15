#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


/* Global constants */
#define TRUE 1
#define FALSE 0
 

void main( int argc, char *argv[] ) {
	FILE *theInFile, *theOutFile;
	char *buf;
	size_t i, size;
	struct stat theStat;
	
	   
	if (argc < 3) {
		printf("Usage: nocr <infile> <outfile>\n");
		printf("\nthe file will be read in, then written out less <CR> characters\n");
		exit(0);
	}
	printf("Reading in %s", argv[1]);
	/* get the size of the file */	
	stat(argv[1],&theStat);
	size = (size_t) theStat.st_size;
	buf = (char *) malloc(size);
	if (!buf) {
		printf("\nInsufficient memory to scan in file\n");
		exit(0);
	}
	/* read in the file */
	if (!(theInFile = fopen(argv[1], "rb"))) {
	  printf("Error: couldnt open inFile\n");
	  exit(1);
	}
	fread(buf, size, 1, theInFile);
	fclose(theInFile);
	   
	/* write out the file */
	if (!(theOutFile = fopen(argv[2], "wb"))) {
	  printf("Error: couldnt open outFile\n");
	  exit(1);
	}
	printf(", Writing it out less <CR>'s to %s", argv[2]);
	for (i=0; i<size; i++)
		if (buf[i] != 13) fputc(buf[i], theOutFile);
	fclose(theOutFile);
}
