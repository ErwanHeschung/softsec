#include "../utils/param/param.h"
#include "../utils/file/file.h"
#include "../libs/client.h"
#include "../libs/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int agrc, char *argv[]){
    startserver(4200);
    char buffer[1024] = {0};
    Param p = get_param(agrc, argv);
    switch (p)
    {
    case UPLOAD:
        snprintf(buffer, 1024, "-up %s", argv[2]);
        sndmsg(buffer, 8080);
        printf("Uploading file: %s\n", argv[2]);
        if (send_file(argv[2], 8080) < 0)
        {
            printf("Error uploading file: %s\n", argv[2]);
        }
        break;
    case DOWNLOAD:
        //down
        snprintf(buffer, 1024, "-down %s", argv[2]);
        sndmsg(buffer, 8080);
        printf("Downloading file: %s\n", argv[2]);
        receive_file(argv[2], "client_files");
        break;
    case LIST:
        printf("List\n");
        break;
    default:
        break;
    }

    stopserver();
    return 0;
}