#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <io.h>

#include "vs2io.h"
#include "vs2iolib.h"

#define BUF_SIZE 8192;

using namespace std;


	class vs2iof: public vs2io
	{
	private:
		fstream p_fs;
		unsigned char p_mode = 0x00;
		__finddata64_t file_info;

		unsigned long int r_crc = 0;										// Dynamic read crc
		unsigned long int w_crc = 0;										// Dynamic write crc
		bool is_r_crc = false;												// R-crc begin?
		bool is_w_crc = false;												// W-crc begin?

	public:

		// Constructor
		vs2iof();

		// Open
		string open(const string& name, const char mode = vs2iolib::FILE_MODE_UNDEFINED, const string key = vs2iolib::ENCRYPTION_UNDEFINED);

		//////////////// READ METHODS ////////////////////
		// Read bytes
		void read(const long long length, unsigned char * data);

		//////////////// WRITE METHODS ///////////////////

		// Write bytes
		void write(const long long length, unsigned char * data);

		//////////////// OTHER METHODS ///////////////////

		// Checks if file exists (not opened)
		bool file_exists();

		// Copy file if file exists (not opened)
		bool file_copy(string to);

		// Remove file (if not opened)
		bool rmfile();

		// Flush stream
		void flush();

		// Close stream
		void close();

		// Position
		long long get_position();

		void set_position(long long value);

		// Length
		long long get_length();

		// Last Write timestamp
		long long get_last_write_time();

		// Get file attributes
		unsigned int get_fs_object_attributes();


		// Set file attributes
		void set_fs_object_attributes(const unsigned int attrs);

		// Set file last write time
		void set_file_last_write_time(const long long timestamp);

		// Calculate CRC32 
		// <param name="position">Start position, -1 - from the current position</param>
		// <param name="length">Length, -1 to end of stream</param>
		unsigned int get_crc32(long long position, long long length);

		// Begin crc calc
		void begin_crc(char mode);

		// End crc calc
		unsigned int end_crc(char mode);

		// Check if crc is in progress
		bool is_crc(char mode);

		// Get mode
		unsigned char get_mode();

		// Set mode
		void set_mode(unsigned char mode);
	};
