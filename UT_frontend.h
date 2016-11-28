/********************************************************************
 File: frontend.h

 Description:
 This file contains the frontend interface definitions.

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
#ifndef _FRONTEND_
#define _FRONTEND_

#include "mbed.h"
#include "CAN.h"
#include "UT_CANMessage.h"

#define LINE_MAXLEN 100
#define BELL 7
#define CR 13
#define LR 10

#define RX_STEP_TYPE 0
#define RX_STEP_ID_EXT 1
#define RX_STEP_ID_STD 6
#define RX_STEP_DLC 9
#define RX_STEP_DATA 10
#define RX_STEP_TIMESTAMP 26
#define RX_STEP_CR 30
#define RX_STEP_FINISHED 0xff

unsigned char transmitStd(char *line);
void parseLine(char * line);
char canmsg2ascii_getNextChar(canmsg_t * canmsg, unsigned char * step);

CANMessage toCANMessage(canmsg_t *msg);
canmsg_t fromCANMessage(CANMessage *msg);

#endif
