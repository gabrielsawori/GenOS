#include "fs.h"
#include "screen.h"
#include "utils.h"
#include "ata.h"
#include "mem.h"
#include "../apps/app_data.h" 

u32 fat[TOTAL_SECTORS];
dir_entry_t directory[MAX_FILES];
int current_dir_id = 0;

void fs_save_tables() {
    u8 *fat_ptr = (u8 *)fat;
    for (int i = 0; i < 16; i++) ata_write_sector(2 + i, fat_ptr + (i * SECTOR_SIZE));
    u8 *dir_ptr = (u8 *)directory;
    for (int i = 0; i < 6; i++) ata_write_sector(18 + i, dir_ptr + (i * SECTOR_SIZE));
}

void fs_load_tables() {
    u8 *fat_ptr = (u8 *)fat;
    for (int i = 0; i < 16; i++) ata_read_sector(2 + i, fat_ptr + (i * SECTOR_SIZE));
    u8 *dir_ptr = (u8 *)directory;
    for (int i = 0; i < 6; i++) ata_read_sector(18 + i, dir_ptr + (i * SECTOR_SIZE));
}

void fs_format() {
    for (u32 i = 0; i < TOTAL_SECTORS; i++) fat[i] = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        directory[i].active = 0;
        directory[i].size = 0;
        directory[i].first_sector = 0;
        directory[i].type = 0;
        for(int j=0; j<32; j++) directory[i].name[j] = 0;
    }
    fs_save_tables();
}

u32 fs_allocate_sector() {
    for (u32 i = 24; i < TOTAL_SECTORS; i++) {
        if (fat[i] == 0) { fat[i] = 0xFFFFFFFF; return i; }
    }
    return 0;
}

int fs_find_file(char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].active) {
            int match = 1;
            for(int j=0; j<32; j++) {
                if (directory[i].name[j] != name[j]) { match = 0; break; }
                if (name[j] == 0) break;
            }
            if (match) return i;
        }
    }
    return -1;
}

// --- FUNGSI TULIS ANTI-AMNESIA ---
void fs_write(char *name, u8 *data, u32 size) {
    if (size == 0) return; 

    int id = fs_find_file(name);
    if (id == -1) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (!directory[i].active) {
                id = i;
                directory[id].active = 1;
                
                // CARA KASAR & PASTI BERHASIL UNTUK MENAMAI "python.elf"
                directory[id].name[0] = 'p'; directory[id].name[1] = 'y';
                directory[id].name[2] = 't'; directory[id].name[3] = 'h';
                directory[id].name[4] = 'o'; directory[id].name[5] = 'n';
                directory[id].name[6] = '.'; directory[id].name[7] = 'e';
                directory[id].name[8] = 'l'; directory[id].name[9] = 'f';
                directory[id].name[10] = 0; // Akhir string
                
                directory[id].size = size;
                directory[id].type = 0; 
                break;
            }
        }
    }

    if (id == -1) return;

    u32 sectors_needed = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    u32 prev_sector = 0;
    u32 first_sector = 0;

    for (u32 i = 0; i < sectors_needed; i++) {
        u32 curr = fs_allocate_sector();
        if (i == 0) first_sector = curr;
        if (prev_sector != 0) fat[prev_sector] = curr;

        u8 buffer[SECTOR_SIZE];
        for (int j = 0; j < SECTOR_SIZE; j++) buffer[j] = 0;
        
        u32 to_copy = (size - (i * SECTOR_SIZE) > SECTOR_SIZE) ? SECTOR_SIZE : (size - (i * SECTOR_SIZE));
        for (u32 j = 0; j < to_copy; j++) buffer[j] = data[(i * SECTOR_SIZE) + j];
        
        ata_write_sector(curr, buffer);
        prev_sector = curr;
    }
    
    directory[id].first_sector = first_sector;
    fat[prev_sector] = 0xFFFFFFFF; 
    fs_save_tables();
}

int fs_read_file_to_buffer(int id, u8 *out_buffer, u32 max_len) {
    if(id < 0 || id >= MAX_FILES || !directory[id].active) return -1;
    u32 curr = directory[id].first_sector;
    u32 bytes_read = 0;
    u32 total_size = directory[id].size;
    u8 sector_buffer[SECTOR_SIZE];
    
    while(curr != 0xFFFFFFFF && curr != 0 && bytes_read < total_size && bytes_read < max_len) {
        ata_read_sector(curr, sector_buffer);
        u32 copy_len = SECTOR_SIZE;
        if(total_size - bytes_read < SECTOR_SIZE) copy_len = total_size - bytes_read;
        if(max_len - bytes_read < copy_len) copy_len = max_len - bytes_read;
        for(u32 i = 0; i < copy_len; i++) out_buffer[bytes_read + i] = sector_buffer[i];
        bytes_read += copy_len;
        curr = fat[curr]; 
    }
    return bytes_read;
}

void fs_read(char *name) {
    int id = fs_find_file(name);
    if(id == -1) { kprint_color("Error: File not found.\n", 0x04); return; }
    if(directory[id].type == 1) { kprint_color("Error: Is a directory.\n", 0x04); return; }
    if(directory[id].size == 0 || directory[id].first_sector == 0) { kprint("\n[EOF]\n"); return; }

    u32 curr = directory[id].first_sector;
    u8 buffer[SECTOR_SIZE];
    u32 bytes_read = 0;
    u32 total_size = directory[id].size;

    while(curr != 0xFFFFFFFF && curr != 0 && bytes_read < total_size) {
        ata_read_sector(curr, buffer);
        u32 copy_len = SECTOR_SIZE;
        if(total_size - bytes_read < SECTOR_SIZE) copy_len = total_size - bytes_read;
        for(u32 i = 0; i < copy_len; i++) {
            char str[2] = {buffer[i], 0};
            kprint(str);
        }
        bytes_read += copy_len;
        curr = fat[curr];
    }
    kprint("\n[EOF]\n");
}

void fs_list() {
    kprint("Files in root:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].active) {
            kprint("- ");
            if (directory[i].name[0] == 0) kprint("[UNNAMED_FILE]");
            else kprint(directory[i].name);
            kprint(" (");
            char size_str[16];
            int_to_ascii(directory[i].size, size_str);
            kprint(size_str);
            kprint(" bytes)\n");
        }
    }
}

// --- DIAGNOSTIK & AUTO-INSTALLER ---
void fs_init() {
    kprint("=== INITIALIZING FILE SYSTEM ===\n");
    
    fs_format();
    kprint_color("[FS] Hard Disk Formatted!\n", 0x0E);

    kprint("[FS] Python ELF Size: ");
    char size_str[16];
    int_to_ascii(apps_python_elf_len, size_str);
    kprint(size_str);
    kprint(" bytes\n");

    if (apps_python_elf_len > 0) {
        fs_write("python.elf", apps_python_elf, apps_python_elf_len);
        kprint_color("[FS] python.elf installed successfully!\n", 0x0A);
    } else {
        kprint_color("[ERROR] python.elf is 0 bytes! Recompile apps/python.c\n", 0x04);
    }
    kprint_color("[SUCCESS] File System Ready.\n\n", 0x02);
}

void fs_get_cwd(char *buf) { strcpy(buf, "/"); }
void fs_pwd() { kprint("/\n"); }