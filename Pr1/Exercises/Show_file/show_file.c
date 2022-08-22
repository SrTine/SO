#include <stdio.h>
#include <stdlib.h>
#include <err.h>

int main(int argc, char* argv[]) {
	FILE* file=NULL;
	int ret;

	if (argc!=3) {
		fprintf(stderr,"Usage: %s <file_name> <fread_size>\n",argv[0]);
		exit(1);
	}

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);

	//Convert the second argument to an unsigned long
	unsigned long size = strtoul(argv[2], NULL, 10);
	
	/* Read file byte by byte */
	char* cptr = malloc(size * sizeof(char));
	int elements_copied;
	//while ((c = getc(file)) != EOF ) {
	while ( (elements_copied = fread(cptr, sizeof(char), size, file)) != 0 ) {
	  
	  /* Print byte to stdout */
	  //ret=putc((unsigned char) c, stdout);
	  ret=fwrite(cptr, sizeof(char), elements_copied, stdout);
		if (ret<1){
			fclose(file);
			err(3,"fwrite() failed!!");
		}
	}

	fclose(file);
	return 0;
}
