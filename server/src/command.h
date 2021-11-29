/**
 * command.h
 * 指令头文件
 */

#pragma once

#include "global.h"
#include "server.h"
#include "file.h"
#include <sys/stat.h>
#include <netdb.h>
#include <time.h>

/**
 * 接收指令
 */
int get_command();

/**
 * 从指令中获取参数
 */
void get_param(char* param);

/**
 * 执行指令
 */
int exec_command(char* param);

/**
 * 判断指令类型
 */
int is_command(char* pattern);

/**
 * 登录
 */
void user(char* param);
void pass(char* param);

/**
 * 查看系统类型
 */
void syst();

/**
 * 查看数据传输类型
 */
void type(char* param);

/**
 * 主动模式
 */
void port(char* param);

/**
 * 被动模式
 */
void pasv(char* param);

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
 * 重命名
 */
void rnfr(char* param);
void rnto(char* param);

/**
 * 获取当前工作目录
 */
void pwd();

/**
 * 切换工作目录
 */
void cwd(char* param);

/**
 * 创建目录
 */
void mkd(char* param);

/**
 * 删除目录
 */
void rmd(char* param);

/**
 * 退出
 */
void quit();