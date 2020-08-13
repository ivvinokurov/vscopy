#pragma once
#include <string>
#include "vscdefs.h"

using namespace std; 

class vscfsor final
{
public:

	// Open FSO-R object
	string Open(vscrout* router, vscdefs::COMMAND_DEFINITION * command);

	// Close FSO-R object
	string Close();

	// Get root name
	string GetName();

	// Get next directory
	vscdefs::DIR_INFO* GetNextDir();

	// Get next file in directory
	vscdefs::FILE_INFO* GetNextFile();

	// Indicator if source is SAF file or no
	bool IsSAF();

	// Indicator if source is file
	bool IsFile();

	// Indicator if compression for target data
	bool IsComressionRequired(string name, bool is_dir = false, short level = -1);

	///////////////////// FILE READ OPERATIONS ///////////////////////

	// Read file data chunk (uncompess and decode if necessary)
	// Return "" - ok, otherwise error 
	vscdefs::FILE_INFO* GetData();

	// Check for file extension
	static bool IsFileExt(const string name, const string ext);

private:

	vscdefs::COMMAND_DEFINITION* cmd = nullptr;

	// Check modes
	static const int	SAF_CHECK_MODE_ALL =	0;		// Check all (-chk option)
	static const int	SAF_CHECK_MODE_TAIL =	1;		// Check file tail

	// Compression header & tail length
	static const long long SAF_HEADER_SIZE = 44;		// SAF header size
	static const long long COMPRESSION_HEADER_SIZE = 32;// Compression block header size (saf;non-saf)
	static const long long CONTROL_FIELD_SIZE = 8;		// CRC/Length

	// VARS
	vscrout*			R_router = nullptr;						// Console/log router

	bool				R_opened = false;				// Open indicator

	// Fiels dor Target compression ******************************
	short				RW_compress_level = -1;			// Level at which compression was defined
	//************************************************************

	unsigned char*		R_parm = 0x00;					//Parm reference
	bool				R_compressed = false;			// COMPRESS (SOURCE)
	bool				R_encrypted = false;			// ENCRYPT (SOURCE)
	bool				R_even = false;					// EVEN condition
	bool				R_empty = false;				// EMPTY condition
	bool				R_saf = false;					// SAF indicator
	bool				R_cpx = false;					// CPX indicator
	string				R_incl = "";					// INCLUDE condition
	string				R_excl = "";					// EXCLUDE condition
	string				R_key = "";						// Source encryption key
	string				R_source = "";					// Source name

	// FILE attributes
	bool				R_file = false;					// FILE indicator

	string				R_saf_core_name = "";			// i-SAF file name w/o extension
	long long			R_saf_total_size = 0;			// Total size of all SAF segments
	int					R_saf_segment_number = 0;		// i-SAF current segment #

	vscdefs::DIR_INFO	R_saf_dir_info;					// SAF dir descriptor
	vscdefs::FILE_INFO	R_saf_file_info;				// SAF file descriptor
	vscdefs::FILE_INFO*	R_last_file_info = nullptr;		// non-SAF last file descriptor

	vscdefs::FILE_HEADER R_hdr;							// File header


	// Arrays
	vector<vscdefs::DIR_INFO*>	R_dir_info;				// Dirs tree  (!saf)
	vector<string>				R_dir_stack;			// Dirs stack SAF/NON-SAF 
	//vector<bool>				R_dir_comp;				// Compression indicator for dirs stack
	unsigned short int			R_dir_level = 0;		// Last level
	vector<vscdefs::FILE_INFO*>	R_file_info;			// File list for dir

	// I/O objects
	vs2iof*				R_saf_io = nullptr;				// SAF i/o object
	vs2iof*				R_file_io = nullptr;			// File i/o object

	string				R_stack_fullpath = "";

	int					R_dir_index = -1;				// Directory index (non-SAF)
	int					R_file_index = -1;				// File index (non-SAF)

	unsigned char*		R_buffer = nullptr;				// I/O file buffer
	unsigned char*		R_w_buffer = nullptr;			// Temp I/O file buffer

	vscdefs::FILE_INFO			err_info;				// Object to return error

	// Read file header for SAF/SEC
	// Return: 0- ok or 8 - error
	unsigned long int read_header(vscdefs::FILE_HEADER* hdr, vs2iof* io);

	// Check SAF file length/crc if -chk parameter is specified 
	string check_saf(const int mode);

	// Switch input SAF file volume
	string switch_saf_volume(bool skip_check = true);

	// Build directory tree  
	void build_dir_tree(unsigned short int level, const string& root, const int root_attrs);

	// Check file name for include/exclude lists
	bool check_name_pattern(const string nm, const string incl, const string excl, const bool is_dir);

	// Check file name for compress list
	bool check_compress_pattern(const string nm, const string incl, const bool is_dir);

	// Check file name for include/exclude lists
	//bool check_name(const string& nm, const bool is_dir);

	// Skip input SAF file
	string skip_saf_file(const bool skip_check = false);

	// Get next SAF file
	vscdefs::FILE_INFO* get_next_file_saf();

	// Get next non-SAF file
	vscdefs::FILE_INFO* get_next_file_nonsaf();

	// Get next SAF dir
	vscdefs::DIR_INFO* get_next_dir_saf();

	// Get next non-SAF dir
	vscdefs::DIR_INFO* get_next_dir_nonsaf();

	// Go to next non-FILE record type
	string bypass_saf_files();

	// Display error and close SAF file
	void file_damaged();

	// Check parm for compress mode
	bool is_sec_mode();

};

