#include "fs.h"
#include "screen.h"
#include "utils.h"
#include "ata.h"
#include "mem.h"

u32 fat[TOTAL_SECTORS];
dir_entry_t directory[MAX_FILES];
int current_dir_id = 0;

void fs_save_tables() {
    u8 *fat_ptr = (u8 *)fat;
    for (int i = 0; i < 16; i++) {
        ata_write_sector(2 + i, fat_ptr + (i * SECTOR_SIZE));
    }
    u8 *dir_ptr = (u8 *)directory;
    for (int i = 0; i < 6; i++) {
        ata_write_sector(18 + i, dir_ptr + (i * SECTOR_SIZE));
    }
}

void fs_load_tables() {
    u8 *fat_ptr = (u8 *)fat;
    for (int i = 0; i < 16; i++) {
        ata_read_sector(2 + i, fat_ptr + (i * SECTOR_SIZE));
    }
    u8 *dir_ptr = (u8 *)directory;
    for (int i = 0; i < 6; i++) {
        ata_read_sector(18 + i, dir_ptr + (i * SECTOR_SIZE));
    }
}

u32 fs_allocate_sector() {
    for (u32 i = 24; i < TOTAL_SECTORS; i++) {
        if (fat[i] == FAT_FREE) {
            fat[i] = FAT_EOF;
            return i;
        }
    }
    return 0;
}

void fs_free_chain(u32 start_sector) {
    u32 curr = start_sector;
    while (curr != FAT_EOF && curr != FAT_FREE && curr < TOTAL_SECTORS) {
        u32 next = fat[curr];
        fat[curr] = FAT_FREE;
        curr = next;
    }
}

void fs_init() {
    fs_load_tables();
    if (directory[0].active != 1 || strcmp(directory[0].name, "/") != 0) {
        kprint("Disk baru terdeteksi. Memformat ke Sistem Berkas v2.0...\n");
        for (int i = 0; i < TOTAL_SECTORS; i++) fat[i] = FAT_FREE;
        for (int i = 0; i < MAX_FILES; i++) {
            directory[i].active = 0;
            directory[i].size = 0;
            directory[i].first_sector = 0;
        }
        directory[0].active = 1;
        strcpy("/", directory[0].name);
        directory[0].type = 1; 
        directory[0].parent_id = 0;
        fs_save_tables();
    }
    current_dir_id = 0;
}

