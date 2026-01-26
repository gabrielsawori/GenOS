#include "fs.h"
#include "screen.h"
#include "utils.h"

#define MAX_FILES 32

File ramdisk[MAX_FILES];
int current_dir_id = 0;

void fs_init() {
    for(int i=0; i<MAX_FILES; i++) {
        ramdisk[i].active = 0;
        ramdisk[i].size = 0;
        ramdisk[i].name[0] = 0;
        ramdisk[i].type = 0;
        ramdisk[i].parent_id = -1;
    }
    
    // Create Root Directory
    ramdisk[0].active = 1;
    strcpy("/", ramdisk[0].name);
    ramdisk[0].type = 1; // Directory
    ramdisk[0].parent_id = 0; // Parent of root is root
    current_dir_id = 0;
}

// Mencari slot kosong di array
int fs_find_empty_slot() {
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 0) return i;
    }
    return -1;
}

// Mencari file berdasarkan nama di direktori saat ini
int fs_find_file(char *name) {
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 1 && 
            ramdisk[i].parent_id == current_dir_id && 
            strcmp(ramdisk[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void fs_list() {
    kprint("--- Directory Listing ---\n");
    int count = 0;
    for(int i=0; i<MAX_FILES; i++) {
        if (ramdisk[i].active == 1 && ramdisk[i].parent_id == current_dir_id) {
            // Don't list itself unless it is root (and even then maybe not)
            if (i == 0 && current_dir_id == 0) continue; 
            
            kprint("- ");
            kprint(ramdisk[i].name);
            if (ramdisk[i].type == 1) {
                 kprint(" <DIR>");
            } else {
                kprint(" (");
                char size_str[10];
                int_to_ascii(ramdisk[i].size, size_str);
                kprint(size_str);
                kprint(" bytes)");
            }
            kprint("\n");
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
    ramdisk[id].type = 0; // File
    ramdisk[id].parent_id = current_dir_id;
    kprint_color("File created.\n", 0x02);
}

void fs_mkdir(char *name) {
     if (fs_find_file(name) != -1) {
        kprint_color("Error: Name exists.\n", 0x04);
        return;
    }
    int id = fs_find_empty_slot();
    if (id == -1) {
        kprint_color("Error: Disk Full.\n", 0x04);
        return;
    }
    
    ramdisk[id].active = 1;
    strcpy(name, ramdisk[id].name);
    ramdisk[id].type = 1; // Directory
    ramdisk[id].size = 0;
    ramdisk[id].parent_id = current_dir_id;
    kprint_color("Directory created.\n", 0x02);
}

void fs_cd(char *name) {
    if (strcmp(name, "..") == 0) {
        if (current_dir_id != 0) {
            current_dir_id = ramdisk[current_dir_id].parent_id;
        }
        return;
    }
    
    int id = fs_find_file(name);
    if (id == -1) {
        kprint_color("Error: Directory not found.\n", 0x04);
        return;
    }
    if (ramdisk[id].type != 1) {
        kprint_color("Error: Not a directory.\n", 0x04);
        return;
    }
    current_dir_id = id;
}

void get_path_recursive(int id, char *buffer) {
    if (id == 0) {
        strcat(buffer, "/");
        return;
    }
    if (ramdisk[id].parent_id != id) { 
        get_path_recursive(ramdisk[id].parent_id, buffer);
    }
    if (ramdisk[id].parent_id != 0) strcat(buffer, "/");
    strcat(buffer, ramdisk[id].name);
}

void fs_get_cwd(char *buffer) {
    buffer[0] = 0;
    if (current_dir_id == 0) {
        strcpy("/", buffer);
    } else {
        get_path_recursive(current_dir_id, buffer);
    }
}

void fs_pwd() {
    char buffer[256];
    fs_get_cwd(buffer);
    kprint("Path: ");
    kprint(buffer);
    kprint("\n");
}

void fs_read(char *name) {
    int id = fs_find_file(name);
    if (id == -1) {
        kprint_color("Error: File not found.\n", 0x04);
        return;
    }
    if (ramdisk[id].type == 1) {
        kprint_color("Error: Is a directory.\n", 0x04);
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
    if (ramdisk[id].type == 1) {
        kprint_color("Error: Is a directory.\n", 0x04);
        return;
    }
    
    int current_len = strlen(ramdisk[id].content);
    int new_len = strlen(text);
    if (current_len + new_len >= 512) {
        kprint_color("Error: File full (512 bytes max).\n", 0x04);
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
    
    // Check if directory is empty
    if (ramdisk[id].type == 1) {
        for(int i=0; i<MAX_FILES; i++) {
            if (ramdisk[i].active && ramdisk[i].parent_id == id) {
                 kprint_color("Error: Directory not empty.\n", 0x04);
                 return;
            }
        }
    }
    
    ramdisk[id].active = 0;
    kprint_color("Deleted.\n", 0x04);
}
