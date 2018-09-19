#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "globals.h"

int createTCPSocket(unsigned int *sd);
void readFromSocket(unsigned int socket, char * buffer, unsigned int len);
unsigned int getFileSize(FILE *file);
void writeFileToTCPSocket(unsigned int sd, FILE *file);
void parseGETResponse(unsigned int sd, FILE *file);

#endif