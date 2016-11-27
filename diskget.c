#include "utils.h"

// copies the file with filename file_name from the disk's root directory
// to the file pointed to by fp
void copy_from_disk(char *disk, char *out_file, char *target_filename) {
  // diskget -> look for file in root dir
  // if there, get first cluster. get FAT entry for the first cluster.
  // copy cluster. if current fat entry says this is the last cluster, we're done.
  // else, get the next cluster from the current entry and get the corresponding next entry from that cluster.
  //
  unsigned int start_byte = root_dir_start_byte(disk);

  int ent = 0;
  for(ent; ent < ROOT_DIR_MAX_ENT; ent++) {
    unsigned int entry_start_byte = start_byte + (32 * ent); // 32 bytes per entry
    unsigned int attr_byte = entry_start_byte + 0x0b;
    if(disk[attr_byte] == 0x0f) { continue; } // long file name entry (fake entry)
    if((disk[attr_byte] & 0x02) != 0) { continue; } // hidden file
    if(disk[entry_start_byte] == 0x00) { // free entry and rest are free
      printf("file %s not found in root directory.\n", target_filename);
      return;
    }
    if(disk[entry_start_byte] == 0xe5) { continue; } // free entry
    if(disk[entry_start_byte] == 0x2e) { continue; } // dot directory
    if((disk[attr_byte] & 0x08) != 0) { continue; } // volume label

    if((disk[attr_byte] & 0x10) == 0) { // not subdirectory (actual file)
      int i;
      char filename[21] = "";
      for(i = 0; i < 21; i++) { filename[i] = '\0'; }
      for(i = 0; i < 8; i++) {
        if(disk[entry_start_byte + i] == 0x20) break;
        filename[i] = disk[entry_start_byte + i];
      }

      char fextension[4] = "";
      for(i = 0; i < 4; i++) { fextension[i] = '\0'; }
      for(i = 0; i < 3; i++) { fextension[i] = disk[entry_start_byte + 0x08 + i]; }

      strncat(filename, ".", 1);
      strncat(filename, fextension, strlen(fextension));

      if(strcmp(filename, target_filename) != 0) { continue; }
      // actually copy shit
      int first_clust = disk[entry_start_byte + 0x1a] | (disk[entry_start_byte + 0x1a + 1] << 8);
      printf("first cluster: %X\n", first_clust);
      return;


    }
  }

  printf("file %s not found in root directory.\n", target_filename);
}

unsigned int root_dir_f_size(char *disk, char *target_filename) {
  unsigned int start_byte = root_dir_start_byte(disk);

  int ent = 0;
  for(ent; ent < ROOT_DIR_MAX_ENT; ent++) {
    unsigned int entry_start_byte = start_byte + (32 * ent); // 32 bytes per entry
    unsigned int attr_byte = entry_start_byte + 0x0b;
    if(disk[attr_byte] == 0x0f) { continue; } // long file name entry (fake entry)
    if((disk[attr_byte] & 0x02) != 0) { continue; } // hidden file
    if(disk[entry_start_byte] == 0x00) { // free entry and rest are free
      printf("file %s not found in root directory.\n", target_filename);
      return 0;
    }
    if(disk[entry_start_byte] == 0xe5) { continue; } // free entry
    if(disk[entry_start_byte] == 0x2e) { continue; } // dot directory
    if((disk[attr_byte] & 0x08) != 0) { continue; } // volume label

    if((disk[attr_byte] & 0x10) == 0) { // not subdirectory (actual file)
      int i;
      char filename[21] = "";
      for(i = 0; i < 21; i++) { filename[i] = '\0'; }

      for(i = 0; i < 8; i++) {
        if(disk[entry_start_byte + i] == 0x20) break;
        filename[i] = disk[entry_start_byte + i];
      }

      char fextension[4] = "";
      for(i = 0; i < 4; i++) { fextension[i] = '\0'; }
      for(i = 0; i < 3; i++) { fextension[i] = disk[entry_start_byte + 0x08 + i]; }
      strncat(filename, ".", 1);
      strncat(filename, fextension, strlen(fextension));

      if(strcmp(filename, target_filename) == 0) {
        printf("file %s found!", target_filename);
        unsigned int byte1 = disk[entry_start_byte + 0x1c] & 0x00FF;
        unsigned int byte2 = disk[entry_start_byte + 0x1c + 1] & 0x00FF;
        unsigned int byte3 = disk[entry_start_byte + 0x1c + 2] & 0x00FF;
        unsigned int byte4 = disk[entry_start_byte + 0x1c + 3] & 0x00FF;
        unsigned long filesize = (byte4 << 24) | (byte3 << 16) | (byte2 << 8 ) | (byte1);
        printf(" size: %d\n", filesize);
        return filesize;
      }
    }
  }

  printf("file not found");
  return 0;
}

