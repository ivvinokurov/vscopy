#pragma once
#include <string>
#include <vector>
#include "vscdefs.h"
#include "vs2iof.h"
#include "vscrout.h"

using namespace std;

class vscfsow final
{
public:
	static const long long SAF_DEFAULT_VOLUME_SIZE = 21474836482;				// Default - 2Gb

	// Open FSO-W object
	string Open(vscrout* router, vscdefs::COMMAND_DEFINITION* command, int attrs = 0);

	// Close FSO-W object
	string Close();

	// Get root name
	string GetName();

	// Set next directory
	string OpenDir(string name, short int level, int attrs);

	// Set next file in directory
	string OpenFile(string name, int attrs, long long length, long long timestamp, string* condition, bool compress);

	// Verify the file name - shall not be the same as input (non-SAF, with path)
	bool VerifyFileName(string name, string in_file_name);

	// Write file data chunk
	// Return "" - ok, otherwise error 
	string WriteData(unsigned char * data, long long length);

	// Close current file
	void CloseFile();

	// Indicator if source is SAF file or no
	bool IsSAF();


private:
	vscdefs::COMMAND_DEFINITION* cmd = nullptr;	// Current command


	static const bool	WDS_FORCE = true;		// Force write dir stack
	static const bool	WDS_FLUSH = false;		// Flush unsaved (if Empty)

	bool				W_opened = false;		// Open indicator

	bool				W_saf =		false;		// SAF indicator
	bool				W_empty = false;		// Empty indicator
	bool				W_file =	false;		// FILE indicator
	unsigned char*		W_parm = nullptr;		// Parm reference
	bool				W_even = false;			// EVEN condition

	bool				W_compressed = false;	// COMPRESS (Target)
	bool				W_encrypted = false;	// ENCRYPT (Target)
	bool				W_is_crc = false;		// CRC indicator
	string				W_name = "";			// Target root
	string				W_key = "";				// Target encryption key
	unsigned char*		W_buffer = nullptr;		// I/O file buffer
	unsigned char*		W_w_buffer = nullptr;	// Temp I/O file buffer
	bool				W_file_opened = false;	// File open indicator

	unsigned char		W_file_mask = 0x00;		// File mask (SAF, ecrypted, compressed

	long long			W_saf_segment_size = 0;	// SAF segment size
	long long			W_saf_segment_numb = 0;	// SAF segment numbere
	unsigned long int	W_crc = 0;				// CRC for out SAF
	
	string				W_saf_core_name = "";	// o-SAF file name w/o extension
	string				W_saf_file_name = "";	// SAF file name (base) with full path and ext
	string				W_root = "";			// Root directory
	int					W_root_attrs = 0;		// Root directory attributes
	int					W_file_attrs = 0;		// Input file attributes
	long long			W_file_timestamp = 0;	// Input file timestamp

	static const int	DEST_UNDEFINED = 0;		// Undefined
	static const int	DEST_FILE = 1;			// File
	static const int	DEST_DIR = 2;			// Directory

	int					W_saf_dest_type = DEST_UNDEFINED;	// Type of destination specified

	// I/O objects
	vs2iof W_saf_io;							// SAF i/o object
	vs2iof W_file_io;							// File i/o object

	vscdefs::FILE_HEADER W_hdr;					// File header

	vscrout*			W_router = nullptr;		// Console/log router
	short int			W_level = 0;			// Current leve relatively to root 

	vector<string>				W_stack_dir;	// Target directory stack
	vector<string>				W_stack_dir_src;// Source directory stack
	vector<bool>				W_stack_save;	// Delayed dir creation indicator (false - not created; true - created)
	vector<unsigned int>		W_stack_attrs;	// Dirs attributes

	// Write file header for SAF/SEC
	void write_header(unsigned int type, vscdefs::FILE_HEADER * hdr, vs2iof * io);

	// Create directory hierarchy (Non-SAF); Write dirs to SAF file (SAF)
	string write_dirs_stack(const bool mode);

	// Check parm for compress mode
	bool is_sec_mode();

	// Check for file extension
	static bool is_file_ext(const string name, const string ext);

	// Check parm byte for compress mode
	bool is_crc_mode();

	// Get full dir path from stack
	string get_relative_path();

	// Check file name for compression requirements
	bool check_name_compress(const string nm, const string cond, const bool is_dir);

};

