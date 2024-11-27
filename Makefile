CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-ljpeg -lm
SOURCES= mandel.c jpegrw.c mandelmovie.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mandel mandelmovie

# all: $(SOURCES) $(EXECUTABLE) 
all: $(EXECUTABLE)
# pull in dependency info for *existing* .o files
# -include $(OBJECTS:.o=.d)

# clean:
# 	rm -rf $(OBJECTS) $(EXECUTABLE) *.d
mandel: mandel.o jpegrw.o
	$(CC) mandel.o jpegrw.o $(LDFLAGS) -o $@

mandelmovie: mandelmovie.o jpegrw.o 
	$(CC) mandelmovie.o jpegrw.o $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
