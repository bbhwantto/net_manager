#pragma once
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <errno.h>
#include <vector>

#include <string>
#include <stdint.h>
#include <iostream>
#include <thread>
#include "requestData.h"
using namespace std;

#pragma warning(disable:4996)
#pragma comment(lib, "Ws2_32.lib")

#define MAX_SESSION_COUNT 5000  // 如修改，需重编译、
#define P_port 9999 
#define MAX_THREADS 20
#define FD_SETSIZE 1024
//#define BUFSIZ 4096



#define ERR_EXIT(m) \
     do \
	 {\
		perror(m);\
		exit(EXIT_FAILURE);\
	 }while(0)

enum Connect_tyep
{
	H_BS = 0,
	H_CS = 1
};

extern Connect_tyep c_type;


struct node
{
	int32_t nError;
	uint16_t wRemotePort;
	string pRemoteIp;
};

extern vector<node> blacklist;

class Ilisten_event
{
public:
	virtual ~Ilisten_event() {};
	// false not call back,so onaccept is ensure succ
	virtual void init_socket(uint16_t wRemotePort) = 0;
	virtual void on_accept(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort) = 0;
	virtual void on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg) = 0;
	virtual void on_read(int32_t nId, const char* pBuffer, uint32_t dwLen) = 0;
	virtual void init_select() = 0;
	virtual uint32_t getmaxfd() = 0;
	virtual uint32_t getlfd() = 0;
	virtual int32_t getmaxi() = 0;
	virtual void handle() = 0;
public:
	FD_SET socketSet;
	FD_SET writeSet;
	FD_SET readSet;
	FD_SET allSet;
};

class Iconnect_event
{
public:
	virtual ~Iconnect_event() {};
	virtual void on_connect(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg) = 0;
	virtual void on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg) = 0;
	virtual void on_read(int32_t nId, const char* pBuffer, uint32_t dwLen) = 0;
	virtual void init_socket(uint16_t wRemotePort, const char* pRemoteIp) = 0;
	virtual int32_t getcfd() = 0;
	virtual int32_t get_conret() = 0;
};

class Ipack_parser
{
public:
	virtual ~Ipack_parser() {};
	virtual int32_t on_parse(const void* pData, uint32_t dwDataSize) = 0;
};

class Itcp_manager
{
public:
	virtual void release() = 0;
	virtual bool init(int32_t nThreadCount) = 0;

	virtual int32_t listen(uint16_t wPort, Ilisten_event* pEvent, Ipack_parser* pParser) = 0;
	virtual int32_t connect(const char* pRemoteIp, uint16_t wPort, Iconnect_event* pEvent, Ipack_parser* pParser) = 0;
	virtual int32_t send(int32_t nId, const void* pData, uint32_t dwDataSize) = 0;
	virtual void close(int32_t nId) = 0;
	virtual void net_init() = 0;
	int32_t ll = 0;
};

class Ilisten_event_ach : public Ilisten_event
{
public:
	void on_accept(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort);
	void on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg);
	void on_read(int32_t nId, const char* pBuffer, uint32_t dwLen);
	void init_socket(uint16_t wRemotePort);
	void init_select();
	uint32_t getmaxfd();
	uint32_t getlfd();
	int32_t getmaxi();
	void handle();

private:
	SOCKET lfd = -3;
	uint32_t maxfd;
	int32_t client[FD_SETSIZE + 1];
	int32_t maxi = 1;
	char buf[BUFSIZ];
	int32_t nread;
	//int32_t nret;
	Ipack_parser* parser;
	Itcp_manager* manager;

};

class Itcp_manager_ach : public Itcp_manager
{
public:
	void release();
	bool init(int32_t nThreadCount);

	int32_t listen(uint16_t wPort, Ilisten_event* pEvent, Ipack_parser* pParser);
	int32_t connect(const char* pRemoteIp, uint16_t wPort, Iconnect_event* pEvent, Ipack_parser* pParser);
	int32_t send(int32_t nId, const void* pData, uint32_t dwDataSize);
	void close(int32_t nId);
	static DWORD WINAPI ser_run(const LPVOID lparam);
	static DWORD WINAPI cli_run(const LPVOID lparam);
	void net_init();


private:
	Ilisten_event* pEvent_lis = nullptr;
	Iconnect_event* pEvent_con = nullptr;
	Ipack_parser* _pParser = nullptr;
	node* q;
	int32_t threadc;
	DWORD dwThreadId[MAX_THREADS];
	HANDLE hThread[MAX_THREADS];

};

class Iconnect_event_ach : public Iconnect_event
{
public:
	void on_connect(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg);
	void on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg);
	void on_read(int32_t nId, const char* pBuffer, uint32_t dwLen);
	void init_socket(uint16_t wRemotePort, const char* pRemoteIp);
	int32_t getcfd();
	int32_t get_conret();

private:

	int32_t cfd;
	int32_t nread;
	char buf[BUFSIZ];
	Ipack_parser* parser;
	Itcp_manager* manager;
	int32_t ret;

};




class Ipack_parser_ach : public Ipack_parser
{
public:
	int32_t on_parse(const void* pData, uint32_t dwDataSize);
private:
	char data[BUFSIZ];
};




