/**
 * command.h
 * 指令头文件
 */

#pragma once

#include "global.h"
#include "client.h"
#include "file.h"
#include <time.h>

/**
 * 主循环，生成指令并发送给服务端
 */
void mainloop();

/**
 * 获取指令的参数
 */
void get_param(char* param);

/**
 * 处理指令
 */
int exec_command(char* param);

/**
 * 判断指令
 */
int is_command(char* pattern);

/**
 * 主动模式
 */
void port(char* param);

/**
 * 被动模式
 */
void pasv();

/**
 * 下载文件
 */
void retr(char* param);

/**
 * 上传文件
 */
void stor(char* param);

/**
 * 获取文件列表
 */
void list();

/**
 * 退出
 */
void quit();