int fs_find_file(char *name) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(directory[i].active == 1 && directory[i].parent_id == current_dir_id && strcmp(directory[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int fs_find_empty_dir_slot() {
    for(int i = 0; i < MAX_FILES; i++) {
        if(directory[i].active == 0) return i;
    }
    return -1;
}

void fs_list() {
    kprint("--- Directory Listing ---\n");
    int count = 0;
    for(int i = 0; i < MAX_FILES; i++) {
        if(directory[i].active == 1 && directory[i].parent_id == current_dir_id) {
            if(i == 0 && current_dir_id == 0) continue; 
            kprint("- "); kprint(directory[i].name);
            if(directory[i].type == 1) kprint(" <DIR>\n");
            else {
                kprint(" (");
                char size_str[10]; int_to_ascii(directory[i].size, size_str);
                kprint(size_str); kprint(" bytes)\n");
            }
            count++;
        }
    }
    if(count == 0) kprint("(Empty)\n");
}

void fs_create(char *name) {
    if(fs_find_file(name) != -1) { kprint_color("Error: File exists.\n", 0x04); return; }
    int id = fs_find_empty_dir_slot();
    if(id == -1) { kprint_color("Error: Directory full.\n", 0x04); return; }

    directory[id].active = 1;
    strcpy(name, directory[id].name);
    directory[id].size = 0;
    directory[id].first_sector = 0; 
    directory[id].type = 0; 
    directory[id].parent_id = current_dir_id;
    fs_save_tables();
    kprint_color("File created.\n", 0x02);
}

void fs_mkdir(char *name) {
    if(fs_find_file(name) != -1) { kprint_color("Error: Name exists.\n", 0x04); return; }
    int id = fs_find_empty_dir_slot();
    if(id == -1) { kprint_color("Error: Directory full.\n", 0x04); return; }

    directory[id].active = 1;
    strcpy(name, directory[id].name);
    directory[id].size = 0;
    directory[id].first_sector = 0;
    directory[id].type = 1; 
    directory[id].parent_id = current_dir_id;
    fs_save_tables();
    kprint_color("Directory created.\n", 0x02);
}

void fs_cd(char *name) {
    if(strcmp(name, "..") == 0) {
        if(current_dir_id != 0) current_dir_id = directory[current_dir_id].parent_id;
        return;
    }
    int id = fs_find_file(name);
    if(id == -1) { kprint_color("Error: Directory not found.\n", 0x04); return; }
    if(directory[id].type != 1) { kprint_color("Error: Not a directory.\n", 0x04); return; }
    current_dir_id = id;
}

void get_path_recursive_v2(int id, char *buffer) {
    if(id == 0) { strcat(buffer, "/"); return; }
    if(directory[id].parent_id != id) get_path_recursive_v2(directory[id].parent_id, buffer);
    if(directory[id].parent_id != 0) strcat(buffer, "/");
    strcat(buffer, directory[id].name);
}

void fs_get_cwd(char *buffer) {
    buffer[0] = 0;
    if(current_dir_id == 0) strcpy("/", buffer);
    else get_path_recursive_v2(current_dir_id, buffer);
}

void fs_pwd() {
    char buffer[256]; fs_get_cwd(buffer);
    kprint("Path: "); kprint(buffer); kprint("\n");
}

void fs_delete(char *name) {
    int id = fs_find_file(name);
    if(id == -1) { kprint_color("Error: File not found.\n", 0x04); return; }
    if(directory[id].type == 1) {
        for(int i = 0; i < MAX_FILES; i++) {
            if(directory[i].active && directory[i].parent_id == id) {
                kprint_color("Error: Directory not empty.\n", 0x04); return;
            }
        }
    }
    if(directory[id].first_sector != 0) fs_free_chain(directory[id].first_sector);
    
    directory[id].active = 0;
    fs_save_tables();
    kprint_color("Deleted.\n", 0x04);
}

void fs_write(char *name, char *text) {
    int id = fs_find_file(name);
    if(id == -1) { kprint_color("Error: File not found.\n", 0x04); return; }
    if(directory[id].type == 1) { kprint_color("Error: Is a directory.\n", 0x04); return; }

    u32 text_len = strlen(text);
    if(directory[id].first_sector != 0) fs_free_chain(directory[id].first_sector);

    u32 current_sec = fs_allocate_sector();
    if(current_sec == 0) { kprint_color("Error: Disk Full.\n", 0x04); return; }
    
    directory[id].first_sector = current_sec;
    directory[id].size = text_len;

    u32 bytes_written = 0;
    u8 buffer[SECTOR_SIZE];

    while(bytes_written < text_len) {
        for(int i = 0; i < SECTOR_SIZE; i++) buffer[i] = 0;
        
        u32 copy_len = SECTOR_SIZE;
        if(text_len - bytes_written < SECTOR_SIZE) copy_len = text_len - bytes_written;

        for(u32 i = 0; i < copy_len; i++) buffer[i] = text[bytes_written + i];
        
        ata_write_sector(current_sec, buffer);
        bytes_written += copy_len;

        if(bytes_written < text_len) {
            u32 next_sec = fs_allocate_sector();
            if(next_sec == 0) { kprint_color("Error: Disk Full during write.\n", 0x04); break; }
            fat[current_sec] = next_sec; 
            current_sec = next_sec;
        }
    }
    fs_save_tables();
    kprint("Data written.\n");
}

// --- FUNGSI BARU UNTUK LIBC / PYTHON ---
// Membaca isi file langsung ke dalam buffer memori (bukan dicetak ke layar)
int fs_read_file_to_buffer(int id, u8 *out_buffer, u32 max_len) {
    if(id < 0 || id >= MAX_FILES || directory[id].active == 0) return -1;
    if(directory[id].type == 1) return -1; // Direktori tidak bisa dibaca sebagai teks

    u32 curr = directory[id].first_sector;
    u8 sector_buffer[SECTOR_SIZE];
    u32 bytes_read = 0;
    u32 total_size = directory[id].size;

    while(curr != FAT_EOF && curr != FAT_FREE && bytes_read < max_len) {
        ata_read_sector(curr, sector_buffer);
        
        u32 copy_len = SECTOR_SIZE;
        if(total_size - bytes_read < SECTOR_SIZE) copy_len = total_size - bytes_read;
        if(max_len - bytes_read < copy_len) copy_len = max_len - bytes_read;

        for(u32 i = 0; i < copy_len; i++) {
            out_buffer[bytes_read + i] = sector_buffer[i];
        }
        
        bytes_read += copy_len;
        curr = fat[curr]; // Lompat ke sektor berikutnya di rantai FAT
    }
    return bytes_read;
}

void fs_read(char *name) {
    int id = fs_find_file(name);
    if(id == -1) { kprint_color("Error: File not found.\n", 0x04); return; }
    if(directory[id].type == 1) { kprint_color("Error: Is a directory.\n", 0x04); return; }

    kprint("Content:\n");
    if(directory[id].size == 0 || directory[id].first_sector == 0) {
        kprint("\n[EOF]\n");
        return;
    }

    u32 curr = directory[id].first_sector;
    u8 buffer[SECTOR_SIZE];
    u32 bytes_read = 0;
    u32 total_size = directory[id].size;

    while(curr != FAT_EOF && curr != FAT_FREE) {
        ata_read_sector(curr, buffer);
        
        u32 print_len = SECTOR_SIZE;
        if(total_size - bytes_read < SECTOR_SIZE) print_len = total_size - bytes_read;

        for(u32 i = 0; i < print_len; i++) {
            char str[2] = {buffer[i], 0};
            kprint_color(str, 0x03);
        }
        
        bytes_read += print_len;
        curr = fat[curr]; 
    }
    kprint("\n[EOF]\n");
}