/**
 * file.h
 * 文件处理头文件
 */

#pragma once

#include "global.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * 检查文件是否存在，如果是返回文件大小
 */
int file_check(char* filename, char* mode);

/**
 * 接收文件
 */
int recv_file(int connfd, char* filename);

/**
 * 发送文件
 */
void send_file(int connfd, char* filename);

/**
 * 接收文件列表
 */
void recv_ls(int connfd);