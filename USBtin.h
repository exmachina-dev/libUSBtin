/********************************************************************
 File: usbtin.h

 Description:
 This file contains global project definitions.

 Authors and Copyright:
 (c) 2012-2016, Thomas Fischl <tfischl@gmx.de>

 Device: PIC18F14K50
 Compiler: Microchip MPLAB XC8 C Compiler V1.37

 License:
 This file is open source. You can use it or parts of it in own
 open source projects. For closed or commercial projects you have to
 contact the authors listed above to get the permission for using
 this source code.

 Change History:
  Rev   Date        Description
  1.0   2011-12-18  Initial release
  1.1   2012-03-09  Minor fixes, code cleanup
  1.2   2013-01-11  Added filter function (command 'm' and 'M')
                    Added write register function (command 'W')
  1.3   2014-04-07  Changed/improved can bit timings
                    Fixed clock_process() (no blocking)
                    Fixed extended frames filtering
                    Added command 'V': return hardware version
                    Empty both can buffers at same loop cycle
                    Trigger USB sending in usb_putch
                    Increased SPI speed to 3 MHz (was 0,75 MHz)
                    Increased CDC buffer size to 128 (was 100)
                    Process received characters in a loop
  1.4   2014-09-20  Increased bulk transfer packet size (now 64)
  1.5   2014-12-05  New buffer structure, added CAN message buffer
                    Use USB ping-pong buffers
                    CDC/putch writes directly to USB ram
                    Printout of can messages within state-machine
                    Fixed handling of messages with DLC > 8
  1.6   2016-04-13  Fixed order of outgoing messages (TX fifo)
                    Added support for usb serial number string
                    Handle "halt endpoint" usb command
  1.7   2016-10-13  Fixed initialization of filter registers
                    Removed manual clearing of the RX flag
                    Changed main loop priorities
                    Enabled MCP2515 Rollover                    

 ********************************************************************/
#ifndef _USBTIN_
#define _USBTIN_

#include "mbed.h"

#include "UT_frontend.h"

#define VERSION_HARDWARE_MAJOR 1
#define VERSION_HARDWARE_MINOR 0
#define VERSION_FIRMWARE_MAJOR 1
#define VERSION_FIRMWARE_MINOR 7

#define CANMSG_BUFFERSIZE 8

#define STATE_CONFIG 0
#define STATE_OPEN 1
#define STATE_LISTEN 2

// Port configuration //
#if (USBTIN_CAN == 0)
#define USBTIN_CAN_RX		(p9)
#define USBTIN_CAN_TX		(p10)
#elif (USBTIN_CAN == 1)
#define USBTIN_CAN_RX		(p30)
#define USBTIN_CAN_TX		(p29)
#endif

#ifndef USBTIN_SERIAL_RX
#define USBTIN_SERIAL_RX        (USBRX)
#endif

#ifndef USBTIN_SERIAL_TX
#define USBTIN_SERIAL_TX        (USBTX)
#endif


extern CAN *USBTIN_CANport;
extern Serial *USBTIN_serialPort;

extern unsigned char deviceState;

void UT_thread(void const *args);

#endif
