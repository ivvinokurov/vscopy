#pragma once

#include <string>
#include <windows.h>

using namespace std;

class vs2iolib final
{
public:
	/** File System Object Info structure **/
	struct FSO_INFO
	{
		string				name = "";			// File name
		unsigned int		attrs = 0;			// File attributes
		unsigned long long	size = 0;			// File size
		unsigned long long	time_write = 0;		// File write timestamp
	};

	static const int COLOR_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	static const int COLOR_RED = FOREGROUND_RED;
	static const int COLOR_GREEN = FOREGROUND_GREEN;
	static const int COLOR_BLUE = FOREGROUND_BLUE;
	static const int COLOR_CYAN = FOREGROUND_BLUE | FOREGROUND_GREEN;
	static const int COLOR_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
	static const int COLOR_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE;
	
	// File mode
	static const char FILE_MODE_CREATE = 'c';
	static const char FILE_MODE_OPEN = 'o';
	static const char FILE_MODE_APPEND = 'a';
	static const char FILE_MODE_UNDEFINED = 'u';

	// Dynamic CRC mode
	static const char CRC_READ = 'r';
	static const char CRC_WRITE = 'w';

	// Not encrypt parameter
	static const string ENCRYPTION_UNDEFINED;

	/*************** COMMON ************/

	// Get application data path. app_name - name of app (create folder if missing)
	static string get_app_data_path(const string& app_name);

	/*********** FILE/DIRECTORY ********/

	// Check if directory exists; create if 'create' = true
	static bool directory_exists(const string& name, const bool create = false);

	// Checks if file exists (by name)
	static bool file_exists(const string& name);

	// Returns last level directory name
	static string get_last_directory_name(const string& path);

	// Parse file name from path
	static string get_file_name(const string& name);

	// Parse path name from the full file path
	static string get_file_path(const string& src);

	// Parse file extension
	static string get_file_ext(const string& src);

	// Parse file name w/o extension
	static string get_file_name_wo_ext(const string& src);

	// Remove file 
	static bool rmfile(const string& path);

	// Create dir 
	static bool mkdir(const string& path);

	// Remove dir 
	static bool rmdir(const string& path);

	// Purge dir - remove all contents (static)
	// root - remove root dir itself (true) or no (false)
	// system - remove system and hidden files (true) or no (false)
	static bool purge(const string& path, const bool root = false, const bool system = false);

	// File length
	static long long get_file_length(const string& path);

	// Get file attributes
	static unsigned int get_fs_object_attributes(const string& path);

	// Set file attributes (static)
	static void set_fs_object_attributes(const string& path, const unsigned int attrs);

	// Last Write timestamp
	static long long get_last_write_time(const string& name);

	// Set file last write time (static)
	static void set_file_last_write_time(const string& path, const long long timestamp);

private:
	// Get directory sub-dirs list
	// dir: false - files; true - dirs
	static vector<string> get_filesystem_object_list(const string& path, const bool dir, const bool system = false);

	// Get filesystem objects INFO list
	static vector<FSO_INFO*> get_filesystem_object_list_info(const string& path, const string& mask, const bool dir, const bool system);

public:
	// Get directory file list (wrapper)
	static vector<string> get_file_list(const string& path, const bool system = false);

	// Get directory sub-dirs list (wrapper)
	static vector<FSO_INFO*> get_directory_list_info(const string& path, const string& mask, const bool system = false);

	// Get directory file list (wrapper)
	static vector<FSO_INFO*> get_file_list_info(const string& path, const string& mask, const bool system = false);

	// Get directory sub-dirs list (wrapper)
	static vector<string> get_directory_list(const string& path, const bool system = false);


	/*********** CONSOLE I/O ***********/
	// Get console input
	static string get_console_input(bool hide = false);

	// Set console font color
	static void set_font_color(short color);

};