void list_root_dir(char *disk) {
  unsigned int start_byte = root_dir_start_byte(disk);

  int ent = 0;
  for(ent; ent < ROOT_DIR_MAX_ENT; ent++) {
    unsigned int entry_start_byte = start_byte + (32 * ent); // 32 bytes per entry
    unsigned int attr_byte = entry_start_byte + 0x0b;
    if(disk[attr_byte] == 0x0f) { continue; } // long file name entry (fake entry)
    if((disk[attr_byte] & 0x02) != 0) { continue; } // hidden file
    if(disk[entry_start_byte] == 0x00) { return; } // free entry and rest are free
    if(disk[entry_start_byte] == 0xe5) { continue; } // free entry
    if(disk[entry_start_byte] == 0x2e) { continue; } // dot directory
    if((disk[attr_byte] & 0x08) != 0) { continue; } // volume label

    if((disk[attr_byte] & 0x10) == 0) { // not subdirectory (actual file)
      unsigned int byte1 = disk[entry_start_byte + 0x1c] & 0x00FF;
      unsigned int byte2 = disk[entry_start_byte + 0x1c + 1] & 0x00FF;
      unsigned int byte3 = disk[entry_start_byte + 0x1c + 2] & 0x00FF;
      unsigned int byte4 = disk[entry_start_byte + 0x1c + 3] & 0x00FF;
      unsigned long filesize = (byte4 << 24) | (byte3 << 16) | (byte2 << 8 ) | (byte1);

      int i;
      char filename[21] = "";
      for(i = 0; i < 21; i++) { filename[i] = '\0'; }

      for(i = 0; i < 8; i++) {
        if(disk[entry_start_byte + i] == 0x20) break;
        filename[i] = disk[entry_start_byte + i];
      }

      char fextension[4] = "";
      for(i = 0; i < 4; i++) { fextension[i] = '\0'; }

      for(i = 0; i < 3; i++) {
        char c = disk[entry_start_byte + 0x08 + i];
        fextension[i] = c;
      }

      unsigned int time_byte1 = disk[entry_start_byte + 0x0e] & 0x0FF;
      unsigned int time_byte2 = disk[entry_start_byte + 0x0e + 1] & 0x0FF;
      unsigned int time = (time_byte1 | time_byte2 << 8) & 0xFFFF;
      int hours = (time >> 11) & 0x1F; // 5 bits (15-11)
      int min = (time >> 5) & 0x3F; // 6 bits (10-5)

      unsigned int date_byte1 = disk[entry_start_byte + 0x10] & 0x0FF;
      unsigned int date_byte2 = disk[entry_start_byte + 0x10 + 1] & 0x0FF;
      unsigned int date = (date_byte1 | date_byte2 << 8) & 0xFFFF;
      unsigned int year = ((date >> 9) & 0x7F) + 1980;  // 7 bits (15-9)
      unsigned int month = (date >> 5) & 0x0F;   // 4 bits (8-5)
      unsigned int day =  date & 0x1F;    // 5 bits (4-0)

      // add a zero for hours or mins less than 10
      char hour_pad = hours < 10 ? '0' : '\0';
      char min_pad = min < 10 ? '0' : '\0';

      printf("F %ld %20s.%s ", filesize, filename,  fextension);
      printf("%d-%d-%d ", year, month, day);
      printf("%c%d:%c%d\n", hour_pad, hours, min_pad, min);
    }
  }

}

int main(int argc, char **argv) {
  if(!argv[1]) {
    printf("No image argument provided. Exiting.\n");
    exit(-1);
  }

  if(!argv[2]) {
    printf("No file argument provided. Exiting.\n");
    exit(-1);
  }

  int fd;
  char *disk;
  struct stat sb;

  fd = open(argv[1], O_RDONLY); // get a file descriptor for the disk
  if(fd == -1) { perror("open"); return 1; }
  if(fstat(fd, &sb) == -1) { perror("fstat"); return 1; }

  disk = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); // mmap the disk to memory
  if (disk == MAP_FAILED) { perror ("mmap"); return 1; }
  if (close (fd) == -1) { perror ("close"); return 1; }

  unsigned long disk_size_bytes = disk_size(argv[1]);

  printf("Files on disk: \n");
  list_root_dir(disk);

  fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  if (fd == -1) { perror("Error opening file for writing"); exit(EXIT_FAILURE); }

  unsigned int file_size = root_dir_f_size(disk, argv[2]);

  size_t outputfsize = (size_t) (file_size + 1);

  // stretch output file size to size we will mmap
  if (lseek(fd, outputfsize - 1, SEEK_SET) == -1) {
    close(fd);
    perror("Error calling lseek() to 'stretch' the file");
    exit(EXIT_FAILURE);
  }

  // write an empty char at the end of the file to actually stretch it
  if (write(fd, "", 1) == -1) {
    close(fd);
    perror("Error calling lseek() to 'stretch' the file");
    exit(EXIT_FAILURE);
  }

  // map output file to memory
  char *out_map = (char *) mmap(0, outputfsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  copy_from_disk(disk, out_map, argv[2]);
}
