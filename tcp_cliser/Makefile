# gcc is used for ubuntu host
#CCS=gcc
# arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core
#CCS=arm-buildroot-linux-gnueabi-gcc
# arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core
CCS=arm-buildroot-linux-gnueabihf-gcc

# gcc is used for ubuntu host
#CCC=gcc
# arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core
#CCC=arm-buildroot-linux-gnueabi-gcc
# arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core
CCC=arm-buildroot-linux-gnueabihf-gcc

all: server client server_multi

.PHONY: clean

server: server.c
	@$(CCS) -o '$@' '$<'

server_multi: server_multi.c
	@$(CCS) -o '$@' '$<'

client: client.c
	@$(CCC) -o '$@' '$<'

clean:
	@rm -rf server
	@rm -rf server_multi
	@rm -rf client
