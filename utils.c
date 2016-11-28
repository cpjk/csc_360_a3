#include "utils.h"

// number of free clusters on the disk
unsigned int free_disk_clusters(char *disk, unsigned long disk_size_bytes) {
  unsigned int free_clusters = 0;
  int i;

  for(i = 2; i < total_sec(disk); i++) {
    if(fat_entry(disk, i) == 0x000) { free_clusters++; }
  }
  return free_clusters;
}
unsigned int free_disk_bytes(char *disk, unsigned long disk_size_bytes) {
  return free_disk_clusters(disk, disk_size_bytes) * 512;
}

// value of the FAT entry number clust_num
unsigned int fat_entry(char *disk, unsigned int clust_num) {
  unsigned int entry;
  unsigned int fat_start_byte = fat1_start_byte(disk);

  if (clust_num % 2 == 0) { // even entry number
    // low four bits in location 1+(3*n)/2 and the 8 bits in location (3*n)/2
    unsigned int hi = ((disk[fat_start_byte + 1 + (3*clust_num/2)] & 0x0F) << 8);
    unsigned int lo = disk[fat_start_byte + (3*clust_num/2)] & 0x0FF;
    entry = hi | lo;
  }
  else {
    // high four bits in location (3*n)/2 and the 8 bits in location 1+(3*n)/2
    unsigned int hi = ((disk[fat_start_byte + 1 + (3*clust_num/2)]) << 4) & 0x0FF0;
    unsigned int lo = ((disk[fat_start_byte + (3*clust_num/2)] & 0xF0) >> 4);
    entry = hi | lo;
  }
  entry = entry & 0x0FFF;

  return entry;
}

// size of file in the root directory in bytes
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
        unsigned int byte1 = disk[entry_start_byte + 0x1c] & 0x00FF;
        unsigned int byte2 = disk[entry_start_byte + 0x1c + 1] & 0x00FF;
        unsigned int byte3 = disk[entry_start_byte + 0x1c + 2] & 0x00FF;
        unsigned int byte4 = disk[entry_start_byte + 0x1c + 3] & 0x00FF;
        unsigned long filesize = (byte4 << 24) | (byte3 << 16) | (byte2 << 8 ) | (byte1);
        return filesize;
      }
    }
  }

  printf("file not found");
  return 0;
}

// length of the root directory in bytes
unsigned int root_dir_bytes() {
  return 32 * ROOT_DIR_MAX_ENT;
}

// number of sectors in the root directory
unsigned int root_dir_sectors() {
  return root_dir_bytes() / BYTES_PER_SEC;
}

// file size of the given file in bytes
unsigned long file_size(FILE *fp) {
  fseek(fp, 0L, SEEK_END); // set position indicator to end of file
  long size = ftell(fp);
  fseek(fp, 0L, SEEK_SET); // reset position indicator

  return size;
}

// number of bytes in the data portion
unsigned long data_size_bytes(char *disk, unsigned long disk_size_bytes) {
  return disk_size_bytes - data_start_byte(disk);
}

// first byte in the data portion
unsigned int data_start_byte(char *disk) {
  //   Address of data region: Address of root directory + Maximum number
  //   of root directory entries * 32
  return root_dir_start_byte(disk) + root_dir_bytes();
}

// number of reserved sectors
unsigned int reserved_sec_cnt(char *disk) {
  return (unsigned int) disk[0x0e] | (disk[0x0f] << 8);
}

// first byte in the root directory
unsigned int root_dir_start_byte(char *disk) {
  // Address of first FAT + Number of FATs * Sectors per FAT
  return (unsigned int) fat1_start_byte(disk) + num_fats(disk) * sec_per_fat(disk) * BYTES_PER_SEC;
}

// sectors per FAT
unsigned int sec_per_fat(char *disk) {
  return (unsigned int) ((disk[0x17] << 8) | disk[0x16]);
}

// first byte in the first FAT
unsigned int fat1_start_byte(char *disk) {
  //Start sector for partition 1 + Reserved sector count) * Bytes per sector
  return (unsigned int) reserved_sec_cnt(disk) * BYTES_PER_SEC;
}

// number of FATs
unsigned int num_fats(char *disk) {
  return (unsigned int) disk[0x10];
}

// total sectors on disk
unsigned int total_sec(char *disk) {
  return disk[0x13] | (disk[0x14] << 8);
}

// volume label
void get_volume_label(char *disk, char *buffer) {
  unsigned int start_byte = root_dir_start_byte(disk);

  int ent = 0;
  for(ent; ent < ROOT_DIR_MAX_ENT; ent++) {
    unsigned int entry_start_byte = start_byte + (32 * ent); // 32 bytes per entry
    unsigned int attr_byte = entry_start_byte + 0x0b;
    if(disk[attr_byte] == 0x0f) { continue; } // long file name entry (fake entry)
    if(disk[entry_start_byte] == 0x00) { return; } // free entry and rest are free
    if(disk[entry_start_byte] == 0xe5) { continue; } // free entry
    if(disk[entry_start_byte] == 0x2e) { continue; } // dot directory
    if((disk[attr_byte] & 0x08) != 0) { // volume label
      int i;
      for(i = 0; i < 8; i++) {
        buffer[i] = disk[entry_start_byte + i];
      }
      return;
    }
  }
  return;
}

// number of files in the root directory
unsigned int num_files_root_dir(char *disk) {
  unsigned int start_byte = root_dir_start_byte(disk);
  unsigned int num_files = 0;

  int ent = 0;
  for(ent; ent < ROOT_DIR_MAX_ENT; ent++) {
    unsigned int entry_start_byte = start_byte + (32 * ent); // 32 bytes per entry
    unsigned int attr_byte = entry_start_byte + 0x0b;
    if(disk[attr_byte] == 0x0f) { continue; } // long file name entry (fake entry)
    if(disk[entry_start_byte] == 0x00) { return num_files; } // free entry and rest are free
    if(disk[entry_start_byte] == 0xe5) { continue; } // free entry
    if(disk[entry_start_byte] == 0x2e) { continue; } // dot directory
    if((disk[attr_byte] & 0x08) != 0) { continue; } // volume label

    if((disk[attr_byte] & 0x10) == 0) { num_files++; } // not subdirectory (actual file)
  }

  return num_files;
}
// size of the disk in bytes
unsigned long disk_size(char *diskname) {
  FILE *fp = fopen(diskname, "r"); // open disk file
  unsigned long size = file_size(fp); /* get disk size */
  fclose(fp); // close the disk file
  return size;
}
