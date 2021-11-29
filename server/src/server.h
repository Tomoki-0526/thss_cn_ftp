/**
 * server.h
 * 服务器头文件
 */

#pragma once

#include "global.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

int listen_port;                // 监听端口
int listen_sock;                // 监听socket
int connect_sock;               // 连接端口（与客户端进行消息传输）
Status status;                  // 客户端连接状态
Mode mode;                      // 客户端连接模式（主动&被动）

char username[MIN_SIZE];        // 用户名
char password[MIN_SIZE];        // 密码
char working_dir[MAX_SIZE];     // 工作目录
char client_msg[MAX_SIZE];      // 客户端发来的指令

int transfer_size;              // 总传输大小
char file_addr[MIN_SIZE];       // 数据传输ip（主动模式下为客户端的ip，被动模式下为服务端本机ip）
int file_port;                  // 数据传输端口（主动模式下为客户端的端口，被动模式下为服务端随机生成的端口）
int file_sock;                  // 数据传输socket（主动模式下负责数据传输，被动模式下负责监听）

char old_name[MAX_SIZE];        // 旧文件名

/**
 * 初始化默认端口和工作路径
 */
void init_port_dir(int argc, char* argv[]);

/**
 * 初始化一些参数
 */
void init_param();

/**
 * 发送消息
 */
int send_msg(int connect_sock, char* msg);

/**
 * 接收消息
 */
int recv_msg(int connect_sock, char* msg);

/**
 * 获取本机ip
 */
void get_ip(char* ip);

/**
 * 与指定的ip和端口进行连接
 */
int establish_data_conn();