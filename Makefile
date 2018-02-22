all:
	gcc -Wall main.c bitmap.c average_filter.c gaussian_filter.c bilateral_filter.c prewitt_filter.c sobel_filter.c canny_edge_detector.c LoG_filter.c png.c common.c
