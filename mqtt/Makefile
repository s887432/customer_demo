##################################################################
# gcc is used for ubuntu host
#CCP=gcc
# arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core
#CCP=arm-buildroot-linux-gnueabi-gcc
# arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core
CCP=arm-buildroot-linux-gnueabihf-gcc
##################################################################
# gcc is used for ubuntu host
#CCS=gcc
# arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core
#CCS=arm-buildroot-linux-gnueabi-gcc
# arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core
CCS=arm-buildroot-linux-gnueabihf-gcc
##################################################################

all: mqtt_pub mqtt_sub

.PHONY: clean

mqtt_pub: async_pub.c
	@$(CCP) -o '$@' '$<' -l paho-mqtt3c

mqtt_sub: async_sub.c
	@$(CCS) -o '$@' '$<' -l paho-mqtt3c

clean:
	@rm -rf mqtt_pub
	@rm -rf mqtt_sub
