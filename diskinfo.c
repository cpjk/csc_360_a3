#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

 /* diskinfo */

unsigned long file_size(FILE *fp);

int main(int argc, char **argv) {
  if(!argv[1]) {
    printf("No file argument provided. Exiting.\n");
    exit(-1);
  }

  FILE *fp;
  int fd;
  void *disk;
  struct stat sb;

  fp = fopen(argv[1], "r"); // open disk file

 /* get disk size */
  unsigned long disk_size = file_size(fp);
  fclose(fp); // close the disk file

  fd = open(argv[1], O_RDONLY);

  if(fd == -1) {
    perror("open");
    return 1;
  }

  if(fstat(fd, &sb) == -1) {
    perror("fstat");
    return 1;
  }

  disk = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);


  printf("Total size of the disk: %lu bytes.\n", disk_size);
  printf("Total size of the disk: %lu bytes.\n", sb.st_size);

}

unsigned long file_size(FILE *fp) {
  fseek(fp, 0L, SEEK_END); // set position indicator to end of file
  long size = ftell(fp);
  fseek(fp, 0L, SEEK_SET); // reset position indicator

  return size;
}


 /* char *mmap = mmap(disk file, ... disk size); */

 /* Free size of disk: check FAT table */
 /* number of files in root dir: check root directory */
 /* other variables: check boot sector */

 /* munmap(disk file) */
 /* close disk file */


 // disklist

 /* open disk file */
 /* get disk size */
 /* char *mmap = mmap(disk file, ... disk size); */

 /* while(mmap[root dir] != 0) { */
 /*         // check attributes for 'D' or 'F' */
 /*          //   get time&date(handle little endian), and print accordingly */

 /*   root dir += offset(32 bytes); //next entry */
 /* } */

// munmap(disk file)
// close disk file

// diskget

// open disk file
//  get disk size
// char *src = mmap(disk file, ... disk size)

//  check for file to be copied in disk root dir, grab its file size & related info
//  open a file in current directory with same size
// char *dest = mmap(new file, ... file size)
/* copy file from src->dest, reading sector by sector */

/* munmap(disk file) */
/* munmap(file) */

/* close disk file */
/* close file */

/* // diskput */

/* open(check) file to be copied in current dir */
/* grab its file size & related info */
/* char *src = mmap(file, ... file size) */
/* open disk file */
/* get disk size */
/* char *dest = mmap(disk file, ... disk size) */
/* check for free space in disk */
/* Add file entry in disk root dir */ 
/* copy file from src->dest, reading sector by sector, update FAT table in the meantime */

/* munmap(disk file) */
/* munmap(file) */

/* close disk file */
/* close file */
