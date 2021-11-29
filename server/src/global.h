/**
 * global.h
 * 宏、常量等头文件
 */

#pragma once

#define MAX_SIZE    8192
#define MID_SIZE    1024
#define MIN_SIZE    128

typedef enum Status {
    CONN,
    USER,
    PASS,
    RNFR,
} Status;

typedef enum Mode {
    DEFAULT,
    PORT,
    PASV
} Mode;

#define MSG_150     "150 Opening BINARY mode data connection."
#define MSG_200T    "200 Type set to I."
#define MSG_200P    "200 Port mode established."
#define MSG_215     "215 UNIX Type: L8"
#define MSG_220     "220 Anonymous FTP server ready."
#define MSG_221     "221 Goodbye, this connection totally transferred"
#define MSG_226R    "226 Download complete."
#define MSG_226S    "226 Upload complete."
#define MSG_226L    "226 List all the files in this directory."
#define MSG_227     "227"
#define MSG_230     "230 Guest login ok, access restrictions apply."
#define MSG_250RN   "250 File or directory renamed ok."
#define MSG_250M    "250 Directory created ok."
#define MSG_250RM   "250 Directory removed ok."
#define MSG_257     "257"
#define MSG_331     "331 Guest login ok, please enter your password."
#define MSG_350     "350 File exists, ready for destination name."
#define MSG_425     "425 Cannot establish connection."
#define MSG_426     "426 Stop connection, transfer failed."
#define MSG_501     "501 Invalid argument."
#define MSG_502R    "502 File not found."
#define MSG_502S    "502 File not found."
#define MSG_502I    "502 Invalid command."
#define MSG_503P    "503 Need username."
#define MSG_503R    "503 Need RNFR"
#define MSG_550RF   "550 No such file or directory."
#define MSG_550RT   "550 Cannot rename file or directory."
#define MSG_550C    "550 Cannot change woking directory."
#define MSG_550M    "550 Directory already exists."
#define MSG_550R    "550 Cannot remove directory."