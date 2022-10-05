# gcc is used for ubuntu host
CC=gcc

# arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core
#CC=arm-buildroot-linux-gnueabi-gcc

# arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core
#CC=arm-buildroot-linux-gnueabihf-gcc

.PHONY: clean

csvGenerate: main.c
	@$(CC) -o '$@' '$<'

clean:
	@rm csvGenerate
