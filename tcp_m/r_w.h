#pragma once


#include "requestData.h"
#include "tcp_interface.h"

#define MAX_BUFF 4096

int readn(int fd, void* buff, size_t n);
int readn(int fd, std::string& inBuffer);
int writen(int fd, void* buff, size_t n);
int writen(int fd, std::string& sbuff);

void send_error(int cfd, int status, const char* title, const char* text);
