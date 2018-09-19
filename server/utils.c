#include "utils.h"

int createTCPSocket(unsigned int *sd)
{
    if ((*sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
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

void parseGETResponse(unsigned int sd, FILE *file)
{
    int fileSize;
    int n, bytesToRead;
    char headerBuffer[5];
    char dataBuffer[FILE_BUFFER_SIZE];
    char *ptr;

    memset(headerBuffer, 0, 5);
    memset(dataBuffer, 0, FILE_BUFFER_SIZE);

    bytesToRead = 5;
    ptr = headerBuffer;

    while ((n = recv(sd, ptr, bytesToRead, 0)) < bytesToRead)
    {
        ptr += n;
        bytesToRead -= n;
    }

    if (headerBuffer[0] != STX)
    {
        perror("Wrong header");
        return;
    }

    fileSize = *(unsigned int *)(headerBuffer + 1);

    while (fileSize > 0)
    {
        ptr = dataBuffer;
        if (fileSize < FILE_BUFFER_SIZE)
        {
            bytesToRead = fileSize;
        }
        else
        {
            bytesToRead = FILE_BUFFER_SIZE;
        }
        while ((n = recv(sd, ptr, bytesToRead, 0)) < bytesToRead)
        {
            ptr += n;
            bytesToRead -= n;
        }

        fwrite(dataBuffer, sizeof(char), n, file);

        fileSize -= FILE_BUFFER_SIZE;
    }
    
}