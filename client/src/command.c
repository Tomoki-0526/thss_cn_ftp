#include "command.h"

void mainloop() {
    while (1) {
        if (gui == 1) {
            char param[MID_SIZE];
            memset(param, 0, sizeof(param));

            strcpy(client_msg, cmd_user);
            get_param(param);
            exec_command(param);
            
            strcpy(client_msg, cmd_pass);
            get_param(param);
            exec_command(param);

            gui = 0;
        }
        else {
            // 获取指令
            printf("Command> ");
            fgets(client_msg, 4096, stdin);
            if (client_msg[0] == '\n') {
                continue;
            }
            int len = strlen(client_msg);
            while (len > 0 && (client_msg[len - 1] == '\n' || client_msg[len - 1] == '\r')) {
                len--;
            }
            client_msg[len] = '\0';
            // 获取参数
            char param[MID_SIZE];
            memset(param, 0, sizeof(param));
            get_param(param);
            // 处理指令
            int res = exec_command(param);
            if (res == 0) {
                break;
            }
        }
    }
}

void get_param(char* param) {
    for (int i = 0; i < strlen(client_msg); ++i) {
        if (client_msg[i] == ' ') {
            strcpy(param, client_msg + i + 1);
            break;
        }
    }
    for (int i = 0; i < strlen(param); ++i) {
        if (param[i] == '\r' || param[i] == '\n' || param[i] == '\0') {
            param[i] = '\0';
            break;
        }
    }
}

int is_command(char* pattern) {
    if (strlen(client_msg) < strlen(pattern)) {
        return 0;
    }
    return strstr(client_msg, pattern) == client_msg;
}

void port(char* param) {
    /* 构建ip和端口 */
    char ip[MIN_SIZE];
    get_ip(ip);
    strcpy(file_addr, ip);
    for (int i = 0; i < strlen(ip); ++i) {
        if (ip[i] == '.') {
            ip[i] = ',';
        }
    }
    srand(time(NULL));
    file_port = rand() % 45536 + 20000;
    int p1 = file_port / 256;
    int p2 = file_port % 256;
    sprintf(param, "%s,%d,%d", ip, p1, p2);

    /* 建立监听socket */
    int listenfd;
    struct sockaddr_in addr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(file_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return;
    }
    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return;
    }

    /* 发送接收消息 */
    char msg[MIN_SIZE];
    memset(msg, 0, sizeof(msg));
    strncpy(msg, client_msg, 4);
    strcat(msg, " ");
    strcat(msg, param);
    send_msg(connect_sock, msg);
    recv_msg(connect_sock, server_msg);
    if (server_msg[0] != '2') {
        return;
    }
    if (file_sock != -1) {
        close(file_sock);
    }
    file_sock = listenfd;
    mode = PORT;
}

void pasv() {
    /* 发送PASV指令 */
    send_msg(connect_sock, client_msg);
    recv_msg(connect_sock, server_msg);
    if (server_msg[0] != '2') {
        return;
    }

    /* 获取服务端ip和端口 */
    int h1, h2, h3, h4, p1, p2;
    char* ch = strchr(server_msg, '(');
    char tmp[MIN_SIZE];
    memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, ch);
    sscanf(tmp, "(%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
    sprintf(file_addr, "%d.%d.%d.%d", h1, h2, h3, h4);
    if (file_sock != -1) {
        close(file_sock);
    }
    file_port = p1 * 256 + p2;
    mode = PASV;
}

void retr(char* param) {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        send_msg(connect_sock, client_msg);
        recv_msg(connect_sock, server_msg);
        return;
    }
    /* 准备文件 */
    char filename[MAX_SIZE];
    strcpy(filename, path);
    strcat(filename, param);
    if (file_check(filename, "w") == -1) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    send_msg(connect_sock, client_msg);
    recv_msg(connect_sock, server_msg);
    if (server_msg[0] != '1') {
        return;
    }
    if (mode == PORT) {
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            return;
        }
    }
    else if (mode == PASV) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            return;
        }
        connfd = file_sock;
    }
    /* 传输文件 */
    int file_size = recv_file(connfd, filename);
    close(connfd);
    if (mode == PORT) {     // 如果处于主动模式，还要关掉监听socket
        close(file_sock);
    }
    transfer_size += file_size;
    mode = DEFAULT;
    file_sock = -1;
    recv_msg(connect_sock, server_msg);
}

void stor(char* param) {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        send_msg(connect_sock, client_msg);
        recv_msg(connect_sock, server_msg);
        return;
    }
    /* 准备文件 */
    char filename[MAX_SIZE];
    strcpy(filename, path);
    strcat(filename, param);
    int file_size = file_check(filename, "r");
    if (file_size == -1) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    send_msg(connect_sock, client_msg);
    recv_msg(connect_sock, server_msg);
    if (server_msg[0] != '1') {
        return;
    }
    if (mode == PORT) {
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            return;
        }
    }
    else if (mode == PASV) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            return;
        }
        connfd = file_sock;
    }
    /* 传输文件 */
    send_file(connfd, filename);
    close(connfd);
    if (mode == PORT) {     // 如果处于主动模式，还要关掉监听socket
        close(file_sock);
    }
    transfer_size += file_size;
    mode = DEFAULT;
    file_sock = -1;
    recv_msg(connect_sock, server_msg);
}

void list() {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        send_msg(connect_sock, client_msg);
        recv_msg(connect_sock, server_msg);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    send_msg(connect_sock, client_msg);
    recv_msg(connect_sock, server_msg);
    if (server_msg[0] != '1') {
        return;
    }
    if (mode == PORT) {
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            return;
        }
    }
    else if (mode == PASV) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            return;
        }
        connfd = file_sock;
    }
    /* 传输文件列表 */
    recv_ls(connfd);
    close(connfd);
    if (mode == PORT) {     // 如果处于主动模式，还要关掉监听socket
        close(file_sock);
    }
    mode = DEFAULT;
    file_sock = -1;
    recv_msg(connect_sock, server_msg);
}

void quit() {
    send_msg(connect_sock, client_msg);
    recv_msg(connect_sock, server_msg);
    close(connect_sock);
}

int exec_command(char* param) {
    if (is_command("PORT")) {
        port(param);
    }
    else if (is_command("PASV")) {
        pasv();
    }
    else if (is_command("RETR")) {
        retr(param);
    }
    else if (is_command("STOR")) {
        stor(param);
    }
    else if (is_command("LIST")) {
        list();
    }
    else if (is_command("QUIT") || is_command("ABOR")) {
        quit();
        return 0;
    }
    else {
        send_msg(connect_sock, client_msg);
        recv_msg(connect_sock, server_msg);
    }

    return 1;
}