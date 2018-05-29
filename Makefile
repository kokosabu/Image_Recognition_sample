CC = gcc
CFLAGS = -Wall
OBJS = main.o bitmap.o average_filter.o gaussian_filter.o bilateral_filter.o prewitt_filter.o sobel_filter.o canny_edge_detector.o LoG_filter.o png.o common.o crc.o

all: a.out

a.out: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<
