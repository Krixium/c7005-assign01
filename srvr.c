/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		    srvr.c - Colleciton of server related functions.
--
--	PROGRAM:		        bftp
--
--	FUNCTIONS:		        
--                          void srvr(int argc, char *argv[])
--                          void serveClient(unsigned int clientSocket, struct sockaddr_in *clientAddress)
--                          void respondGETRequest(struct sockaddr_in *clientAddress, char *buffer)
--                          void sendFileOverTCP(FILE *file, unsigned int sd)
--                          void respondSENDRequest(unsigned int sd, struct sockaddr_in *clientAddress, char *buffer)
--                          void sendACK(unsigned int sd)
--                          void parseSENDPayload(unsigned int sd, FILE *file, int length);
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
--                          This file contains functions that are used only for the server side of the protocol.
---------------------------------------------------------------------------------------*/
#include "srvr.h"

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                srvr
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void srvr(int argc, char *argv[])
--                              argc:   The number of command line arguements.
--                              argv:   The command line arguements.
--
-- NOTES:
--                          The main entry point for srvr. A socket is created to listen for requests
--                          and whenever a new request is received the function will fork the server.
--                          The parent process will continue to listen for new requests while the child
--                          process will handle the newly received request.
--------------------------------------------------------------------------------------------------*/
void srvr(int argc, char *argv[])
{
    pid_t pid;
    unsigned int clientLength;
    unsigned int acceptSocket, newSocket;
    struct sockaddr_in client;

    // create listen socket
    if (!createTCPSocket(&acceptSocket)) 
    {
        perror("Could not create listen socket.");
        return;
    }
    
    // bind
    if (!bindListenSocket(acceptSocket, LISTEN_PORT))
    {
        perror("Cannot bind socket.");
        close(acceptSocket);
        return;
    }

    // listen
    listen(acceptSocket, 5);

    while (1)
    {
        printf("Listening...\n");
        clientLength = sizeof(client);

        if ((newSocket = accept(acceptSocket, (struct sockaddr *)&client, &clientLength)) == -1)
        {
            perror("Cannot accept new client.");
            continue;
        }

        pid = fork();

        if (pid == -1)
        {
            perror("Fork error");
            close(newSocket);
        }
        else if (pid == 0)
        {
            serveClient(newSocket, &client);
            close(newSocket);
        }
        else
        {
            continue;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                serveClient
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void serveClient(unsigned int clientSocket, struct sockaddr_in *clientAddress)
--                              clientSocket:   The socket descriptor that is connected to the client.
--                              clientAddress:  The address of the client.
--
-- NOTES:
--                          Reads the request from the client and then parses it. Depending on the
--                          request the program will either handle a GET request, SEND request, or
--                          ignore the request if it is invalid.
--------------------------------------------------------------------------------------------------*/
void serveClient(unsigned int clientSocket, struct sockaddr_in *clientAddress)
{
    printf("Client received, address: %s\n", inet_ntoa(clientAddress->sin_addr));

    char buffer[REQUEST_SIZE];
    readFromSocket(clientSocket, buffer, REQUEST_SIZE);

    if (buffer[0] != STX)
    {
        perror("Wrong packet header");
        return;
    }

    if (buffer[1] == 'G')
    {
        respondGETRequest(clientAddress, buffer);
    }
    else if (buffer[1] == 'S')
    {
        respondSENDRequest(clientAddress, buffer);
    }
    else
    {
        perror("Invalid request");
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                respondGETRequest
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void respondGETRequest(struct sockaddr_in *clientAddress, char *buffer)
--                              clientAddress:  The address of the client.
--                              buffer:         The buffer of the data that was received.
--
-- NOTES:
--                          Responds to a GET request. If the request is valid a file is openned and
--                          is sent along the data port. Otherwise, the request is ignored.
--------------------------------------------------------------------------------------------------*/
void respondGETRequest(struct sockaddr_in *clientAddress, char *buffer)
{
    unsigned int dataSocket;
    struct sockaddr_in client;
    FILE *file;
    char filename[256];

    strcpy(filename, buffer + 2);

    printf("GET request received, filename: %s\n", filename);

    // Open file
    file = fopen(filename, "r");

    if (file == 0)
    {
        perror("Could not open file");
        return;
    }

    printf("File openned\n");

    // Create connection on port 7006

    if (!createTCPSocket(&dataSocket))
    {
        perror("Could not create new socket");
        fclose(file);
        return;
    }

    // Connect
    memset(&client, 0, sizeof(struct sockaddr_in));
    client.sin_family = AF_INET;
    client.sin_port = htons(DATA_PORT);
    memcpy(&client.sin_addr, &clientAddress->sin_addr, 8);

    if (connect(dataSocket, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("Could not connect");
        close(dataSocket);
        fclose(file);
        return;
    }

    printf("Connected to client\n");

    sendFileOverTCP(file, dataSocket);

    fclose(file);
    printf("File closed\n");

    close(dataSocket);
    printf("Socket closed\n");
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                sendFileOverTCP
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void sendFileOverTCP(FILE *file, unsigned int sd)
--                              file:   The file to send.
--                              sd:     The TCP socket to send on.
--
-- NOTES:
--                          Writes all of the contents of file onto sd with the appropriate header
--                          preceding it.
--------------------------------------------------------------------------------------------------*/
void sendFileOverTCP(FILE *file, unsigned int sd)
{
    // create header
    char header[5];
    unsigned int fileLength = getFileSize(file);
    header[0] = STX;
    memcpy(header + 1, &fileLength, sizeof(unsigned int));

    // send header
    send(sd, header, 5, 0);
    printf("Header sent\n");

    writeFileToTCPSocket(sd, file);
    printf("File sent\n");
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                respondSENDRequest
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void respondSENDRequest(struct sockaddr_in *clientAddress, char *buffer)
--                              clientAddress:      The address of the client.
--                              buffer:             The buffer of the data that was received.
--
-- NOTES:
--                          Responds to a send request. If the request is valid, creates a new file
--                          and writes all data received from the data port into it. Otherwise, the
--                          request is ignored.
--------------------------------------------------------------------------------------------------*/
void respondSENDRequest(struct sockaddr_in *clientAddress, char *buffer)
{
    printf("Send request received\n");

    int length;
    unsigned int dataSocket;
    struct sockaddr_in client;
    char filename[256];
    FILE *file;

    strcpy(filename, buffer + 2);

    printf("SEND request received, filename: %s\n", filename);

    memcpy(&length, buffer + REQUEST_SIZE - sizeof(unsigned int), sizeof(unsigned int));

    if (!createTCPSocket(&dataSocket))
    {
        perror("Could not create socket");
        return;
    }

    // Create connection on port 7006
    memset(&client, 0, sizeof(struct sockaddr_in));
    client.sin_family = AF_INET;
    client.sin_port = htons(DATA_PORT);
    memcpy(&client.sin_addr, &clientAddress->sin_addr, 8);

    if (connect(dataSocket, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("Could not connect");
        close(dataSocket);
        return;
    }
    printf("Connected on data port\n");

    printf("Creating new file\n");
    // Create file with filename
    file = fopen(filename, "w+");

    printf("Sending ACK\n");
    sendACK(dataSocket);

    // Read and write data to file
    parseSENDPayload(dataSocket, file, length);

    fclose(file);
    close(dataSocket);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                sendACK
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void sendACK(unsigned int sd)
--                              sd:     The socket to send the ACK on.
--
-- NOTES:
--                          Writes an ACK packet to sd.
--------------------------------------------------------------------------------------------------*/
void sendACK(unsigned int sd)
{
    char buffer[2];
    buffer[0] = STX;
    buffer[1] = ACK;

    send(sd, buffer, 2, 0);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                parseSENDPayload
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void parseSENDPayload(unsigned int sd, FILE *file, int length)
--                              sd:     The socket to read on.
--                              file:   The file to write data into.
--                              length: The length of the data to be received.
--
-- NOTES:
--                          If the header is valid, all the data in sd is read and written into file
--                          until length amount of data has been read and written.
--------------------------------------------------------------------------------------------------*/
void parseSENDPayload(unsigned int sd, FILE *file, int length)
{
    int n;
    char headerBuffer;
    char dataBuffer[FILE_BUFFER_SIZE];

    printf("Reading header\n");

    readFromSocket(sd, &headerBuffer, 1);

    if (headerBuffer != STX)
    {
        perror("Invalid header");
        return;
    }

    printf("Header validated\n");

    while (length > 0)
    {
        memset(dataBuffer, 0, FILE_BUFFER_SIZE);

        if (length < FILE_BUFFER_SIZE)
        {
            n = readFromSocket(sd, dataBuffer, length);
        }
        else
        {
            n = readFromSocket(sd, dataBuffer, FILE_BUFFER_SIZE);
        }

        fwrite(dataBuffer, sizeof(char), n, file);
        length -= n;
    }
}