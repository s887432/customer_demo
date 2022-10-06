# customer_demo

This project is a demostration for customer application.<br>
In this demo, will use following boards.<br>
a. ATSAMA5D27C-WLSOM1-EK: used for simulate server. MQTT broker will be launched automatically.<br>
b. ATSAMA5D27C-SOM1-EK: used for client devices. will use two sets.<br>
c. Arduino UNO: used for emulate PLC. report data via UART.<br>

## PLC_emu
Arduino PLC emu project<br>

## mqtt
MATT example code. including publisher and subscriber example code.<br>
MQTT subscriber. used in server.<br>

## tcp_cliser
TCP client server example code. incuding client, single client server and multi-client server.<br>
server_multi used in server.<br>

## uartReceiver
UART receiver example code.<br>
not used directly. integrated in demo code.<br>

## csvGenerate
write CSV example code.<br>
not used alone. integrating in demo code.(TODO..)<br>

##demo
customer demo code.<br>

## How to use
Using Makefile to build application.<br>
$ cd demo<br>
$ make<br>

suport X64-32 (Ubuntu), ARM926ej-s (SAM9x60), and Cortex-Ax (SAMA5, SAMA7) platform.<br>
modify Makefile to select target platform.<br>

#gcc is used for ubuntu host<br>
#CC=gcc<br>
#arm-buildroot-linux-gnueabi-gcc is used for ARM926ej-s core<br>
#CC=arm-buildroot-linux-gnueabi-gcc<br>
#arm-buildroot-linux-gnueabihf-gcc us used for Cortex-A5/A7 core<br>
CC=arm-buildroot-linux-gnueabihf-gcc<br>

in case building MQTT code, need to install mosquitto package in Ubuntu.<br>

Have fun!!!
