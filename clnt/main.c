#include "main.h"


int main(int argc, char *argv[])
{
    char filename[MAX_FILENAME_SIZE];

    if (argc != 4)
    {
        printUsage();
        exit(0);
    }

    strcpy(filename, argv[3]);

    if (!strcmp("GET", argv[2]))
    {
       handleGET(argv[1], filename);
    }
    else if (!strcmp("SEND", argv[2]))
    {
        handleSEND(argv[1], filename);
    }
    else
    {
        printUsage();
        exit(0);
    }

    exit(0);
}

void printUsage()
{
    printf("Usage: clnt server [GET|SEND] filename\n");
}

void handleGET(char *serverAddress, char *filename)
{
    FILE *file;
    unsigned int length;
    unsigned int serverSocket, listenSocket, dataSocket;
    struct sockaddr_in serverAddr, recvAddr;

    if (!createTCPSocket(&serverSocket))
    {
        perror("Could not create socket");
        return;
    }

    if (!createAddrFromHostname(&serverAddr, serverAddress, LISTEN_PORT))
    {
        perror("Could not establish host");
        return;
    }

    printf("Host established\n");

    if (connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Could not connect");
        close(serverSocket);
        return;
    }

    printf("Server connected\n");

    // make request
    sendGETRequest(serverSocket, filename);
    close(serverSocket);

    // accept new connection
    if (!createTCPSocket(&listenSocket))
    {
        perror("Could not create a socket");
        return;
    }

    if (!bindListenSocket(listenSocket, DATA_PORT))
    {
        perror("Cannot bind socket");
        return;
    }

    listen(listenSocket, 1);

    length = sizeof(recvAddr);
    if ((dataSocket = accept(listenSocket, (struct sockaddr *)&recvAddr, &length)) == -1)
    {
        perror("Could not accept connection");
        return;
    }

    close(listenSocket);

    file = fopen(filename, "w+");

    parseGETResponse(dataSocket, file);

    fclose(file);
    close(dataSocket);
}

void sendGETRequest(unsigned int sd, char *filename)
{
    char requestPacket[REQUEST_SIZE];

    memset(requestPacket, 0, REQUEST_SIZE);
    requestPacket[0] = STX;
    requestPacket[1] = 'G';
    strcpy(requestPacket + 2, filename);

    send(sd, requestPacket, REQUEST_SIZE, 0);
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

void handleSEND(char *serverAddress, char *filename)
{

}