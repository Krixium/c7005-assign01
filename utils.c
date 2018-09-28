/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		    utils.c - A collection of utility functions.
--
--	PROGRAM:		        bftp
--
--	FUNCTIONS:		
--                          void printUsage()
--                          int createAddrFromHostname(struct sockaddr_in *addr, char *hostname, short port)
--                          int createTCPSocket(unsigned int *sd)
--                          int bindListenSocket(unsigned int sd, short port)
--                          int readFromSocket(unsigned int socket, char * buffer, unsigned int len)
--                          int getFileSize(FILE *file)
--                          void writeFileToTCPSocket(unsigned int sd, FILE *file)
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
---------------------------------------------------------------------------------------*/
#include "utils.h"

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                printUsage
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void printUsage()
--
-- NOTES:
--                          Prints the usage of the program when invalid command line arguements are
--                          supplied.
--------------------------------------------------------------------------------------------------*/
void printUsage()
{
    printf("Usage: ./bftp [server_hostname [GET|SEND] file_name]\n");
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                createAddrFromHostname
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int createAddrFromHostname(struct sockaddr_in *addr, char *hostname, short port)
--                              addr:       The addr to fill.
--                              hostname:   The name of the host.
--                              port:       The desired port.
--
-- RETURNS:                 1 if the addr has been filled successfully, 0 otherwise.
--
-- NOTES:                   Fills in the addr struct with the appropriate data for hostname and port and returns 1.
--                          If hostname cannot be resolved addr will be left unchanged and 0 is returned.
--------------------------------------------------------------------------------------------------*/
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
    memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);

    return 1;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                createTCPSocket
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int createTCPSocket(unsigned int *sd)
--                              sd:     Address to store the openned socket.
--
-- RETURNS:                 1 if the socket was openned, 0 otherwise.
--
-- NOTES:
--                          Attempts to open a TCP socket. If a socket is openned 1 is returned,
--                          otherwise 0.
--------------------------------------------------------------------------------------------------*/
int createTCPSocket(unsigned int *sd)
{
    if ((*sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                bindListenSocket
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int bindListenSocket(unsigned int sd, short port)
--                              sd:     Socket to bind.
--                              port:   Port to bind.
--
-- RETURNS:                 1 if socket was bound, otherwise 0.
--
-- NOTES:
--                          Binds sd to port port and returns 1 if bound, otherwise 0.
--------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                readFromSocket
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int readFromSocket(unsigned int socket, char * buffer, unsigned int len)
--                              socket:     The socket to read.
--                              buffer:     The buffer to place data into.
--                              len:        The number of bytes to read.
--
-- RETURNS:                 Number of bytes read.
--
-- NOTES:                   Reads len bytes from socket into buffer. Returns the number of bytes that
--                          were read and buffered.
--------------------------------------------------------------------------------------------------*/
int readFromSocket(unsigned int socket, char * buffer, unsigned int len)
{
    int n, bytesToRead, total;
    bytesToRead = len;

    char * bufferPointer;
    bufferPointer = buffer;

    total = 0;
    while (1)
    {
        n = recv(socket, bufferPointer, bytesToRead, 0);
        bufferPointer += n;
        bytesToRead -= n;
        total += n;

        if (n >= len)
        {
            break;
        }
    }

    return total;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                getFileSize
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int getFileSize(FILE *file)
--                              file:   The file to count.
--
-- RETURNS:                 The size of the file.
--
-- NOTES:
--                          Gets the size of the given file. The file cursor is returned to SEEK_SET
--                          afterwords.
--------------------------------------------------------------------------------------------------*/
int getFileSize(FILE *file)
{
    unsigned int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                writeFileToTCPSocket
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void writeFileToTCPSocket(unsigned int sd, FILE *file)
--                              sd:     The socket to write too.
--                              file:   The file to write.
--
-- NOTES:
--                          Writes all of file to sd.
--------------------------------------------------------------------------------------------------*/
void writeFileToTCPSocket(unsigned int sd, FILE *file)
{
    int n;
    char fileBuffer[FILE_BUFFER_SIZE];

    fseek(file, 0, SEEK_SET);

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