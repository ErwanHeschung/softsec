#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libs/client.h"
#include "../libs/server.h"
#include "../utils/param/param.h"
#include "../utils/file/file.h"
#include <libgen.h>

int main(int agrc, char *argv[]){
    startserver(8080);
    char buffer[1024] = {0};
    while(1){
        getmsg(buffer);

        char *command = strtok(buffer, " ");
        char *filename_with_path = strtok(NULL, " ");

        char *filename = basename(filename_with_path);

        Param p = get_param_from_string(command);
        switch (p)
        {
            case UPLOAD:
                receive_file(filename, "server_files");
                break;
            case DOWNLOAD:
                char *dest_dir = "server_files";    
                char full_path[1024];
                sprintf(full_path, "%s/%s", dest_dir, filename);
                send_file(full_path, 4200);
                break;
            case LIST:
            default:
                break;
        }
    }
    return 0;
}