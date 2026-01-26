#ifndef USER_H
#define USER_H

typedef struct {
    char username[32];
    char password[32];
    int active;
} User;

void user_init();
int user_login(char *username, char *password);
void user_add(char *username, char *password);
char* user_get_current_user();

#endif
