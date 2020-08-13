#include "pch.h"

#include "vs2config.h"
#include "string"
#include "vector"
#include "vs2iof.h"
#include "vs2iom.h"
#include "vs2comlib.h"
#include "vs2iolib.h"
#include "vs2crc32.h"


using namespace std;

// Constructor
vs2config::vs2config()
{
	buf = (unsigned char *)malloc(4096);
}

// Destructor
vs2config::~vs2config()
{
	for (unsigned int i = 0; i < data.size(); i++)
		delete(data[i]);

	free (buf);
}

// Load  config file - 
string vs2config::load(const string& name, const string& key)
{
	if (name == "")
		return "Load: name is not specified";

	file_name = name;
	
	file_key = key;

	is_changed = false;

	string old_crc_hex = "";
	string new_crc_hex = "";
	
	unsigned int crc = 0;

	long long f_length = 0;

	const char *P_newline = "\r\n";

	string s = "";
	if (!vs2iolib::file_exists(name))
	{
		IO.open(name, vs2iolib::FILE_MODE_CREATE, key);

		s = generate_header(vs2crc32::calculate_crc(nullptr, 0)) + '\r' + '\n';

		IO.write_string(s);

		IO.close();

		new_file = true;
	}
	else
	{
		IO.open(name, vs2iolib::FILE_MODE_OPEN, key);

		f_length = IO.get_length();

		if (f_length > 0)
			s = IO.read_string(f_length);

		IO.close();
		
		new_file = false;
	}

	// Parse file content
	long long pos = s.find_first_of(P_newline, 0);		// Find end of 1st str
	if (pos != 37)
		return IO.name + ": Header damage";

	old_crc_hex = s.substr(19, 8);						// Old crc from header
	pos += 1;

	crc = vs2crc32::begin_crc();

	int s_index = -1;									// Section index
	// Handle content cycle
	while (pos < f_length)
	{
		long long s_length = 0;							// Length of the string
		string str = "";

		long long new_pos = (long long)s.find_first_of(P_newline, (size_t)pos);
		if (new_pos == pos)
		{
			pos++;
			s_length = 0;
			str = "";
		}
		else if (new_pos < 0)
		{
			s_length = f_length - pos;
			str = vs2comlib::trim(s.substr((size_t)pos, (size_t)s_length));
			pos = f_length;
		}
		else
		{
			s_length = new_pos - pos;
			str = vs2comlib::trim(s.substr((size_t)pos, (size_t)s_length));
			pos = new_pos + 1;
		}
		
		// Process string
		if (s_length > 0)
		{
			string s_name = "";
			
			string s_value = "";

			long long s_end = 1;

			bool s_sys = false;

			////////////// Section /////////////
			if (str.substr(0, 1) == "[")						
			{

				s_end = str.find_first_of("]", 1);	// Find end of section name

				if (s_end > 1)
					s_name = vs2comlib::to_lower(vs2comlib::trim(str.substr(1, (size_t)(s_end - 1))));

				if (s_name.size() == 0)
					return "Missing section name";

				s_index = -1;
				for (unsigned int i=0; i< data.size(); i++)
					if (data[i]->name == s_name)
					{
						s_index = i;
						break;
					}
				if (s_index < 0)							// If new section
				{
					S_record *sr = new S_record();
					sr->name = s_name;
					s_index = (int)data.size();
					data.push_back(sr);
				}
			}
			////////////// Parameter /////////////
			else
			{
				if (s_index < 0)							// Section undefined: create 'default' section
				{
					S_record *sr = new S_record();
					sr->name = "default";
					s_index = (int)data.size();
					data.push_back(sr);
				}

				string s_name = "";

				long long s_end = str.find_first_of("=", 0);	// Find end of section name
				if (s_end < 1)
					return "Missing parameter name";

				s_name = vs2comlib::to_lower(vs2comlib::trim(str.substr(0, (size_t)s_end)));

				s_end++;

				data[s_index]->K_name.push_back(s_name);

				if (((size_t)(s_end + 1)) < str.size())
				{
					s_value = vs2comlib::trim(str.substr((size_t)s_end, (size_t)(str.size() - s_end)));
					if (s_value.size() > 0)
						if (s_value.substr(0, 1) == "^")
						{
							s_sys = true;
							s_value.erase(0, 1);
							int ln = (int)s_value.size();
							vs2comlib::convert_string_to_char(s_value, (unsigned char *)buf);
							crc = vs2crc32::add_crc(crc, buf, ln);
						}
				}
				else
					s_value = "";

				data[s_index]->K_value.push_back(s_value);

				data[s_index]->K_system.push_back(s_sys);
			}
		}
	}
	crc = vs2crc32::end_crc(crc);

	new_crc_hex = vs2comlib::convert_int_to_hex_string(crc);
	
	if (old_crc_hex != new_crc_hex)
		return "Load: protected parameters content incorrect";

	return "";
}


