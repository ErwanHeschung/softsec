#include "../../libs/client.h"
#include "../../libs/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int send_file(char *filename, int port)
{
    //send the whole file
    FILE *file = fopen(filename, "rb");
    printf("Sending file %s\n", filename);
    if (file == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    char buffer[1024] = {0};

    while(!feof(file)){
        fread(buffer, 1024, 1, file);
        sndmsg(buffer, port);
        memset(buffer, 0, sizeof(buffer));
    }

    snprintf(buffer, 1024, "EOF");
    sndmsg(buffer, port);

    return 0;
}

int receive_file(char *filename, char *dest_dir)
{
    char full_path[1024];
    sprintf(full_path, "%s/%s", dest_dir, filename);
    FILE *file = fopen(full_path, "wb");
    printf("Receiving file %s\n", full_path);
    if (file == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    char buffer[1024] = {0};
    while(1){
        getmsg(buffer);
        if (strncmp(buffer, "EOF", 3) == 0)
        {
            break;
        }
        fwrite(buffer, strlen(buffer), 1, file);
    }
    fclose(file);
    return 0;
}

    