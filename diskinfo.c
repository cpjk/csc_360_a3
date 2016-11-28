#include "utils.h"

unsigned int data_clusters(char *disk, unsigned long disk_size_bytes) {
  unsigned int clust = data_size_bytes(disk, disk_size_bytes) / 512;
  unsigned int dstartsec = data_start_byte(disk)/512;
  unsigned int dsec = total_sec(disk) - dstartsec;

  return dsec;
}
unsigned int entries_per_fat(char *disk) {
  return sec_per_fat(disk) * BYTES_PER_SEC / 3; // 3 bytes per entry
}

unsigned int free_disk_clusters(char *disk, unsigned long disk_size_bytes) {
  unsigned int free_clusters = 0;
  unsigned int in_use_clust = 0;

  int i;
  for(i = 2; i < total_sec(disk); i++) {
    unsigned int ent_val = fat_entry(disk, i);
    if(ent_val == 0x000) {
      free_clusters++;
    }
    else {
      in_use_clust++;
    }
  }
  /* printf("In use clusters: %d", in_use_clust); */
  return free_clusters;

  /* return total_data_clust - in_use_clust; */
}
unsigned int free_disk_bytes(char *disk, unsigned long disk_size_bytes) {
  return free_disk_clusters(disk, disk_size_bytes) * 512;
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

  unsigned long disk_size_bytes = disk_size(argv[1]);

  fd = open(argv[1], O_RDONLY); // get a file descriptor for the disk
  if(fd == -1) { perror("open"); return 1; }
  if(fstat(fd, &sb) == -1) { perror("fstat"); return 1; }

  disk = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); // mmap the disk to memory
  if (disk == MAP_FAILED) { perror ("mmap"); return 1; }
  if (close (fd) == -1) { perror ("close"); return 1; }

  char *sysname = malloc(9*sizeof(char));  // get OS name
  sysname[8] = '\0';

  strncpy(sysname, &disk[0x03], 8);

  char volume_label[9];
  volume_label[8] = '\0';
  get_volume_label(disk, volume_label);

  printf("OS Name: %s\n", sysname);
  printf("Label of disk: %s\n", volume_label);
  printf("Total size of the disk: %lu bytes.\n", disk_size_bytes);
  printf("Free size of disk: %d\n", free_disk_bytes(disk, disk_size_bytes));
  printf("================\n");
  printf("Number of files in the root directory (not including subdirectories): %d\n",
      num_files_root_dir(disk));
  printf("================\n");
  printf("Number of FAT copies: %d\n", num_fats(disk));
  printf("Sectors per FAT: %d\n", sec_per_fat(disk));
}
