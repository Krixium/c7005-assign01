#include "utils.h"

int createAddrFromHostname(struct sockaddr_in *addr, char *hostname, short port)
{
    struct hostent *hp;
    if ((hp = gethostbyname(hostname)) == NULL)
    {
        return 0;
    }

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(LISTEN_PORT);
    memcpy(hp->h_addr, &addr->sin_addr, hp->h_length);

    return 1;
}

int createTCPSocket(unsigned int *sd)
{
    if ((*sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        return 0;
    }

    return 1;
}

int bindListenSocket(unsigned int sd, short port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        return 0;
    }

    return 1;
}

void readFromSocket(unsigned int socket, char * buffer, unsigned int len)
{
    unsigned int n, bytesToRead;
    bytesToRead = len;

    char * bufferPointer;
    bufferPointer = buffer;

    while ((n = recv(socket, bufferPointer, bytesToRead, 0)) < len)
    {
        bufferPointer += n;
        bytesToRead -= n;
    }
}

unsigned int getFileSize(FILE *file)
{
    unsigned int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void writeFileToTCPSocket(unsigned int sd, FILE *file)
{
    int n;
    char fileBuffer[FILE_BUFFER_SIZE];

    while ((n = fread(fileBuffer, 1, FILE_BUFFER_SIZE, file)) != 0)
    {
        if (n < FILE_BUFFER_SIZE)
        {
            send(sd, fileBuffer, n, 0);
        }
        else
        {
            send(sd, fileBuffer, FILE_BUFFER_SIZE, 0);
        }
    }
}