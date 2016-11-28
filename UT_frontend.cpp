/********************************************************************
File: frontend.c

Description:
This file contains the frontend interface functions.

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

#include "frontend.h"

#include "mbed.h"

#include "usbtin.h"

unsigned char timestamping = 0;

unsigned char deviceState;

/**
 * Parse hex value of given string
 *
 * @param line Input string
 * @param len Count of characters to interpret
 * @param value Pointer to variable for the resulting decoded value
 * @return 0 on error, 1 on success
 */
unsigned char parseHex(char * line, unsigned char len, unsigned long * value) {
    *value = 0;
    while (len--) {
        if (*line == 0)
            return 0;
        *value <<= 4;
        if ((*line >= '0') && (*line <= '9')) {
            *value += *line - '0';
        } else if ((*line >= 'A') && (*line <= 'F')) {
            *value += *line - 'A' + 10;
        } else if ((*line >= 'a') && (*line <= 'f')) {
            *value += *line - 'a' + 10;
        } else
            return 0;
        line++;
    }
    return 1;
}

/**
 * Send given value as hexadecimal string
 *
 * @param value Value to send as hex over the UART
 * @param len Count of characters to produce
 */
void sendHex(unsigned long value, unsigned char len) {

    uint8_t s[20];
    s[len] = 0;

    while (len--) {

        unsigned char hex = value & 0x0f;
        if (hex > 9)
            hex = hex - 10 + 'A';
        else
            hex = hex + '0';
        s[len] = hex;

        value = value >> 4;
        USBTIN_serialPort->putc(s[len]);
    }

}

/**
 * Send given byte value as hexadecimal string
 *
 * @param value Byte value to send over UART
 */
void sendByteHex(unsigned char value) {

    //    sendHex(value, 2);

    unsigned char ch = value >> 4;
    if (ch > 9)
        ch = ch - 10 + 'A';
    else
        ch = ch + '0';
    USBTIN_serialPort->putc(ch);

    ch = value & 0xF;
    if (ch > 9)
        ch = ch - 10 + 'A';
    else
        ch = ch + '0';
    USBTIN_serialPort->putc(ch);

}

/**
 * Interprets given line and transmit can message
 *
 * @param line Line string which contains the transmit command
 */
unsigned char transmitStd(char *line) {
    canmsg_t canmsg;
    unsigned long temp;
    unsigned char idlen;

    canmsg.flags.rtr = ((line[0] == 'r') || (line[0] == 'R'));

    // upper case -> extended identifier
    if (line[0] < 'Z') {
        canmsg.flags.extended = 1;
        idlen = 8;
    } else {
        canmsg.flags.extended = 0;
        idlen = 3;
    }

    if (!parseHex(&line[1], idlen, &temp))
        return 0;
    canmsg.id = temp;

    if (!parseHex(&line[1 + idlen], 1, &temp))
        return 0;
    canmsg.dlc = temp;

    if (!canmsg.flags.rtr) {
        unsigned char i;
        unsigned char length = canmsg.dlc;
        if (length > 8)
            length = 8;
        for (i = 0; i < length; i++) {
            if (!parseHex(&line[idlen + 2 + i * 2], 2, &temp))
                return 0;
            canmsg.data[i] = temp;
        }
    }

    return USBTIN_CANport->write(toCANMessage(&canmsg));
}

/**
 * Parse given command line
 *
 * @param line Line string to parse
 */
