///////////////////////////////////////////////////////////////////////////////
//
// File Name	: 'tea.h'
// Title		: The Tiny Encryption Algorithm (TEA)
// Author		: Skytrack ltd - Copyright (C) 2006
// Created		: 23/07/2006
// Version		: 1.0
//
// This code is property of Skytrack company
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TEA_H

#define _TEA_H

#define TEA_KEY_LEN	16
#define TEA_BLOCK_LEN	8	

#define DELTA		0x9E3779B9
#define ROUNDS		32

void tea_enc( unsigned int *const v, const unsigned int *const k );
void tea_dec( unsigned int *const v, const unsigned int *const k );

void TEAStreamEncrypt(unsigned char *lpBuffer, unsigned int len, 
						const unsigned char *lpSessionKey);
void TEAStreamDecrypt(unsigned char *lpBuffer, unsigned int len, 
						const unsigned char *lpSessionKey);

#endif

// end


