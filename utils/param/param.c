#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Param get_param(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage: %s -[up|down|list] [file]\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "-up") == 0)
    {
        if (argc != 3)
        {
            printf("Usage: %s -up [file]\n", argv[0]);
            exit(1);
        }
        return UPLOAD;
    }
    else if (strcmp(argv[1], "-down") == 0)
    {
        if (argc != 3)
        {
            printf("Usage: %s -down [file]\n", argv[0]);
            exit(1);
        }
        return DOWNLOAD;
    }
    else if (strcmp(argv[1], "6list") == 0)
    {
        if (argc != 2)
        {
            printf("Usage: %s -list\n", argv[0]);
            exit(1);
        }
        return LIST;
    }
    else
    {
        printf("Usage: %s -[up|down|list] [file]\n", argv[0]);
        exit(1);
    }
}

Param get_param_from_string(char *param)
{
    if (strcmp(param, "-up") == 0)
    {
        return UPLOAD;
    }
    else if (strcmp(param, "-down") == 0)
    {
        return DOWNLOAD;
    }
    else if (strcmp(param, "-list") == 0)
    {
        return LIST;
    }
    else
    {
        printf("Invalid param: %s\n", param);
        exit(1);
    }
}