#pragma once
#include "tcp_interface.h"
#include <WS2tcpip.h>

//已完成
void Ilisten_event_ach::init_socket(uint16_t wRemotePort)
{
	//cout << "Ilisten_event_ach::init_socket" << endl;
	int32_t retVal;


	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == SOCKET_ERROR)
	{

		//ERR_EXIT("socket");
	}
	maxfd = lfd;

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(P_port);
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	retVal = bind(lfd, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR_IN));
	if (retVal == SOCKET_ERROR)
	{
		ERR_EXIT("bind");
	}

	retVal = listen(lfd, SOMAXCONN);
	if (retVal == SOCKET_ERROR)
	{
		ERR_EXIT("listen");
	}

}

//已完成
uint32_t Ilisten_event_ach::getmaxfd()
{
	int p;
	return maxfd;
}

//已完成
uint32_t Ilisten_event_ach::getlfd()
{
	return lfd;
}

//已完成
int32_t Ilisten_event_ach::getmaxi()
{
	return maxi;
}

//已完成
void Ilisten_event_ach::init_select()
{
	//cout << "Ilisten_event_ach::init_select()" << endl;
	FD_ZERO(&allSet);
	FD_SET(lfd, &allSet);

	maxfd = lfd;

	for (int32_t i = 0; i < FD_SETSIZE; i++)
	{
		client[i] = -1;
	}

}

//已完成
void Ilisten_event_ach::on_accept(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort)
{
	sockaddr_in addr;
	int32_t len = sizeof(addr);
	int32_t connfd;
	char str[INET_ADDRSTRLEN];
	//cout << "on_accept" << endl;

	connfd = accept(nId, (sockaddr*)&addr, &len);
	if (connfd == INVALID_SOCKET || maxi > FD_SETSIZE)
	{
		cout << "connfd" << endl;
		on_close(nId, pRemoteIp, wRemotePort, connfd, "accept error");
	}
	else if (inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, str, sizeof(str)) == pRemoteIp)
	{
		cout << "connfd" << endl;
		printf("received from %s at PORT %d\n",
			inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, str, sizeof(str)),
			ntohs(addr.sin_port));

		printf("该IP被拉入黑名单，不能连接");
	}
	else
	{


		if (maxi > FD_SETSIZE)
		{
			on_close(nId, pRemoteIp, wRemotePort, connfd, "accept error");
		}

		client[maxi] = connfd;


		FD_SET(connfd, &allSet);

		if (connfd > maxfd)
			maxfd = connfd;



		maxi++;

	}

}

//已完成
int32_t Iconnect_event_ach::getcfd()
{
	return cfd;
}

//已完成
void Iconnect_event_ach::on_connect(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg)
{

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(wRemotePort);//端口号
	addrSrv.sin_addr.S_un.S_addr = inet_addr(pRemoteIp);//IP地址

	ret = connect(nId, (sockaddr*)&addrSrv, sizeof(addrSrv));
	if (ret == nError)
	{
		on_close(nId, pRemoteIp, wRemotePort, nError, pErrorMsg);
	}

}

//已完成
int32_t Itcp_manager_ach::send(int32_t nId, const void* pData, uint32_t dwDataSize)
{
	//cout << "int32_t Itcp_manager_ach::send\n";
	int32_t biglen, ret, count, len, tui;
	count = dwDataSize;
	char* data = (char*)malloc(dwDataSize + 4);

	biglen = htonl(dwDataSize);
	memcpy(data, &biglen, 4);
	memcpy(data + 4, (char*)pData, dwDataSize);
	count += 4;
	tui = count;
	while (count > 0)
	{

		len = ::send(nId, data, count, 0);
		if (len == -1)
		{
			close(nId);
			return -1;
		}
		else if (len == 0)
		{
			continue;
		}

		data += len;
		count -= len;
	}

	free(data - tui);

	return dwDataSize;
}

//已完成
void Itcp_manager_ach::close(int32_t nId)
{
	//cout<<"void Itcp_manager_ach::close(int32_t nId)"<<endl;
	closesocket(nId);
}

