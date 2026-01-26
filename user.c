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
    // Create root user
    users[0].active = 1;
    strcpy("root", users[0].username);
    strcpy("admin", users[0].password);
}

int user_login(char *username, char *password) {
    for(int i=0; i<MAX_USERS; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                current_user_idx = i;
                return 1; // Success
            }
        }
    }
    return 0; // Failed
}

void user_add(char *username, char *password) {
    // Check if user exists
    for(int i=0; i<MAX_USERS; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0) {
            kprint_color("Error: User exists.\n", 0x04);
            return;
        }
    }
    
    // Find empty slot
    int slot = -1;
    for(int i=0; i<MAX_USERS; i++) {
        if (!users[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        kprint_color("Error: Max users reached.\n", 0x04);
        return;
    }
    
    users[slot].active = 1;
    strncpy(username, users[slot].username, 31);
    users[slot].username[31] = 0; // Ensure null termination
    strncpy(password, users[slot].password, 31);
    users[slot].password[31] = 0;
    kprint_color("User created.\n", 0x02);
}

char* user_get_current_user() {
    if (current_user_idx != -1) {
        return users[current_user_idx].username;
    }
    return "unknown";
}
