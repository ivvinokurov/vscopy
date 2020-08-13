#pragma once

#include <string>
#include <vector>

#include "vs2iof.h"
#include "vs2iom.h"

	class vs2trans final
	{
	private:
		vs2iof IO;

		string p_path = "";

		static const int MODE_NONE =    0;					// Transaction is not active
		static const int MODE_STARTED = 1;					// Transaction is in progress
		static const int MODE_ROLL =   -1;					// Transaction is in rollback mode

		long long p_current_pos = -1; // Current position

		bool p_eof = false;

		int p_mode = MODE_NONE;								// Transaction current mode

		string p_key = "";

		// Transaction level 1 (physical) record definitions
	public:
		///// Transaction record //////
		long long T_id = -1;				// Space ID
		long long T_address = -1;			// Base address
		long long T_length = -1;			// Data package length
		unsigned char * T_data = nullptr;	// Data package
		
		long long buf_length = 0;			// Buffer length
										//////////////////////////////////////////////////////
		// +4(length -8) TA_RECORD
		// +length-4)(4) length
		//////////////////////////////////////////////////////

		// Empty _ta_file - In Memory Option

		vs2trans(const string &ta_file, const string &key);

		vs2trans();

		////////////////////////////////////////////////////////////////
		////////////////  TRANSACTION LOG MODE METHODS /////////////////
		////////////////////////////////////////////////////////////////

		// Begin transaction 
		void Begin();

		/// Write record to transaction log
		void Write(const long long id, const long long address, const unsigned char * data, const long long length);

		// Commit - close and truncate all files
		void Commit();

		
		////////////////////////////////////////////////////////////////
		////////////////  ROLLBACK MODE METHODS ////////////////////////
		////////////////////////////////////////////////////////////////
		
		// Open for Rollback
		void Open();

		// Read record from log
		bool Read();

		// Close and cleanup log
		void Close();

		/////////////////////////////////////////////////
		/////////////// PROPERTIES //////////////////////
		/////////////////////////////////////////////////

		// Transaction state: true - opened; false - no
		bool Started() const;

		// Transaction roll mode: true - yes; false - no
		bool RollMode() const;

		// Check is rollback pending 
		bool Pending();

		/////////////////////////////////////////////////
		/////////////// GET RECORD FIELDS ///////////////
		/////////////////////////////////////////////////

		long long get_T_id();

		long long get_T_address();

		long long get_T_length();

		void get_T_data(unsigned char * data);
	};
