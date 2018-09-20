#ifndef CLNT_H
#define CLNT_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "utils.h"

void clnt(int argc, char *argv[]);
void handleGET(char *serverAddress, char *filename);
void handleSEND(char *serverAddress, char *filename);
void sendGETRequest(unsigned int sd, char *filename);
void sendSENDRequest(unsigned int sd, char *filename, unsigned int fileSize);
void parseGETResponse(unsigned int sd, FILE *file);
void parseSENDResponse(unsigned int sd, FILE *file);

#endif