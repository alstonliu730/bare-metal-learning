# Intro to UART
Universal Asynchronous Receiver/Transmitter (UART) is a popular serial communication protocol used for exchanging data between devices, often found in micro-controllers and other electronic components.

It uses two-wires to communicate with a "receiver" wire and a "transceiver" wire. Before USB, devices like mice, printers, and modems were connected using UART.

## Properties of UART
- **Asynchronous Data Transfer**

    The advantage of using UART protocols is that it's asynchronous meaning, the transmitter and receiver do not share a common clock signal. However, since they do not share a clock, both ends of the communication must transmit and receive at the same bit timing or baud rate. The most common UART baud rates are `4800`, `9600`, `192,200`, `572600`, & `115,200`. 

- **Format of UART Packet**

    Here we can see the UART address format. The data frame in the packet can range from 5 to 9 bits. The **start bit** will always be 1 bit and there can be an optional **parity bit**. There will also be 1-2 bits for the **stop bits**.
    The parity bit tells the UART protocol the *evenness* or *oddness* of a number. This is to ensure the device receives the right amount of data. It checks if the number of enabled bits in the data frame is even or odd. If the parity bit is 0 (even parity), the number of enabled bits must also be even. If it is 1 (odd parity), then the number of enabled bits must be odd. Bits can be easily changed by electromagnetic radiation, mistmatched baud rates, or long-distance data transfer. This simple technique helps prevent errors in data packets.

    ![UART Bit Frame Format](assets/uart_bit_format.png)


## Connecting the serial cable

## Getting UART going in code
In order to get UART working, we need to write some functions in order to read the bits at the UART addresses. Here we have defined two functions to read and write the bits at the given address. 

```C
void mmio_write(long reg, unsigned int val) {
    *(volatile unsigned int *)reg = val;
}

unsiged int mmio_read(long reg) {
    return *(volatile unsigned int *)reg;
}
```

These functions are used to read and write from the memory-mapped io addresses. Please refer to the BCM2711 manual to see the address table on page 5 and page 8.