#include "fs.h"
#include "screen.h"
#include "utils.h"

#define MAX_FILES 10

File ramdisk[MAX_FILES];

void fs_init() {
    for(int i=0; i<MAX_FILES; i++) {
        ramdisk[i].active = 0;
        ramdisk[i].size = 0;
        ramdisk[i].name[0] = 0;
    }
}

// Mencari slot kosong di array
int fs_find_empty_slot() {
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 0) return i;
    }
    return -1;
}

// Mencari file berdasarkan nama
int fs_find_file(char *name) {
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 1 && strcmp(ramdisk[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void fs_list() {
    kprint("--- Filesystem ---\n");
    int count = 0;
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 1) {
            kprint("- ");
            kprint(ramdisk[i].name);
            kprint(" (");
            char size_str[10];
            int_to_ascii(ramdisk[i].size, size_str);
            kprint(size_str);
            kprint(" bytes)\n");
            count++;
        }
    }
    if (count == 0) kprint("(Empty)\n");
}

void fs_create(char *name) {
    if (fs_find_file(name) != -1) {
        kprint_color("Error: File exists.\n", 0x04);
        return;
    }
    int id = fs_find_empty_slot();
    if (id == -1) {
        kprint_color("Error: Disk Full.\n", 0x04);
        return;
    }
    
    ramdisk[id].active = 1;
    strcpy(name, ramdisk[id].name);
    ramdisk[id].content[0] = 0;
    ramdisk[id].size = 0;
    kprint_color("File created.\n", 0x02);
}

void fs_read(char *name) {
    int id = fs_find_file(name);
    if (id == -1) {
        kprint_color("Error: File not found.\n", 0x04);
        return;
    }
    kprint("Content:\n");
    kprint_color(ramdisk[id].content, 0x03); // Cyan
    kprint("\n[EOF]\n");
}

void fs_write(char *name, char *text) {
    int id = fs_find_file(name);
    if (id == -1) {
        kprint_color("Error: File not found.\n", 0x04);
        return;
    }
    // Append (Menambahkan) teks ke akhir file
    strcat(ramdisk[id].content, text);
    ramdisk[id].size = strlen(ramdisk[id].content);
    kprint("Data written.\n");
}

void fs_delete(char *name) {
    int id = fs_find_file(name);
    if (id == -1) {
        kprint_color("Error: File not found.\n", 0x04);
        return;
    }
    ramdisk[id].active = 0;
    kprint_color("File deleted.\n", 0x04);
}