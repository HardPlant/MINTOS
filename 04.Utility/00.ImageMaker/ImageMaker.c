#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#define O_BINARY 0x00 // Do not separated in Linux, remove this on Windows
#define BYTESOFSECTOR 512

int AdjustInSectorSize(int iFd, int iSourceSize);
void WriteKernelInformation(int iTargetFd, int iKernelSectorCount);
int CopyFile(int iSourceFd, int iTargetFd);

int main(int argc, char* argv[])
{
	int iSourceFd;
	int iTargetFd;
	int iBootLoaderSize;
	int iKernel32SectorCount;
	int iSourceSize;
	
	if(argc <3)
	{
		fprintf(stderr, "[ERROR] ImageMaker.exe BootLoader.bin Kernel32.bin\n");
		exit(-1);
	}
	
	iTargetFd = open("Disk.img",
			O_RDWR|O_CREAT|O_TRUNC/*|O_BINARY|S_IREAD|S_IWRITE*/);
	
	if( iTargetFd == -1)
	{
		fprintf(stderr, "[ERROR] Disk.img open failed.\n");
		exit(-1);
	}
	
	/////
	//Copies Bootloader.bin context to Disk Image File
	/////
	printf("[INFO] Copy bootloader to image file\n");
	
	iSourceFd = open(argv[1], O_RDONLY|O_BINARY);
	if(iSourceFd == -1)
	{
		fprintf(stderr, "[ERROR] %s open failed\n", argv[1]);
		exit(-1);
	}
	
	iSourceSize = CopyFile(iSourceFd, iTargetFd);
	close(iSourceFd);
	
	iBootLoaderSize = AdjustInSectorSize(iTargetFd, iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n"
			,argv[1], iSourceSize, iBootLoaderSize);
			
	/////
	//Copies 32Bit Kernel.bin to Disk Image File
	/////
	printf("[INFO] Copy protected mode kernel to image file\n");
	
	iSourceFd = open(argv[2],O_RDONLY | O_BINARY);
	if(iSourceFd == -1)
	{
		fprintf(stderr, "[ERROR] %s open failed\n", argv[2]);
		exit(-1);
	}
	
	iSourceSize = CopyFile(iSourceFd, iTargetFd);
	close(iSourceFd);
	
	iKernel32SectorCount = AdjustInSectorSize(iTargetFd, iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n"
			,argv[1], iSourceSize, iKernel32SectorCount);
	////
	//Update Kernel Information to Disk Image	
	////
	printf("[INFO] Start to write kernel information");
	
	WriteKernelInformation(iTargetFd, iKernel32SectorCount);
	
	printf("[INFO] Image file create complete\n");
	
	close(iTargetFd);
	return 0;
}

// Fill 0x00 from current location to scale of 512byte
int AdjustInSectorSize(int iFd, int iSourceSize)
{
	int i;
	int iAdjustSizeToSector;
	char cCh;
	int iSectorCount;
	
	iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
	cCh = 0x00;
	
	if(iAdjustSizeToSector!=0)
	{
		iAdjustSizeToSector = 512 - iAdjustSizeToSector;
		printf("[INFO] File size[%lu] and fill [%u] byte\n"
			,iSourceSize, iAdjustSizeToSector);
		for(i=0; i<iAdjustSizeToSector;i++)
		{
			write(iFd, &cCh, 1);
		}
		
	}
	else
	{
		printf("[INFO] File size is aligned 512 byte\n");
	}
	
	iSectorCount = (iSourceSize + iAdjustSizeToSector) / BYTESOFSECTOR;
	return iSectorCount;
}
//Insert Kernel information to Bootloader
void WriteKernelInformation(int iTargetFd, int iKernelSectorCount)
{
	unsigned short usData;
	long lPosition;
	
	//5byte offset from file, total sector of kernel
	lPosition = lseek(iTargetFd, 5, SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "lseek failed. Return value = %d,errno=%d, %d\n"
			, lPosition, errno, SEEK_SET);
		exit(-1);
	}
	
	usData=(unsigned short) iKernelSectorCount;
	write(iTargetFd, &usData, 2);
	
	printf("[INFO] Total sectour count except bootloader [%d]\n"
		,iKernelSectorCount);
	
}


int CopyFile(int iSourceFd, int iTargetFd)
{
	int iSourceFileSize;
	int iRead;
	int iWrite;
	char vcBuffer[BYTESOFSECTOR];
	
	iSourceFileSize = 0;
	
	while(1)
	{
		iRead = read(iSourceFd, vcBuffer, sizeof(vcBuffer));
		iWrite = write(iTargetFd ,vcBuffer, iRead);
		
		if(iRead != iWrite)
		{
			fprintf(stderr, "[ERROR] iRead != iWrite..\n");
			exit(-1);
		}
		iSourceFileSize += iRead;
		
		if(iRead != sizeof(vcBuffer))
		{
			break;
		}
	}
	return iSourceFileSize;
}
