PROJECT = depress
CC = gcc
CFLAGS = -O3 -Wall -pthread -fopenmp
LDFLAGS = -lm
RM = rm -f
OBJS = depress.o depress_converter.o depress_document.o depress_image.o depress_maker_djvu.o depress_outlines.o depress_paths.o depress_tasks.o depress_threads.o ppm_save.o interlocked_ptr.o waccess.o wfopen.o wmain_stdc.o wmkdir.o wpopen.o wremove.o wrmdir.o wtoi.o wcstombsl.o wgetcwd.o noteshrink.o

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o : ../src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o : ../src/unixsupport/%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o : ../src/extclib/%.c
	$(CC) -c $(CFLAGS) $< -o $@
	
%.o : ../src/third_party/%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(PROJECT) $(OBJS)
