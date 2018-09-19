#include "main.h"

int main(int argc, char *argv[])
{
    pid_t pid;
    unsigned int clientLength;
    unsigned int acceptSocket, newSocket;
    struct sockaddr_in server, client;

    // Create accept socket
    if (!createTCPSocket(&acceptSocket)) 
    {
        perror("Could not create listen socket.");
        exit(1);
    }

    // Create default struct
    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(LISTEN_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind
    if (bind(acceptSocket, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Cannot bind socket.");
        exit(1);
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
        }
        else if (pid == 0)
        {
            // Child does work
            serveClient(newSocket, &client);
        }
        else
        {
            // Parent continues on
            continue;
        }
    }

    return 0;
}

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
        GETRequest(clientAddress, buffer);
    }
    else if (buffer[1] == 'S')
    {
        SENDRequest(clientAddress, buffer);
    }
    else
    {
        perror("Invalid request");
    }

    close(clientSocket);
}

void GETRequest(struct sockaddr_in *clientAddress, char *buffer)
{
    printf("Get request received\n");

    char filename[256];
    strcpy(filename, buffer + 2);
    printf("Filename: %s\n", filename);

    // Open file
    FILE *file = fopen(filename, "r");

    if (file == 0)
    {
        perror("Could not open file");
        return;
    }

    printf("File openned\n");

    // Create connection on port 7006
    unsigned int dataSocket;

    if (!createTCPSocket(&dataSocket))
    {
        perror("Could not create new socket");
        fclose(file);
        return;
    }

    // Connect
    struct sockaddr_in client;
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

    // create header
    char header[5];
    unsigned int fileLength = getFileSize(file);
    header[0] = STX;
    memcpy(header + 1, &fileLength, sizeof(unsigned int));

    // send header
    send(dataSocket, header, 5, 0);
    printf("Header sent\n");

    writeFileToTCPSocket(dataSocket, file);
    printf("File sent\n");

    fclose(file);
    printf("File closed\n");

    close(dataSocket);
    printf("Socket closed\n");
}

void SENDRequest(struct sockaddr_in *clientAddress, char *buffer)
{
    printf("Send request received\n");

    char filename[256];
    strcpy(filename, buffer + 2);
    printf("Filename: %s\n", filename);

    // Create file with filename

    // Create connection on port 7006

    // Read and write data to file
}