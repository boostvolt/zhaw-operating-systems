# ---------------------------------------------------------------------------
# Makefile for Lab2 MPC - Multicore threads and processes 
# Module:     MPC
# Changes:  donn
# File:     makefile 
# Version:  v.fs21
# ---------------------------------------------------------------------------

# Macros

CC    = gcc -std=gnu99
CFLGS = -Wall 
LFLGS = -pthread

TARGET := out

source  = sobel_rgb2g.c MEM_lab.c
objects = sobel_rgb2g.o MEM_lab.o
header  = sobel_rgb2g.h makefile


.PHONY : clean all out

out:	$(source) $(objects) $(header) 
		@echo "Linking"
		$(CC) $(CFLGS) $(objects) -o $@.t -lm
		
.c.o:	$< 
		$(CC) $(CFLGS) -c $< -o $*.o

clean:		
		rm -f *.e *.o
		@echo "directory cleaned"
    
all:
		@rm -f *.o
#-----------------------------------------------------------------------------
