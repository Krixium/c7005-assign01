#ifndef MAIN_H
#define MAIN_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "utils.h"

void printUsage();

void handleGET(char *serverAddress, char *filename);
void handleSEND(char *serverAddress, char *filename);

void sendGETRequest(unsigned int sd, char *filename);

#endif