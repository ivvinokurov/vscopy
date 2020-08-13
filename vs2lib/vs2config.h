#pragma once
#include <string>
#include <vector>

//#include "vlio.h"
#include "vs2iof.h"
#include "vs2iom.h"
#include "vs2comlib.h"
#include "vs2crc32.h"


using namespace std;


class vs2config final
{
private:

	// Section record
	class S_record
	{
	public:
		string name;						// Section name
		vector<string> K_name;				// Vector of key name
		vector<string> K_value;				// Vector of K-values
		vector<bool> K_system;				// Vector of 'system' indicators
	};

	// Configuration data
	vector<S_record *> data;				

	// Config file name
	string file_name = "";

	string file_key = "";

	// IO object
	vs2iof IO;

	// New file indicator
	bool new_file = true;

	bool is_changed = false;

	unsigned char* buf = nullptr;

	// PRIVATE METHODS
	string generate_header(unsigned int crc);

	unsigned long long find_key(const string & section, const string &key);

public:

	// Constructor
	vs2config();
	
	// Destructor
	~vs2config();

	// Load  config file
	string load(const string& name, const string& key = "");

	// Save config file
	string save();

	// Backup config file
	void backup();

	// Set key (create or replace)
	void set_key(const string & section, const string &key, const string &value, const bool system_only = false);

	// Get key
	string get_key(const string & section, const string &key);

	// Delete key
	bool delete_key(const string & section, const string &key);

	// Get changes status
	bool changed();

	// Get list of all sections
	vector<string> get_sections();

	// Check if section exists
	bool section_exists(const string &name);

	// Delete section
	bool delete_section(const string & section);

};
