CC = gcc
CX = gcc

CFLAGS = `pkg-config gtk+-2.0 gtkglext-1.0 --cflags` -Wall
CXFLAGS = `pkg-config gtk+-2.0 gtkglext-1.0 --libs` -lm

OBJDIR = .obj
OBJ = $(OBJDIR)/core.o $(OBJDIR)/main.o

EXE = ../waves-gtk


Release: CFLAGS += -O2
Release: CXFLAGS += -s
Release: build $(OBJ)

Debug: CFLAGS += -g
Debug: build $(OBJ)

cleanRelease: clean
cleanDebug: clean


clean:
	if [ -f $(EXE) ]; then rm $(EXE); fi
	rm -rf $(OBJDIR)

build:
	mkdir -p $(OBJDIR)
$(OBJDIR)/core.o: ../core/core.c
	$(CC) $(CFLAGS) -c ../core/core.c -o $(OBJDIR)/core.o
$(OBJDIR)/main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o
	$(CX) $(CXFLAGS) $(OBJ) -o $(EXE)
