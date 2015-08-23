///////////////////////////////////////////////////////////////////////////////
//
// File Name	: 'tea.c'
// Title		: The Tiny Encryption Algorithm (TEA)
// Author		: Skytrack ltd - Copyright (C) 2006
// Created		: 23/07/2006
// Version		: 1.0
//
// This code is property of Skytrack company
//
///////////////////////////////////////////////////////////////////////////////

#include "tea.h"

//**************************************************************************
// Performs an encryption of a buffer
// Arguments: 
//		lpBuffer	- [IN, OUT] pointer to an input/output buffer
//		len		- [IN] length of input data
//		lpSessionKey	- [IN] crypto key
// Returns: void
//**************************************************************************
void TEAStreamEncrypt(unsigned char *lpBuffer, unsigned int len, 
					const unsigned char *lpSessionKey) {
	unsigned long i;

	for (i = 0; i < len; i+=8, lpBuffer += 8)
		tea_enc((unsigned int *)lpBuffer, 
			(unsigned int *)lpSessionKey);
}


//**************************************************************************
// Performs an decryption of a buffer
// Arguments: 
//		lpBuffer	- [IN, OUT] pointer to an input/output buffer
//		len		- [IN] length of input data
//		lpSessionKey	- [IN] crypto key
// Returns: void
//**************************************************************************
void TEAStreamDecrypt(unsigned char *lpBuffer, unsigned int len, 
					const unsigned char *lpSessionKey) {
	unsigned long i;

	for (i = 0; i < len; i+=8, lpBuffer += 8)
		tea_dec((unsigned int *)lpBuffer, 
			(unsigned int *)lpSessionKey);
}


//**************************************************************************
// Performs an encryption of a long
// Arguments: 
//		v - [IN, OUT] pointer to a long to be encrypted
//		k - [IN] crypto key
// Returns: void
//**************************************************************************
void tea_enc( unsigned int *const v, const unsigned int *const k )
{
	unsigned int y=v[0], z=v[1], sum=0, n=ROUNDS;

	while(n-->0) {
		sum += DELTA;
		y += ((z<<4)+k[0]) ^ (z+sum) ^ ((z>>5)+k[1]);
		z += ((y<<4)+k[2]) ^ (y+sum) ^ ((y>>5)+k[3]);
	}

	v[0]=y; v[1]=z;
}

//**************************************************************************
// Performs an decryption of a long
// Arguments: 
//		v - [IN, OUT] pointer to a long to be decrypted
//		k - [IN] crypto key
// Returns: void
//**************************************************************************
void tea_dec( unsigned int *const v, const unsigned int *const k )
{
	unsigned int y=v[0], z=v[1], sum=DELTA*ROUNDS, n=ROUNDS;

	while(n-->0) {
		z -= ((y<<4)+k[2]) ^ (y+sum) ^ ((y>>5)+k[3]);
		y -= ((z<<4)+k[0]) ^ (z+sum) ^ ((z>>5)+k[1]);
		sum -= DELTA;
	}

	v[0]=y; v[1]=z;
}

