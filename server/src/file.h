/**
 * file.h
 * 文件处理相关的头文件
 */

#pragma once

#include "global.h"
#include "server.h"

/**
 * 检查文件是否存在，如果是，返回文件大小
 */
int file_check(char* filename, char* mode);

/**
 * 发送文件
 */
void send_file(int connfd, char* filename);

/**
 * 接收文件，返回文件大小
 */
int recv_file(int connfd, char* filename);

/**
 * 获取原始文件名
 */
void get_raw_filename(char* raw_filename, char* param);

/**
 * 发送文件列表
 */
void send_ls(int connfd);