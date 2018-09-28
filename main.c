/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		    main.c - The main entry point of the program
--
--	PROGRAM:		        bftp
--
--	FUNCTIONS:		        int main(int argc, char *argv[])
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
--                          This is the main entry point of the application. Depending on
--                          the supplied command line arguments the program will start as
--                          a server, start as a client, or close due to incorrect arguements.
---------------------------------------------------------------------------------------*/

#include "srvr.h"
#include "clnt.h"

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                main
--
-- DATE:                    September 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               int main(int argc, char *argv[])
--                              argc:   The amount of command line arguements supplied.
--                              argv:   The command line arguements supplied.
--
-- RETURNS:                 Exit code.
--
-- NOTES:
--                          Parses the command line arguements and starts a server or client if the
--                          arguements are valid. Otherwise, the program prints the usage and exits.
--------------------------------------------------------------------------------------------------*/
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