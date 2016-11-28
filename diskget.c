#include "utils.h"

// copies the file with filename file_name from the disk's root directory
// to the file pointed to by fp
void copy_from_disk(char *disk, char *out_file, char *target_filename, unsigned long disk_size_bytes, unsigned int target_fsize) {
  // diskget -> look for file in root dir
  // if there, get first cluster. get FAT entry for the first cluster.
  // copy cluster. if current fat entry says this is the last cluster, we're done.
  // else, get the next cluster from the current entry and get the corresponding next entry from that cluster.
  //
  unsigned int start_byte = root_dir_start_byte(disk);
  unsigned int bytes_rem = target_fsize;

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

      // FAT entry for given cluster gives number of next cluster in file or tells that this is last cluster in file

      // first cluster of file
      unsigned int curr_clust = disk[entry_start_byte + 0x1a] | (disk[entry_start_byte + 0x1a + 1] << 8);
      unsigned long curr_ent_val = fat_entry(disk, curr_clust);
      unsigned int outfile_clust_num = 0;

      while(1) {
        // write cluster to file
        int j;
        for(j = 0; j < BYTES_PER_SEC; j++) {
          if(bytes_rem <= 0) { return; }

          // Address of data region + (Cluster number-2) * Sectors per cluster * Bytes per sector
          unsigned long disk_loc = data_start_byte(disk) + ((curr_clust - 2)*BYTES_PER_SEC) + j;
          unsigned long outfile_loc = (outfile_clust_num * BYTES_PER_SEC) + j;

          out_file[outfile_loc] = disk[disk_loc];
          bytes_rem --;
        }

        if(curr_ent_val >= 0xFF8 && curr_ent_val <= 0xFFF) { return; }
        else if(curr_ent_val >= 0x002 && curr_ent_val <= 0xFEF) {
          curr_clust = curr_ent_val; // get next cluster
          curr_ent_val = fat_entry(disk, curr_clust);
          outfile_clust_num++;
        }
        else {
          printf("different entry val found: %X\n", curr_ent_val); return;
        }
      }
      return;
    }
  }

  printf("file %s not found in root directory.\n", target_filename);
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

  copy_from_disk(disk, out_map, argv[2], disk_size_bytes, file_size);
}