void parseLine(char * line) {

    unsigned char result = BELL;

    switch (line[0]) {
        case 'S': // Setup with standard CAN bitrates
            if (deviceState == STATE_CONFIG) {
                switch (line[1]) {
                    case '0':
                        USBTIN_CANport->frequency(10000);
                        result = CR;
                        break;
                    case '1':
                        USBTIN_CANport->frequency(20000);
                        result = CR;
                        break;
                    case '2':
                        USBTIN_CANport->frequency(50000);
                        result = CR;
                        break;
                    case '3':
                        USBTIN_CANport->frequency(100000);
                        result = CR;
                        break;
                    case '4':
                        USBTIN_CANport->frequency(125000);
                        result = CR;
                        break;
                    case '5':
                        USBTIN_CANport->frequency(250000);
                        result = CR;
                        break;
                    case '6':
                        USBTIN_CANport->frequency(500000);
                        result = CR;
                        break;
                    case '7':
                        USBTIN_CANport->frequency(800000);
                        result = CR;
                        break;
                    case '8':
                        USBTIN_CANport->frequency(1000000);
                        result = CR;
                        break;
                }

            }
            break;
        case 's': // Setup with user defined timing settings for CNF1/CNF2/CNF3
            if (deviceState == STATE_CONFIG) {
                unsigned long cnf1, cnf2, cnf3;
                if (parseHex(&line[1], 2, &cnf1) && parseHex(&line[3], 2, &cnf2)
                        && parseHex(&line[5], 2, &cnf3)) {
                    // mcp2515_set_bittiming(cnf1, cnf2, cnf3);
                    result = CR;
                }
            }
            break;
        case 'G': // Read given MCP2515 register
            {
                unsigned long address;
                if (parseHex(&line[1], 2, &address)) {
                    // unsigned char value = mcp2515_read_register(address);
                    sendByteHex(0xFF);
                    result = CR;
                }
            }
            break;
        case 'W': // Write given MCP2515 register
            {
                unsigned long address, data;
                if (parseHex(&line[1], 2, &address) && parseHex(&line[3], 2, &data)) {
                    // mcp2515_write_register(address, data);
                    result = CR;
                }

            }
            break;
        case 'V': // Get hardware version
            {

                USBTIN_serialPort->putc('V');
                sendByteHex(VERSION_HARDWARE_MAJOR);
                sendByteHex(VERSION_HARDWARE_MINOR);
                result = CR;
            }
            break;
        case 'v': // Get firmware version
            {

                USBTIN_serialPort->putc('v');
                sendByteHex(VERSION_FIRMWARE_MAJOR);
                sendByteHex(VERSION_FIRMWARE_MINOR);
                result = CR;
            }
            break;
        case 'N': // Get serial number
            {
                USBTIN_serialPort->putc('N');
                /* USBTIN_serialPort->putc(USBSerial::deviceDesc()[7]);
                   USBTIN_serialPort->putc(USBTIN_serialPort->deviceDesc()[8]);
                   USBTIN_serialPort->putc(USBTIN_serialPort->deviceDesc()[9]);
                   USBTIN_serialPort->putc(USBTIN_serialPort->deviceDesc()[10]); */
                USBTIN_serialPort->putc('F');
                USBTIN_serialPort->putc('F');
                USBTIN_serialPort->putc('F');
                USBTIN_serialPort->putc('F');
                result = CR;
            }
            break;
        case 'O': // Open CAN channel
            if (deviceState == STATE_CONFIG) {
                USBTIN_CANport->reset();

                deviceState = STATE_OPEN;
                result = CR;
            }
            break;
        case 'l': // Loop-back mode
            if (deviceState == STATE_CONFIG) {
                USBTIN_CANport->monitor(false);
                USBTIN_CANport->reset();

                deviceState = STATE_OPEN;
                result = CR;
            }
            break;
        case 'L': // Open CAN channel in listen-only mode
            if (deviceState == STATE_CONFIG) {
                USBTIN_CANport->monitor(true); // set listen-only mode
                USBTIN_CANport->reset();

                deviceState = STATE_LISTEN;
                result = CR;
            }
            break;
        case 'C': // Close CAN channel
            if (deviceState != STATE_CONFIG) {
                USBTIN_CANport->reset();

                deviceState = STATE_CONFIG;
                result = CR;
            }
            break;
        case 'r': // Transmit standard RTR (11 bit) frame
        case 'R': // Transmit extended RTR (29 bit) frame
        case 't': // Transmit standard (11 bit) frame
        case 'T': // Transmit extended (29 bit) frame
            if (deviceState == STATE_OPEN) {
                if (transmitStd(line)) {
                    if (line[0] < 'Z')
                        USBTIN_serialPort->putc('Z');
                    else
                        USBTIN_serialPort->putc('z');
                    result = CR;
                }

            }
            break;
        case 'F': // Read status flags
            {
                unsigned char flags = 0; //mcp2515_read_register(MCP2515_REG_EFLG);
                unsigned char status = 0;

                if (flags & 0x01)
                    status |= 0x04; // error warning
                if (flags & 0xC0)
                    status |= 0x08; // data overrun
                if (flags & 0x18)
                    status |= 0x20; // passive error
                if (flags & 0x20)
                    status |= 0x80; // bus error

                USBTIN_serialPort->putc('F');
                sendByteHex(status);
                result = CR;
            }
            break;
        case 'Z': // Set time stamping
            {
                unsigned long stamping;
                if (parseHex(&line[1], 1, &stamping)) {
                    timestamping = (stamping != 0);
                    result = CR;
                }
            }
            break;
        case 'm': // Set accpetance filter mask
            if (deviceState == STATE_CONFIG) {
                unsigned long am0, am1, am2, am3;
                if (parseHex(&line[1], 2, &am0) && parseHex(&line[3], 2, &am1)
                        && parseHex(&line[5], 2, &am2)
                        && parseHex(&line[7], 2, &am3)) {
                    //mcp2515_set_SJA1000_filter_mask(am0, am1, am2, am3);
                    result = CR;
                }
            }
            break;
        case 'M': // Set accpetance filter code
            if (deviceState == STATE_CONFIG) {
                unsigned long ac0, ac1, ac2, ac3;
                if (parseHex(&line[1], 2, &ac0) && parseHex(&line[3], 2, &ac1)
                        && parseHex(&line[5], 2, &ac2)
                        && parseHex(&line[7], 2, &ac3)) {
                    //mcp2515_set_SJA1000_filter_code(ac0, ac1, ac2, ac3);
                    result = CR;
                }
            }
            break;

    }

    USBTIN_serialPort->putc(result);
}

