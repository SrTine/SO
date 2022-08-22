#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes) {
  char c = 0, nCopy = 0, copy;
  while(nCopy < nBytes && (c = getc(origin))!= EOF) {
    copy = putc(c, destination);
    // Si se ha acabado el fichero antes de tiempo, devolvemos error
    if(copy == EOF) {
      return -1;
    }
    nCopy++;
  }
  return nCopy;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char* loadstr(FILE * file) {
  char* string;
  char c;
  int length = 0;
  while((c = getc(file) != '\0') && c != EOF) {
    length++;
  }
  // Si se acaba el archivo y no ha terminado de leer un string
  // devuelve NULL, error
  if(c == EOF) {
    return NULL;
  }

  // Reservamos memoria para el string
  string = malloc(length * sizeof(char));

  // Volvemos al principio del string
  fseek(file, -((length+1) * sizeof(char)), SEEK_CUR);

  // Bucle para leerlo hasta \0
  int j = 0;
  while((c = getc(file)) != '\0') {
    string[j] = c;
    j++;
  }
  return string;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry* readHeader(FILE * tarFile, int *nFiles) {
  stHeaderEntry* array = NULL;

  // Leemos el primer int, numero de archivos
  fread(nFiles, 1, sizeof(int), tarFile);

  /*Allocate memory for the array*/
  array = malloc(sizeof(stHeaderEntry)*(*nFiles));

  /*Read the (pathname, size) pairs from tarFile and store them in the  array*/
  for(int i = 0; i <  *nFiles; i++) {
    //Si falla la lectura del string devolvemos NULL, error
    if ((array[i].name = loadstr(tarFile)) == NULL) {
      return NULL;
    }
    fread(&array[i].size, 1,  sizeof(unsigned int), tarFile);
  }

  return array;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[]) {
 int header_size;
  FILE *tarFile, *inputFile;
  unsigned int * fileSizes;
  stHeaderEntry* headerArray;
  
  //Get Header size 
  header_size = sizeof(int) + nFiles*sizeof(unsigned int);
  for(int i = 0; i < nFiles; i++) {
    header_size += strlen(fileNames[i]) + 1;
  }

  //Create the file
  tarFile = fopen(tarName, "w");
  if(tarFile == NULL) 
    return EXIT_FAILURE;

  //Skip the header and copy the files
  fileSizes = malloc(nFiles*sizeof(unsigned int));
  fseek(tarFile, header_size, SEEK_SET);
  for(int  i = 0; i < nFiles;i++) {
    inputFile = fopen(fileNames[i], "r");
    if(inputFile == NULL)
      return EXIT_FAILURE;
    int tempint = copynFile(inputFile, tarFile, INT_MAX);
    if(tempint == -1) 
      return EXIT_FAILURE;
    fileSizes[i] = tempint;
    fclose(inputFile);
  }

  //Create header in memory
  headerArray = malloc(nFiles * sizeof(stHeaderEntry));
  for(int  j = 0; j < nFiles; j++) {
    headerArray[j].name = fileNames[j];
    headerArray[j].size = fileSizes[j];
  }

  //Nos posicionamos en el principio de nuevo
  fseek(tarFile, 0, SEEK_SET);
  
  fwrite(&nFiles, 1, sizeof(int), tarFile);
  //Escribimos cada entrada de stHeaderEntry en la tar
  for(int h = 0; h < nFiles; h++) {
    fwrite(headerArray[h].name, 1, strlen(fileNames[h]), tarFile);
    putc('\0', tarFile);
    fwrite(&(headerArray[h].size), 1, sizeof(unsigned int), tarFile);
  }

  free(fileSizes);
  free(headerArray);
  fclose(tarFile);

  printf("Mtar file created succesfully.\n");
  return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[]) {
  stHeaderEntry* header;
  int nFiles;
  FILE *tarFile, *outputFile;

  //Creamos el archivo mtar
  tarFile = fopen(&tarName[0], "r");
  if(tarFile == NULL)
    return EXIT_FAILURE;

  //Leemos el header y lo dejamos en memoria
  if((header = readHeader(tarFile, &nFiles)) == NULL) 
    return EXIT_FAILURE;

  //Vamos creando los archivos y copiando sus contenidos
  for(int i = 0; i < nFiles; i++) {
    outputFile = fopen(header[i].name, "w");
    printf("[%d]: Creating file %s, size %d Bytes...", i, header[i].name, header[i].size);
    unsigned int s = copynFile(tarFile, outputFile, header[i].size);
    if(s == header[i].size) {
      printf("Ok\n");
    } else {
      perror("Error: ");
    }
    fclose(outputFile);
  }

  //Liberamos la memoria
  for(int i = 0; i < nFiles; i++) {
    free(header[i].name);
  }
  free(header);
  fclose(tarFile);
  return EXIT_SUCCESS;
}
