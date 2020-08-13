#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>

using namespace std;


class vs2io
{
protected:
	bool p_encrypt = false;					// Encryption indicator
	bool p_opened = false;					// Open status
	unsigned char xkey_b[256] = { 0 };		// Encryption key
	int xkey_b_len = 0;						// Keylen
	long long current_size = 0;				// Total uses space  (to control boundaries in read/write)

public:
	string name;							// IO name (IMO name or file name)
	string error_msg = "";					// Error messare if occured
	
	//////////////// VIRTUAL METHODS ///////////////////////////////////
	// Calculate CRC32 
	// position:	start position, -1 - from the current position
	// length:		length, -1 to end of stream
	virtual unsigned int get_crc32(const long long position, const long long length);

	// Read bytes
	virtual void read(const long long length, unsigned char * data);

	// Write bytes
	virtual void write(const long long length, unsigned char * data);

	// Flush stream
	virtual void flush();

	// Close stream
	virtual void close();

	// Position
	virtual long long get_position();

	virtual void set_position(const long long value);

	// Length
	virtual long long get_length();

	//////////////// NON-VIRTUAL READ METHODS ////////////////////
	// Read ALL bytes
	void read_all(unsigned char * data);

	// Read short
	short read_short();

	// Read int
	int read_int();

	// Read long
	long long read_long();

	// Read string
	string read_string(const long long len);

	// Read byte array
	vector<unsigned char> read_byte_array(long long len);

	//////////////// NON-VIRTUAL WRITE METHODS ////////////////////
	// Write short
	void write_short(const short data);

	// Write int
	void write_int(const int data);

	// Write long
	void write_long(const long long data);

	// Write string
	void write_string(const string &data);

	// Write byte array
	void write_byte_array(const vector<unsigned char> &data);

	//////////////// NON-VIRTUAL OTHER METHODS ////////////////////

	/// Encrypt or not content
	bool encrypted();

	void set_encryption(const bool value);

	/// <summary>
	/// File name with path
	/// </summary>
	string get_name();

	/// <summary>
	/// If stream is opened
	/// </summary>
	bool opened();

};
