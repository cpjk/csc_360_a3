#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

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
