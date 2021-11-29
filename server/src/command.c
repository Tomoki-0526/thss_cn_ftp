#include "command.h"

int get_command() {
    recv_msg(connect_sock, client_msg);     // 接收指令
    char param[MAX_SIZE];
    get_param(param);                       // 获取参数
    int res = exec_command(param);          // 执行指令
    return res;
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

void user(char* param) {
    strcpy(username, param);
    send_msg(connect_sock, MSG_331);
    status = USER;
}

void pass(char* param) {
    if (status != USER) {
        send_msg(connect_sock, MSG_503P);
        return;
    }
    strcpy(password, param);
    send_msg(connect_sock, MSG_230);
    status = PASS;
}

void syst() {
    send_msg(connect_sock, MSG_215);
}

void type(char* param) {
    if (param[0] != 'I') {
        send_msg(connect_sock, MSG_501);
        return;
    }
    send_msg(connect_sock, MSG_200T);
}

void port(char* param) {
    int h1, h2, h3, h4, p1, p2;
    sscanf(param, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    sprintf(file_addr, "%d.%d.%d.%d", h1, h2, h3, h4);
    if (file_sock != -1) {
        close(file_sock);
    }
    send_msg(connect_sock, MSG_200P);
    mode = PORT;
    file_port = p1 * 256 + p2;
}

void pasv(char* param) {
    int p1, p2;
    char msg[MAX_SIZE];
    srand(time(NULL));
    file_port = rand() % 45536 + 20000;
    /* 建立新的监听 */
    // 网络通信地址
    struct sockaddr_in addr;
    // 监听socket
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        send_msg(connect_sock, MSG_425);
        return;
    }
    // 设置本机ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(file_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 绑定
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        send_msg(connect_sock, MSG_425);
        return;
    }
    printf("New connection at port %d\n", file_port);
    p1 = file_port / 256;
    p2 = file_port % 256;
    // 获取本机ip
    char ip[MIN_SIZE];
    get_ip(ip);
    strcpy(file_addr, ip);
    for (int i = 0; i < strlen(ip); ++i) {
        if(ip[i] == '.') {
            ip[i] = ',';
        }
    }
    // 监听
    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        send_msg(connect_sock, MSG_425);
        return;
    }
    // 发送消息给客户端
    sprintf(msg, "%s entering passive mode(%s,%d,%d)", MSG_227, ip, p1, p2);
    if (send_msg(connect_sock, msg) == -1) {
        close(listenfd);
        return;
    }
    if (file_sock != -1) {
        close(file_sock);
    }
    file_sock = listenfd;
    mode = PASV;
}

void retr(char* param) {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        printf("need port/pasv\n");
        send_msg(connect_sock, MSG_425);
        return;
    }
    /* 准备文件 */
    char filename[MAX_SIZE];
    strcpy(filename, working_dir);
    char raw_filename[MIN_SIZE];
    get_raw_filename(raw_filename, param);
    strcat(filename, raw_filename);
    int file_size = file_check(filename, "r");
    if (file_size == -1) {
        send_msg(connect_sock, MSG_502R);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    if (mode == PORT) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            send_msg(connect_sock, MSG_426);
            return;
        }
        connfd = file_sock;
        send_msg(connect_sock, MSG_150);
    }
    else if (mode == PASV) {
        send_msg(connect_sock, MSG_150);
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            send_msg(connect_sock, MSG_426);
            return;
        }
    }
    /* 传输文件 */
    send_file(connfd, filename);
    close(connfd);
    if (mode == PASV) {     // 如果处于被动模式，还要关掉监听socket
        close(file_sock);
    }
    transfer_size += file_size;
    mode = DEFAULT;
    file_sock = -1;
    send_msg(connect_sock, MSG_226R);
}

void stor(char* param) {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        printf("need port/pasv\n");
        send_msg(connect_sock, MSG_425);
        return;
    }
    /* 准备文件 */
    char filename[MAX_SIZE];
    strcpy(filename, working_dir);
    char raw_filename[MIN_SIZE];
    get_raw_filename(raw_filename, param);
    strcat(filename, raw_filename);
    if (file_check(filename, "w") == -1) {
        send_msg(connect_sock, MSG_502S);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    if (mode == PORT) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            send_msg(connect_sock, MSG_426);
            return;
        }
        connfd = file_sock;
        send_msg(connect_sock, MSG_150);
    }
    else if (mode == PASV) {
        send_msg(connect_sock, MSG_150);
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            send_msg(connect_sock, MSG_426);
            return;
        }
    }
    /* 传输文件 */
    int file_size = recv_file(connfd, filename);
    close(connfd);
    if (mode == PASV) {     // 如果处于被动模式，还要关掉监听socket
        close(file_sock);
    }
    transfer_size += file_size;
    mode = DEFAULT;
    file_sock = -1;
    send_msg(connect_sock, MSG_226S);
}

