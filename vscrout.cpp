#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include "vscrout.h"
#include "vs2comlib.h"

const string vscrout::PREFIX_VALUE[5] = { "INFO", "WARNING", "ERROR", "ABNORMAL", "FAILED"};

// Write console message line
void vscrout::write_line_to_console(string msg)
{
	cout << msg << endl;
}

// Open router
// Console - console output (true)
// Logfile - route output to file (!="")
// Cls     - clean console and/or rewrite log file (true - default)
bool vscrout::open(const bool console, const string logfile, const bool cls)
{
	if (state == STATE_CLOSED)
	{
		mode = 0x00;

		state = STATE_OPENED;

		if (console)
		{
			mode |= MODE_CONSOLE;

			if (cls)
				system("cls");
		}

		if (logfile != "")
			set_log(logfile, cls);
	}
	else
	{
		write_line_to_console("OPEN: Log router is already opened");

		return false;
	}

	return false;
}

// Close router
bool vscrout::vscrout::close()
{
	if (state == STATE_OPENED)
	{
		if ((mode & MODE_LOG) != 0x00)
			log_file.close();

		mode = 0x00;
		state = 0x00;

		return true;
	}
	else
	{
		write_line_to_console("CLOSE: Log router is not opened");

		return false;
	}
}

// Write message
void vscrout::write_msg_ext(const string prefix, const string p1, const string p2, const string p3)
{
	string s = vs2comlib::current_time() + " " + prefix;
	
	if (p1 != "")
		s += (" " + p1);				// Type [F]/[D]

	if (p2 != "")
		s += (" " + p2);

	if (p3 != "")
		s += (" " + p3);

	if ((state == STATE_OPENED) & (!(suppress_output)))
	{
		if ((mode & MODE_CONSOLE) != 0x00)
			write_line_to_console(s);

		if ((mode & MODE_LOG) != 0x00)
			log_file.write_string(s + "\r\n");
	}
}

// Write error message
void vscrout::write_msg_error(const string prefix, const string p1, const string p2, const string p3)
{
	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

	string s = vs2comlib::current_time() + " " + prefix;

	if (p1 != "")
		s += (" " + p1);				// Type [F]/[D]

	if (p2 != "")
		s += (" " + p2);

	if (p3 != "")
		s += (" " + p3);

	if ((state == STATE_OPENED) & (!(suppress_output)))
	{
		if ((mode & MODE_CONSOLE) != 0x00)
			write_line_to_console(s);

		if ((mode & MODE_LOG) != 0x00)
			log_file.write_string(s + "\r\n");
	}

	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

// Set log file
bool vscrout::set_log(const string &logfile, const bool cls)
{
	if (state == STATE_OPENED)
	{
		if ((mode & MODE_LOG) != 0)
		{
			log_file.close();
			mode &= (MODE_LOG ^ 0xff);
		}
		
		string lf = logfile;

		if (!vs2iolib::file_exists(lf))
		{
			string l_dir = vs2iolib::get_file_path(logfile);
			if (l_dir != "")
			{
				if (!vs2iolib::directory_exists(l_dir))
				{
					write_line_to_console("LOG: Log file directory doesn't exist - '" + l_dir + "'");

					return false;
				}
			}
		}

		//log_file.set(logfile, (cls ? vliof::FILE_MODE_CREATE : vliof::FILE_MODE_APPEND));

		string rc = log_file.open(logfile, (cls ? vs2iolib::FILE_MODE_CREATE : vs2iolib::FILE_MODE_APPEND));

		mode |= MODE_LOG;

	}
	else
	{
		write_line_to_console("SET LOG: Log router is not opened");

		return false;
	}

	return true;
}

// Write empty line
void vscrout::empty_line()
{
	write_msg_ext();
}

// Suppress on/off output
void vscrout::suppress(bool smode)
{
	suppress_output = smode;
}

// Check is supressed output
bool vscrout::is_suppressed()
{
	return suppress_output;
}
