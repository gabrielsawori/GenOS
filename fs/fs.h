#ifndef FS_H
#define FS_H

#include "utils.h"

#define MAX_FILES 64
#define TOTAL_SECTORS 2048  // Hard disk kita 1MB (2048 sektor x 512 byte)
#define SECTOR_SIZE 512

// Kode status untuk tabel FAT
#define FAT_FREE 0x00000000
#define FAT_EOF  0xFFFFFFFF

// Struktur Directory Entry
typedef struct {
    char name[32];
    u32 size;
    u32 first_sector; 
    u8 active;
    u8 type;          // 0 = File, 1 = Direktori
    int parent_id;
} __attribute__((packed)) dir_entry_t;

void fs_init();
void fs_list();
void fs_create(char *name);
void fs_mkdir(char *name);
void fs_cd(char *name);
void fs_pwd();
void fs_delete(char *name);
void fs_write(char *name, char *text);
void fs_read(char *name);
void fs_get_cwd(char *buffer);
int fs_find_file(char *name);

// FUNGSI BARU UNTUK SYSCALL & PYTHON
int fs_read_file_to_buffer(int id, u8 *out_buffer, u32 max_len);

#endif