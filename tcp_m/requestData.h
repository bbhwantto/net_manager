#pragma once
#include <string>
#include <unordered_map>
#include <cstring>
#include <iostream>
#include <fstream>
//#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>

#include "r_w.h"
#include "tcp_interface.h"
using namespace std;

extern const char* file;

enum HTTP_REQUEST
{
    H_ERROR = -1,
    H_NOW,
    H_READ,
    H_GET,
    H_POST,
    H_CONNECT,
    H_REPLY

};


enum HeaderState
{
    h_start = 0,
    h_key,
    h_colon,
    h_spaces_after_colon,
    h_value,
    h_CR,
    h_LF,
    h_end_CR,
    h_end_LF
};


class requestData
{
private:
    std::string path;
    int againTimes;
    std::string content;
    int fd;
    int epfd;
    int method;
    int HTTPversion;
    std::string file_name;
    int now_read_pos;
    int state;
    int h_state;
    bool isfinishl;
    bool keep_alive;
    std::unordered_map<string, string> headers;



private:
    //  int parse_URI();
    //  int parse_Headers();
     // int analysisRequest();

public:
    requestData();
    requestData(int _epollfd, int _fd, std::string _path);
    ~requestData();
    void reset();
    int get_fd();
    void setFd(int _fd);
    void handleRequest();




    int hexit(char c);
    void send_respond(int fd, int number, const char* disp, const char* type, int len);
    void decode_str(char* to, char* from);
    void send_file(int fd, const char* file);
    const char* get_file_type(const char* name);
    void encode_str(char* to, int tosize, const char* from);//1
    void send_dir(int fd, const char* file);//1
};

