PROJECT=depress
CC=gcc
CFLAGS=-O3 -c -Wall -pthread
CPP=g++
CPPFLAGS=-pthread
RM =rm -f
OBJS = depress.o depress_converter.o depress_document.o depress_image.o depress_maker_djvu.o depress_outlines.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o interlocked_ptr.o waccess.o wfopen.o wmain_stdc.o wmkdir.o wpopen.o wremove.o wrmdir.o wtoi.o wcstombsl.o wgetcwd.o noteshrink.o

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

%.o : ../src/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o : ../src/unixsupport/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o : ../src/extclib/%.c
	$(CC) $(CFLAGS) $< -o $@
	
%.o : ../src/third_party/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(PROJECT) $(OBJS)