//已完成
int32_t Itcp_manager_ach::listen(uint16_t wPort, Ilisten_event* pEvent, Ipack_parser* pParser)
{

	pEvent_lis = pEvent;
	pEvent_lis->init_socket(P_port);
	pEvent_lis->init_select();
	//cout << "1111111" << endl;


	while (1)
	{

		//FD_ZERO(&pEvent_lis->readSet);
		//FD_ZERO(&pEvent_lis->writeSet);
		//读写都监听
		pEvent_lis->readSet = pEvent_lis->allSet;
		pEvent_lis->writeSet = pEvent_lis->allSet;

		timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 100;



		int ret = select(0, &pEvent_lis->readSet, NULL, NULL, &tv);

		printf("ret: %d\n", ret);


		if (ret == -1)
		{
			cout << WSAGetLastError() << endl;
			ERR_EXIT("select error");
		}



		if (FD_ISSET(pEvent_lis->getlfd(), &pEvent_lis->readSet))
		{
			pEvent->on_accept(pEvent->getlfd(), "9.9.9.9", wPort);
		}

		pEvent_lis->handle();

	}
}


void Ilisten_event_ach::handle()
{
	int32_t i;

	for (i = 0; i <= maxi; i++)
	{

		if (FD_ISSET(client[i], &readSet))
		{
			if (c_type == H_CS)
			{
				manager = new(Itcp_manager_ach);
				on_read(client[i], buf, sizeof(buf));

				manager->close(client[i]);
				FD_CLR(client[i], &allSet);
				client[i] = -1;
				delete(manager);
			}
			else
			{
				requestData* req = new requestData();
				req->setFd(client[i]);
				req->handleRequest();
			}

		}


	}

}


//已完成
void Ilisten_event_ach::on_read(int32_t nId, const char* pBuffer, uint32_t dwLen)
{
	cout << "Ilisten_event_ach::on_read\n";
	int32_t len, count;
	count = dwLen;
	len = 0;
	nread = 1;
	//memcpy(buf, pBuffer, dwLen);
	parser = new (Ipack_parser_ach);
	while (nread > 0)
	{
		nread = recv(nId, buf + len, dwLen, 0);


		if (nread > 0)
		{

			len = parser->on_parse(buf, nread);

			nread = len;
		}
		else
		{
			manager->close(nId);
			break;
		}



	}

	free(parser);
	cout << endl << endl;
}

//已完成
int32_t Ipack_parser_ach::on_parse(const void* pData, uint32_t dwDataSize)
{

	//cout << "Ipack_parser_ach::on_parse\n";
	memcpy(data, (char*)pData, dwDataSize);
	int32_t klen, len, ret;
	klen = 0;
	if (dwDataSize == BUFSIZ)
	{
		while (klen < dwDataSize)
		{
			int32_t* temp = (int*)(data + klen);
			len = ntohl(*temp);
			if (klen + len > BUFSIZ)
			{
				ret = klen + len - BUFSIZ;
				memcpy(data, data + klen, BUFSIZ - klen + 1);

				return ret;
			}
			char* tem = (char*)malloc(len + 1);
			memcpy(tem, data + klen + 4, len);
			cout << "服务端接收到的数据：" << endl;
			printf("%s\n", tem);
			free(tem);
			klen = klen + 4 + len;
		}

	}
	else
	{

		while (klen < dwDataSize)
		{
			int32_t* temp = (int*)(data + klen);
			len = ntohl(*temp);

			char* tem = (char*)malloc(len + 1);
			memcpy(tem, data + klen + 4, len);
			cout << "服务端接收到的数据：" << endl;
			printf("%s\n", tem);
			free(tem);
			klen = klen + 4 + len;
		}


	}
	return 0;

}

//已完成
int32_t Itcp_manager_ach::connect(const char* pRemoteIp, uint16_t wPort, Iconnect_event* pEvent, Ipack_parser* pParser)
{
	pEvent_con = pEvent;
	pEvent_con->init_socket(wPort, pRemoteIp);
	pEvent_con->on_connect(pEvent_con->getcfd(), pRemoteIp, wPort, SOCKET_ERROR, "connect fail");


	char* buffk = new char(100);
	char buffs[] = "下面咱们开始聊天了";
	memcpy(buffk, buffs, sizeof(buffs));


	send(pEvent_con->getcfd(), buffk, sizeof(buffs));

	return pEvent_con->get_conret();
}

