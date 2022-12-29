CC=gcc
CPP=g++
CFLAGS=-O3 -c -Wall -DFASTIMAGE_USE_LIBCURL

all: depress

test: depress.o depress_converter.o depress_document.o depress_image.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o wmain_stdc.o
	$(CPP) depress.o depress_converter.o depress_document.o depress_image.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o wmain_stdc.o -lcurl -o test
	
depress.o:
	$(CC) $(CFLAGS) ../src/depress.c
	
depress_converter.o:
	$(CC) $(CFLAGS) ../src/depress_converter.c
	
depress_document.o:
	$(CC) $(CFLAGS) ../src/depress_document.c
	
depress_image.o:
	$(CC) $(CFLAGS) ../src/depress_image.c
	
depress_paths.o:
	$(CC) $(CFLAGS) ../src/depress_paths.c
	
depress_tasks.o:
	$(CC) $(CFLAGS) ../src/depress_tasks.c
	
depress_threads.o:
	$(CC) $(CFLAGS) ../src/depress_threads.c
	
ppm_save.o:
	$(CC) $(CFLAGS) ../src/ppm_save.c
	
wmain_stdc.o:
	$(CC) $(CFLAGS) ../unixsupport/wmain_stdc.c
	
clean:
	rm -f *.o depress
