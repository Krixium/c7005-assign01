#ifndef SRVR_H
#define SRVR_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "globals.h"
#include "utils.h"

void srvr(int argc, char *argv[]);
void serveClient(unsigned int clientSocket, struct sockaddr_in *clientAddress);
void respondGETRequest(struct sockaddr_in *clientAddress, char *buffer);
void sendFileOverTCP(FILE *file, unsigned int sd);
void respondSENDRequest(unsigned int sd, struct sockaddr_in *clientAddress, char *buffer);
void sendACK(unsigned int sd);
void parseSENDPayload(unsigned int sd, FILE *file, int length);

#endif