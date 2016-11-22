 /* diskinfo */

 /* open disk file */
 /* get disk size */
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

 while(mmap[root dir] != 0) {
         // check attributes for 'D' or 'F'
          //   get time&date(handle little endian), and print accordingly

   root dir += offset(32 bytes); //next entry
 }

// munmap(disk file)
// close disk file

// diskget

// open disk file
//  get disk size
// char *src = mmap(disk file, ... disk size)

  check for file to be copied in disk root dir, grab its file size & related info
  open a file in current directory with same size
char *dest = mmap(new file, ... file size)
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
