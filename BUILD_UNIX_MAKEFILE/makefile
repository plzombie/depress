CC=gcc
CPP=g++
CFLAGS=-O3 -c -Wall -DFASTIMAGE_USE_LIBCURL -pthread
CPPFLAGS=-pthread

all: depress

depress: depress.o depress_converter.o depress_document.o depress_image.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o interlocked_ptr.o waccess.o wfopen.o wmain_stdc.o wmkdir.o wpopen.o wremove.o wrmdir.o wtoi.o wcstombsl.o
	$(CPP) $(CPPFLAGS) depress.o depress_converter.o depress_document.o depress_image.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o interlocked_ptr.o waccess.o wfopen.o wmain_stdc.o wmkdir.o wpopen.o wremove.o wrmdir.o wtoi.o wcstombsl.o -o depress
	
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
	
interlocked_ptr.o:
	$(CC) $(CFLAGS) ../src/interlocked_ptr.c
	
waccess.o:
	$(CC) $(CFLAGS) ../unixsupport/waccess.c
	
wfopen.o:
	$(CC) $(CFLAGS) ../unixsupport/wfopen.c
	
wmain_stdc.o:
	$(CC) $(CFLAGS) ../unixsupport/wmain_stdc.c
	
wmkdir.o:
	$(CC) $(CFLAGS) ../unixsupport/wmkdir.c
	
wpopen.o:
	$(CC) $(CFLAGS) ../unixsupport/wpopen.c
	
wremove.o:
	$(CC) $(CFLAGS) ../unixsupport/wremove.c
	
wrmdir.o:
	$(CC) $(CFLAGS) ../unixsupport/wrmdir.c
	
wtoi.o:
	$(CC) $(CFLAGS) ../unixsupport/wtoi.c
	
wcstombsl.o:
	$(CC) $(CFLAGS) ../extclib/wcstombsl.c
	
clean:
	rm -f *.o depress