// Save config file
string vs2config::save()
{
	if (!is_changed)
		return "";

	if (file_name == "")
		return "Save: cannot be used for in-memory option";

	// Calculate crc
	unsigned int crc = vs2crc32::begin_crc();

	for (unsigned int i = 0; i < data.size(); i++)
	{
		for (unsigned int j = 0; j < data[i]->K_name.size(); j++)
		{
			if (data[i]->K_system[j])
			{
				long long ln = data[i]->K_value[j].size();					// Value length

				if (ln > 0)
				{
					vs2comlib::convert_string_to_char(data[i]->K_value[j], (unsigned char *)buf);	// Char array

					crc = vs2crc32::add_crc(crc, buf, (int)ln);
				}
			}
		}
	}

	crc = vs2crc32::end_crc(crc);

	/////////// Save file //////////

	// Write header
	IO.open(file_name, vs2iolib::FILE_MODE_CREATE, file_key);
	IO.write_string(generate_header(crc) + '\r' + '\n');

	// Write sections
	for (unsigned int i = 0; i < data.size(); i++)
	{
		IO.write_string((string)('[' + data[i]->name + ']' + '\r' + '\n'));
		for (unsigned int j = 0; j < data[i]->K_name.size(); j++)
		{
			string s = (data[i]->K_system[j]) ? "^" : "";
			IO.write_string((string)(data[i]->K_name[j] + " = " + s + data[i]->K_value[j] + '\r' + '\n'));
		}
	}

	IO.close();
	is_changed = false;

	return "";
}

// Backup config file
void vs2config::backup()
{
	// Create backup copy
	if (!new_file)
		IO.file_copy(file_name + ".bak");
}


// Set key (create or replace)
void vs2config::set_key(const string & section, const string &key, const string &value, const bool system_only)
{
	string s = vs2comlib::trim(vs2comlib::to_lower(section));
	string k = vs2comlib::trim(vs2comlib::to_lower(key));

	// Find section
	int i_s = -1;
	for (unsigned int i = 0; i < data.size(); i++)
	{
		if (data[i]->name == s)
		{
			i_s = i;
			break;
		}
	}

	// Section not found - add new
	if (i_s < 0)
	{
		S_record *rec = new S_record();
		rec->name = s;
		data.push_back(rec);
		i_s = (int)data.size() - 1;
	}

	// Find key
	int j_s = -1;
	for (unsigned int j = 0; j < data[i_s]->K_name.size(); j++)
	{
		if (data[i_s]->K_name[j] == k)
		{
			data[i_s]->K_value[j] = value;
			data[i_s]->K_system[j] = system_only;
			j_s = j;
			break;
		}
	}

	// If key not found - add new
	if (j_s < 0)
	{
		data[i_s]->K_name.push_back(k);
		data[i_s]->K_value.push_back(value);
		data[i_s]->K_system.push_back(system_only);
	}
	
	is_changed = true;
}

// Get key
string vs2config::get_key(const string & section, const string &key)
{
	unsigned long long idx = find_key(section, key);

	if (idx == 0xffffffffffffffff)
		return "";

	unsigned int i_sec = (idx >> 32);
	unsigned int i_key = (idx << 32 >> 32);
	
	return data[i_sec]->K_value[i_key];
}

// Get key
bool vs2config::delete_key(const string & section, const string &key)
{
	unsigned long long idx = find_key(section, key);

	if (idx == 0xffffffffffffffff)
		return false;

	unsigned int i_sec = (idx >> 32);
	unsigned int i_key = (idx << 32 >> 32);

	data[i_sec]->K_name.erase(data[i_sec]->K_name.begin() + i_key);
	data[i_sec]->K_value.erase(data[i_sec]->K_value.begin() + i_key);
	data[i_sec]->K_system.erase(data[i_sec]->K_system.begin() + i_key);

	// Delete section if no keys
	if (data[i_sec]->K_name.size() == 0)
	{
		delete(data[i_sec]);
		data.erase(data.begin() + i_sec);
	}

	is_changed = true;

	return true;
}

// Delete key
bool vs2config::changed()
{
	return is_changed;
}

// Get list of all sections
vector<string> vs2config::get_sections()
{
	vector<string> ret = vector<string>();

	for (size_t i = 0; i < data.size(); i++)
		ret.push_back(data[i]->name);

	return ret;
}

// Check if section exists
bool vs2config::section_exists(const string &name)
{
	string s = vs2comlib::trim(vs2comlib::to_lower(name));
	
	for (size_t i = 0; i < data.size(); i++)
		if (data[i]->name == s)
			return true;

	return false;
}

// Delete section
bool vs2config::delete_section(const string & section)
{
	string s = vs2comlib::trim(vs2comlib::to_lower(section));

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i]->name == s)
		{
			data[i]->K_name.clear();
			data[i]->K_system.clear();
			data[i]->K_value.clear();
			data.erase(data.begin() + i);

			is_changed = true;

			return true;
		}
	}
	return false;
}


/////////////// PRIVATE METHODS ////////////////

// Get key
unsigned long long vs2config::find_key(const string & section, const string &key)
{
	string s = vs2comlib::trim(vs2comlib::to_lower(section));
	string k = vs2comlib::trim(vs2comlib::to_lower(key));

	int i_s = -1;
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i]->name == s)
		{
			i_s = (int)i;
			break;
		}
	}

	if (i_s < 0)
		return 0xffffffffffffffff;

	// Find key
	for (unsigned int j = 0; j < data[i_s]->K_name.size(); j++)
	{
		if (data[i_s]->K_name[j] == k)
		{
			unsigned long long ret = i_s;
			ret = ret << 32;
			ret += j;
			return ret;
		}
	}

	return 0xffffffffffffffff;
}

// Generate config header
string vs2config::generate_header(unsigned int crc)
{
	return "[vstorage$00010001$" + vs2comlib::convert_int_to_hex_string(crc) + "$5Q462000]";
}
