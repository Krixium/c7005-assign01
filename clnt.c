/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		    clnt.c - Collection of client related functions.
--
--	PROGRAM:		        bftp.exe
--
--	FUNCTIONS:		        
--                          clnt(int argc, char *argv[])
--                          void handleGET(char *serverAddress, char *filename)
--                          void sendGETRequest(unsigned int sd, char *filename)
--                          void parseGETResponse(unsigned int sd, FILE *file)
--                          void handleSEND(char *serverAddress, char *filename)
--                          void sendSENDRequest(unsigned int sd, char *filename, unsigned int fileSize)
--                          void parseSENDResponse(unsigned int sd, FILE *file)
--
--	DATE:			        September 27, 2018
--
--	REVISIONS:		        N/A
--
--	DESIGNERS:		        Benny Wang
--
--	PROGRAMMERS:		    Benny Wang
--
--	NOTES:
--                          This file contains functions that are used only for the client side of the protocol.
---------------------------------------------------------------------------------------*/
#include "clnt.h"


/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                clnt
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void clnt(int argc, char *argv[])
--                              argc:   The number of command line arguements passed by the user.
--                              argv:   The command line arguements passed by the user.
--
-- NOTES:
--                          This is the main entry point for the client.
--------------------------------------------------------------------------------------------------*/
void clnt(int argc, char *argv[])
{
    char filename[MAX_FILENAME_SIZE];

    if (argc != 4)
    {
        printUsage();
        return;
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
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                handleGET
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void handleGet(char *serverAddress, char *filename)
--                              serverAddress:  The address of the server.
--                              filename:       The name of the file to get.
--
-- NOTES:
--                          Performs the GET request following the protocol. This funciton will open
--                          and close files.
--------------------------------------------------------------------------------------------------*/
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
        close(listenSocket);
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

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                sendGETRequest
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void sendGETRequest(unsigned int sd, char *filename)
--                              sd:         The socket descriptor to write the GET request too.
--                              filename:   The name of the file to request.
--
-- NOTES:
--                          Creates a GET request header for filename and sends it on the socket
--                          described by sd.
--------------------------------------------------------------------------------------------------*/
void sendGETRequest(unsigned int sd, char *filename)
{
    char requestPacket[REQUEST_SIZE];

    memset(requestPacket, 0, REQUEST_SIZE);
    requestPacket[0] = STX;
    requestPacket[1] = 'G';
    strcpy(requestPacket + 2, filename);

    send(sd, requestPacket, REQUEST_SIZE, 0);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                parseGETResponse
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void parseGETResponse(unsigned int sd, FILE *file)
--                              sd:         The socket descriptor to read from.
--                              file:       The file to write too.
--
-- NOTES:
--                          Parses the GET response from ther server. Reads all the data from sd
--                          and writes the data to file.
--------------------------------------------------------------------------------------------------*/
void parseGETResponse(unsigned int sd, FILE *file)
{
    int fileSize, n;
    char headerBuffer[5];
    char dataBuffer[FILE_BUFFER_SIZE];

    memset(headerBuffer, 0, 5);
    memset(dataBuffer, 0, FILE_BUFFER_SIZE);

    readFromSocket(sd, headerBuffer, 5);

    if (headerBuffer[0] != STX)
    {
        perror("Wrong header");
        return;
    }

    fileSize = *(unsigned int *)(headerBuffer + 1);

    while (fileSize > 0)
    {
        if (fileSize < FILE_BUFFER_SIZE)
        {
            n = readFromSocket(sd, dataBuffer, fileSize);
        }
        else
        {
            n = readFromSocket(sd, dataBuffer, FILE_BUFFER_SIZE);
        }
        fwrite(dataBuffer, sizeof(char), n, file);
        fileSize -= n;
    }
    
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                handleSEND
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void handleSEND(char *serverAddress, char *filename)
--                              serverAddress:  The address of the server.
--                              fielname:       The name of the file.
--
-- NOTES:
--                          Performs a SEND request following the protocol. This function will open
--                          and close sockets and files.
--------------------------------------------------------------------------------------------------*/
void handleSEND(char *serverAddress, char *filename)
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

    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Could not open file");
        close(serverSocket);
        return;
    }

    // make request
    sendSENDRequest(serverSocket, filename, getFileSize(file));
    close(serverSocket);

    // accept new connection
    if (!createTCPSocket(&listenSocket))
    {
        perror("Could not create a socket");
        fclose(file);
        return;
    }

    if (!bindListenSocket(listenSocket, DATA_PORT))
    {
        perror("Cannot bind socket");
        close(listenSocket);
        fclose(file);
        return;
    }

    printf("Created new socket\n");

    listen(listenSocket, 1);

    printf("Listening...\n");

    length = sizeof(recvAddr);
    if ((dataSocket = accept(listenSocket, (struct sockaddr *)&recvAddr, &length)) == -1)
    {
        perror("Could not accept connection");
        fclose(file);
        close(listenSocket);
        return;
    }
    close(listenSocket);

    printf("Connection accepted\n");

    parseSENDResponse(dataSocket, file);

    fclose(file);
    close(dataSocket);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                sendSENDRequest
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void sendSENDRequest(unsigned int sd, char *filename, unsigned int fileSize)
--                              sd:         The socket descriptor to write requests onto.
--                              filename:   The name of the file to send.
--                              fileSize:   The size of the file to send.
--
-- NOTES:
--                          Sends a SEND request for file filename of size fileSize to socket sd.
--------------------------------------------------------------------------------------------------*/
void sendSENDRequest(unsigned int sd, char *filename, unsigned int fileSize)
{
    char requestPacket[REQUEST_SIZE];

    memset(requestPacket, 0, REQUEST_SIZE);
    requestPacket[0] = STX;
    requestPacket[1] = 'S';
    strcpy(requestPacket + 2, filename);
    memcpy(requestPacket + REQUEST_SIZE - sizeof(unsigned int), (char *)&fileSize, sizeof(unsigned int));

    send(sd, requestPacket, REQUEST_SIZE, 0);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                parseSENDResponse
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void parseSENDResponse(unsigned int sd, FILE *file)
--                              sd:     The socket descriptor to write too.
--                              file:   The file to send.
--
-- NOTES:
--                          Writes file to the socket sd.
--------------------------------------------------------------------------------------------------*/
void parseSENDResponse(unsigned int sd, FILE *file)
{
    char ackBuffer[2];
    memset(ackBuffer, 0, 2);

    readFromSocket(sd, ackBuffer, 2);

    printf("ACK received\n");

    if (ackBuffer[0] != STX || ackBuffer[1] != ACK)
    {
        perror("Incorrect header");
        return;
    }
    printf("ACK is valid\n");

    // send header
    send(sd, &STX, sizeof(char), 0);

    // send file
    writeFileToTCPSocket(sd, file);
}