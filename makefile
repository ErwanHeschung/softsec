CC = gcc
CFLAGS = -Wall -Iutils/param -Iutils/file -Ilibs

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = libs
UTILS_PARAM_DIR = utils/param
UTILS_FILE_DIR = utils/file

SRCS = $(SRC_DIR)/sectrans.c $(SRC_DIR)/server.c $(UTILS_PARAM_DIR)/param.c $(UTILS_FILE_DIR)/file.c
CLIENT_OBJS = $(OBJ_DIR)/sectrans.o $(OBJ_DIR)/param.o $(OBJ_DIR)/file.o
SERVER_OBJS = $(OBJ_DIR)/server.o $(OBJ_DIR)/param.o $(OBJ_DIR)/file.o

CLIENT_TARGET = sectrans
SERVER_TARGET = server

# Default target to build both client and server
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Rule for building the client target
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $(CLIENT_TARGET) -L$(LIB_DIR) -lclient -lserver -Wl,-rpath=$(LIB_DIR)

# Rule for building the server target
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $(SERVER_TARGET) -L$(LIB_DIR) -lclient -lserver -Wl,-rpath=$(LIB_DIR)

# Rule to compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/param.o: $(UTILS_PARAM_DIR)/param.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/file.o: $(UTILS_FILE_DIR)/file.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build artifacts
clean:
	rm -rf $(OBJ_DIR) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all clean
