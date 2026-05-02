#include "user.h"
#include "utils.h"
#include "screen.h"

#define MAX_USERS 5

User users[MAX_USERS];
int current_user_idx = -1;

void user_init() {
    for(int i=0; i<MAX_USERS; i++) {
        users[i].active = 0;
    }
    // SEBELUMNYA TERBALIK: strcpy("root", users[0].username); -> SALAH
    // PERBAIKAN:
    users[0].active = 1;
    strcpy(users[0].username, "root");
    strcpy(users[0].password, "admin");
}

int user_login(char *username, char *password) {
    for(int i=0; i<MAX_USERS; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                current_user_idx = i;
                return 1; // Berhasil
            }
        }
    }
    return 0; // Gagal
}

void user_add(char *username, char *password) {
    // Cek apakah user sudah ada
    for(int i=0; i<MAX_USERS; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0) {
            kprint_color("Error: User sudah ada.\n", 0x04);
            return;
        }
    }
    
    // Cari slot kosong
    int slot = -1;
    for(int i=0; i<MAX_USERS; i++) {
        if (!users[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        kprint_color("Error: Kapasitas user penuh.\n", 0x04);
        return;
    }
    
    users[slot].active = 1;
    strcpy(users[slot].username, username);
    strcpy(users[slot].password, password);
}

char* user_get_current_user() {
    if (current_user_idx == -1) return "guest";
    return users[current_user_idx].username;
}