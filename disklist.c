#include "utils.h"

// list contents of the root directory
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

    char *type = ((disk[attr_byte] & 0x10) == 0) ? "F" : "D"; // 0x10 indicates a subdirectory

    unsigned int byte1 = disk[entry_start_byte + 0x1c] & 0x00FF;
    unsigned int byte2 = disk[entry_start_byte + 0x1c + 1] & 0x00FF;
    unsigned int byte3 = disk[entry_start_byte + 0x1c + 2] & 0x00FF;
    unsigned int byte4 = disk[entry_start_byte + 0x1c + 3] & 0x00FF;
    unsigned long filesize = (byte4 << 24) | (byte3 << 16) | (byte2 << 8 ) | (byte1);

    // get file name
    int i;
    char filename[21] = "";
    for(i = 0; i < 21; i++) { filename[i] = '\0'; }
    for(i = 0; i < 8; i++) {
      if(disk[entry_start_byte + i] == 0x20) break;
      filename[i] = disk[entry_start_byte + i];
    }

    // get file extension
    char fextension[4] = "";
    for(i = 0; i < 4; i++) { fextension[i] = '\0'; }
    for(i = 0; i < 3; i++) {
      char c = disk[entry_start_byte + 0x08 + i];
      fextension[i] = c;
    }

    // get time
    unsigned int time_byte1 = disk[entry_start_byte + 0x0e] & 0x0FF;
    unsigned int time_byte2 = disk[entry_start_byte + 0x0e + 1] & 0x0FF;
    unsigned int time = (time_byte1 | time_byte2 << 8) & 0xFFFF;
    int hours = (time >> 11) & 0x1F; // 5 bits (15-11)
    int min = (time >> 5) & 0x3F; // 6 bits (10-5)

    // get date
    unsigned int date_byte1 = disk[entry_start_byte + 0x10] & 0x0FF;
    unsigned int date_byte2 = disk[entry_start_byte + 0x10 + 1] & 0x0FF;
    unsigned int date = (date_byte1 | date_byte2 << 8) & 0xFFFF;
    unsigned int year = ((date >> 9) & 0x7F) + 1980;  // 7 bits (15-9)
    unsigned int month = (date >> 5) & 0x0F;   // 4 bits (8-5)
    unsigned int day =  date & 0x1F;    // 5 bits (4-0)

    // add a zero for hours or mins less than 10
    char hour_pad = hours < 10 ? '0' : '\0';
    char min_pad = min < 10 ? '0' : '\0';

    printf("%s %ld %20s.%s ", type, filesize, filename,  fextension);
    printf("%d-%d-%d ", year, month, day);
    printf("%c%d:%c%d\n", hour_pad, hours, min_pad, min);
  }

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

  fd = open(argv[1], O_RDONLY); // get a file descriptor for the disk
  if(fd == -1) { perror("open"); return 1; }
  if(fstat(fd, &sb) == -1) { perror("fstat"); return 1; }

  disk = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); // mmap the disk to memory
  if (disk == MAP_FAILED) { perror ("mmap"); return 1; }
  if (close (fd) == -1) { perror ("close"); return 1; }

  unsigned long disk_size_bytes = disk_size(argv[1]);

  list_root_dir(disk);
}
