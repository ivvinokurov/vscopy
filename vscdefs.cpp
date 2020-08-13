#include <stdafx.h>
#include <string>

#include "vscdefs.h"

const string vscdefs::CP_RET_CODES[5] = { "0", "4", "8", "16", "20" };	// Return codes (text)

/** Misc constants **/
const string vscdefs::NAME_PREFIX_DIR = "$";			// Directory prefix in INCLUDE/EXCLUDE

/** Error levels **/
const string vscdefs::MSGLEVEL_ABNORMAL		= "A";			// Error level ABNORMAL
const string vscdefs::MSGLEVEL_ERROR		= "E";			// Error level ERROR

const string vscdefs::SAF_MSG_PREFIX		= "SAF::";		// Message prefix for SAF

const string vscdefs::SEC_FILE_EXT			= "vsec";		// Compressed/encryped file extension
const string vscdefs::SAF_FILE_EXT			= "vsaf";		// SAF file extension

// SIGNATURES
const unsigned char vscdefs::SG_DIR[8] =  { 0x03, 0x09, 0x47, 0x05, 0x77, 0x39, 0x17, 0x00 };	// Dir signature
const unsigned char vscdefs::SG_FILE[8]	= { 0x04, 0x21, 0x19, 0x05, 0x14, 0x11, 0x01, 0x00 };	// File signature
const unsigned char vscdefs::SG_SAF[8]	= { 0x05, 0x17, 0x34, 0x47, 0x52, 0x05, 0x15, 0x00 };	// SAF signature


/*------- COMMANDS BEGIN -------*/
/*------- SINGLE COMMANDS ------*/
const string vscdefs::CMD_COPY			= "cp";		// Copy file/directory(autodetect)
const string vscdefs::CMD_COPYX			= "cpx";	// Copy file/directory(autodetect) - X mode
const string vscdefs::CMD_LIST			= "ls";		// List SAF file content
const string vscdefs::CMD_RUN			= "run";	// Run script from the file (next parameter)
const string vscdefs::CMD_OS_COMMAND	= "cmd";	// Run OS command
	/*------- SCRIPT COMMANDS ------*/
const string vscdefs::CMD_PARM			= "parm";	// Parm line (1st)
const string vscdefs::CMD_SET			= "set";	// Force set result code
const string vscdefs::CMD_IF			= "if";		// Conditional operator
const string vscdefs::CMD_END			= "end";	// End of 'if'/'else' scope (if command is not specified)
const string vscdefs::CMD_ELSE			= "else";	// Else for 'if' command
const string vscdefs::CMD_STOP			= "stop";	// Stop script
const string vscdefs::CMD_ON			= "on";		// On handling command
const string vscdefs::CMD_ON_ERROR		= "error";	// On error handling operator
/*------- COMMANDS END ==-------*/

/*----- SCRIPT CONDS BEGIN -----*/
const string vscdefs::COND_ERROR		= "error";
const string vscdefs::COND_SUCCESS		= "success";
const string vscdefs::COND_ON_STOP		= "stop";
const string vscdefs::COND_ON_CONT		= "continue";
/*----- SCRIPT CONDS END -------*/

/*------- REPLACE BEGIN ------------*/
const string vscdefs::C_REPL_ALL		= "-all";		// Replace all files no matter what timestamp is
const string vscdefs::C_REPL_NONE		= "-none";		// Dont replace anything (keep existing)
/*------- REPLACE END --------------*/

/*------- TARGET BEGIN -------------*/
const string vscdefs::C_TARG_MIRROR		= "-m";		// Mirror dirs
const string vscdefs::C_TARG_PURGE		= "-p";		// Delete everything from the target dir
const string vscdefs::C_TARG_SAF		= "-s";		// Create SAF archive file
/*------- TARGET END ---------------*/


