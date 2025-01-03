#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libs/client.h"
#include "../libs/server.h"
#include "../utils/param/param.h"
#include "../utils/file/file.h"
#include "../utils/database/database.h"
#include "../utils/key_manager/key_manager.h"
#include <libgen.h>
#include <pthread.h>
#include <unistd.h>

void *token_cleaner(void *arg)
{
    int wait_time = *((int *)arg);
    while (1)
    {
        printf("Cleaning tokens\n");
        clean_tokens();
        sleep(wait_time);
    }
}

int main(int agrc, char *argv[])
{
    startserver(8080);
    init_mysql_database();
    pthread_t thread_id;
    int wait_time = 600;
    pthread_create(&thread_id, NULL, token_cleaner, &wait_time);
    char buffer[1024] = {0};
    while (1)
    {
        getmsg(buffer);
        printf("Received: %s\n", buffer);

        char *token_flag = strstr(buffer, "--token");
        char *username_flag = strstr(buffer, "--username");
        unsigned char *token = NULL;
        char *username = NULL;
        if (token_flag != NULL)
        {
            token = token_flag + strlen("--token") + 1;
            char *end = strchr(token, ' ');
            if (end != NULL)
                *end = '\0';
        }

        if (username_flag != NULL)
        {
            username = username_flag + strlen("--username") + 1;
            char *end = strchr(username, ' ');
            if (end != NULL)
                *end = '\0';
        }

        char *command = strtok(buffer, " ");

        char *filename_with_path = NULL;
        char *filename = NULL;
        char *password = NULL;

        printf("Command: %s\n", buffer);
        printf("token: %s\n", token);
        printf("username: %s\n", username);

        Param p = get_param_from_string(command);
        printf("Param: %d\n", p);
        switch (p)
        {
        case UPLOAD:
            filename_with_path = strtok(NULL, " ");
            filename = basename(filename_with_path);
            char *blob = add_file(filename, token);
            if (blob == NULL)
            {
                send_string("Sorry the file is unavailable. Please check the file name or your permissions and try again.", 4200);
                break;
            }
            receive_file(blob, "server_files");
            break;
        case DOWNLOAD:
            filename_with_path = strtok(NULL, " ");
            filename = basename(filename_with_path);
            printf("Filename: %s\n", filename);
            char *server_filename = get_file(filename, token);
            printf("Server filename: %s\n", server_filename);
            if (server_filename == NULL)
            {
                send_string("Sorry the file is unavailable. Please check the file name or your permissions and try again.", 4200);
                break;
            }
            if (check_file_owner(server_filename, token) == 0)
            {
                send_string("Sorry the file is unavailable. Please check the file name or your permissions and try again.", 4200);
                break;
            }
            char *dest_dir = "server_files";
            char full_path[1024];
            sprintf(full_path, "%s/%s", dest_dir, server_filename);
            printf("Full path: %s\n", full_path);
            send_file(full_path, 4200);
            break;
        case LISTFILES:
            send_string(list_user_files(token), 4200);
            break;
        case LOGIN:
            username = strtok(NULL, " ");
            password = strtok(NULL, " ");
            unsigned char *user_token = login_user(username, password);
            if (user_token != NULL)
            {
                send_string(user_token, 4200);
            }
            else
            {
                send_string("Login failed", 4200);
            }
            break;
        case SIGNUP:
            username = strtok(NULL, " ");
            password = strtok(NULL, " ");
            unsigned char *new_user_token = signup_user(username, password);
            if (new_user_token != NULL)
            {
                send_string(new_user_token, 4200);
            }
            else
            {
                send_string("Signup failed", 4200);
            }
            break;
        case CHECK:
            if (check_token(token, username) == 0)
            {
                send_string("Token is not valid", 4200);
            }
            else
            {
                send_string("Token is valid", 4200);
            }
            break;
        default:
            break;
        }
    }
    return 0;
}
