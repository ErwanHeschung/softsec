#ifndef _PARAM_H_
#define _PARAM_H_

typedef enum
{
    UPLOAD,
    DOWNLOAD,
    LISTFILES,
    LOGIN,
    SIGNUP,
    CHECK
} Param;

Param get_param(int argc, char *argv[]);

Param get_param_from_string(char *param);

#endif