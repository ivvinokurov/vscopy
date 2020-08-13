#include "pch.h"
#include <iostream>
#include <string>
#include <windows.h>

#include "vs2comlib.h"
#include "vs2io.h"

using namespace std;

/************************ VIRTUAL METHODS *******************************/
// Get crc32 - VIRTUAL
unsigned int vs2io::get_crc32(const long long position, const long long length) { return 0; }

// Read bytes
void vs2io::read(const long long len, unsigned char * data) {}

// Write bytes
void vs2io::write(const long long length, unsigned char * data){}

// Flush - VIRTUAL
void vs2io::flush(){}

// Close - VIRTUAL
void vs2io::close() {}

// Get position - VIRTUAL
long long vs2io::get_position() { return 0; }

// Set position - VIRTUAL
void vs2io::set_position(const long long value){}

// Get stream length - VIRTUAL
long long vs2io::get_length() { return 0; }

/************************ NON-VIRTUAL READ METHODS *********************/

// Read ALL bytes
void vs2io::read_all(unsigned char * data)
{
	this->set_position(0);
	this->read(this->get_length(), data);
}

// Read short
short vs2io::read_short()
{
	unsigned char b[2];
	read(2, b);
	return vs2comlib::convert_char_to_short(b);
}

// Read int
int vs2io::read_int()
{
	unsigned char b[4];
	read(4, b);
	return vs2comlib::convert_char_to_int(b);
}

// Read long
long long vs2io::read_long()
{
	unsigned char b[8];
	read(8, b);
	return vs2comlib::convert_char_to_long(b);
}

// Read string
string vs2io::read_string(const long long len)
{
	unsigned char * b = (unsigned char *)malloc((size_t)len);
	read(len, b);
	string s = vs2comlib::convert_char_to_string(b, 0, len);
	free(b);
	return s;
}

// Read byte array
vector<unsigned char> vs2io::read_byte_array(long long len)
{
	unsigned char* b = (unsigned char*)malloc((size_t)len);
	read(len, b);
	vector<unsigned char> v = vector<unsigned char>((size_t)len);
	for (long long i = 0; i < len; i++)
		v[(size_t)i] = *(b + i);
	free(b);
	return v;
}

/************************ NON-VIRTUAL WRITE METHODS *********************/
// Write short
void vs2io::write_short(const short data)
{
	unsigned char b[2];
	vs2comlib::convert_short_to_char(data, b);
	write(2, b);
}

// Write int
void vs2io::write_int(const int data)
{
	unsigned char b[4];
	vs2comlib::convert_int_to_char(data, b);
	write(4, b);
}

// Write long
void vs2io::write_long(const long long data)
{
	unsigned char b[8];
	vs2comlib::convert_long_to_char(data, b);
	write(8, b);
}

// Write string
void vs2io::write_string(const string &data)
{
	unsigned char * b = (unsigned char *)malloc(data.size());
	vs2comlib::convert_string_to_char(data, b);
	write((long long)data.size(), b);
	free(b);
}

/// Write byte array
void vs2io::write_byte_array(const vector<unsigned char> &data)
{
	int s = (int)data.size();
	unsigned char * b = (unsigned char *)malloc(s);
	if (b)
	{
		for (int i = 0; i < s; i++)
			b[i] = data[i];
		write((long long)s, b);
		free(b);
	}
}

/************************ NON-VIRTUAL OTHER METHODS *********************/
//Stream encrypted
bool vs2io::encrypted() { return p_encrypt; }

// Turn encryption on/off
void vs2io::set_encryption(const bool value) { p_encrypt = xkey_b_len > 0 ? value : false; }

//Stream name
string vs2io::get_name() { return name; }

// If stream is opened
bool vs2io::opened() { return p_opened; }
 