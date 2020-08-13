#pragma once
#include "string"
#include "vs2iof.h"

using namespace std;

class vscdefs final
{
public:
	// Vscopy command definition (interpreted)
	struct COMMAND_DEFINITION
	{
		string				CD_COMMAND = "";			// Original command
		string				CD_COMMAND_DSP = "";		// Command for display 
		string				CD_SOURCE = "";
		string				CD_TARGET = "";
		unsigned char		CD_BYTE_PARM = 0x00;
		unsigned char		CD_BYTE_TARG = 0x00;
		unsigned char		CD_BYTE_REPL = 0x00;		// By default - replace older
		unsigned char		CD_BYTE_MODE = 0x00;		// Mode
		unsigned char		CD_BYTE_PADL = 0x00;		// Pad left bytes
		bool				CD_CHK = false;
		string				CD_INCL = "";
		string				CD_EXCL = "";
		string				CD_PR = "";
		string				CD_PW = "";
		string				CD_COMPRESS = "";			// Selective compress string
		unsigned long long	CD_VOL = 0;
		string				CD_CMDLINE = "";
		int					CD_RC = 0;					// Retcode for STOP command
		string				CD_IE_COND = "";			// 'If'/'Else' only - condition
		int					CD_LEVEL = 0;				// 'If'/'Else' only - level
		string				CD_ON = "";					// ON 1st operand (error)
		string				CD_ON_COND = "";			// ON 2ndt operand (stop/continue)
		string				CD_RAW = "";				// Raw command line

		// RUNTIME
		string				CD_SOURCE_DIR = "";
		string				CD_SOURCE_FILE = "";
		bool				CD_SAF_I = false;			// Indicator if source is SAF file
		bool				CD_SAF_O = false;			// Indicator if target is SAF file

		// STAT
		long long			CD_STAT_FILES = 0;				// # of copies/listed files
		long long			CD_STAT_FILES_SIZE = 0;			// Size of copies/listed files
		long long			CD_STAT_FILES_SKIPPED = 0;		// # of skipped files
		long long			CD_STAT_FILES_SKIPPED_SIZE = 0;	// Size of skipped files
		long long			CD_STAT_DIRS = 0;				// # of copies/listed dirs
		long long			CD_STAT_DIRS_SKIPPED = 0;		// # of skipped dirs
	} ;
	
	/** DIR_INFO structure **/
	struct DIR_INFO
	{
		short int			level = -1;			// Directory level; -1 - eof indicator
		string				name = "";			// Directory name
		unsigned int		attrs = 0;			// Directory attributes
		string				rc = "";			// Last operation return code ("" - OK)
	};

	/** FILE_INFO structure **/
	struct FILE_INFO
	{

		string				name = "";			// File name
		unsigned long long	timestamp = 0;		// File last write timestamp
		unsigned long long	original_length = 0;// Original file length
		unsigned long long	file_length = 0;	// Actual file length (non-SAF)
		unsigned int		attrs = 0;			// Directory attributes
		vs2iof*				io = nullptr;		// positioned I/O object for SAF file, opened I/O object for non-SAF
		// File data read fields
		unsigned char* buffer = nullptr;		// Data pointer (unpacked and decripted)
		unsigned long long	length = 0;			// Last read data chunk length
		unsigned long long	position = 0;		// Current file position (non-SAF)
		char				mode = FILE_MODE_UNDEFINED; // Initially closed mode
		string				rc = "";			// Last operation return code ("" - OK)
	};

	/** FILE header in VSAF and VSEC files (physical structure) **/
	struct FILE_HEADER
	{
		unsigned long int	type = 0;			// +00(04) Object type
		unsigned char		sg[7] = { 0x00 };	// +04(07) File signature
		unsigned char		mask = 0x00;		// +11(01) File mask
			// xxxx xxx1 - crc32
			// xxxx xx1x - compress
			// xxxx x1xx - encrypt			
		unsigned long long	timestamp = 0;		// +12(08) File last write timestamp
		unsigned long long	original_length = 0;// +20(08) Original file length
		unsigned int		attrs = 0;			// +28(04) Attributes
		unsigned short int	level = 0;			// +32(02) Level (dir only)
		string				name = "";			// +44(nn) File name
	};

