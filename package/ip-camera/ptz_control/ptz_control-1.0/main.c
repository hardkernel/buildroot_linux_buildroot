#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "ptzimpl.h"


static const char *help_str =
        " ===============  Help  ===============\n"
        " name:  " DAEMON_NAME "                \n"
#ifdef  DEBUG
        " Build  mode:  debug\n"
#else
        " Build  mode:  release\n"
#endif
        " Build  date:  " __DATE__ "\n"
        " Build  time:  " __TIME__ "\n\n"
        "Options:                      description:\n\n"
        "       --ttys         [value] Set serial port (default = 1)\n"
        "       --naddr        [value] Set the address of ptz device (default = 1)\n"
        "       --pelco_type   [value] Set the protocol type of pelco (default = D)\n"
        "       --baudrate     [value] Set the baudrate of pelco (default = 2400，if pelco-P, default 4800)\n"
        "       --command      [value] Set PTZ command (default = PTZ_STOP)\n"
        "       --data         [value] Set pan or lilt speed (default = 0x0A)\n"
        "  -r,  --recv_test            Display received infomation\n"
        "  -h,  --help                 Display this help\n\n";

// indexes for long_opt function
enum
{
    HELP    = 'h',
    RECV_TEST = 'r',

    //ptz info
    TTYS,
    NADDR,
    PELCO_TYPE,
    BAUDRATE,
    COMMAND,
    DATA
};

static const char *short_opts = "hr";

static const struct option long_opts[] =
{
    { "recv_test",    no_argument,       NULL, RECV_TEST     },
    { "help",         no_argument,       NULL, HELP          },

    //ptz info
    { "ttys",         required_argument, NULL, TTYS          },
    { "naddr",        required_argument, NULL, NADDR         },
    { "pelco_type",   required_argument, NULL, PELCO_TYPE    },
    { "baudrate",     required_argument, NULL, BAUDRATE      },
    { "command",      required_argument, NULL, COMMAND       },
    { "data",         required_argument, NULL, DATA          },

    { NULL,           no_argument,       NULL,  0            }
};

PTZPRAMAS ptzpramas;
int nrecv = 0;

void processing_cmd(int argc, char *argv[])
{
    int opt;
    int naddress;

    InitPtzPramas(&ptzpramas);

    while ( (opt = getopt_long(argc, argv, short_opts, long_opts, NULL) ) != -1 )
    {
        switch ( opt )
        {
            case RECV_TEST:
                nrecv = 1;
                break;

            case HELP:
                puts(help_str);
                exit(0);
                break;

            //ptz info
            case TTYS:
                ptzpramas.nttys = atoi(optarg);
                break;

            case NADDR:
                naddress = atoi(optarg);
                if ( naddress < 0x00 || naddress > 0xff )
                {
                    PRINTF("naddr error, exceed capacity (0x00~0xff)\n\n");
                    exit(-1);
                }
                ptzpramas.naddr = naddress;
                break;

            case PELCO_TYPE:
                GetPelcoType(optarg, &ptzpramas);
                break;

            case BAUDRATE:
                ptzpramas.nbaudrate = atoi(optarg);
                break;

            case COMMAND:
                ptzpramas.ncommand = atoi(optarg);
                break;

            case DATA:
                ptzpramas.ndata = atoi(optarg);
                break;

            default:
                PRINTF("for more detail see help\n\n");
                exit(-1);
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    processing_cmd(argc, argv);

    if ( Init485(&ptzpramas) != 0 )
    {
        PRINTF("ERROR. Init 485\n");
        return -1;
    }

    if (nrecv == 0)
    {
        DeviceControl(&ptzpramas);
    }
    else
    {
        ReadFrom485();
    }

    Deinit485();

    return 0;
}