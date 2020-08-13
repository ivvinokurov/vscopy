#pragma once

#include <string>
#include <vector>

#include "vs2io.h"

using namespace std;


	class vs2iom: public vs2io
	{
	private:
		static const long long P_SIZE = 65536;		// Page size
		vector <unsigned char *> page_table;		// Page table
		long long p_position = 0;						// Current position
		long long allocated_size = 0;				// Total allocated space

		long long i_size = 0;


	public:
		vs2iom();

		// Open
		// name - symbolic name
		// key - encryption key ("" - no encryption)
		// initial_size - initial size (bytes). 0 - default (1 page == 64K). Ceiling to 64K.
		string open(const string& name, const string& key = "", const long long initial_size = 0);

		//////////////// READ METHODS ////////////////////
		// Read bytes
		void read(long long len, unsigned char * data);

		//////////////// WRITE METHODS ///////////////////

		// Write bytes
		void write(long long length, unsigned char * data);

		//////////////// OTHER METHODS ///////////////////

		// Close stream
		void close();

		// Position
		long long get_position();

		void set_position(const long long value);

		// Length
		long long get_length();

		/// <summary>
		/// Calculate CRC32 
		/// </summary>
		/// <param name="position">Start position, -1 - from the current position</param>
		/// <param name="length">Length, -1 to end of stream</param>
		/// <returns></returns>
		unsigned int get_crc32(const long long position, const long long length);

	};
