/*
 * Types.h
 *
 *  Created on: Feb 8, 2017
 *      Author: kiiren
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int
#define QWORD unsigned long
#define BOOL unsigned char

#define TRUE 1
#define FALSE 0
#define NULL 0

#pragma pack(1)

typedef struct kCharactorStruct
{
	BYTE bCharacter;
	BYTE bAttribute;
} CHARACTER;

#endif /* 01_KERNEL32_SOURCE_TYPES_H_ */
