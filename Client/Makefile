# the compiler: gcc for C program, define as g++ for C++
CC = gcc -pthread 

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the list of header files
#INCL   = server.h
#the list of source files
SRC = Client.c mqttpacket.c
OBJ = $(SRC:.c=.o)

# the build target executable:
TARGET = client

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ): $(INCL)

clean:
	$(RM) $(TARGET) $(OBJ)