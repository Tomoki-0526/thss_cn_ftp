#include "server.h"

void init_port_dir(int argc, char** argv) {
    listen_port = 21;
    strcpy(working_dir, "/tmp");

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-port") == 0) {
            listen_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-root") == 0) {
            strcpy(working_dir, argv[++i]);
        }
        else {
            printf("wrong arguments![%s]", argv[i]);
            return;
        }
    }

    printf("Server on port[%d] at root[%s]\n", listen_port, working_dir);
    strcat(working_dir, "/");
}

void init_param() {
    status = CONN;
    mode = DEFAULT;
    file_port = -1;
    file_sock = -1;
    transfer_size = 0;
    memset(client_msg, 0, sizeof(client_msg));
    memset(file_addr, 0, sizeof(file_addr));
    memset(old_name, 0, sizeof(old_name));
}

int send_msg(int connect_sock, char* msg) {
    char buf[MAX_SIZE];
    strcpy(buf, msg);
    strcat(buf, "\r\n");
    if (write(connect_sock, buf, strlen(buf)) == -1) {
        printf("Write error.\n");
        return -1;
    }
    printf("Write: /%s\n", msg);
    return 0;
}

int recv_msg(int connect_sock, char* msg) {
    char buf[MAX_SIZE];
    memset(buf, 0, sizeof(buf));
    if (read(connect_sock, buf, MAX_SIZE) == -1) {
        printf("Read error.\n");
        return -1;
    }
    int len = strlen(buf);
    while(len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
        len--;
    }
    buf[len] = '\0';
    strcpy(msg, buf);
    printf("Read: /%s\n", msg);
    return 0;
}

void get_ip(char* ip) {
    struct ifaddrs *head = NULL, *iter = NULL;
    if (getifaddrs(&head) == -1) {
        return;
    }
    for (iter = head; iter != NULL; iter = iter->ifa_next) {
        if (iter->ifa_addr == NULL) {
            continue;
        }
        if (iter->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        char mask[INET_ADDRSTRLEN];
        void* ptr = &((struct sockaddr_in*) iter->ifa_netmask)->sin_addr;
        inet_ntop(AF_INET, ptr, mask, INET_ADDRSTRLEN);
        if (strcmp(mask, "255.0.0.0") == 0) {
            continue;
        }        
        void* tmp = &((struct sockaddr_in *) iter->ifa_addr)->sin_addr;
        char *rlt = (char*)malloc(20);
        memset(rlt, 0, 20);
        inet_ntop(AF_INET, tmp, rlt, INET_ADDRSTRLEN);
        strcpy(ip, rlt);
    }
}

int establish_data_conn() {
    struct sockaddr_in addr;
    if ((file_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(file_port);
    if (inet_pton(AF_INET, file_addr, &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    if (connect(file_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

int main(int argc, char** argv) {
    /* 多线程使用 */
    pid_t pid;

    /* 初始化端口和工作路径 */
    init_port_dir(argc, argv);

    /* 建立监听socket */
    // 网络通信地址
    struct sockaddr_in addr;
    // 创建socket
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        exit(1);
    }
    // 设置本机ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // 监听0.0.0.0
    // 将本机的ip和port与socket绑定
    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        exit(1);
    }
    // 启动监听
    if (listen(listen_sock, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        exit(1);
    }
    // 持续监听
    while (1) {
        // 等待client的连接
        if ((connect_sock = accept(listen_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            break;
        }

        pid = fork();
        if (pid < 0) {
            printf("Error forking.\n");
        }
        else if (pid == 0) {
            close(listen_sock);                 // 关掉子进程的监听socket
            init_param();                       // 初始化参数
            send_msg(connect_sock, MSG_220);
            while (1) {                         // 循环处理客户端发来的指令
                int res = get_command();
                if (res == 0) {
                    break;
                }
            }
            close(connect_sock);                // 关掉子进程的连接socket
            return 0;
        }
        close(connect_sock);                    // 关掉父进程的连接socket
    }
    // 关掉监听socket
    close(listen_sock);
    return 0;
}