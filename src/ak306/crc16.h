///////////////////////////////////////////////////////////////////////////////
//
// File Name	: 'crc16.h'
// Title		: CRC computation logic
// Created		: 23/07/2006
// Version		: 1.0
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CRC16_H

#define CRC16_H

int crcbuf(int crc, unsigned char *buf, unsigned int len);
extern const unsigned short crctab[];

#endif

// End

