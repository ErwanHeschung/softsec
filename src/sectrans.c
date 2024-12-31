#include "../utils/param/param.h"
#include "../utils/file/file.h"
#include "../libs/client.h"
#include "../libs/server.h"
#include "../utils/key_manager/key_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int agrc, char *argv[])
{
    startserver(4200);
    char buffer[1024] = {0};
    Param p = get_param(agrc, argv);
    unsigned char *key = get_or_generate_keys();
    switch (p)
    {
    case UPLOAD:
        snprintf(buffer, 1024, "-up %s", argv[2]);
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
        break;
    case DOWNLOAD:
        snprintf(buffer, 1024, "-down %s", argv[2]);
        sndmsg(buffer, 8080);
        printf("Downloading file: %s\n", argv[2]);
        receive_file("encrypted_file.bin", ".");
        printf("Verifying signature\n");
        if (verify_signature("encrypted_file.bin", argv[2]) == 1)
        {
            printf("Signature verified\nDecrypting file\n");
            decrypt_file("encrypted_file.bin", key);
            delete_file_client("encrypted_file.bin");
        }
        else
        {
            printf("Signature not verified\nAborting decryption\n");
            delete_file_client("encrypted_file.bin");
        }
        break;
    case LIST:
        snprintf(buffer, 1024, "-list");
        sndmsg(buffer, 8080);
        getmsg(buffer);
        printf("%s\n", buffer);
        break;
    default:
        break;
    }

    stopserver();
    return 0;
}
