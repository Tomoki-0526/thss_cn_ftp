#include "client.h"

void initClient(int argc, char** argv) {
    status = NONE;
    mode = DEFAULT;
    memset(path, 0, sizeof(path));
    memset(file_addr, 0, sizeof(file_addr));
    server_port = htons(21);
    strcpy(server_host, "127.0.0.1");
    file_port = -1;
    file_sock = -1;
    gui = 0;
    memset(server_msg, 0, sizeof(server_msg));
    transfer_size = 0;
    memset(cmd_user, 0, sizeof(cmd_user));
    memset(cmd_pass, 0, sizeof(cmd_pass));

    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-gui") == 0) {
            gui = 1;
        }
        else if (strcmp(argv[i], "-user") == 0) {
            sprintf(cmd_user, "USER %s", argv[++i]);
            int len = strlen(cmd_user);
            cmd_user[len] = '\0';
        }
        else if (strcmp(argv[i], "-pass") == 0) {
            sprintf(cmd_pass, "PASS %s", argv[++i]);
            int len = strlen(cmd_pass);
            cmd_pass[len] = '\0';
        }
        else if (strcmp(argv[i], "-port") == 0) {
            sscanf(argv[++i], "%d", &server_port);
            server_port = htons(server_port);
        }
        else if (strcmp(argv[i], "-host") == 0) {
            sprintf(server_host, "%s", argv[++i]);
        }
    }
}

int recv_msg(int connect_sock, char* msg) {
    char buf[MAX_SIZE];
    memset(buf, 0, sizeof(buf));
    if (read(connect_sock, buf, MAX_SIZE) == -1) {
        printf("Read error.\n");
        return -1;
    }
    int len = strlen(buf);
    for (int i = 0; i < len; ++i) {
        if (buf[i] == '\r') {
            buf[i] = '\0';
        }
    }
    strcpy(msg, buf);
    printf("Receive> %s\n", msg);
    return 0;
}

int send_msg(int connect_sock, char* msg) {
    char buf[MAX_SIZE];
    strcpy(buf, msg);
    strcat(buf, "\r\n");
    if (write(connect_sock, buf, strlen(buf)) == -1) {
        printf("Write error.\n");
        return -1;
    }
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
    /* 初始化客户端 */
    initClient(argc, argv);

    /* 建立连接 */
    // 网络通信地址
    struct sockaddr_in addr;
    // 建立socket
    if ((connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		exit(1);
    }
    // 设置对方的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = server_port;
    if (inet_pton(AF_INET, server_host, &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		exit(1);
    }
    // 发起连接
    if (connect(connect_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        exit(1);
    }
    // 设置状态
    status = CONN;
    recv_msg(connect_sock, server_msg);
    // 循环发出指令
    mainloop();
    // 关闭连接
    close(connect_sock);
    return 0;
}