	/*------- COMMANDS BEGIN -------*/
	/*------- SINGLE COMMANDS ------*/
	static const string CMD_COPY;			// Copy file/directory(autodetect)
	static const string CMD_COPYX;			// Copy file/directory(autodetect) - X mode
	static const string CMD_LIST;			// List SAF file content
	static const string CMD_RUN;			// Run script from the file (next parameter)
	static const string CMD_OS_COMMAND;		// Run OS command

	/*------- SCRIPT COMMANDS ------*/
	static const string CMD_PARM;			// Parm line (1st)
	static const string CMD_SET;			// Force set result code
	static const string CMD_IF;				// Conditional operator
	static const string CMD_END;			// End of 'if'/'else' scope (if command is not specified)
	static const string CMD_ELSE;			// Else for 'if' command
	static const string CMD_STOP;			// Stop script
	static const string CMD_ON;				// On handling command
	static const string CMD_ON_ERROR;		// On error handling operator
	/*------- COMMANDS END ==-------*/

	/*----- SCRIPT CONDS BEGIN -----*/
	static const string COND_ERROR;
	static const string COND_SUCCESS;
	static const string COND_ON_STOP;
	static const string COND_ON_CONT;
	/*----- SCRIPT CONDS END -------*/

	/*-------- PARM BEGIN---------------*/
	static const string C_PARM_RECURSIVE;	// Recursive process directories
	static const string C_PARM_COMPRESS;	// Compress files in auto mode
	static const string C_PARM_COMPRESS1;	// Selective compress
	static const string C_PARM_CRC;			// Calculate CRC32 for files/SAF
	static const string C_PARM_DIR_EMPTY;	// Include empty directories
	static const string C_PARM_SYSOBJ;		// Include protected system files
	static const string C_PARM_XMODE;		// X-Mode (no saf/compression for in or out
	static const string C_PARM_ATTRIBUTES;	// Restore original file attributes (not for saf/compression)
	static const string C_PARM_ZERO_LEN;	// Include zero-length files
	/*-------- PARM END-----------------*/

	/*------- TARGET BEGIN -------------*/
	static const string C_TARG_SAF;			// Create SAF archive file
	static const string C_TARG_PURGE;		// Delete everything from the target dir
	static const string C_TARG_MIRROR;		// Mirror dirs
	/*------- TARGET END ---------------*/

	/*------- REPLACE BEGIN ------------*/
	static const string C_REPL_OLDER;		// Replace only older files
	static const string C_REPL_ALL;			// Replace all files no matter what timestamp is
	static const string C_REPL_NONE;		// Dont replace anything
	/*------- REPLACE END --------------*/

	/*------- MODES BEGIN --------------*/
	static const string C_MODE_EVEN;		//  Continue if not critical errors
	static const string C_MODE_NOLIST;		// Do not create file/directories listing in output
	/*------- MODES END ----------------*/

	/*------- MISCELLANEOUS PARAMETERS BEGIN --------*/
	// Volume size for SAF (default if value is not defined vol[:nnnnnn]
	static const string PARM_VOL_S1;		// Volume size for SAF - default
	static const string PARM_VOL_L1;		// Volume size for SAF - vol[:nnnnnn]
	static const string PARM_EXCL;			// Exclude pattern for files/dirs (dir name shall be preceded by '$')
	static const string PARM_LOG;			// Log file
	static const string PARM_CHK;			// Check SAF consistency (length/crc)
	static const string PARM_LS;			// List source script lines
	static const string PARM_KEY_SHOW;		// Show key on console
	static const string PARM_KEY_HIDE;		// Hide key on console
	static const string PARM_PARM_SHOW;		// Show parm on console
	static const string PARM_PARM_HIDE;		// Hide parm on console


