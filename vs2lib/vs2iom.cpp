#include "pch.h"
#include "vs2iom.h"
#include "vs2comlib.h"
#include "vs2crc32.h"
#include "vs2crypto.h"

using namespace std;

// Constructor
vs2iom::vs2iom(){}

//Open
string vs2iom::open(const string& name, const string& key, const long long initial_size)
{
	if (!p_opened)
	{
		p_encrypt = false;

		xkey_b_len = (int)key.size();

		if (xkey_b_len > 0)
		{
			vs2comlib::convert_string_to_char(key, (unsigned char*)&xkey_b);
			p_encrypt = true;
		}

		this->name = name;

		this->i_size = initial_size;

		long long n_init = 1;
		if (i_size > 0)
		{
			long long p1 = i_size / P_SIZE;
			long long p2 = i_size ^ P_SIZE;
			if (p2 > 0)
				p1++;
			if (p1 > n_init)
				n_init = p1;
		}

		allocated_size = 0;
		for (int i = 0; i < n_init; i++)
		{
			unsigned char* b = (unsigned char*)malloc(P_SIZE);
			page_table.push_back(b);
			allocated_size += P_SIZE;
		}

		p_opened = true;
	}

	return "";
}

//////////////// READ METHODS ////////////////////

// Read bytes
void vs2iom::read(const long long length, unsigned char * data)
{
	if (!p_opened)
		throw "Invalid operation 'read', stream is not opened";

	long long save_offs = p_position;

	if ((p_position + length) > this->current_size)
		throw exception("Out of stream (read)");

	long long o_offs = 0;
	long long l = length;

	while (l > 0)
	{
		long long pn = p_position / P_SIZE;				// Page number
		long long p_offs = p_position - (pn * P_SIZE);	// Offset within page
		
		long long ln = P_SIZE - p_offs;						// Calculate length Min(remain, page remain)
		if (ln > l)
			ln = l;
		
		// Copy bytes
		vs2comlib::copy_char_array(data, (int)o_offs, page_table[(size_t)pn], p_offs, ln);

		l -= ln;										// Cut remain length
		o_offs += ln;									// Shift target offset
		p_position += length;									// Shift source offset

	}

	p_position = save_offs + length;						// Set new position

	if (!p_encrypt)
		return;

	// Decrypt data
	vs2crypto::decrypt(data, data, save_offs, length, xkey_b, xkey_b_len);

	return;
}

//////////////// WRITE METHODS ///////////////////

void vs2iom::write(const long long length, unsigned char * data)
{
	if (!p_opened)
		throw "Invalid operation 'write', stream is not opened";

	long long save_offs = p_position;

	if (p_position > this->current_size)
		throw exception("Out of stream (write)");


	// if out of space - extend
	if ((p_position + length) > this->allocated_size)
	{
		while ((p_position + length) > this->allocated_size)
		{
			unsigned char * b = (unsigned char *)malloc(P_SIZE);
			page_table.push_back(b);
			allocated_size += P_SIZE;
		}
	}

	long long i_offs = 0;
	long long l = length;

	while (l > 0)
	{
		long long pn = p_position / P_SIZE;					// Page number
		long long p_offs = p_position - (pn * P_SIZE);		// Offset within page

		long long ln = P_SIZE - p_offs;						// Calculate length Min(remain, page remain)
		if (ln > l)
			ln = l;

		// Copy bytes
		vs2comlib::copy_char_array(page_table[(size_t)pn], (int)p_offs, data, i_offs, ln);

		l -= ln;										// Cut remain length
		i_offs += ln;									// Shift target offset
		p_position += length;							// Shift source offset

	}

	p_position = save_offs + length;						// Set new position
	if (p_position > current_size)
		current_size = p_position;

	if (!p_encrypt)
		return;

	// Decrypt data
	vs2crypto::encrypt(data, data, save_offs, length, xkey_b, xkey_b_len);

	return;
}

//////////////// OTHER METHODS ///////////////////

void vs2iom::close()
{
	if (p_opened)
	{

		for (unsigned int i = 0; i < page_table.size(); i++)
			delete(page_table[i]);

		vs2io::close();
	}
}

long long vs2iom::get_position()
{
	if (!p_opened)
		throw "Invalid operation 'get_position', stream is not opened";

	return p_position;
}

void vs2iom::set_position(const long long value)
{
	if (!p_opened)
		throw "Invalid operation 'set_position', stream is not opened";

	if ((value >= 0) & (value < allocated_size))
		p_position = value;
}

// Get stream length
long long vs2iom::get_length()
{
	if (!p_opened)
		throw "Invalid operation 'get_length', stream is not opened";

	return current_size;
}

// Calculate CRC32
unsigned int vs2iom::get_crc32(const long long position, const long long length)
{
	if (!p_opened)
		throw "Invalid operation 'get_crc32', stream is not opened";

	constexpr int chunk = 1024;
	unsigned char b[chunk];

	// Save position
	long long save_pos = position;

	// Get steam length
	long long fslength = get_length();

	// Calculate pos
	long long pos = (position < 0) ? save_pos : position;
	if (pos >= fslength)
	{
		pos = 0;
	}

	// Calculate len
	long long len = (length <= 0) ? (fslength - pos) : length;

	if ((pos + len) > fslength)
	{
		len = fslength - pos;
	}

	if (len == 0)
	{
		return 0;
	}

	// Calculate CRC32
	unsigned int c = vs2crc32::begin_crc();

	p_position = pos;

	while (len > 0)
	{
		int l = static_cast<int>((len <= chunk) ? len : chunk);

		read(l, b);

		c = vs2crc32::add_crc(c, b, l);

		len -= l;
	}

	// Restore position
	p_position = save_pos;

	return vs2crc32::end_crc(c);
}
