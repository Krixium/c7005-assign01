#ifndef MAIN_H
#define MAIN_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "globals.h"
#include "utils.h"

void serveClient(unsigned int clientSocket, struct sockaddr_in *clientAddress);
void GETRequest(struct sockaddr_in *clientAddress, char *buffer);
void SENDRequest(struct sockaddr_in *clientAddress, char *buffer);

#endif