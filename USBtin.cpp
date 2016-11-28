/********************************************************************
File: main.c

Description:
This file contains the main logic of the USBtin project.
USBtin is a simple USB to CAN interface. It uses the USB class CDC
to connect to the host. Configuration and bus communication is
handled over this CDC virtual comport.

Authors and Copyright:
(c) 2012-2016, Thomas Fischl <tfischl@gmx.de>

Device: PIC18F14K50
Compiler: Microchip MPLAB XC8 C Compiler V1.37

License:
This file is open source. You can use it or parts of it in own
open source projects. For closed or commercial projects you have to
contact the authors listed above to get the permission for using
this source code.

 ********************************************************************/

#include <stdio.h>
#include <string.h>

#include "USBtin.h"

CAN USBTIN_CANport0(USBTIN_CAN_RX, USBTIN_CAN_TX);
CAN *USBTIN_CANport = &USBTIN_CANport0;

Serial USBTIN_serialPort0(USBTIN_SERIAL_TX, USBTIN_SERIAL_RX);
Serial *USBTIN_serialPort = &USBTIN_serialPort0;

extern unsigned char deviceState;

Timer UT_t;
/**
 * Main thread. Entry point for USBtin application.
 * Handles initialization and the the main processing loop.
 *
 * Can be called with:
 * 
 * int main(void) {
 *     Thread USBtin_thread(UT_thread);
 *
 *     while (1);
 * }
 */
void UT_thread(void const *args) {
    deviceState = STATE_CONFIG;
    UT_t.start();

    // buffer for incoming characters
    char line[LINE_MAXLEN];
    unsigned char linepos = 0;

    // buffer for incoming can messages
    canmsg_t canmsg_buffer[CANMSG_BUFFERSIZE];
    unsigned char canmsg_buffer_filled = 0;
    unsigned char canmsg_buffer_canpos = 0;
    unsigned char canmsg_buffer_usbpos = 0;
    unsigned char rxstep = 0;

    unsigned short led_lastclock = UT_t.read_ms();
    unsigned char led_ticker = 0;

    CANMessage cmsg;
    USBTIN_serialPort->baud(115200);

    // main loop
    while (1) {
        // handles interrupt requests of MCP2515 controller: receive message and store it to buffer
        if (deviceState != STATE_CONFIG) {
            if (USBTIN_CANport->read(cmsg)) {
                canmsg_buffer[canmsg_buffer_canpos] = fromCANMessage(&cmsg);
                canmsg_buffer_canpos = (canmsg_buffer_canpos + 1) % CANMSG_BUFFERSIZE;
                canmsg_buffer_filled++;
            }

        }

        // process can messages in receive buffer
        while (canmsg_buffer_filled > 0) {
            USBTIN_serialPort->putc(
                    canmsg2ascii_getNextChar(
                        &canmsg_buffer[canmsg_buffer_usbpos], &rxstep));
            if (rxstep == RX_STEP_FINISHED) {
                // finished this frame
                rxstep = 0;
                canmsg_buffer_usbpos = (canmsg_buffer_usbpos + 1)
                    % CANMSG_BUFFERSIZE;
                canmsg_buffer_filled--;
            }
        }

        // receive characters from virtual serial port and collect the data until end of line is indicated
        while (USBTIN_serialPort->readable() && (rxstep == 0)) {
            unsigned char ch = USBTIN_serialPort->getc();

            if (ch == CR) {
                line[linepos] = 0;
                parseLine(line);
                linepos = 0;
            } else if (ch != LR) {
                line[linepos] = ch;
                if (linepos < LINE_MAXLEN - 1)
                    linepos++;
            }
        }

        // led signaling
        if ((unsigned short) (UT_t.read_ms() - led_lastclock) > 500) {
            led_lastclock = UT_t.read_ms();
            led_ticker++;
        }

        Thread::wait(1);

    }
}
