// Common library V2
#include "pch.h"
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>  
#include <direct.h>
#include <time.h>
#include "vs2comlib.h"

using namespace std;

#define _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
static const long long BS_BUFFER_SIZE = 256;
//////////////////////////////////////////////////////////////////////
////////////////////////    Conversions   ////////////////////////////
//////////////////////////////////////////////////////////////////////
const unsigned char hd[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

//////////////// TO BYTE ARRAY CONVERSIONS //////////////////////////
// High-to-low (sort order)
void vs2comlib::convert_long_to_char(const long long n, unsigned char* b)
{
	b[0] = (n >> 56) & 0xFF;
	b[1] = (n >> 48) & 0xFF;
	b[2] = (n >> 40) & 0xFF;
	b[3] = (n >> 32) & 0xFF;
	b[4] = (n >> 24) & 0xFF;
	b[5] = (n >> 16) & 0xFF;
	b[6] = (n >> 8) & 0xFF;
	b[7] = n & 0xFF;
}

// As Is in memory
void vs2comlib::convert_long_to_char_asis(const long long n, unsigned char* b)
{
	unsigned char* bs = (unsigned char*)&n;
	for (int i = 0; i < 8; i++)
		b[i] = bs[i];
}

// High-to-low (sort order)
void vs2comlib::convert_int_to_char(const int n, unsigned char* b)
{
	b[0] = (n >> 24) & 0xFF;
	b[1] = (n >> 16) & 0xFF;
	b[2] = (n >> 8) & 0xFF;
	b[3] = n & 0xFF;
}

// As Is in memory
void vs2comlib::convert_int_to_char_asis(const int n, unsigned char* b)
{
	unsigned char* bs = (unsigned char*)&n;
	for (int i = 0; i < 4; i++)
		b[i] = bs[i];
}

// High-to-low (sort order)
void vs2comlib::convert_short_to_char(short int n, unsigned char* b)
{
	b[0] = (n >> 8) & 0xFF;
	b[1] = n & 0xFF;
}

// As Is in memory
void vs2comlib::convert_short_to_char_asis(short int n, unsigned char* b)
{
	unsigned char* bs = (unsigned char*)&n;
	for (int i = 0; i < 2; i++)
		b[i] = bs[i];
}

void vs2comlib::convert_string_to_char(const string& value, unsigned char* b)
{
	unsigned long int l = (unsigned long int)value.size();
	const char* p = value.c_str();
	for (unsigned long int i = 0; i < l; i++)
		b[i] = p[i];
}

//////////////// FROM BYTE ARRAY CONVERSIONS //////////////////////////

// High-to-low (sort order)
long long vs2comlib::convert_char_to_long(const unsigned char* value)
{
	unsigned char b[8];
	for (int i = 0; i < 8; i++)
		b[i] = value[7 - i];

	return (long long)*((long long*)b);
}

// As Is in memory
long long vs2comlib::convert_char_to_long_asis(const unsigned char* value)
{
	return (long long)*((long long*)value);
}

// High-to-low (sort order)
int vs2comlib::convert_char_to_int(const unsigned char* value)
{
	unsigned char b[4];
	for (int i = 0; i < 4; i++)
		b[i] = value[3 - i];

	return (int)*((int*)b);
}

// As Is in memory
int vs2comlib::convert_char_to_int_asis(const unsigned char* value)
{
	return (int)*((int*)value);
}

// High-to-low (sort order)
short vs2comlib::convert_char_to_short(const unsigned char* value)
{
	unsigned char b[2];
	for (int i = 0; i < 2; i++)
		b[i] = value[1 - i];

	return (int)*((int*)b);
}

// As Is in memory
short vs2comlib::convert_char_to_short_asis(const unsigned char* value)
{
	return (int)*((int*)value);
}

string vs2comlib::convert_char_to_string(const unsigned char* value, long long index, long long length)
{
	if (index == 0)
	{
		std::string s((char*)value, (size_t)length);
		return s;
	}
	else
	{
		const unsigned char* p = value + index;
		std::string s((char*)p, (size_t)length);
		return s;

	}
}

//////////////////////// TO HEX CONVERSIONS //////////////////////////

// Convert byte array to hex string representation (private)
void vs2comlib::convert_char_to_hex_char(const unsigned char* source, const int length, unsigned char* target)
{
	for (int i = 0; i < length; i++)
	{
		unsigned char y = source[i];

		unsigned char x = y >> 4;
		target[i * 2] = hd[x];

		x = y & 0x0f;
		target[(i * 2) + 1] = hd[x];
	}
}

string vs2comlib::convert_short_to_hex_string(short value)
{
	unsigned char b[2];

	unsigned char bh[4];

	convert_short_to_char(value, b);

	convert_char_to_hex_char(b, 2, bh);

	return convert_char_to_string(bh, 0, 4);
}

string vs2comlib::convert_int_to_hex_string(int value)
{
	unsigned char b[4];

	unsigned char bh[8];

	convert_int_to_char(value, b);

	convert_char_to_hex_char(b, 4, bh);

	return convert_char_to_string(bh, 0, 8);
}

string vs2comlib::convert_long_to_hex_string(long long value)
{
	unsigned char b[8];
	unsigned char bh[16];

	convert_long_to_char(value, b);

	convert_char_to_hex_char(b, 8, bh);

	return convert_char_to_string(bh, 0, 16);
}

string vs2comlib::convert_string_to_hex_string(const std::string& value)
{
	size_t l = value.size();

	unsigned char* b = (unsigned char*)malloc(l * 3);

	convert_string_to_char(value, b);

	convert_char_to_hex_char(b, (int)value.length(), b + l);

	string s = convert_char_to_string(b + l, (long long)0, l * (long long)2);

	free(b);

	return s;
}

// Convert byte array to hex string
string vs2comlib::convert_char_to_hex_string(const unsigned char* value, long long index, long long length)
{
	string s = convert_char_to_string(value, index, length);
	return convert_string_to_hex_string(s);
}

//////////////// SIMPLE TYPES CONVERSIONS (TO/FROM STRING) //////////////////////////

long long vs2comlib::convert_string_to_long(const string& value)
{
	return  value == ""? 0 : std::stoll(value);
}

int vs2comlib::convert_string_to_int(const string& value)
{
	return value == ""? 0 : std::stoi(value);
}

float vs2comlib::convert_string_to_float(const string& value)
{
	return static_cast<float>(std::stod(value));
}

double vs2comlib::convert_string_to_double(const string& value)
{
	return std::stod(value);
}

//////////////////////////////////////////////////////////////////////
////////////////////////    Operations    ////////////////////////////
//////////////////////////////////////////////////////////////////////

// Parse string
vector<string> vs2comlib::parse(const std::string& value, const std::string& delimiters)
{
	if (value.length() == 0)
	{
		return std::vector<std::string>(0);
	}

	std::vector<std::string> ret(0);

	std::string s = "";

	short pos = 0;

	for (unsigned int i = 0; i < value.length(); i++)
	{
		bool delim = false;

		for (unsigned int j = 0; j < delimiters.length(); j++)
		{

			if (value.substr(i, 1) == delimiters.substr(j, 1))
			{
				if (s.length() > 0)
				{
					ret.push_back(s);
				}

				s = "";

				delim = true;

				break;
			}
		}

		if (!delim)
		{
			s += value.substr(i, 1);
		}
	}

	if (s.length() > 0)
	{
		ret.push_back(s);
	}
	return ret;
}

// Parse string extended
vector<string> vs2comlib::parse_ext(const string& value, const string& delimiters)
{
	if (value.length() == 0)
		return vector<string>(0);

	vector<string> ret(0);

	string s = "";

	short pos = 0;

	string qt = " ";

	unsigned int i = 0;

	while (i < value.length())
	{
		if (qt == " ")
		{ // Not quoted text
			if ((value.substr(i, 1) == "'") | (value.substr(i, 1) == "\""))
			{
				qt = value.substr(i, 1);
			}
			else
			{
				bool delim = false;

				for (unsigned int j = 0; j < delimiters.length(); j++)
				{

					if (value.substr(i, 1) == delimiters.substr(j, 1))
					{
						if (s.length() > 0)
						{
							ret.push_back(s);
						}

						s = "";

						delim = true;

						break;
					}
				}

				if (!delim)
					s += value.substr(i, 1);
			}
		}
		else
		{ // Quoted text
			if (value.substr(i, 1) == qt)
			{
				if (value.size() > ((size_t)i + 1))		// Not last char
				{
					if (value.substr((size_t)i + 1, 1) == qt)
					{							// Double quote
						s += qt;
						i++;
					}
					else
						qt = " ";				// Single quote
				}
			}
			else
				s += value.substr(i, 1);
		}

		i++;
	}

	if (s.length() > 0)
	{
		ret.push_back(s);
	}
	return ret;
}


// Compare values
bool vs2comlib::compare(const std::string& pattern, const std::string& value)
{
	if (pattern.compare(value) == 0)
	{
		return true;		// Equal
	}
	else if (value.empty())
	{
		if (pattern == "*")
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (pattern.length() == 0)
	{
		return false;
	}
	else if (pattern[0] == '?')
	{
		return compare(pattern.substr(1), value.substr(1));
	}
	else if (pattern[pattern.length() - 1] == '?')
	{
		return compare(pattern.substr(0, pattern.length() - 1), value.substr(0, value.length() - 1));
	}
	else if (pattern[0] == '*')
	{
		if (compare(pattern.substr(1), value))
		{
			return true;
		}
		else
		{
			return compare(pattern, value.substr(1));
		}
	}
	else if (pattern[pattern.length() - 1] == '*')
	{
		if (compare(pattern.substr(0, pattern.length() - 1), value))
		{
			return true;
		}
		else
		{
			return compare(pattern, value.substr(0, value.length() - 1));
		}
	}
	else if (pattern[0] == value[0])
	{
		return compare(pattern.substr(1), value.substr(1));
	}
	return false;
}

// Compare binary keys
int vs2comlib::compare_keys(unsigned char* x, int keylenx, unsigned char* y, int keyleny, bool partial)
{

	int l = keylenx;

	if (keylenx > keyleny)
		l = keyleny;

	for (int i = 0; i < l; i++)
	{
		unsigned char cx = x[i];
		unsigned char cy = y[i];
		if (cx > cy)
		{
			return 1;
		}
		else if (cx < cy)
		{
			return -1;
		}
	}
	if (!partial)
	{
		if (keylenx > keyleny)
		{
			return 1;
		}
		else if (keylenx < keyleny)
		{
			return -1;
		}
	}
	return 0;
}


void vs2comlib::copy_char_array(unsigned char* target, long long target_offset, unsigned char* source, long long source_offset, long long length)
{
	unsigned char* ta = target + target_offset;
	unsigned char* so = source + source_offset;
	for (long long i = 0; i < length; i++)
	{
		*(ta + i) = *(so + i);
	}

}

//////////////////////////////////////////////////////////////////////
//////////////////////// Basic functions  ////////////////////////////
//////////////////////////////////////////////////////////////////////

// Convert to Lowercase
string vs2comlib::to_lower(const std::string& str)
{
	std::string s = str;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

// Convert to Uppercase
string vs2comlib::to_upper(const std::string& str)
{
	std::string s = str;
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

// Pad left by chars
string vs2comlib::pad_left(const string& str, const unsigned int len, const string sym)
{
	if (str.length() >= len)
		return str;

	std::string s = str;

	for (unsigned int i = 0; i < (len - str.length()); i++)
		s.insert(0, sym);

	return s;
}

// Pad right by chars
string vs2comlib::pad_right(const string& str, const unsigned int len, const string sym)
{
	if (str.length() >= len)
		return str;

	std::string s = str;

	for (unsigned int i = 0; i < (len - str.length()); i++)
		s.append(sym);

	return s;
}

// Trim all spaces
string vs2comlib::trim(const string& str)
{
	const std::string whitespace(" \t");

	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

// Check if string represents number
bool vs2comlib::is_numeric(const string& value)
{
	bool is_d = true;

	unsigned char* d = (unsigned char*)malloc(256);

	if (d)
	{
		convert_string_to_char(value, d);

		for (size_t i = 0; i < value.size(); i++)
		{
			if (d[i] > 0x09)
			{
				is_d = false;
				break;
			}
		}

		free(d);

		return is_d;
	}

	return false;
}

// Get current date-time representation
string vs2comlib::current_datetime()
{
	char buffer[256];
	struct tm newtime;
	__time64_t long_time;

	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(buffer, 256, "%Y-%m-%d %H:%M:%S.", &newtime);
	string st(buffer);
	return st.erase(st.size() - 1);
}

// Get current time representation
string vs2comlib::current_time()
{
	char buffer[256];
	struct tm newtime;
	__time64_t long_time;

	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(buffer, 256, "%H:%M:%S.", &newtime);
	string st(buffer);
	return st.erase(st.size() - 1);
}


//////////////////////// BIT STEAM OPERATIONS ////////////////////////
// Set bits
// Write bit data from the 'source' starting at bit 0 to 'target' starting at bit 'pos'
// Parms:
// src - source data (bits starts from 0)
// trg - target data (bits starts from 'pos')
// pos - target bit index (0..n)
// len - number of bits
void vs2comlib::bs_set_bits(unsigned char* src, unsigned char* trg, unsigned long int pos, unsigned long int len)
{
	//static const unsigned char bit_set_mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	//static const unsigned char bit_clr_mask[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

	// Suppress masks
	//static const unsigned char bit_tail_mask[8] = { 0xff, 0xf7, 0xf3, 0xf1, 0xf0, 0x70, 0x30, 0x10 };

	static const unsigned char bs_bit_mask[8] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

	unsigned long int src_byte_pos = 0;								// Source: next byte index

	unsigned long int trg_byte_pos = pos >> 3;						// Target: byte index
	unsigned long int trg_bit_pos = pos & 0x00000007;				// Target: relative bit index in byte
	unsigned long int trg_bit_cnt = 0;								// Target bit counter

	unsigned short int w2 = 0x0000;									// Byte to write + next byte
	unsigned char* p_w2 = (unsigned char*)(&w2);

	// Set 1st byte if starts not from 1st bit
	if (trg_bit_pos > 0)
	{
		// Set left bits
		p_w2[0] = src[src_byte_pos];

		if (len < 8)
			p_w2[0] &= (bs_bit_mask[len] ^ 0xff);

		unsigned long int tail_bits = trg_bit_pos + len;

		unsigned char b_mask = (bs_bit_mask[trg_bit_pos] ^ 0xff);	// Zero right bits

		if (tail_bits < 8)
			b_mask |= (bs_bit_mask[tail_bits]);					// Zero left bits of the last byte (target)

		trg[trg_byte_pos] &= b_mask;								// Zero 1st byte used bits 

		w2 >>= trg_bit_pos;											// Shift unused bits

		trg[trg_byte_pos] |= p_w2[0];								// Set bits

		trg_byte_pos++;												// Shift byte pos if start not at 0-bit

		trg_bit_cnt += (8 - trg_bit_pos);							// Increase bit counter
	}

	while (trg_bit_cnt < len)
	{
		unsigned long int bl = ((len - trg_bit_cnt) > 8) ? 8 : (len - trg_bit_cnt);

		p_w2[1] = src[src_byte_pos];

		if (((src_byte_pos + 1) * 8) < len)
			p_w2[0] = src[src_byte_pos + 1];						// Load 2-nd byte

		if (trg_bit_pos > 0)										// Start pos !=0 - shift
			w2 <<= (8 - trg_bit_pos);

		if (bl == 8)												// Full byte
			trg[trg_byte_pos] = p_w2[1];
		else
		{
			trg[trg_byte_pos] &= (bs_bit_mask[bl]);				// Zero left bits of the last byte (target)
			p_w2[1] &= (bs_bit_mask[bl] ^ 0xff);					// Zero right bits of the last byte (source)
			trg[trg_byte_pos] |= p_w2[1];							// Set bits
		}

		src_byte_pos++;
		trg_byte_pos++;

		trg_bit_cnt += bl;
	}
}

// Get bits
// Write bit data from the 'source' starting at bit 'pos' to 'target' starting at bit 0
// Parms:
// src - source data (bits starts from 'pos')
// trg - target data (bits starts from 0)
// pos - source bit index (0..n)
// len - number of bits
void vs2comlib::bs_get_bits(unsigned char* src, unsigned char* trg, unsigned long int pos, unsigned long int len)
{
	static const unsigned char bs_bit_mask[9] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };

	unsigned long int src_byte_pos = pos >> 3;					// Source: next byte index
	unsigned long int src_bit_pos = pos & 0x00000007;			// Source: bit index within byte

	unsigned long int trg_byte_pos = 0;							// Target: byte index
	//unsigned long int trg_bit_pos =		0;						// Target: relative bit index in byte
	unsigned long int trg_bit_cnt = 0;							// Target bit counter

	unsigned short int w2 = 0x0000;									// Byte to write + next byte
	unsigned char* p_w2 = (unsigned char*)(&w2);

	// Set 1st byte if starts not from 1st bit
	while (trg_bit_cnt < len)
	{
		p_w2[1] = src[src_byte_pos];
		if (src_bit_pos != 0)										// Starts NOT from the byte boundary
		{
			if (trg_bit_cnt <= len)
				p_w2[0] = src[src_byte_pos + 1];					// Load 2-nd byte

			w2 <<= src_bit_pos;
		}

		unsigned long int bl = ((len - trg_bit_cnt) > 8) ? 8 : (len - trg_bit_cnt);

		if (bl < 8)
			p_w2[1] &= (bs_bit_mask[bl] ^ 0xff);	// Set right bits to 0 

		trg[trg_byte_pos] &= bs_bit_mask[bl];

		trg[trg_byte_pos] |= p_w2[1];

		trg_byte_pos++;

		src_byte_pos++;

		trg_bit_cnt += bl;
	}
}

// Copy bits
// Write bit data from the 'source' starting at bit 'src_pos' to 'target' starting at bit trg_pos
// Parms:
// src - source data
// trg - target data
// src_pos - source bit index (0..n)
// trg_pos - target bit index (0..n)
// len - number of bits
void vs2comlib::bs_copy_bits(unsigned char* src, unsigned char* trg, unsigned long int src_pos, unsigned long int trg_pos, unsigned long int len)
{
	if (src_pos == 0)
	{
		bs_set_bits(src, trg, trg_pos, len);
	}
	else if (trg_pos == 0)
	{
		bs_get_bits(src, trg, src_pos, len);
	}
	else
	{
		unsigned char b[BS_BUFFER_SIZE];
		unsigned long int seg = BS_BUFFER_SIZE * 8;
		unsigned long int cnt = 0;
		unsigned long int l = 0;
		while (cnt < len)
		{
			l = ((len - cnt) > seg) ? seg : (len - cnt);
			bs_get_bits(src, b, src_pos + cnt, l);
			bs_set_bits(b, trg, trg_pos + cnt, l);
			cnt += l;
		}
	}
}

// Set bit value
void vs2comlib::bs_set(unsigned char* trg, unsigned long int pos, bool value)
{
	unsigned char b = value ? 0x80 : 0x00;
	bs_set_bits(&b, trg, pos, 1);
}

// Get bit value
bool vs2comlib::bs_get(unsigned char* src, unsigned long int pos)
{
	unsigned char b = 0x00;
	bs_get_bits(src, &b, pos, 1);
	return (b & 0x80) != 0;
}

// Compare bit values
bool vs2comlib::bs_compare(unsigned char* src1, unsigned char* src2, unsigned long int pos1, unsigned long int pos2, unsigned long int len)
{
	unsigned char b1[BS_BUFFER_SIZE];
	unsigned char b2[BS_BUFFER_SIZE];
	unsigned long int seg = BS_BUFFER_SIZE * 8;
	unsigned long int cnt = 0;
	unsigned long int l = 0;
	while (cnt < len)
	{
		l = ((len - cnt) > seg) ? seg : (len - cnt);

		unsigned long int byte_pos = l >> 3;
		unsigned long int bit_pos = l & 0x00000007;

		if (bit_pos != 0)
		{
			b1[byte_pos] = b2[byte_pos] = 0x00;
			byte_pos++;
		}

		bs_get_bits(src1, b1, pos1 + cnt, l);
		bs_get_bits(src2, b2, pos2 + cnt, l);

		cnt += l;

		for (unsigned long int i = 0; i < byte_pos; i++)
		{
			if (b1[i] != b2[i])
				return false;
		}
	}
	return true;
}

// Get integer value
unsigned long long vs2comlib::bs_get_int(unsigned char* src, unsigned long int pos, unsigned long int len)
{
	unsigned long long n = 0;
	bs_get_bits(src, (unsigned char*)&n, pos, len);
	return n;
}

//////////////////////////////////////////////////////////
////////////////// SORT //////////////////////////////////	
//////////////////////////////////////////////////////////

void vs2comlib::q_sort32(long int* items, long int* refs, long int left, long int right, bool asc)
{
	register long int i, j;
	long int x, y;


	i = left; j = right;
	x = items[(left + right) / 2];		// Select comparand 

	do {
		if (asc)
		{
			while ((items[i] < x) && (i < right)) i++;
			while ((x < items[j]) && (j > left)) j--;
		}
		else
		{
			while ((items[i] > x) && (i < right)) i++;
			while ((x > items[j]) && (j > left)) j--;
		}

		if (i <= j) {
			// Switch values
			y = *(items + i);
			*(items + i) = *(items + j);
			*(items + j) = y;

			if (refs != nullptr)
			{
				// Switch refs
				y = *(refs + i);
				*(refs + i) = *(refs + j);
				*(refs + j) = y;
			}

			i++; j--;
		}
	} while (i <= j);

	if (left < j) q_sort32(items, refs, left, j, asc);
	if (i < right) q_sort32(items, refs, i, right, asc);
}

// Main sort
void vs2comlib::sort32(long int* items, long int* refs, long int count, bool asc)
{
	q_sort32(items, refs, 0, count - 1, asc);
}

void vs2comlib::q_sort64(long long* items, long long* refs, long long left, long long right, bool asc)
{
	long long i, j;
	long long x, y;


	i = left; j = right;
	x = items[(left + right) / 2];		// Select comparand 

	do {
		if (asc)
		{
			while ((items[i] < x) && (i < right)) i++;
			while ((x < items[j]) && (j > left)) j--;
		}
		else
		{
			while ((items[i] > x) && (i < right)) i++;
			while ((x > items[j]) && (j > left)) j--;
		}

		if (i <= j) {
			// Switch values
			y = *(items + i);
			*(items + i) = *(items + j);
			*(items + j) = y;

			if (refs != nullptr)
			{
				// Switch refs
				y = *(refs + i);
				*(refs + i) = *(refs + j);
				*(refs + j) = y;
			}

			i++; j--;
		}
	} while (i <= j);

	if (left < j) q_sort64(items, refs, left, j, asc);
	if (i < right) q_sort64(items, refs, i, right, asc);
}

void vs2comlib::sort64(long long* items, long long* refs, long long count, bool asc)
{
	q_sort64(items, refs, 0, count - 1, asc);
}

// Find value in the sorted array
long long vs2comlib::find64(long long* data, long long left, long long size, long long value)
{
	long long m = -1;

	if (size <= 0)
		return m;

	long long lo = left;
	long long hi = left + size;

	while (lo <= hi) {
		m = (hi + lo) / 2;

		if (data[m] < value)
			lo = m + 1;
		else if (data[m] > value)
			hi = m - 1;
		else if (data[m] == value)
			break;
	}

	if (lo > hi)    /* if it doesn't we assign it -1 */
		m = -1;

	return m;
}

// Find value in the sorted array
// Return: 0x80nnnnnnnnnnnnnn - NOT found, nnn - index 'before'; <min = 0; >max=max+1
//		   0x00nnnnnnnnnnnnnn - found, nnn - index (0..n)
long long vs2comlib::find64ge(long long* data, long long size, long long value)
{
	long long m = -1;

	if (size <= 0)
		return 0x8000000000000000;

	if (value < data[0])
		return 0x8000000000000000;


	if (value > data[size - 1])
		return size | 0x8000000000000000;

	long long ret = 0;
	long long lo = 0;
	long long hi = size;

	while (lo <= hi)
	{
		ret = hi;

		m = (hi + lo) / 2;

		if (data[m] < value)
			lo = m + 1;
		else if (data[m] > value)
			hi = m - 1;
		else if (data[m] == value)
			break;
	}

	if (lo > hi)    // Not found
		return (hi + 1) | 0x8000000000000000;

	return m;
}

// Return formatted number with delimiters
string vs2comlib::formatted_number(long long sz)
{
	string ret = "";
	string rv = to_string(sz);
	while (rv.size() > 3)
	{
		ret = ("," + rv.substr(rv.size() - 3, 3)) + ret;
		rv = rv.erase(rv.size() - 3, 3);
	}

	ret = rv + ret;
	return ret;
}

// Return formatted size: bytes, Kb, Mb, Gb, Tb
string vs2comlib::formatted_size(long long sz)
{
	const long double Kb = 1024;
	const long double Mb = Kb * 1024;
	const long double Gb = Mb * 1024;
	const long double Tb = Gb * 1024;

	double la = (long double)sz;

	string rv = " ";
	string n1 = to_string(sz);
	string n2 = "";

	if (sz >= Kb)
	{
		if (sz < Mb)
		{
			la = (long double)sz / Kb;
			rv = "K";
		}
		else if (sz < Gb)
		{
			la = (long double)sz / Mb;
			rv = "M";
		}
		else if (sz < Gb)
		{
			la = (long double)sz / Gb;
			rv = "G";
		}
		else
		{
			la = (long double)sz / Tb;
			rv = "T";
		}
		n1 = to_string(la);
		n2 = n1.substr(n1.size() - 6, 2);
		n1 = n1.erase(n1.size() - 7, 7);

	}

	string x = (n2 == "") ? n1 + rv : n1 + "." + n2 + rv;

	return x;
}

// Find first index of the substring
int vs2comlib::index_of_first(string input, string substring)
{
	if (substring.size() == 0)
		return -1;

	int li = (int)input.size();

	int ls = (int)substring.size();

	for (int i = 0; i < (li - ls); i++)
		if (input.substr(i, ls) == substring)
			return i;

	return -1;
}

// Find last index of the substring
int vs2comlib::index_of_last(string input, string substring)
{
	if (substring.size() == 0)
		return -1;

	int li = (int)input.size();

	int ls = (int)substring.size();

	for (int i = (li - ls); i >= 0; i--)
		if (input.substr(i, ls) == substring)
			return i;

	return -1;
}
