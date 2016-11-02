CC = g++
CFLAGS = -Wall -I. -std=c++11 -I../include -I/usr/local/include
MACROS = -DASIO_STANDALONE
LIBS = -lpthread -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
TARGET = genjio
all: 
	$(CC) $(CFLAGS) $(MACROS) files.cpp main.cpp task.cpp videocheck.cpp -o main $(LIBS)
