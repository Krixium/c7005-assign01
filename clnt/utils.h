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

int createAddrFromHostname(struct sockaddr_in *addr, char *hostname, short port);
int createTCPSocket(unsigned int *sd);
int bindListenSocket(unsigned int sd, short port);
int readFromSocket(unsigned int socket, char * buffer, unsigned int len);
int getFileSize(FILE *file);
void writeFileToTCPSocket(unsigned int sd, FILE *file);

#endif