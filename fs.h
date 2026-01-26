#ifndef FS_H
#define FS_H

#include "utils.h"

// Struktur File (Inode Sederhana)
typedef struct {
    char name[32];
    char content[512]; // Kapasitas per file 512 bytes
    int size;
    int active; // 1 = ada, 0 = kosong
    int type; // 0 = File, 1 = Directory
    int parent_id; // ID induk
} File;

void fs_init();
void fs_list();
void fs_create(char *name);
void fs_mkdir(char *name);
void fs_cd(char *name);
void fs_pwd();
void fs_get_cwd(char *buffer);
void fs_read(char *name);
void fs_delete(char *name);
void fs_write(char *name, char *text);

#endif