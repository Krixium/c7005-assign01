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
    unsigned int length;
    unsigned int serverSocket, listenSocket, dataSocket;
    struct sockaddr_in server, recvAddr;

    if (!createTCPSocket(&serverSocket))
    {
        perror("Could not create socket");
        return;
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(LISTEN_PORT);

    struct hostent *hp;
    if ((hp = gethostbyname(serverAddress)) == NULL)
    {
        perror("Unknown server");
        return;
    }
    memcpy(hp->h_addr, &server.sin_addr, hp->h_length);

    printf("Host established\n");

    if (connect(serverSocket, (struct sockaddr *)&server, sizeof(server)) == -1)
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

    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(DATA_PORT);
    recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenSocket, (struct sockaddr *)&recvAddr, sizeof(recvAddr)) == -1)
    {
        perror("Cannot bind socket");
        return;
    }

    listen(listenSocket, 1);

    length = sizeof(server);
    if ((dataSocket = accept(listenSocket, (struct sockaddr *)&server, &length)) == -1)
    {
        perror("Could not accept connection");
        return;
    }

    close(listenSocket);

    FILE *file;
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

void handleSEND(char *serverAddress, char *filename)
{

}