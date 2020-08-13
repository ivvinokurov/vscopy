#pragma once
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

class vs2crc32 final
{
	/// <summary>
	/// Table of CRC-32's of all single byte values
	/// </summary>

private:
	static const unsigned int crctab[256];

public:
	static unsigned long int calculate_crc(unsigned char *pBuf, unsigned long long length);

	///  Beging CRC count
	static unsigned long int begin_crc();

	///  End CRC count
	static unsigned long int end_crc(unsigned long int crc);

	/// Calculate crc portion for char array
	static unsigned long int add_crc(unsigned long int oldcrc, unsigned char* pBuf, unsigned long long length);

	/// Calculate crc portion for string
	static unsigned long int add_crc(unsigned long int oldcrc, string value);

	/// Calculate crc portion for long
	static unsigned long int add_crc(unsigned long int oldcrc, long long value);

	/// Calculate crc portion for int
	static unsigned long int add_crc(unsigned long int oldcrc, long int value);

	/// Calculate crc portion for short
	static unsigned long int add_crc(unsigned long int oldcrc, short int value);

	/// Calculate crc portion for char
	static unsigned long int add_crc(unsigned long int oldcrc, unsigned char *value);
};
