lib_LTLIBRARIES = libstack.la\
				  libdecoder.la
bin_PROGRAMS = my_image_decoder

libstack_la_LDFLAGS = -no-undefined
libstack_la_SOURCES = stack.c

libdecoder_la_LDFLAGS = -no-undefined
libdecoder_la_SOURCES = bitmap.c\
						bitmap.h\
						average_filter.c\
						average_filter.h\
						gaussian_filter.c\
						gaussian_filter.h\
						bilateral_filter.c\
						bilateral_filter.h\
						prewitt_filter.c\
						prewitt_filter.h\
						sobel_filter.c\
						sobel_filter.h\
						canny_edge_detector.c\
						canny_edge_detector.h\
						LoG_filter.c\
						LoG_filter.h\
						png.c\
						png.h\
						common.c\
						common.h\
						crc.c\
						crc.h

# Cコンパイラへ渡すオプション(ここではコメントアウトしています)
# test_CFLAGS = -g

my_image_decoder_CC = gcc
my_image_decoder_CFLAGS = -Wall
my_image_decoder_SOURCES = main.c\
						   bitmap.c\
						   bitmap.h\
						   average_filter.c\
						   average_filter.h\
						   gaussian_filter.c\
						   gaussian_filter.h\
						   bilateral_filter.c\
						   bilateral_filter.h\
						   prewitt_filter.c\
						   prewitt_filter.h\
						   sobel_filter.c\
						   sobel_filter.h\
						   canny_edge_detector.c\
						   canny_edge_detector.h\
						   LoG_filter.c\
						   LoG_filter.h\
						   png.c\
						   png.h\
						   common.c\
						   common.h\
						   crc.c\
						   crc.h