void list() {
    /* 检查连接模式 */
    if (mode == DEFAULT) {
        printf("need port/pasv\n");
        send_msg(connect_sock, MSG_425);
        return;
    }
    /* 建立连接 */
    int connfd;     // 用于数据传输
    if (mode == PORT) {
        if (establish_data_conn() == -1) {
            printf("Establish connection failed.\n");
            send_msg(connect_sock, MSG_426);
            return;
        }
        connfd = file_sock;
        send_msg(connect_sock, MSG_150);
    }
    else if (mode == PASV) {
        send_msg(connect_sock, MSG_150);
        if ((connfd = accept(file_sock, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            send_msg(connect_sock, MSG_426);
            return;
        }
    }
    /* 数据传输 */
    send_ls(connfd);
    close(connfd);
    if (mode == PASV) {     // 如果处于被动模式，还要关掉监听socket
        close(file_sock);
    }
    mode = DEFAULT;
    file_sock = -1;
    send_msg(connect_sock, MSG_226L);
}

void rnfr(char* param) {
    char path[MAX_SIZE];
    memset(path, 0, sizeof(path));
    strcpy(path, working_dir);
    char raw_name[MIN_SIZE];
    get_raw_filename(raw_name, param);
    strcat(path, raw_name);

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        send_msg(connect_sock, MSG_550RF);
        return;
    }
    fclose(file);

    send_msg(connect_sock, MSG_350);
    strcpy(old_name, path);
    status = RNFR;
}

void rnto(char* param) {
    if (status != RNFR) {
        send_msg(connect_sock, MSG_503R);
        return;
    }

    char new_name[MAX_SIZE];
    strcpy(new_name, working_dir);
    char raw_name[MIN_SIZE];
    get_raw_filename(raw_name, param);
    strcat(new_name, raw_name);

    char cmdmv[MAX_SIZE];
    sprintf(cmdmv, "sudo mv %s %s", old_name, new_name);
    int res = system(cmdmv);
    if (res == 0) {
        send_msg(connect_sock, MSG_250RN);
    }
    else {
        send_msg(connect_sock, MSG_550RT);
    }

    status = PASS;
    memset(old_name, 0, sizeof(old_name));
}

void pwd() {
    char msg[MAX_SIZE];
    sprintf(msg, "%s \"%s\"", MSG_257, working_dir);
    send_msg(connect_sock, msg);
}

void cwd(char* param) {
    char new_path[MAX_SIZE];
    strcpy(new_path, working_dir);
    char raw_path[MIN_SIZE];
    get_raw_filename(raw_path, param);
    strcat(new_path, raw_path);

    if (chdir(new_path) >= 0) {
        getcwd(working_dir, MAX_SIZE);
        char msg[MAX_SIZE];
        sprintf(msg, "%s Change woking directory ok, \"%s\" is current directory.", MSG_257, working_dir);
        strcat(working_dir, "/");
        send_msg(connect_sock, msg);
    }
    else {
        send_msg(connect_sock, MSG_550C);
    }
}

void mkd(char* param) {
    struct stat st = {0};
    char new_dir[MAX_SIZE];
    strcpy(new_dir, working_dir);
    strcat(new_dir, param);

    if (stat(new_dir, &st) == -1) {
        mkdir(new_dir, 0777);
        send_msg(connect_sock, MSG_250M);
    }
    else {
        send_msg(connect_sock, MSG_550M);
    }
}

void rmd(char* param) {
    char rm_dir[MAX_SIZE];
    strcpy(rm_dir, working_dir);
    strcat(rm_dir, param);

    char cmdrm[MAX_SIZE];
    sprintf(cmdrm, "rm -r %s", rm_dir);
    int res = system(cmdrm);
    if (res == 0) {
        send_msg(connect_sock, MSG_250RM);
    }
    else {
        send_msg(connect_sock, MSG_550R);
    }
}

void quit() {
    char msg[MAX_SIZE];
    sprintf(msg, "%s %d %s", MSG_221, transfer_size, "bytes.");
    send_msg(connect_sock, msg);
}

int exec_command(char* param) {
    if (is_command("USER")) {
        user(param);
    }
    else if (is_command("PASS")) {
        pass(param);
    }
    else if (is_command("SYST")) {
        syst();
    }
    else if (is_command("TYPE")) {
        type(param);
    }
    else if (is_command("PORT")) {
        port(param);
    }
    else if (is_command("PASV")) {
        pasv(param);
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
    else if (is_command("RNFR")) {
        rnfr(param);
    }
    else if (is_command("RNTO")) {
        rnto(param);
    }
    else if (is_command("PWD")) {
        pwd();
    }
    else if (is_command("CWD")) {
        cwd(param);
    }
    else if (is_command("MKD")) {
        mkd(param);
    }
    else if (is_command("RMD")) {
        rmd(param);
    }
    else if (is_command("QUIT") || is_command("ABOR")) {
        quit();
        return 0;
    }
    else {
        send_msg(connect_sock, MSG_502I);
    }
    return 1;
}