int32_t Iconnect_event_ach::get_conret()
{
	return ret;
}

//已完成
void Iconnect_event_ach::init_socket(uint16_t wRemotePort, const char* pRemoteIp)
{
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd == SOCKET_ERROR)
	{
		ERR_EXIT("SOCKET");
	}
}

//已完成
void Iconnect_event_ach::on_read(int32_t nId, const char* pBuffer, uint32_t dwLen)
{
	int32_t len, count;
	count = dwLen;
	len = 0;
	memcpy(buf, pBuffer, dwLen);
	while (count > 0)
	{
		nread = recv(nId, buf + len, dwLen, 0);
		if (nread > 0)
		{
			len = parser->on_parse(buf, nread);
			count -= nread;
		}
		else
		{
			manager->close(nId);
			break;
		}

	}
}

//已完成
void Ilisten_event_ach::on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg)
{
	node p;
	p.nError = nError;
	p.pRemoteIp = pRemoteIp;
	p.wRemotePort = wRemotePort;
	blacklist.push_back(p);
	ERR_EXIT(pErrorMsg);
	manager->close(nId);
}

//已完成
void Iconnect_event_ach::on_close(int32_t nId, const char* pRemoteIp, uint16_t wRemotePort, int32_t nError, const char* pErrorMsg)
{
	node p;
	p.nError = nError;
	p.pRemoteIp = pRemoteIp;
	p.wRemotePort = wRemotePort;
	blacklist.push_back(p);
	ERR_EXIT(pErrorMsg);
	manager->close(nId);
}


bool Itcp_manager_ach::init(int32_t nThreadCount)
{
	int32_t i;
	node* q = new(node);
	threadc = nThreadCount + 1;
	net_init();

	hThread[0] = CreateThread(NULL, 0, &ser_run, NULL, 0, &dwThreadId[0]);


	if (c_type == H_CS)
	{
		for (i = 1; i < threadc; i++)
		{

			q->pRemoteIp = "127.0.0.1";
			q->wRemotePort = 9999;

			q->nError = 0;

			hThread[i] = CreateThread(NULL, 0, &cli_run, q, 0, &dwThreadId[i]);

		}
	}

	return true;
}

DWORD WINAPI Itcp_manager_ach::ser_run(const LPVOID lparam)
{
	//cout << "Itcp_manager_ach::ser_run\n";
	Itcp_manager_ach* it = new(Itcp_manager_ach);
	it->pEvent_lis = new(Ilisten_event_ach);
	it->pEvent_con = new(Iconnect_event_ach);
	it->_pParser = new(Ipack_parser_ach);
	it->listen(P_port, it->pEvent_lis, it->_pParser);
	free(it->_pParser);
	free(it->pEvent_lis);
	free(it->pEvent_con);
	free(it);
	return 0;
}

DWORD WINAPI Itcp_manager_ach::cli_run(const LPVOID lparam)
{
	Itcp_manager_ach* it = new(Itcp_manager_ach);
	node* tmp = (node*)lparam;


	it->pEvent_lis = new(Ilisten_event_ach);
	it->pEvent_con = new(Iconnect_event_ach);
	it->_pParser = new(Ipack_parser_ach);

	it->connect(tmp->pRemoteIp.c_str(), tmp->wRemotePort, it->pEvent_con, it->_pParser);

	delete(it->_pParser);
	delete(it->pEvent_lis);
	delete(it->pEvent_con);
	delete(it);
	return 0;
}


void Itcp_manager_ach::net_init()
{
	//启动windows网络环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
}

void Itcp_manager_ach::release()
{
	free(q);
	for (int32_t i = 0; i < threadc; i++)
	{
		CloseHandle(hThread[i]);
	}
	cout << "void Itcp_manager_ach::release()\n";
}