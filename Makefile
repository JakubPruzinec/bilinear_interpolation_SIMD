CFLAGS = -std=c99 -pedantic -Wall -Wextra -I ./include -O5
CCX = gcc

HDRDEP = $(wildcard *.h)

all:
	make image-info
	make image-info_avx

clear:
	rm build/*.o build/image-info*

# COMPILE OBJECTS
main.o: $(HDRDEP) src/main.c
	$(CCX) $(CFLAGS) src/main.c -c -o build/main.o

image.o: $(HDRDEP) src/image.c
	$(CCX) $(CFLAGS) src/image.c -c -o build/image.o

image_resize.o: $(HDRDEP) src/image_resize.c
	$(CCX) $(CFLAGS) src/image_resize.c -c -o build/image_resize.o

image_resize_avx.o: $(HDRDEP) src/image_resize_avx.c
	$(CCX) $(CFLAGS) -mavx src/image_resize_avx.c -c -o build/image_resize_avx.o


# LINK OBJECTS
image-info: main.o image.o image_resize.o
	$(CCX) $(CFLAGS) build/image.o build/image_resize.o build/main.o -o build/image-info

image-info_avx: main.o image.o image_resize_avx.o
	$(CCX) $(CFLAGS) build/image.o build/image_resize_avx.o build/main.o -o build/image-info_avx
