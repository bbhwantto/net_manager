#include "r_w.h"
#include "r_w.h"


int readn(int fd, void* buff, size_t n)
{
    size_t nleft = n;
    int nread = 0;
    int readSum = 0;
    char* ptr = (char*)buff;
    while (nleft > 0)
    {
        if ((nread = recv(fd, ptr, nleft, 0)) < 0)
        {

            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
            break;
        cout << "nread" << nread << endl;
        readSum += nread;
        nleft -= nread;
        ptr += nread;
        if (nleft > 0)break;
    }
    return readSum;
}


int readn(int fd, std::string& inBuffer)
{
    int nread = 0;
    int readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = recv(fd, buff, MAX_BUFF, 0)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {

                return readSum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
            break;
        //printf("before inBuffer.size() = %d\n", inBuffer.size());
        //printf("nread = %d\n", nread);
        readSum += nread;
        //buff += nread;
        inBuffer += std::string(buff, buff + nread);
        //printf("after inBuffer.size() = %d\n", inBuffer.size());
    }
    return readSum;
}


int writen(int fd, void* buff, size_t n)
{
    size_t nleft = n;
    int nwritten = 0;
    int writeSum = 0;
    char* ptr = (char*)buff;

    while (nleft > 0)
    {
        if ((nwritten = send(fd, ptr, nleft, 0)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    return writeSum;
                }
                else
                    return -1;
            }
        }


        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}


int writen(int fd, std::string& sbuff)
{
    size_t nleft = sbuff.size();
    int nwritten = 0;
    int writeSum = 0;
    const char* ptr = sbuff.c_str();
    while (nleft > 0)
    {
        if ((nwritten = send(fd, ptr, nleft, 0)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == sbuff.size())
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}


void send_error(int cfd, int status, const char* title, const char* text)
{

    char buf[4096] = { 0 };
    sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
    sprintf(buf + strlen(buf), "Content-Type:%s\r\n", "text/html");
    sprintf(buf + strlen(buf), "Content-Length:%d\r\n", -1);
    sprintf(buf + strlen(buf), "Connection: close\r\n");
    send(cfd, buf, strlen(buf), 0);
    send(cfd, "\r\n", 2, 0);

    memset(buf, 0, sizeof(buf));

    sprintf(buf, "<html><head><title>%d %s</title></head>\n", status, title);
    sprintf(buf + strlen(buf), "<body bgcolor=\"#cc99cc\"><h2 align=\"center\">%d %s</h4>\n", status, title);
    sprintf(buf + strlen(buf), "%s\n", text);
    sprintf(buf + strlen(buf), "<hr>\n</body>\n</html>\n");
    send(cfd, buf, strlen(buf), 0);

    return;
}
