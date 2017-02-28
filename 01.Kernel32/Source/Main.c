/*
 * Main.c
 *
 *  Created on: Feb 8, 2017
 *      Author: kiiren
 */

#include "Types.h"
#include "Page.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);

void Main(void)
{
	DWORD i;

	kPrintString(0,3, "C Lang Kernel Start.........................[PASS]");
	///////////////////0....5....10...15...20...25...30...35...40...45...50
	kPrintString(0,4, "Minimum Memory Size Check...................[    ]");
	if (kIsMemoryEnough() == FALSE)
	{
		kPrintString(45,4, "Fail");
		kPrintString(0, 5, "Not Enough Memory : Requires at least 64MByte");
		while (1);
	}
	else
	{
		kPrintString(45,4, "PASS");
	}
	kPrintString(0,5, "IA-32e Kernel Area Init.....................[    ]");
	if (kInitializeKernel64Area() == FALSE)
	{
		kPrintString(45,5, "Fail");
		kPrintString(0, 6, "Kernel Area Init Failed..");
		while (1);
	}
	else
	{
		kPrintString(45,5, "PASS");
	}
	kPrintString(0,6, "IA-32e Page Tables Init.....................[    ]");
	kInitializePageTables();
	kPrintString(45,6, "PASS");

	while (1);
}

void kPrintString(int iX, int iY, const char* pcString)
{
	CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
	int i;

	pstScreen += (iY * 80) + iX;
	for(i=0;pcString[i]!=0;i++)
	{
		pstScreen[i].bCharacter = pcString[i];
	}
}

BOOL kInitializeKernel64Area(void)
{
	DWORD* pdwCurrentAddress;

	pdwCurrentAddress = (DWORD*) 0x100000;
	while((DWORD) pdwCurrentAddress < 0x60000)
	{
		*pdwCurrentAddress = 0x00;

		if(*pdwCurrentAddress !=0) // if error
			return FALSE;

		pdwCurrentAddress++;
	}
	return TRUE;
}


BOOL kIsMemoryEnough(void)
{
	DWORD* pdwCurrentAddress;

	pdwCurrentAddress = (DWORD*) 0x100000; // 1MB

	while((DWORD)pdwCurrentAddress < 0x4000000) // to 64MB
	{
		*pdwCurrentAddress = 0x12345678;
		if (*pdwCurrentAddress != 0x12345678) // check if written
		{
			return FALSE;
		}
		pdwCurrentAddress += (0x100000 / 4); // jmp 1MB
	}
	return TRUE;
}
