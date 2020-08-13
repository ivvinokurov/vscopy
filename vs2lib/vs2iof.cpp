#include "pch.h"
#include <iostream>
#include <fstream>
#include <io.h>
#include <direct.h>
#include <Windows.h>

#include "vs2iof.h"
#include "vs2comlib.h"
#include "vs2iolib.h"
#include "vs2crc32.h"
#include "vs2crypto.h"

using namespace std;


// Constructor
vs2iof::vs2iof()
{
	file_info.time_write = 0;
	file_info.size = 0;
	file_info.attrib = 0;
	file_info.name[0] = 0x00;
	p_mode = vs2iolib::FILE_MODE_UNDEFINED;
	p_encrypt = false;

}

// Open
string vs2iof::open(const string& name, const char mode, const string key)
{
	if (p_opened)
		return "";

	this->name = name;

	if (mode != vs2iolib::FILE_MODE_UNDEFINED)
		this->p_mode = mode;

	if (key != (vs2iolib::ENCRYPTION_UNDEFINED))
	{
		p_encrypt = false;

		if (key != "")
		{
			xkey_b_len = (int)key.size();

			if (xkey_b_len > 0)
			{
				vs2comlib::convert_string_to_char(key, (unsigned char*)&xkey_b);
				p_encrypt = true;
			}

		}
	}

	intptr_t handle;

	if ((handle = _findfirst64(this->name.c_str(), &file_info)) == -1L)
	{
		file_info.time_write = 0;
		file_info.size = 0;
		file_info.attrib = 0;
		file_info.name[0] = 0x00;
	}

	_findclose(handle);

	if (this->p_mode == vs2iolib::FILE_MODE_CREATE)
		p_fs.open(name, std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
	else if (this->p_mode == vs2iolib::FILE_MODE_APPEND)
		p_fs.open(name, std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::app);
	else if (this->p_mode == vs2iolib::FILE_MODE_OPEN)
		p_fs.open(name, std::fstream::binary | std::fstream::in | std::fstream::out);
	else
		return "Invalid open mode (not c/o/a)";

	if (p_fs.is_open())
	{
		p_opened = true;
		this->current_size = this->get_length();
		this->error_msg = "";
	}
	else
	{
		p_opened = false;
		this->error_msg = "Error when opening: " + this->name;
	}

	return this->error_msg;
}


//////////////// READ METHODS ////////////////////

// Read bytes
void vs2iof::read(const long long length, unsigned char * data)
{
	if (!p_opened)
		throw "Invalid operation 'read', stream is not opened";

	long long pos = p_fs.tellg();

	if ((pos + length) > this->current_size)
		throw exception("Out of stream (read)");

	p_fs.read((char *)data, length);

	if (is_r_crc)
		r_crc = vs2crc32::add_crc(r_crc, data, length);

	if (!p_encrypt)
		return;

	// Decrypt data
	vs2crypto::decrypt(data, data, pos, length, xkey_b, xkey_b_len);

	return;
}

//////////////// WRITE METHODS ///////////////////

void vs2iof::write(const long long length, unsigned char * data)
{
	if (!p_opened)
		throw "Invalid operation 'write', stream is not opened";

	long long offs = p_fs.tellp();

	if (p_encrypt)
	{
		unsigned char * e_data = (unsigned char *)malloc((size_t)length);


		vs2crypto::encrypt(e_data, data, offs, length, xkey_b, xkey_b_len);

		p_fs.write((const char *)e_data, length);

		if (is_w_crc)
			w_crc = vs2crc32::add_crc(w_crc, e_data, length);

		free(e_data);
	}
	else
	{
		p_fs.write((const char *)data, length);

		if (is_w_crc)
			w_crc = vs2crc32::add_crc(w_crc, data, length);
	}

	if (this->get_position() > current_size)
		current_size = this->get_position();

}

//////////////// OTHER METHODS ///////////////////

/// <summary>
/// Checks if file exists (not opened)
/// </summary>
bool vs2iof::file_exists()
{
	if (this->p_opened)
		return true;

	return vs2iolib::file_exists(this->name);
}


/// <summary>
/// Copy file if file exists (not opened)
/// </summary>
bool vs2iof::file_copy(string to)
{
	const long long block_size = 65536;

	if (this->p_opened)
		return false;

	if (!this->file_exists())
		return false;

	if (name == to)
		return false;

	if (!this->file_exists())
		return false;

	if (vs2iolib::file_exists(to))
		remove(to.c_str());
	
	fstream fs_in;
	fstream fs_out;

	long long file_len = vs2iolib::get_file_length(name);

	fs_in.open (name, std::fstream::binary | std::fstream::in);
	fs_out.open(to, std::fstream::binary | std::fstream::out);

	char * buf = (char *)malloc(block_size);

	while (file_len > 0)
	{
		long long inc = (file_len > block_size) ? block_size : file_len;
		fs_in.read(buf, inc);
		fs_out.write(buf, inc);
		file_len -= inc;
	}

	free(buf);
	fs_in.close();
	fs_out.close();

	return true;
}



/// <summary>
/// Remove file (if not opened)
/// </summary>
bool vs2iof::rmfile()
{
	if (this->p_opened)
		return false;

	return (remove(name.c_str()) == 0);
}

void vs2iof::flush()
{
	if (p_opened)
		p_fs.flush();
}

void vs2iof::close()
{
	if (p_opened)
	{
		p_fs.close();
		vs2io::close();
		p_opened = false;
	}
}

long long vs2iof::get_position()
{
	if (!p_opened)
		throw "Invalid operation 'get_position', stream is not opened";

	return p_fs.tellg();
}

void vs2iof::set_position(long long value)
{
	if (!p_opened)
		throw "Invalid operation 'set_position', stream is not opened";
	
	p_fs.seekg(value);
}

// Get stream length
long long vs2iof::get_length()
{
	if (!p_opened)
		return file_info.size;

	long long savep = p_fs.tellg();
	
	p_fs.seekg(0, ios::end);

	long long endp = p_fs.tellg();

	p_fs.seekg(savep, ios::beg);

	return endp;
}

// Last Write timestamp
long long vs2iof::get_last_write_time()
{
	return vs2iolib::get_last_write_time(this->name);
}

// Get file attributes
unsigned int vs2iof::get_fs_object_attributes()
{
	return vs2iolib::get_fs_object_attributes(this->name);
}

// Set file attributes
void vs2iof::set_fs_object_attributes(const unsigned int attrs)
{
	vs2iolib::set_fs_object_attributes(this->name, attrs);
}

// Set file last write time
void vs2iof::set_file_last_write_time(const long long timestamp)
{
	vs2iolib::set_file_last_write_time(this->name, timestamp);
}

// Calculate CRC32
unsigned int vs2iof::get_crc32(long long position, long long length)
{
	if (!p_opened)
		throw "Invalid operation 'get_crc32', stream is not opened";

	constexpr int chunk = 1024;
	unsigned char b[chunk];

	// Save position
	long long save_pos = p_fs.tellg();

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

	p_fs.seekg(pos, ios::beg);

	while (len > 0)
	{
		int l = static_cast<int>((len <= chunk) ? len : chunk);

		read(l, b);

		c = vs2crc32::add_crc(c, b, l);

		len -= l;
	}

	// Restore position
	p_fs.seekg(save_pos, ios::beg);

	return vs2crc32::end_crc(c);
}

// Get mode
unsigned char vs2iof::get_mode()
{
	return p_mode;
}

// Set mode
void vs2iof::set_mode(unsigned char mode)
{

	if (!p_opened)
		p_mode = mode;
}

////////// CRC Methods ///////////////
// Begin crc calc
void vs2iof::begin_crc(char mode)
{
	if (mode == vs2iolib::CRC_READ)
	{
		is_r_crc = true;
		r_crc = vs2crc32::begin_crc();
	}
	else
	{
		is_w_crc = true;
		w_crc = vs2crc32::begin_crc();
	}
}

// End crc calc
unsigned int vs2iof::end_crc(char mode)
{
	if (mode == vs2iolib::CRC_READ)
	{
		if (!is_r_crc)
			throw "Invalid 'end_crc' - READ CRC is not in progress";

		r_crc = vs2crc32::end_crc(r_crc);

		is_r_crc = false;
		
		return r_crc;
	}
	else 
	{
		if (!is_w_crc)
			throw "Invalid 'end_crc' - WRITE CRC is not in progress";

		w_crc = vs2crc32::end_crc(w_crc);

		is_w_crc = false;

		return w_crc;
	}
}

// Check crc status
bool vs2iof::is_crc(char mode)
{
	return (mode == vs2iolib::CRC_READ)? is_r_crc : is_w_crc;
}