/*-------- PARM BEGIN---------------*/
const string vscdefs::C_PARM_ATTRIBUTES = "-a";		// Restore original file attributes (not for saf/compression)
const string vscdefs::C_PARM_COMPRESS	= "-c";		// Compress files in auto mode
const string vscdefs::C_PARM_COMPRESS1 = "-c:";		// Selective compress

const string vscdefs::C_PARM_CRC		= "-crc";	// Calculate CRC32 for files/SAF
const string vscdefs::C_PARM_DIR_EMPTY	= "-e";		// Include empty directories
const string vscdefs::C_PARM_RECURSIVE	= "-r";		// Recursive process directories
const string vscdefs::C_PARM_SYSOBJ		= "-sys";	// Include hidden and protected system files
const string vscdefs::C_PARM_ZERO_LEN	= "-z";		// Include zero-length files

/*-------- PARM END-----------------*/



/*------- MODES BEGIN --------------*/
const string vscdefs::C_MODE_EVEN		= "-even";		// Continue if not critical errors
const string vscdefs::C_MODE_NOLIST		= "-nl";		// Do not create file/directories listing in output
/*------- MODES END ----------------*/

	/*------- MESSAGE PREFIXES AND COMMANDS BEGIN --------*/
//const string vscdefs::DSP_JOB =		"JOB";	// Cmd line, script
//const string vscdefs::DSP_TASK =	"TASK";	// Cmd line, script
const string vscdefs::DSP_MIR =		"CP[mirror]";	// Mirror
const string vscdefs::DSP_PUR =		"CP[purge]";	// Purge
const string vscdefs::DSP_LST =		"LS";	// List SAF/Non-SAF
const string vscdefs::DSP_CPY =		"CP";	// Copy
const string vscdefs::DSP_CMD =		"CMD";	// OS command
//const string vscdefs::DSP_RUN =		"RUN";	// Run script

const string vscdefs::DSPT_FIL = "F";		// File
const string vscdefs::DSPT_DIR = "D";		// Directory

const string vscdefs::DSPA_ADDED	= "+";	// New file
const string vscdefs::DSPA_REPLACED	= "#";	// File replaced
const string vscdefs::DSPA_SKIPPED	= "~";	// File/dir skipped
const string vscdefs::DSPA_DELETED	= "-";	// File/dir deleted
const string vscdefs::DSPA_DUMMY	= " ";	// Dummy

const string vscdefs::DSPC_STARTED	= "STARTED";	// Action began
const string vscdefs::DSPC_ENDED	= "ENDED";		// Action ended
const string vscdefs::DSPC_ERROR	= "ERROR:";		// Error occured

/*------- MESSAGE PREFIXES AND COMMANDS END   --------*/


/*------- MISCELLANEOUS PARAMETERS BEGIN --------*/
// Volume size for SAF (default if value is not defined vol[:nnnnnn]
const string vscdefs::PARM_VOL_S1		= "-v";		// Volume size for SAF - default
const string vscdefs::PARM_VOL_L1		= "-v:";	// Volume size for SAF - vol[:nnnnnn]
const string vscdefs::PARM_EXCL			= "-x:";	// Exclude pattern for files/dirs (dir name shall be preceded by '$')
const string vscdefs::PARM_LOG			= "-log:";	// Log file
const string vscdefs::PARM_CHK			= "-chk";	// Check SAF consistency (length/crc)
const string vscdefs::PARM_LS			= "-l";		// List source script lines

const string vscdefs::PARM_KEY_SHOW		= "?";		// Show key on console
const string vscdefs::PARM_KEY_HIDE		= "*";		// Hide key on console

const string vscdefs::PARM_PARM_SHOW	= "(?)";	// Show parm on console
const string vscdefs::PARM_PARM_HIDE	= "(*)";	// Hide parm on console


const char* vscdefs::P_NEWLINE			= "\r\n";	// Newline
const string vscdefs::P_PWD_DELIMITER	= "::";		// Password delimiter
