#include "srvr.h"
#include "clnt.h"

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        srvr(argc, argv);
    }
    else if (argc == 4)
    {
        clnt(argc, argv);
    }
    else
    {
        printUsage();
    }
}