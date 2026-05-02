#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define SECTOR_SIZE 512
#define TOTAL_SECTORS 2048
#define MAX_FILES 64
#define FAT_FREE 0x00000000
#define FAT_EOF  0xFFFFFFFF

// Struktur direktori persis seperti di fs.h GenOS
typedef struct {
    char name[32];
    u32 size;
    u32 first_sector;
    u8 active;
    u8 type;
    int parent_id;
} __attribute__((packed)) dir_entry_t;

u32 fat[TOTAL_SECTORS];
dir_entry_t directory[MAX_FILES];

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Cara pakai: ./injector <file_hdd.img> <file_app.elf>\n");
        return 1;
    }

    FILE *hdd = fopen(argv[1], "r+b");
    FILE *app = fopen(argv[2], "rb");

    if (!hdd || !app) {
        printf("Error: Gagal membuka file disk atau aplikasi.\n");
        return 1;
    }

    // 1. Baca Tabel FAT & Direktori dari hdd.img
    fseek(hdd, 2 * SECTOR_SIZE, SEEK_SET);
    fread(fat, sizeof(u32), TOTAL_SECTORS, hdd);
    
    fseek(hdd, 18 * SECTOR_SIZE, SEEK_SET);
    fread(directory, sizeof(dir_entry_t), MAX_FILES, hdd);

    // 2. Hitung ukuran aplikasi .elf
    fseek(app, 0, SEEK_END);
    u32 app_size = ftell(app);
    fseek(app, 0, SEEK_SET);

    // 3. Cari slot kosong di direktori GenOS
    int dir_idx = -1;
    for (int i = 1; i < MAX_FILES; i++) { // Mulai dari 1 untuk lewati root ("/")
        if (directory[i].active == 0) {
            dir_idx = i; break;
        }
    }

    // 4. Suntikkan file ke sektor hard disk yang kosong
    u32 bytes_written = 0;
    u32 current_sec = 0;
    u32 first_sec = 0;

    // PERBAIKAN: Mulai injeksi data dari Sektor 24
    for (u32 i = 24; i < TOTAL_SECTORS && bytes_written < app_size; i++) {
        if (fat[i] == FAT_FREE) {
            if (first_sec == 0) first_sec = i;
            if (current_sec != 0) fat[current_sec] = i; // Sambung rantai FAT

            current_sec = i;
            fat[current_sec] = FAT_EOF; // Tandai akhir file sementara

            // Baca 512 byte dari aplikasi dan tulis ke hard disk
            u8 buffer[SECTOR_SIZE] = {0};
            u32 copy_len = (app_size - bytes_written < SECTOR_SIZE) ? app_size - bytes_written : SECTOR_SIZE;
            fread(buffer, 1, copy_len, app);

            fseek(hdd, current_sec * SECTOR_SIZE, SEEK_SET);
            fwrite(buffer, 1, SECTOR_SIZE, hdd);

            bytes_written += copy_len;
        }
    }

    // 5. Daftarkan file ke dalam direktori GenOS
    directory[dir_idx].active = 1;
    strcpy(directory[dir_idx].name, "app.elf");
    directory[dir_idx].size = app_size;
    directory[dir_idx].first_sector = first_sec;
    directory[dir_idx].type = 0;
    directory[dir_idx].parent_id = 0;

    // 6. Simpan kembali pembaruan tabel FAT & Direktori ke hard disk
    fseek(hdd, 2 * SECTOR_SIZE, SEEK_SET);
    fwrite(fat, sizeof(u32), TOTAL_SECTORS, hdd);
    
    fseek(hdd, 18 * SECTOR_SIZE, SEEK_SET);
    // Kita menyimpan 6 SEKTOR ke hardisk untuk direktori
    fwrite(directory, sizeof(dir_entry_t), MAX_FILES, hdd);

    fclose(hdd);
    fclose(app);
    printf("Sukses! Aplikasi %s berhasil disuntikkan ke dalam hdd.img!\n", argv[2]);
    return 0;
}