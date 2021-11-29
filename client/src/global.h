/**
 * global.h
 * 宏、常量等头文件
 */

#pragma once

#define MAX_SIZE    8192
#define MID_SIZE    1024
#define MIN_SIZE    128

typedef enum Status {
    NONE,
    CONN,
    USER,
    PASS
} Status;

typedef enum Mode {
    DEFAULT,
    PORT,
    PASV
} Mode;