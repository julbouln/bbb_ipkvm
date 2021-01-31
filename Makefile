CC=gcc
CFLAGS=-I/usr/local/include -O3
LDFLAGS=-L/usr/local/lib -lvncserver -lpthread -lm -lasound

INCLUDE_DIR=.

all: ipkvm_vnc_server

OBJ_DEPS = video.o server.o input.o usb_hid_gadget.o audio.o rfb_audio_extension.o ipkvm.o
OBJ = $(patsubst %,%, $(OBJ_DEPS))

.cpp.o:
	$(CC) -c -o $@ $< $(CFLAGS)

ipkvm_vnc_server: $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o ipkvm_vnc_server

ipkvm_vnc_client:
	gcc client.c -I/usr/include/SDL2 -lSDL2 -lvncclient -o ipkvm_vnc_client

clean:
	rm -rf *.o *~ ipkvm_vnc_client ipkvm_vnc_server
