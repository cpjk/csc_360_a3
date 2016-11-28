#include "utils.h"

unsigned int fat_entry(char *disk, unsigned long disk_size_bytes, unsigned int clust_num) {
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

unsigned int root_dir_bytes() {
  return 32 * ROOT_DIR_MAX_ENT;
}

unsigned int root_dir_sectors() {
  return root_dir_bytes() / BYTES_PER_SEC;
}

unsigned long file_size(FILE *fp) {
  fseek(fp, 0L, SEEK_END); // set position indicator to end of file
  long size = ftell(fp);
  fseek(fp, 0L, SEEK_SET); // reset position indicator

  return size;
}

unsigned long data_size_bytes(char *disk, unsigned long disk_size_bytes) {
  return disk_size_bytes - data_start_byte(disk);
}

unsigned int data_start_byte(char *disk) {
/* fat12.data_start_addr = fat12.root_dir_sector*fat12.bytes_per_sector + */
/* fat12.max_root_dir_entries*32; */

  //   Address of data region: Address of root directory + Maximum number
  //   of root directory entries * 32
  return root_dir_start_byte(disk) + root_dir_bytes();
}

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

//read 3 bytes block at a time. 2 entries per block
unsigned int free_disk_size(char *disk, unsigned long disk_size_bytes) {
  int num_free_clust = 0;
  int in_use_clust = 0;// 2 clusters

  int entries_to_read = data_size_bytes(disk, disk_size_bytes) / 512; // equals number of data sectors
  printf("entries to read in FAT: %d\n", entries_to_read);

  int blocks_to_read = (entries_to_read / 2) + (entries_to_read % 2 == 0 ? 0 : 1); // 2 entries per block of 3 bytes
  printf("blocks to read in FAT: %d\n", blocks_to_read);

  unsigned int start_byte = fat1_start_byte(disk);
  printf("fat1 start byte: %X\n", start_byte);


  int b = 1; // start at 1 since ent 0,1 (block 1) are bad. 2 entries per block
  for(b; b < blocks_to_read; b++) {
    // read 2 entries
    int byte1_addr = start_byte + b*3;

    printf("%X -> ", byte1_addr);
    int hi_nibble1 = ((disk[byte1_addr + 1] & 0x0f) << 8) & 0xf00;
    int low_byte = disk[byte1_addr] & 0x0ff;
    int ent1 = (hi_nibble1 | low_byte) & 0x0FFF;
    printf("high: %X, low: %X, entry: %X ", hi_nibble1, low_byte, ent1);

    if(ent1 == 0x00) { num_free_clust++; }
    else if(ent1 >= 0x002  && ent1 <= 0xFEF) { in_use_clust++; }
    else if(ent1 >= 0xFF8  && ent1 <= 0xFFF) { in_use_clust++; }

    int hi_byte2 = (disk[byte1_addr + 2] << 4) & 0x0fff;
    int low_nibble2 = (disk[byte1_addr + 1] & 0xf0) >> 4;
    int ent2 = hi_byte2 | low_nibble2;
    printf("high: %X, low: %X, entry: %X\n", hi_byte2, low_nibble2, ent2);
    /* printf("entry: %X \n", ent2); */

    if(ent2 == 0x00) { num_free_clust++; }
    else if(ent2 >= 0x002  && ent2 <= 0xFEF) { in_use_clust++; }
    else if(ent2 >= 0xFF8  && ent2 <= 0xFFF) { in_use_clust++; }
  }

  printf("bytes in use: %d\n", in_use_clust * 512);
  return disk_size_bytes - (in_use_clust * 512); // 1024 bytes too much free space reported. 2 sectors more are used
}

unsigned int fat1_start_byte(char *disk) {
  //Start sector for partition 1 + Reserved sector count) * Bytes per sector
  return (unsigned int) reserved_sec_cnt(disk) * BYTES_PER_SEC;
}

unsigned int num_fats(char *disk) {
  return (unsigned int) disk[0x10];
}

unsigned int total_sec(char *disk) {
  return disk[0x13] | (disk[0x14] << 8);
}

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

// worry about hidden files?
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
unsigned long disk_size(char *diskname) {
  FILE *fp = fopen(diskname, "r"); // open disk file
  unsigned long size = file_size(fp); /* get disk size */
  fclose(fp); // close the disk file
  return size;
}
