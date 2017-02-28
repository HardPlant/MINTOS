/*
 * Page.c
 *
 *  Created on: Feb 11, 2017
 *      Author: kiiren
 */
#include "Page.h"

//Create Page Table for IA-32e mode kernel

void kInitializePageTables(void)
{
	PML4TENTRY* pstPML4TEntry;
	PDPTENTRY* pstPDPTEntry;
	PDENTRY* pstPDTEntry;
	DWORD dwMappingAddress;
	int i;

	//Create PML4 Table
	//Init 0 except first entry
	pstPML4TEntry = (PML4TENTRY* ) 0x100000;
	kSetPageEntryData(&(pstPML4TEntry[0]), 0x00, 0x101000,PAGE_FLAGS_DEFAULT,0);
	for(i=1;i<PAGE_MAXENTRYCOUNT;i++)
	{
		kSetPageEntryData(&(pstPML4TEntry[i]), 0,0,0,0);
	}

	//Create Page Directory Pointer Table
	//1 PDPT maps to 512GB
	//set 64 Entry to 64GB
	pstPDPTEntry = (PDPTENTRY*) 0x101000;
	for(i=0; i<64; i++)
	{
		kSetPageEntryData(&(pstPDPTEntry[i]), 0,0,0,0);
	}

	//Create Page Direcory Table
	//1 PDT maps to 1GB
	//set 64 PDT, supports 64GB
	pstPDTEntry = (PDENTRY*) 0x102000;
	dwMappingAddress = 0;
	for(i=0;i<PAGE_MAXENTRYCOUNT * 64; i++)
	{
		//32bit can't represent 64bit upper address
		//calculate as MB unit first,
		//then divides result by 4 to calculate up than 32bit
		kSetPageEntryData(&(pstPDTEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12
				,dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}
}


void kSetPageEntryData(PTENTRY* pstEntry, DWORD dwUpperBaseAddress
		,DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags)
{
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}
