#ifndef LIBC_UNISTD_H
#define LIBC_UNISTD_H

int open(char *filename);
int read(int fd, char *buffer, int length);
int close(int fd);

#endif