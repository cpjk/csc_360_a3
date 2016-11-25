#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

unsigned long file_size(FILE *fp);
unsigned int sec_per_fat(char *disk);
unsigned int fat1_start_byte(char *disk);
unsigned int num_fats(char *disk);

/* const unsigned int ROOT_DIR_START_BYTE = 9728; */
/* const unsigned int FAT1_START_BYTE = 512; */
const unsigned int BYTES_PER_SEC = 512;

unsigned int reserved_sec_cnt(char *disk) {
  return (unsigned int) disk[0x0e] | (disk[0x0f] << 8);
}

unsigned int root_dir_start_byte(char *disk) {
  // Address of first FAT + Number of FATs * Sectors per FAT
  return (unsigned int) fat1_start_byte(disk) + num_fats(disk) * sec_per_fat(disk) * BYTES_PER_SEC;
}

unsigned int sec_per_fat(char *disk) {
  return (unsigned int) ((disk[0x17] << 8) | disk[0x16]);
}

unsigned int fat1_start_byte(char *disk) {
  //Start sector for partition 1 + Reserved sector count) * Bytes per sector
  return (unsigned int) reserved_sec_cnt(disk) * BYTES_PER_SEC;
}

unsigned int num_fats(char *disk) {
  return (unsigned int) disk[0x10];
}

unsigned int num_files_root_dir(char *disk) {
  // count files
}

int main(int argc, char **argv) {
  if(!argv[1]) {
    printf("No file argument provided. Exiting.\n");
    exit(-1);
  }

  FILE *fp;
  int fd;
  char *disk;
  struct stat sb;

  fp = fopen(argv[1], "r"); // open disk file
  unsigned long disk_size = file_size(fp); /* get disk size */
  fclose(fp); // close the disk file

  fd = open(argv[1], O_RDONLY); // get a file descriptor for the disk
  if(fd == -1) { perror("open"); return 1; }
  if(fstat(fd, &sb) == -1) { perror("fstat"); return 1; }

  disk = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); // mmap the disk to memory
  if (disk == MAP_FAILED) { perror ("mmap"); return 1; }
  if (close (fd) == -1) { perror ("close"); return 1; }

  char *sysname = malloc(9*sizeof(char));  // get OS name
  sysname[8] = '\0';

  strncpy(sysname, &disk[0x03], 8);

  unsigned int res_sec_cnt = reserved_sec_cnt(disk);
  printf("reserved sector count: %d\n", res_sec_cnt);
  printf("fat1 start byte: %d\n", fat1_start_byte(disk));
  printf("rootdir start byte: %d\n", root_dir_start_byte(disk));

  printf("================\n");
  printf("OS Name: %s\n", sysname);
  printf("Label of disk: TODO\n");
  printf("Total size of the disk: %lu bytes.\n", disk_size);
  printf("Free size of disk: TODO\n");
  printf("================\n");
  printf("Number of files in the root directory (not including subdirectories): TODO\n");
  printf("================\n");
  printf("Number of FAT copies: %d\n", num_fats(disk));
  printf("Sectors per FAT: %d\n", sec_per_fat(disk));
 /* Free size of disk: check FAT table */

 /* number of files in root dir: check root directory */

}

unsigned long file_size(FILE *fp) {
  fseek(fp, 0L, SEEK_END); // set position indicator to end of file
  long size = ftell(fp);
  fseek(fp, 0L, SEEK_SET); // reset position indicator

  return size;
}

unsigned long free_disk_size(void *disk) {

}


 /* char *mmap = mmap(disk file, ... disk size); */

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
