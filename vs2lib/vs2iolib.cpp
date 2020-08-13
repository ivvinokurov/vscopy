// Common I/O library V2
#include "pch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <io.h>
#include <windows.h>
#include <direct.h>


#include "vs2iolib.h"
#include "vs2comlib.h"

using namespace std;

// Not encrypt parameter
const string vs2iolib::ENCRYPTION_UNDEFINED = "$$ENCRYPTION_UNDEFINED$$";

// Checks if file exists (by name)
bool vs2iolib::file_exists(const string& name)
{
	long long dwAttrib = GetFileAttributesA(name.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Checks if directory exists
bool vs2iolib::directory_exists(const string& name, const bool create)
{
	long long dwAttrib = GetFileAttributesA(name.c_str());

	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
	{
		if (create)
		{
			if (!mkdir(name.c_str()))
				return false;
			else
				return true;
		}
		else
			return false;

	}
	
	return	(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

// Returns last level directory name
string vs2iolib::get_last_directory_name(const string& path)
{
	vector<string> v = vs2comlib::parse(path, "\\");

	return v.size() == 0 ? "" : v[v.size() - 1];
}

// Parse file name from path
string vs2iolib::get_file_name(const string& path)
{
	return get_last_directory_name(path);
}

// Parse path name from the full file path
string vs2iolib::get_file_path(const string& src)
{
	long long s = src.find_last_of("/\\");
	return s >= 0 ? src.substr(0, s) : "";
}

// Parse file extension
string vs2iolib::get_file_ext(const string& src)
{
	string s = get_file_name(src);
	long long l = s.find_last_of(".");
	return l < 0 ? "" : s.substr((size_t)(l + 1));
}

// Parse file name w/o extension
string vs2iolib::get_file_name_wo_ext(const string& src)
{
	long long l = src.find_last_of(".");
	return (l < 0) ? src : src.substr(0, l);
}

// Get filesystem objects list 
vector<string> vs2iolib::get_filesystem_object_list(const string& path, const bool dir, const bool system)
{
	vector<string> ret;

	intptr_t handle;

	_finddata_t info;

	if ((handle = _findfirst64i32(path.c_str(), &info)) != -1L)
	{
		do
		{
			bool d = ((info.attrib & _A_SUBDIR) != 0);
			if (d == dir)
			{
				bool incl = true;

				unsigned int sys = info.attrib & (_A_SYSTEM + _A_HIDDEN);

				if (!system)
					incl &= (sys == 0);

				if (incl)
				{
					long long l = 0;
					for (int i = 0; i < 260; i++)
						if (info.name[i] != 0x00)
							l++;
						else
							break;

					string s = vs2comlib::trim(vs2comlib::convert_char_to_string((const unsigned char*)info.name, 0, l));

					if ((s != ".") & (s != ".."))
						ret.push_back(s);
				}
			}
		} while (_findnext(handle, &info) == 0);
	}

	_findclose(handle);

	return ret;
}

// Get filesystem objects INFO list
vector<vs2iolib::FSO_INFO*> vs2iolib::get_filesystem_object_list_info(const string& path, const string& mask, const bool dir, const bool system)
{
	vector<vs2iolib::FSO_INFO*> ret;

	intptr_t handle;

	string rp = path + "\\" + ((mask == "") ? "*" : mask);


	_finddata_t info;

	if ((handle = _findfirst64i32(rp.c_str(), &info)) != -1L)
	{
		do
		{
			bool d = ((info.attrib & _A_SUBDIR) != 0);
			if (d == dir)
			{
				bool incl = true;

				unsigned int sys = info.attrib & (_A_SYSTEM + _A_HIDDEN);

				if (!system)
					incl &= (sys == 0);

				if (incl)
				{
					long long l = 0;
					for (int i = 0; i < 260; i++)
						if (info.name[i] != 0x00)
							l++;
						else
							break;
					string s = vs2comlib::trim(vs2comlib::convert_char_to_string((const unsigned char*)info.name, 0, l));

					if ((s != ".") & (s != ".."))
					{
						vs2iolib::FSO_INFO* fso = new vs2iolib::FSO_INFO();
						fso->name = s;
						fso->attrs = info.attrib;
						string fullnm = path + "\\" + s;
						fso->time_write = vs2iolib::get_last_write_time(fullnm);
						fso->size = info.size;

						ret.push_back(fso);
					}
				}
			}
		} while (_findnext(handle, &info) == 0);
	}

	_findclose(handle);

	return ret;
}

// Get file list 
vector<vs2iolib::FSO_INFO*> vs2iolib::get_file_list_info(const string& path, const string& mask, const bool system)
{
	return vs2iolib::get_filesystem_object_list_info(path, mask, false, system);
}

// Get directory sub-dirs list
vector<vs2iolib::FSO_INFO*> vs2iolib::get_directory_list_info(const string& path, const string& mask, const bool system)
{
	return vs2iolib::get_filesystem_object_list_info(path, mask, true, system);
}

// Get file list 
vector<string> vs2iolib::get_file_list(const string& path, const bool system)
{
	return vs2iolib::get_filesystem_object_list(path, false, system);
}

// Get directory sub-dirs list
vector<string> vs2iolib::get_directory_list(const string& path, const bool system)
{
	return vs2iolib::get_filesystem_object_list(path, true, system);
}


// Get application data path. app_name - name of app (create folder if missing)
std::string vs2iolib::get_app_data_path(const string& app_name)
{
	char* path;

	size_t path_size;

	getenv_s(&path_size, NULL, 0, "APPDATA");

	path = (char*)malloc(path_size * sizeof(char));

	// Get the value of the LIB environment variable.  
	getenv_s(&path_size, path, path_size, "APPDATA");

	string s = vs2comlib::convert_char_to_string((const unsigned char*)path, 0, path_size - 1) + "\\" + app_name;

	free(path);

	bool b = directory_exists(s, true);

	return s;
}

// Remove file
bool vs2iolib::rmfile(const string& path)
{
	return (remove(path.c_str()) == 0);
}

// Create dir
bool vs2iolib::mkdir(const string& path)
{
	return (_mkdir(path.c_str()) == 0);
}

// Remove dir recursively
bool vs2iolib::rmdir(const string& path)
{
	return vs2iolib::purge(path, true, true);
}

// Purge dir - remove all contents
// root - remove root dir itself (true) or no (false)
bool vs2iolib::purge(const string& path, const bool root, const bool system)
{
	bool rc = true;

	// 1. Check if src dir exists 
	if (!directory_exists(path))
		return rc;

	// 2. remove files
	vector<string> vf = get_file_list(path + (string)"\\*", true);

	for (unsigned int i = 0; i < vf.size(); i++)
	{
		string nm = path + "\\" + vf[i];
		if (remove(nm.c_str()) != 0)
		{
			rc = false;
			break;
		}
	}

	vf.clear();

	if (rc)
	{
		// 3. remove dirs
		vector<string> vd = get_directory_list(path + "\\*", true);

		for (unsigned int i = 0; i < vd.size(); i++)
		{
			string nm = path + "\\" + vd[i];

			if (!rmdir(nm))
			{
				rc = false;
				break;
			}
		}

		vd.clear();
	}

	// Remove root if required
	if (rc & root)
		return (_rmdir(path.c_str()) == 0);
	else
		return rc;
}

// Length (static)
long long vs2iolib::get_file_length(const string& path)
{
	__finddata64_t info;
	intptr_t handle;

	if ((handle = _findfirst64(path.c_str(), &info)) == -1L)
		info.size = 0;

	_findclose(handle);

	return info.size;
}

// Get file attributes
unsigned int vs2iolib::get_fs_object_attributes(const string& path)
{
	return GetFileAttributesA(path.c_str());
}

// Set file attributes
void vs2iolib::set_fs_object_attributes(const string& path, const unsigned int attrs)
{
	SetFileAttributesA(path.c_str(), attrs);
}

// Last Write timestamp 
long long vs2iolib::get_last_write_time(const string& name)
{
	BOOL bRet = FALSE;

	FILETIME ft;
	long long dtstamp = 0;

	HANDLE hFile = CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		bRet = GetFileTime(hFile, NULL, NULL, &ft);

		if (bRet == FALSE)
			return 0;

		CloseHandle(hFile);

		dtstamp = ft.dwHighDateTime;
		dtstamp <<= 32;
		dtstamp |= ft.dwLowDateTime;

		return dtstamp;
	}
	else
		return 0;
}

// Set file last write time
void vs2iolib::set_file_last_write_time(const string& path, const long long timestamp)
{
	FILETIME ft;
	ft.dwHighDateTime = 0;
	ft.dwLowDateTime = 0;

	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		ft.dwHighDateTime = (timestamp >> 32);
		ft.dwLowDateTime = (timestamp << 32 >> 32);
		SetFileTime(hFile, NULL, NULL, &ft);
		CloseHandle(hFile);
	}
}

/*********** CONSOLE I/O ***********/

// Get console input
string vs2iolib::get_console_input(bool hide)
{
	string s = "";

	set_font_color(hide ? COLOR_YELLOW : COLOR_GREEN);

	if (hide)
	{
		while (!(GetAsyncKeyState(VK_RETURN) & (SHORT)1))
		{
			for (int i = 0x30; i < 0x7A; i++)
			{
				if (GetAsyncKeyState(i) & 1)
				{
					if (i >= 65 && i <= 90 && ((GetKeyState(VK_CAPITAL) & 1) == 0 || GetKeyState(VK_SHIFT) & 1))
						s += ((char)i + 32);
					else if (i >= 97 && i <= 122 && ((GetKeyState(VK_CAPITAL) & 1) == 0 || GetKeyState(VK_SHIFT) & 1))
						s += ((char)i - 32);
					else
						s += (char)i;

					cout << "*";
					Sleep(50);
				}
				else if (GetAsyncKeyState(VK_BACK) & 1)
				{
					s.erase(s.size() - 1);
					system("cls");
					for (int j = 0; i < s.size(); i++)
					{
						cout << '*';
					}
					Sleep(50);
				}
			}
		}
	}
	else
		getline(cin, s);

	set_font_color(COLOR_WHITE);

	return s;
}

// Set console font color
void vs2iolib::set_font_color(short color)
{
	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}