/**
 * Get next character of given can message in ascii format
 * 
 * @param canmsg Pointer to can message
 * @param step Current step
 * @return Next character to print out
 */
char canmsg2ascii_getNextChar(canmsg_t * canmsg, unsigned char * step) {

    char ch = BELL;
    char newstep = *step;

    if (*step == RX_STEP_TYPE) {

        // type

        if (canmsg->flags.extended) {
            newstep = RX_STEP_ID_EXT;
            if (canmsg->flags.rtr)
                ch = 'R';
            else
                ch = 'T';
        } else {
            newstep = RX_STEP_ID_STD;
            if (canmsg->flags.rtr)
                ch = 'r';
            else
                ch = 't';
        }

    } else if (*step < RX_STEP_DLC) {

        // id

        unsigned char i = *step - 1;
        unsigned char * id_bp = (unsigned char*) &canmsg->id;
        ch = id_bp[3 - (i / 2)];
        if ((i % 2) == 0)
            ch = ch >> 4;

        ch = ch & 0xF;
        if (ch > 9)
            ch = ch - 10 + 'A';
        else
            ch = ch + '0';

        newstep++;

    } else if (*step < RX_STEP_DATA) {

        // length

        ch = canmsg->dlc;

        ch = ch & 0xF;
        if (ch > 9)
            ch = ch - 10 + 'A';
        else
            ch = ch + '0';

        if ((canmsg->dlc == 0) || canmsg->flags.rtr)
            newstep = RX_STEP_TIMESTAMP;
        else
            newstep++;

    } else if (*step < RX_STEP_TIMESTAMP) {

        // data

        unsigned char i = *step - RX_STEP_DATA;
        ch = canmsg->data[i / 2];
        if ((i % 2) == 0)
            ch = ch >> 4;

        ch = ch & 0xF;
        if (ch > 9)
            ch = ch - 10 + 'A';
        else
            ch = ch + '0';

        newstep++;
        if (newstep - RX_STEP_DATA == canmsg->dlc * 2)
            newstep = RX_STEP_TIMESTAMP;

    } else if (timestamping && (*step < RX_STEP_CR)) {

        // timestamp

        unsigned char i = *step - RX_STEP_TIMESTAMP;
        if (i < 2)
            ch = (canmsg->timestamp >> 8) & 0xff;
        else
            ch = canmsg->timestamp & 0xff;
        if ((i % 2) == 0)
            ch = ch >> 4;

        ch = ch & 0xF;
        if (ch > 9)
            ch = ch - 10 + 'A';
        else
            ch = ch + '0';

        newstep++;

    } else {

        // linebreak

        ch = CR;
        newstep = RX_STEP_FINISHED;
    }

    *step = newstep;
    return ch;
}

CANMessage toCANMessage(canmsg_t *msg) {
    CANMessage Cmsg;
    Cmsg.format = msg->flags.extended ? CANExtended : CANStandard;
    Cmsg.type = msg->flags.rtr ? CANRemote : CANData;
    Cmsg.id = msg->id;
    Cmsg.len = msg->dlc;
    memcpy(Cmsg.data, msg->data, msg->dlc);
    return Cmsg;
}

canmsg_t fromCANMessage(CANMessage *msg) {
    canmsg_t cmsg;
    cmsg.flags.extended = (msg->format == CANExtended);
    cmsg.flags.rtr = (msg->type == CANRemote);
    cmsg.id = msg->id;
    cmsg.dlc = msg->len;
    memcpy(cmsg.data, msg->data, msg->len);

    // msg.id = (uint32_t)buffer->ident & 0x07FFU;
    // msg.len = (uint32_t)buffer->ident >> 12U & 0xFU;
    // msg.type = ((uint32_t)buffer->ident & 0x8000U) == 0x8000 ? CANRemote : CANData;
    // memcpy(buffer->data, msg.data, msg.len)

    return cmsg;
}