	/*----- RETURN CODES -----*/
	static const int RC_SUCCESS = 0;
	static const int RC_ERROR = 8;
	static const int RC_ABNORMAL = 12;
	static const int RC_FAILED = 16;

	static const char* P_NEWLINE;			// Newline
	static const string P_PWD_DELIMITER;	// Password delimiter

	/*------- MESSAGE PREFIXES AND COMMANDS BEGIN --------*/
	//static const string DSP_JOB;			// Cmd line, script
	//static const string DSP_TASK;			// Cmd line, script
	static const string DSP_MIR;			// Mirror
	static const string DSP_PUR;			// Purge
	static const string DSP_LST;			// List SAF
	static const string DSP_CPY;			// Copy
	static const string DSP_CPX;			// Copy mode X
	static const string DSP_CMD;			// OS command
	//static const string DSP_RUN;			// Run script

	static const string DSPT_FIL;			// File
	static const string DSPT_DIR;			// Directory

	static const string DSPA_ADDED;		// New file
	static const string DSPA_REPLACED;		// File replaced
	static const string DSPA_SKIPPED;		// File/dir skipped
	static const string DSPA_DELETED;		// File/dir deleted

	static const string DSPC_STARTED;		// Action began
	static const string DSPC_ENDED;			// Action ended
	static const string DSPC_ERROR;			// Error
	static const string DSPA_DUMMY;			// Dummy


	/*------- MESSAGE PREFIXES AND COMMANDS END   --------*/

	/*************** CONSTANTS AND TYPES END   ***************/


	/*************** VSCOPY CONSTANTS ***************/

	/** Misc constants **/
	static const string SAF_MSG_PREFIX;					// Message prefix for SAF

	static const string CP_RET_CODES[5];				// Return codes (text)

	/** Error levels **/
	static const string MSGLEVEL_ABNORMAL;				// Error level ABNORMAL
	static const string MSGLEVEL_ERROR;					// Error level ERROR

	static const string NAME_PREFIX_DIR;				// Directory prefix in INCLUDE/EXCLUDE

	/** Descriptor masks **/
	static const unsigned char MASK_EMPTY =		0x00;
	static const unsigned char MASK_CRC =		0x01;
	static const unsigned char MASK_COMPRESS =	0x02;
	static const unsigned char MASK_ENCRYPT =	0x04;

	static const unsigned char PAD_CHAR = ' ';			// Character for pad left messages
	   
	
	static const long long CHUNK_SIZE = 131072;			// Chunk size
	
	// PARM 4 bytes
	// Byte 0: Parm bits
	static const unsigned char PARM_RECURSIVE =			0x01;		// 0000 0001 Subtree
	static const unsigned char PARM_COMPRESS =			0x02;		// 0000 0010 Compression 
	static const unsigned char PARM_CRC =				0x04;		// 0000 0100 CRC
	static const unsigned char PARM_DIR_EMPTY =			0x08;		// 0000 1000 Empty Dirs

	static const unsigned char PARM_SYSOBJ =			0x10;		// 0001 0000 System objects (dirs & files)
	static const unsigned char PARM_CPX =				0x20;		// 0010 0000 CPX mode: copy files as-is (no compession, SAF detecting)
	static const unsigned char PARM_ATTRIBUTES =		0x40;		// 0100 0000 ATTRS mode: restore original file attributes
	static const unsigned char PARM_ZERO_LEN =			0x80;		// 1000 0000 ZERO mode: include zero-length files

	// Applicable parms for commands
	static const unsigned char ALLOWED_PARM_COPY =		PARM_ZERO_LEN + PARM_COMPRESS + PARM_CRC + PARM_SYSOBJ + PARM_RECURSIVE + PARM_DIR_EMPTY + PARM_CPX + PARM_ATTRIBUTES;
	static const unsigned char ALLOWED_PARM_LIST =		PARM_CRC + PARM_RECURSIVE + PARM_ZERO_LEN;

