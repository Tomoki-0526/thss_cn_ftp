#include "file.h"

int file_check(char* filename, char* mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        printf("Cannot open file: %s\n", filename);
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    return ftell(file);
}

void send_file(int connfd, char* filename) {
    FILE* file = fopen(filename, "rb");
    
    int cnt, p;
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    while (1) {
        cnt = fread(buf, 1, 4096, file);
        if (cnt == 0) {
            break;
        }
        p = 0;
        while (p < cnt) {
            int n = write(connfd, buf + p, cnt - p);
            if (n < 0) {
                printf("Write error.\n");
                return;
            }
            else {
                p += n;
            }
        }
        if (cnt != 4096) {
            break;
        }
    }

    fclose(file);
}

int recv_file(int connfd, char* filename) {
    FILE* file = fopen(filename, "wb");
    int n, file_size = 0;
    char buf[4096];

    do {
        n = recv(connfd, buf, 4096, 0);
        fwrite(buf, 1, n, file);
        file_size += n;
    } while (n > 0);

    fclose(file);
    return file_size;
}

void send_ls(int connfd) {
    char cmdls[MIN_SIZE];
    sprintf(cmdls, "ls -l %s", working_dir);

    FILE* fname = popen(cmdls, "r");
    char buf[4096];
    int cnt, p;
    while (1) {
        cnt = fread(buf, 1, 4096, fname);
        if (cnt == 0) {
            break;
        }
        p = 0;
        while (p < cnt) {
            int n = write(connfd, buf + p, cnt - p);
            if (n < 0) {
                printf("Write error.\n");
                return;
            }
            else {
                p += n;
            }
        }
        if (cnt != 4096) break;
    }
    pclose(fname);
}

void get_raw_filename(char* raw_filename, char* param) {
    char* ch = strrchr(param, '/');
    if (ch == NULL) {
        strcpy(raw_filename, param);
    }
    else {
        strcpy(raw_filename, ch + 1);
    }
}