#pragma once
#include "string"
#include "vector"
#include "vs2iof.h"
#include "vs2comlib.h"

using namespace std;
class vscrout final
{
private:
	vs2iof log_file;
	unsigned char mode =	0x00;
	unsigned char state =	0x00;

	bool suppress_output = false;

	void write_line_to_console(string msg);
public:
	// Modes
	static const unsigned char MODE_CONSOLE =	0x01;
	static const unsigned char MODE_LOG =		0x02;

	// States
	static const unsigned char STATE_CLOSED =	0x00;
	static const unsigned char STATE_OPENED =	0x01;

	// Prefix codess
	static const int PREFIX_NA =			   -1;			// No prefix
	static const int PREFIX_INFO =				0;			
	static const int PREFIX_WARN =				1;
	static const int PREFIX_ERROR =				2;
	static const int PREFIX_ABNORMAL =			3;
	static const int PREFIX_FAILED =			4;

	static const string PREFIX_VALUE[5];

	// Open router
	bool open(const bool console = true, const string logfile = "", const bool cls = true);

	// Close router
	bool close();

	// Write message extended
	void write_msg_ext(const string prefix = "", const string p1 = "", const string p2= "", const string p3 = "");

	// Write error message
	void write_msg_error(const string prefix = "", const string p1 = "", const string p2 = "", const string p3 = "");

	// Set log file
	bool set_log(const string &logfile, const bool cls = true);

	// Write empty line
	void empty_line();

	// Suppress on/off output
	void suppress(bool smode);

	// Check is supressed output
	bool is_suppressed();
};

