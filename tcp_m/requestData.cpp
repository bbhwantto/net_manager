#include "requestData.h"
#include <WinSock2.h>

requestData::requestData() :
	now_read_pos(0), state(1), h_state(h_start),
	keep_alive(false), againTimes(0)
{
	cout << "requestData constructed ! " << endl;
}


requestData::~requestData()
{
	cout << "~requestData()" << endl;

	//close(fd);
}


void requestData::handleRequest()
{
	do {
		int i;
		char line[4096] = { 0 };
		char path[2048];
		int tem = 0;
		char tchar[1024];
		int len;
		if (state == H_READ) {
			len = readn(fd, line, 4096);

			cout << "len: " << len << endl;
			cout << line << endl;

			if (len < 0) {
				send_error(fd, 404, "Not Found", "NO such file or direntry");

				state = H_ERROR;
				break;
			}
			else if (len == 0) {
				state == H_ERROR;
				break;
			}

			for (i = tem; i < len; ++i) {
				if (line[i] == '\r' && line[i + 1] == '\n') {
					tchar[i - tem] = line[i];
					tchar[i - tem + 1] = line[i + 1];
					break;
				}
				else
					tchar[i - tem] = line[i];
			}
			if (i >= len) {
				state = H_ERROR;
				break;
			}
			tem = i + 2;
			if (strncmp(tchar, "GET", 3) == 0)
				state = H_GET;
			else if (strncmp(tchar, "POST", 4) == 0)
				state = H_POST;
			else {
				state = H_ERROR;
				break;
			}

		}



		if (state == H_GET) {
			int j;
			int judge = 0;
			for (j = 4;; ++j) {
				if (line[j] == '?') judge = 1;
				if (line[j] == ' ') break;
				path[j - 4] = line[j];
			}
			path[j - 4] = '\0';

			if (judge == 1)
				state = H_CONNECT;
			else
				state = H_REPLY;
		}


		if (state == H_POST) {
			cout << "it is post !!!!!" << endl;
			while (tem < len) {
				if (line[tem] == '\r')
					break;

				for (i = tem; i < len; i++) {

					if (line[i] == '\r' && line[i + 1] == '\n') {
						tchar[i - tem] = line[i];
						tchar[i - tem + 1] = line[i + 1];
						break;
					}
					else {
						tchar[i - tem] = line[i];
					}
				}

				if (i >= len) {
					state = H_ERROR;
					break;
				}

				tchar[i - tem + 2] = '\0';
				tem = i + 2;

				char first[128];
				char second[128];

				for (i = 0;; i++) {
					if (tchar[i] == '\0') {
						state = H_ERROR;
						break;
					}
					if (tchar[i] == ':') {
						break;
					}
					first[i] = tchar[i];
				}

				first[i] = '\0';
				i += 2;
				int j = i;

				for (;; ++i) {
					if (tchar[i] == '\0') {
						state = H_ERROR;
						break;
					}
					if (tchar[i] == '\r' && tchar[i + 1] == '\n') {
						break;
					}
					second[i - j] = tchar[i];
				}

				second[i - j] = '\0';

				if (strcmp(second, "keep-alive") == 0) {
					keep_alive = true;
				}

			}

			state = H_REPLY;
		}


		if (state == H_CONNECT) {

		}


		if (state == H_REPLY) {





			struct stat sbuf;






			int ret = stat(file, &sbuf);
			cout << "fd: " << fd << endl;
			if (ret == -1) {
				send_error(fd, 404, "Not Found", "NO such file or direntry");
				state = H_ERROR;
				break;
			}


			this->send_respond(fd, 200, "OK", get_file_type(file), sbuf.st_size);

			//发送文件

			this->send_file(fd, file);
		}








	} while (false);


	if (state == H_ERROR || keep_alive == false) {
		delete this;
		return;
	}








}

//解码  码->中文
void requestData::decode_str(char* to, char* from)
{
	for (; *from != '\0'; ++to, ++from) {
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
			*to = this->hexit(from[1]) * 16 + this->hexit(from[2]);
			from += 2;
		}
		else {
			*to = *from;
		}
	}
	*to = '\0';
}


int requestData::hexit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

void requestData::send_file(int fd, const char* file) {
	FILE* pFile = NULL;
	pFile = fopen(file, "rb");



	if (pFile == NULL) {
		send_error(fd, 404, "Not Found", "NO such file or direntry");
		return;
	}

	fseek(pFile, 0, SEEK_END);
	int FileSize = ftell(pFile);

	fseek(pFile, 0, SEEK_SET);

	char* pFileBuffer = (char*)malloc(FileSize);


	int n = 0;



	while ((n = fread(pFileBuffer, 1, FileSize, pFile)) > 0) {
		int ret;
		ret = send(fd, pFileBuffer, n, 0);
		if (ret == -1) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			}
			else
				break;
		}
	}

}


void requestData::send_respond(int fd, int number, const char* disp, const char* type, int len) {
	char buf[1024] = { 0 };
	sprintf(buf, "HTTP/1.1 %d %s\r\n", number, disp);
	writen(fd, buf, strlen(buf));

	sprintf(buf, "Content-Type:%s\r\n", type);
	sprintf(buf + strlen(buf), "Content-Length:%d\r\n", len);

	writen(fd, buf, strlen(buf));
	writen(fd, (void*)"\r\n", 2);
}



const char* requestData::get_file_type(const char* name)
{
	const char* dot;

	// 自右向左查找‘.’字符, 如不存在返回NULL
	dot = strrchr(name, '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8";
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}


void requestData::setFd(int _fd)
{
	fd = _fd;
}