	// Byte 1: Target mode
	static const unsigned char TARG_MODE_SAF =			0x01;		// 0000 0001 Create SAF file
	static const unsigned char TARG_MODE_MIRROR =		0x02;		// 0000 0010 Mirror files & dirs
	static const unsigned char TARG_MODE_PURGE =		0x04;		// 0000 0100 Remove all target contents

	// Applicable target for commands
	static const unsigned char ALLOWED_TARG_COPY =		TARG_MODE_SAF + TARG_MODE_PURGE + TARG_MODE_MIRROR;
	static const unsigned char ALLOWED_TARG_LIST =		0;

	// Byte 2: Replace mode (by default if not specified - replace older)
	static const unsigned char REPL_ALL =			0x01;		// 0000 0001 Replace all
	static const unsigned char REPL_NONE =			0x02;		// 0000 0010 Replace nothing

	// Applicable repl for commands
	static const unsigned char ALLOWED_REPL_COPY =		REPL_ALL + REPL_NONE;
	static const unsigned char ALLOWED_REPL_LIST =		0;

	// Not applicable repl&targ for file copy
	static const unsigned char NOT_APPLICABLE_FILE_PARM	=	PARM_RECURSIVE + PARM_DIR_EMPTY;
	static const unsigned char NOT_APPLICABLE_FILE_TARG	=	TARG_MODE_SAF + TARG_MODE_MIRROR + TARG_MODE_PURGE;
	// Applicable to both CPX and ATTRS
	static const unsigned char NOT_APPLICABLE_CPX_PARM	=	PARM_COMPRESS + PARM_CRC;
	static const unsigned char NOT_APPLICABLE_CPX_TARG	=	TARG_MODE_SAF;

	// Byte 3: Common modes
	static const unsigned char MODE_EVEN =				0x01;		// 0000 0001 Even
	static const unsigned char MODE_NOLIST =			0x02;		// 0000 0010 No console output

		// Applicable parms for commands
	static const unsigned char ALLOWED_MODE_COPY = MODE_EVEN + MODE_NOLIST;
	static const unsigned char ALLOWED_MODE_LIST = MODE_EVEN + MODE_NOLIST;


	// Bytes indexes
	static const unsigned long long BYTE_PARM =			0;
	static const unsigned long long BYTE_TARGET =		1;
	static const unsigned long long BYTE_REPLACE =		2;
	static const unsigned long long BYTE_MODE =			3;
	
	// Descriptor masks
	//static const unsigned char MASK_EMPTY = 0x00;
	//static const unsigned char MASK_CRC = 0x01;
	//static const unsigned char MASK_COMPRESS = 0x02;

	// FILE_INFO modes
	static const char FILE_MODE_UNDEFINED = 0x00;		// Initial mode, file is undefined
	static const char FILE_MODE_OPENED = 0x01;		// Read happened, but NOT EOF
	static const char FILE_MODE_CLOSED = 0x02;		// EOF reached

	// Chunk size (SAF)
	//static const long long	CHUNK_SIZE = 131072; // 32768;
	static const string		SEC_FILE_EXT;			// Compressed file extension
	static const string		SAF_FILE_EXT;				// SAF file extension

	// SIGNATURES
	static const unsigned char SG_DIR[8];	// Dir signature
	static const unsigned char SG_FILE[8];	// File signature
	static const unsigned char SG_SAF[8];	// SAF signature

		// Data object type consts
	static const unsigned long int DATA_DIR =		0xfffffff1;			// SAF directory pattern
	static const unsigned long int DATA_FIL =		0xfffffff2;			// SAF/VSEC file pattern
	static const unsigned long int DATA_SAF =		0xfffffff3;			// SAF header

	static const unsigned long int DATA_EOF =		0xffffffff;			// End of SAF file
	static const unsigned long int DATA_EOD =		0xfffffffe;			// End of SAF file data
	static const unsigned long int DATA_EOV =		0xfffffffd;			// End of SAF volume

	static const unsigned long int DATA_MAX_CHUNK = 0xffffff00;			// Maximum chunk length


};
