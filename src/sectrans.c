#include "../utils/param/param.h"
#include "../utils/file/file.h"
#include "../libs/client.h"
#include "../libs/server.h"
#include "../utils/key_manager/key_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_token_still_valid(unsigned char *token, char *user)
{
    char buffer[1024] = {0};
    snprintf(buffer, 1024, "-check --token %s --username %s", token, user);
    sndmsg(buffer, 8080);
    getmsg(buffer);
    return strcmp(buffer, "Token is valid") == 0;
}

int main(int agrc, char *argv[])
{
    startserver(4200);
    char buffer[1024] = {0};
    Param p = get_param(agrc, argv);
    unsigned char *key = get_or_generate_keys();
    char *username = get_db_username();
    unsigned char *token = get_db_user_token(username);
    switch (p)
    {
    case UPLOAD:
        if (check_token_still_valid(token, username) == 0)
        {
            printf("Token is not valid\n");
            return 0;
        }
        snprintf(buffer, 1024, "-up %s --token %s", argv[2], token);
        sndmsg(buffer, 8080);
        printf("Encrypting file: %s\n", argv[2]);
        encrypt_file(argv[2], key);
        char *just_filename = strrchr(argv[2], '/');
        printf("Generating signature for file: %s\n", argv[2]);
        generate_signature("encrypted_file.bin", just_filename);
        printf("Uploading file: %s\n", argv[2]);
        if (send_file("encrypted_file.bin", 8080) < 0)
        {
            printf("Error uploading file: %s\n", argv[2]);
        }
        delete_file_client("encrypted_file.bin");
        printf("File uploaded\n");
        break;
    case DOWNLOAD:
        if (check_token_still_valid(token, username) == 0)
        {
            printf("Token is not valid\n");
            return 0;
        }
        snprintf(buffer, 1024, "-down %s --token %s", argv[2], token);
        sndmsg(buffer, 8080);
        printf("Downloading file: %s\n", argv[2]);
        receive_file("encrypted_file.bin", ".");
        printf("Verifying signature\n");
        if (verify_signature("encrypted_file.bin", argv[2]) == 1)
        {
            printf("Signature verified\nDecrypting file\n");
            decrypt_file("encrypted_file.bin", key, argv[2]);
            delete_file_client("encrypted_file.bin");
            printf("File decrypted\n");
        }
        else
        {
            printf("Signature not verified\nAborting decryption\n");
            delete_file_client("encrypted_file.bin");
        }
        break;
    case LISTFILES:
        if (check_token_still_valid(token, username) == 0)
        {
            printf("Token is not valid\n");
            return 0;
        }
        snprintf(buffer, 1024, "-list --token %s", token);
        sndmsg(buffer, 8080);
        getmsg(buffer);
        printf("%s\n", buffer);
        break;
    case LOGIN:
        snprintf(buffer, 1024, "-login %s %s", argv[2], argv[3]);
        sndmsg(buffer, 8080);
        getmsg(buffer);
        if (strcmp(buffer, "Login failed") == 0)
        {
            printf("Login failed\n");
            break;
        }
        printf("Login successful\n");
        write_token_to_file(buffer, argv[2]);
        break;
    case SIGNUP:
        snprintf(buffer, 1024, "-signup %s %s", argv[2], argv[3]);
        sndmsg(buffer, 8080);
        getmsg(buffer);
        if (strcmp(buffer, "Signup failed") == 0)
        {
            printf("Signup failed\n");
            break;
        }
        printf("Signup successful\n");
        write_token_to_file(buffer, argv[2]);
        break;
    default:
        break;
    }

    stopserver();
    return 0;
}
