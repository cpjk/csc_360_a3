#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

 // equals bytes per cluster
#define BYTES_PER_SEC 512

// rootdir size: 14 sec*16 entries/sec = 224 max entries
#define ROOT_DIR_MAX_ENT 224
#define BYTES_PER_FAT_ENT 1.5

unsigned int free_disk_bytes(char *disk, unsigned long disk_size_bytes);
unsigned int free_disk_clusters(char *disk, unsigned long disk_size_bytes);
unsigned int root_dir_f_size(char *disk, char *target_filename);
unsigned int fat_entry(char *disk, unsigned int clust_num);
unsigned long file_size(FILE *fp);
unsigned int sec_per_fat(char *disk);
unsigned int fat1_start_byte(char *disk);
unsigned int num_fats(char *disk);
unsigned int num_files_root_dir(char *disk);
unsigned int reserved_sec_cnt(char *disk);
void get_volume_label(char *disk, char *buffer);
unsigned int root_dir_start_byte(char *disk);
unsigned int data_start_byte(char *disk);
unsigned long disk_size(char *diskname);
unsigned long file_size(FILE *fp);
unsigned int root_dir_sectors();
unsigned long data_size_bytes(char *disk, unsigned long disk_size_bytes);
unsigned int free_disk_size(char *disk, unsigned long disk_size_bytes);
unsigned int total_sec(char *disk);
