CPP = g++
CC = gcc
CFLAGS = -Wall -I. -std=c++11 -I../include -I/usr/local/include
MACROS = -DASIO_STANDALONE
LIBS = -ldl -lpthread -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
TARGET = genjio

sqlite:
        $(CC) -c sqlite3.c -o sqlite3.o

all: sqlite
        $(CPP) $(CFLAGS) $(MACROS) sqlite3.o files.cpp main.cpp task.cpp videocheck.cpp db.cpp -o main $(LIBS)
