/**
 * client.h
 * 客户端头文件
 */

#pragma once

#include "global.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

int gui;                        // 图形界面
char cmd_user[MIN_SIZE];        // 命令行参数的USER指令
char cmd_pass[MIN_SIZE];        // 命令行参数的PASS指令

int server_port;                // 服务端端口
char server_host[MIN_SIZE];     // 服务端ip
char server_msg[MAX_SIZE];      // 服务端发来的消息

int connect_sock;               // 连接到服务端的socket
Status status;                  // 连接状态
Mode mode;                      // 连接模式（主动&被动）

char path[MAX_SIZE];            // 客户端工作路径
char client_msg[MAX_SIZE];      // 客户端产生的消息

int file_port;                  // 数据传输端口（主动模式下为客户端端口，被动模式下为服务端发来的端口）
int file_sock;                  // 数据传输socket（主动模式下负责监听，被动模式下负责传输）
char file_addr[MIN_SIZE];       // 数据传输地址（主动模式下为客户端本机ip，被动模式下为服务端ip）

int transfer_size;              // 总传输大小

/**
 * 初始化客户端
 */
void initClient(int argc, char** argv);

/**
 * 接收消息
 */
int recv_msg(int connect_sock, char* msg);

/**
 * 发送消息
 */
int send_msg(int connect_sock, char* msg);

/**
 * 获取本机ip
 */
void get_ip(char* ip);

/**
 * 与指定的ip和端口进行连接
 */
int establish_data_conn();