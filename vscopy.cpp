// vscopy.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include <time.h>
#include <ctime>
#include <string>
#include <direct.h>
#include "vs2comlib.h"
#include "vs2iolib.h"
#include "vs2iof.h"
#include "vscdefs.h"
#include "vscrout.h"
#include "vscfsor.h"
#include "vscfsow.h"
#include "vscdefs.h"

using namespace std;

static const string vscp_version = "1.1.007";

vscdefs::COMMAND_DEFINITION* cmd = nullptr;	// Current command

int n_line = 0;
int rc = 0;							// Ret code
int rc_max = 0;						// Max ret code

string command_line = "";			// Command line

vector<string> cmd_lines_src;		// source lines w/o any transformation
vector<string> cmd_lines_cln;		// Lines after PARM processing
vector<int>    cmd_line_cln_no;		// Line# from the source
vector<string> cmd_lines;			// Lines for preprocessor
vector<int>	   cmd_line_no;			// Line# from the source

	// Global keys
string log_g = "";					// Log file name
bool even_g = false;				// Global 'even' for script (all commands)
bool nolist_g = false;				// No listing except errors
bool ls_g = false;					// List source script lines

vector<string> parm_names;			// Script header PARM names
vector<string> parm_values;			// Script command string PARM values

bool is_script = false;
string script_file = "";

int level = 0;						// If/Else level (0-no)
string* if_lv_cond = new string[255];// 'IF/ELSE' conditions for levels
string* if_lv_op = new string[255];	// Condition types 'IF/ELSE' for levels

vector<vscdefs::COMMAND_DEFINITION*> defs;
vector<vscdefs::COMMAND_DEFINITION*> defs_cond;
vscdefs::COMMAND_DEFINITION* def = nullptr;	// Current def

vscrout router;						// Console/log router

int task_count = 0;

void tst()
{
	string s = "1234:567::89a::qwe";
	int i = vs2comlib::index_of_last(s, (string)"::");
	int a = i;
}

/// <summary>
/// Purge dir - remove all contents (static)
// root - remove root dir itself (true) or no (false)
/// </summary>
bool purge(string& path, bool root, bool system)
{
	bool rcp = true;

	// 1. Check if src dir exists 
	if (!vs2iolib::directory_exists(path))
		return rcp;

	// 2. remove files
	vector<string> vf = vs2iolib::get_file_list(path + (string)"\\*", true);

	for (unsigned int i = 0; i < vf.size(); i++)
	{
		string nm = path + "\\" + vf[i];
		if (remove(nm.c_str()) != 0)
		{
			router.write_msg_error(vscdefs::DSP_PUR, vscdefs::DSPT_FIL, "Purge error", nm);

			rcp = false;
			break;
		}
		else
			router.write_msg_ext(vscdefs::DSP_PUR, vscdefs::DSPA_DELETED + vscdefs::DSPT_FIL, vf[i]);
	}

	vf.clear();

	if (rcp)
	{
		// 3. remove dirs
		vector<string> vd = vs2iolib::get_directory_list(path + "\\*", true);

		for (unsigned int i = 0; i < vd.size(); i++)
		{
			string nm = path + "\\" + vd[i];

			if (!purge(nm, true, system))
			{
				router.write_msg_error(vscdefs::DSP_PUR, vscdefs::DSPT_DIR, "Purge error", nm);
				rcp = false;
				break;
			}
		}

		vd.clear();
	}

	// Remove root if required
	if (rcp & root)
	{
		if (_rmdir(path.c_str()) == 0)
		{
			router.write_msg_ext(vscdefs::DSP_PUR, vscdefs::DSPA_DELETED + vscdefs::DSPT_DIR, path);

			return true;
		}
		else
		{
			router.write_msg_ext(vscdefs::DSP_PUR, vscdefs::DSPT_DIR, "Purge error", path);
			return false;
		}
	}
	else
		return rcp;
}


// Compare for mirror_dirs. Predicted compare (compressed/uncompressed)
static bool compare_fuction(string& src, string& trg, bool compress)
{
	string s0 = vs2comlib::to_lower(src);
	string t0 = vs2comlib::to_lower(trg);

	if (compress)
	{
		if (vscfsor::IsFileExt(t0, vscdefs::SEC_FILE_EXT))
			t0 = vs2iolib::get_file_name_wo_ext(t0);

		if (vscfsor::IsFileExt(s0, vscdefs::SEC_FILE_EXT))
			s0 = vs2iolib::get_file_name_wo_ext(s0);

	}

	return (s0 == t0);
}

// Mirror directories
string mirror(const string source_dir, const string target_dir, unsigned long int dir_level, bool compress)
{
	////////// Remove excessive contents (MIRROR)
	string source = source_dir;
	string target = target_dir;

	string m_rc = "";

	if (dir_level == 0)
	{
		string s1 = vs2iolib::get_file_name(source);
		string s2 = vs2iolib::get_file_name(target);

		if (vs2comlib::to_lower(s1) != vs2comlib::to_lower(s2))
			if (vs2iolib::directory_exists(target + "\\" + s1))			// Target name != source, but child name = source 
				target = target + "\\" + s1;
	}
	// Files
	vector<string> src_f = vs2iolib::get_file_list(source + "\\*", ((cmd->CD_BYTE_PARM & vscdefs::PARM_SYSOBJ) != 0));
	vector<string> trg_f = vs2iolib::get_file_list(target + "\\*", ((cmd->CD_BYTE_PARM & vscdefs::PARM_SYSOBJ) != 0));

	for (unsigned int i = 0; i < trg_f.size(); i++)
	{
		bool f = false;
		for (unsigned int j = 0; j < src_f.size(); j++)
		{
			if (compare_fuction(src_f[j], trg_f[i], compress))
			{
				f = true;
				break;
			}
		}
		if (!f)
		{
			string nm = target + "\\" + trg_f[i];

			router.write_msg_ext(vscdefs::DSP_MIR, vscdefs::DSPA_DELETED + vscdefs::DSPT_FIL, nm);


			if (!vs2iolib::rmfile(nm))
			{
				router.write_msg_error(vscdefs::DSPC_ERROR, "Error when deleting file", nm);
				if (((cmd->CD_BYTE_MODE && vscdefs::MODE_EVEN) == 0))
				{
					m_rc = vscdefs::MSGLEVEL_ERROR;
					break;
				}
			}
		}
	}

	src_f.clear();
	trg_f.clear();

	if (m_rc == "")
	{
		string rc2 = "";

		// Dirs
		vector<string> src_d = vs2iolib::get_directory_list(source + "\\*", true);	// Source
		vector<string> trg_d = vs2iolib::get_directory_list(target + "\\*", true); // Target

		unsigned int i = 0;
		while (i < trg_d.size())
		{
			bool f = false;
			unsigned int j = 0;
			while (j < src_d.size())
			{
				if (vs2comlib::to_lower(trg_d[i]) == vs2comlib::to_lower(src_d[j]))
				{
					f = true;
					break;
				}
				j++;
			}

			if (f)
			{
				rc2 = mirror(source + "\\" + src_d[j], target + "\\" + trg_d[i], dir_level + 1, compress);
				i++;

				if (rc2 != "")
				{
					if (((cmd->CD_BYTE_MODE && vscdefs::MODE_EVEN) == 0))
					{
						m_rc = rc2;
						break;
					}
				}
			}
			else
			{
				string nm = target + "\\" + trg_d[i];

				router.write_msg_ext(vscdefs::DSP_MIR, vscdefs::DSPA_DELETED + vscdefs::DSPT_DIR, nm);

				if (!vs2iolib::rmdir(nm))
				{
					router.write_msg_error(vscdefs::DSPC_ERROR, "Error when deleting directory", nm);
					rc2 = vscdefs::MSGLEVEL_ERROR;
					if (((cmd->CD_BYTE_MODE && vscdefs::MODE_EVEN) == 0))
					{
						m_rc = rc2;
						break;
					}
				}
				else

				trg_d.erase(trg_d.begin() + i);
			}
		}

		src_d.clear();
		trg_d.clear();
	}
	return m_rc;
}


// Copy file/directory
string cp(vscdefs::COMMAND_DEFINITION* command)
{
	clock_t begin = clock();

	// Prepare space string
	cmd = command;						// Save command

	vscfsor* fsor = new vscfsor();
	vscfsow* fsow = new vscfsow();

	//router.write_msg_ext(vscdefs::DSP_CPY, "", vscdefs::DSPC_STARTED, cmd->CD_SOURCE);

	string m_rc = fsor->Open(&router, command);

	if (m_rc != "")
		return m_rc;											// Error happened on Open Source

	bool m_even = ((cmd->CD_BYTE_MODE & vscdefs::MODE_EVEN) != 0);

	string f_name = cmd->CD_TARGET;

	/* INPUT IS FILE */
	if (fsor->IsFile())
	{
		if (vs2iolib::directory_exists(f_name))					// Output is directory?
			f_name += ("\\" + vs2iolib::get_file_name(cmd->CD_SOURCE));
		else
		{
			string path = vs2iolib::get_file_path(f_name);

			if (path == "")				// No path - consider file name in the same directory
				f_name = vs2iolib::get_file_path(cmd->CD_SOURCE) + "\\" + f_name;
			else if (!vs2iolib::directory_exists(path))					// Output directory exists?
			{
				router.write_msg_error(vscdefs::DSPC_ERROR, "Destination directory is not found", path);
				m_rc = vscdefs::MSGLEVEL_ERROR;								// Error happened on Open Target
			}
		}

		if (m_rc == "")
			m_rc = fsow->Open(&router, cmd, 0);
	}
	else
	{
		/* INPUT IS DIR */

		m_rc = fsow->Open(&router, cmd, 0);

		if (m_rc == "")
		{
			// Mirror
			if ((cmd->CD_BYTE_TARG & vscdefs::TARG_MODE_MIRROR) != 0)
			{
				router.write_msg_ext(vscdefs::DSP_MIR, vscdefs::DSPC_STARTED, cmd->CD_SOURCE + " --> " + cmd->CD_TARGET);

				m_rc = mirror(cmd->CD_SOURCE, fsow->GetName(), 0, (cmd->CD_BYTE_PARM & vscdefs::PARM_COMPRESS) != 0);

				router.write_msg_ext(vscdefs::DSP_MIR, vscdefs::DSPC_ENDED);

				if ((m_rc != "") && (m_even))
					m_rc = "";
			}
			// Purge
			else if ((cmd->CD_BYTE_TARG & vscdefs::TARG_MODE_PURGE) != 0)
			{
				router.write_msg_ext(vscdefs::DSP_PUR, vscdefs::DSPC_STARTED, cmd->CD_TARGET);

				if (!purge((string&)fsow->GetName(), false, (cmd->CD_BYTE_PARM & vscdefs::PARM_SYSOBJ) != 0))
				{
					if (!m_even)
						m_rc = vscdefs::MSGLEVEL_ERROR;
				}
				router.write_msg_ext(vscdefs::DSP_PUR, vscdefs::DSPC_ENDED);
			}
		}
	}

	if (m_rc == "")
	{
		vscdefs::DIR_INFO* di = (vscdefs::DIR_INFO*)(fsor->GetNextDir());


		while ((di != nullptr) && (m_rc == ""))
		{
			bool cmp_dir = fsor->IsComressionRequired(di->name, true, di->level);


			if (di->rc != "")
			{
				m_rc = di->rc;
				break;
			}

			m_rc = fsor->IsFile() ? fsow->OpenDir(vs2iolib::get_file_path(f_name), 0, 0) : fsow->OpenDir(vs2iolib::get_file_name(di->name), di->level, di->attrs);

			if (m_rc != "")
			{
				if (m_rc != "")
				{
					if (m_even)
						m_rc = "";
					else
						break;
				}
			}
			else
			{
				if ((cmd->CD_COMMAND == vscdefs::CMD_LIST) && ((cmd->CD_BYTE_PARM & vscdefs::PARM_ZERO_LEN) == 0))
				{
					router.write_msg_ext(cmd->CD_COMMAND_DSP, vscdefs::DSPT_DIR, cmd->CD_SOURCE_DIR);
					cmd->CD_STAT_DIRS++;
				}

				string fcond = "";

				vscdefs::FILE_INFO* fi = (vscdefs::FILE_INFO*)(fsor->GetNextFile());

				while ((fi != nullptr) && (m_rc == ""))
				{
					m_rc = fi->rc;

					if (m_rc != "")
					{
						if (m_even & (!fsor->IsSAF()))
							m_rc = "";
						else
							break;
					}
					else
					{
						if (cmd->CD_COMMAND == vscdefs::CMD_COPY)
						{
							// Check for zero length
							if ((fi->original_length == 0) && ((cmd->CD_BYTE_PARM & vscdefs::PARM_ZERO_LEN) == 0))
							{
								router.write_msg_ext(cmd->CD_COMMAND_DSP, vscdefs::DSPA_SKIPPED + vscdefs::DSPT_FIL, cmd->CD_SOURCE_DIR + "\\" + cmd->CD_SOURCE_FILE + +"{ZERO LENGTH}");
								cmd->CD_STAT_FILES_SKIPPED++;
							}
							else
							{
								string fn = fsor->IsFile() ? f_name : vs2iolib::get_file_name(fi->name);

								if (!fsow->VerifyFileName(fn, fi->name))
								{
									router.write_msg_ext(cmd->CD_COMMAND_DSP, vscdefs::DSPA_SKIPPED + vscdefs::DSPT_FIL, "Source and destination files are equal: '" + fi->name + "'");
									cmd->CD_STAT_FILES_SKIPPED++;
									cmd->CD_STAT_FILES_SKIPPED_SIZE += fi->original_length;
									if (m_even)
										m_rc = vscdefs::MSGLEVEL_ERROR;								// Error happened on Open Target
								}
								else
								{
									
									bool cmp_file = cmp_dir? true : fsor->IsComressionRequired(fn, false);

									m_rc = fsow->OpenFile(fn, fi->attrs, fi->original_length, fi->timestamp, &fcond, cmp_file);


									if (m_rc != "")
									{
										if (m_even & (!fsow->IsSAF()))
											m_rc = "";
										else
											break;
									}

									short cl = ((fcond == vscdefs::DSPA_SKIPPED) ? vs2iolib::COLOR_BLUE : ((fcond == vscdefs::DSPA_REPLACED) ? vs2iolib::COLOR_YELLOW : vs2iolib::COLOR_WHITE));

									vs2iolib::set_font_color(cl);

									router.write_msg_ext(cmd->CD_COMMAND_DSP, fcond + vscdefs::DSPT_FIL, vs2comlib::pad_left(vs2comlib::formatted_size(fi->original_length), 9, " ") + (cmp_file? " *" : "  ") + cmd->CD_SOURCE_FILE);

									vs2iolib::set_font_color(vs2iolib::COLOR_WHITE);

									if (fcond != vscdefs::DSPA_SKIPPED)
									{
										cmd->CD_STAT_FILES++;
										cmd->CD_STAT_FILES_SIZE += fi->original_length;

										fi = fsor->GetData();

										while ((fi->mode == vscdefs::FILE_MODE_OPENED) && (fi->rc == ""))
										{
											m_rc = fsow->WriteData(fi->buffer, fi->length);

											if (m_rc != "")
												break;

											fi = fsor->GetData();
										}
									}
									else
									{
										cmd->CD_STAT_FILES_SKIPPED_SIZE += fi->original_length;
										cmd->CD_STAT_FILES_SKIPPED++;
									}

									fsow->CloseFile();
								}
							}
						}
						else
						{
							if ((cmd->CD_BYTE_PARM & vscdefs::PARM_ZERO_LEN) != 0)
							{
								if (fi->original_length == 0)
								{
									router.write_msg_ext(cmd->CD_COMMAND_DSP, vscdefs::DSPT_FIL, vs2comlib::pad_left(vs2comlib::formatted_size(fi->original_length), 9, " "), cmd->CD_SOURCE_DIR + "\\" + cmd->CD_SOURCE_FILE);
									cmd->CD_STAT_FILES++;
								}
								else
								{
									cmd->CD_STAT_FILES_SKIPPED++;
									cmd->CD_STAT_FILES_SKIPPED_SIZE += fi->original_length;
								}
							}
							else
							{
								router.write_msg_ext(cmd->CD_COMMAND_DSP, vscdefs::DSPT_FIL, vs2comlib::pad_left(vs2comlib::formatted_size(fi->original_length), 9, " "), cmd->CD_SOURCE_FILE);
								cmd->CD_STAT_FILES++;
								cmd->CD_STAT_FILES_SIZE += fi->original_length;
							}
						}
					}

					if ((m_rc == "") && (fi->rc == ""))
						fi = (vscdefs::FILE_INFO*)(fsor->GetNextFile());
				}
			}

			if ((m_rc == "") && (di->rc == ""))
				di = (vscdefs::DIR_INFO*)(fsor->GetNextDir());
		}
	}

	string rcw = fsow->Close();
	string rcr = fsor->Close();


	string op = (cmd->CD_COMMAND == vscdefs::CMD_COPY) ? "Copied" : "Listed";
	router.empty_line();
	router.write_msg_ext("SUMMARY:               Total              " + op + "             Skipped");
	router.write_msg_ext("Dirs   :" + vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_DIRS + cmd->CD_STAT_DIRS_SKIPPED), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_DIRS), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_DIRS_SKIPPED), 20, " "));

	router.write_msg_ext("Files  :" + vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES + cmd->CD_STAT_FILES_SKIPPED), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES_SKIPPED), 20, " "));

	router.write_msg_ext("Bytes  :" + vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES_SIZE + cmd->CD_STAT_FILES_SKIPPED_SIZE), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES_SIZE), 20, " ") +
		vs2comlib::pad_left(vs2comlib::formatted_number(cmd->CD_STAT_FILES_SKIPPED_SIZE), 20, " "));


	clock_t end = clock();
	string es = to_string(((double)end - (double)begin) / CLOCKS_PER_SEC);

	router.empty_line();
	router.write_msg_ext("ELAPSED TIME: " + es.substr(0, es.size() - 4) + (string)" Sec");

	delete fsor;
	delete fsow;

	return (m_rc != "") ? m_rc : ((rcr != "") ? rcr : rcw);
}

// Return formatted command
string get_cmd(string arg = "")
{
	return "#" + vs2comlib::to_upper(((arg == "")? def->CD_COMMAND : arg));
}

// Display preprocessor error message 
void display_msg(string pref, int line_n, string msg1, string msg2 = "")
{
	string s = msg1;

	if (line_n > 0)
		s += " at script line #" + to_string(line_n);

	router.write_msg_ext(pref, s, msg2);
}

// Cur off head/tail ('") 
string get_cond_string(string value)
{
	if (value.size() < 2)
		return value;

	if ((value.substr(0, 1) == "'") | (value.substr(0, 1) == """"))
	{
		if (value.substr(0, 1) == value.substr(value.size() - 1, 1))
			return (value.size() == 2) ? "" : value.substr(1, value.size() - 2);
		else
			return value;
	}
	else return value;
}

// Get password from console
string get_input_value(string msg, string value)
{
	string s = get_cond_string(value);
	
	if (s != "")
	{
		if ((s.substr(0, 1) == vscdefs::PARM_KEY_SHOW) | (s.substr(0, 1) == vscdefs::PARM_KEY_HIDE))
		{
			display_msg(msg, -1, "");
			s = vs2iolib::get_console_input((s.substr(0, 1) == vscdefs::PARM_KEY_HIDE));
			router.empty_line();
		}
	}
	return s;
}

////////////////////////////////////////////////////////////////////////////
///// Parse initial input: command line [+script]                      /////
///// Command line is always 1st line, scripr starts from line #2(run) /////
///// Result in: cmd_lines_src                                         /////
////////////////////////////////////////////////////////////////////////////
int parse_input(int argn, char* args[])
{
	string msg = "";

	if (argn > 1)
	{
		string cm = args[1];

		if (vs2comlib::to_lower(cm) == vscdefs::CMD_RUN)
		{
			is_script = true;
			script_file = (string)args[2];

			if (argn < 3)
			{
				msg = "Missing script file name";
				rc = vscdefs::RC_FAILED;
			}
			else if (!vs2iolib::file_exists(args[2]))
			{
				msg = "Script file is not found - " + (string)args[2];
				rc = vscdefs::RC_FAILED;
			}
			if (rc == vscdefs::RC_SUCCESS)
			{
				vs2iof sf;
				sf.open(args[2], vs2iolib::FILE_MODE_OPEN);
				string s = sf.read_string(sf.get_length());
				sf.close();

				long long pos = 0;
				// Read and parse script
				while (pos < (long long)s.size())
				{
					long long s_length = 0;							// Length of the string
					string str = "";

					long long new_pos = (long long)s.find_first_of(vscdefs::P_NEWLINE, (size_t)pos);
					if (new_pos == pos)
					{
						cmd_lines_src.push_back("");
						pos+=2;
					}
					else
					{
						if (new_pos < 0)
						{
							s_length = s.size() - pos;
							str = vs2comlib::trim(s.substr((size_t)pos, (size_t)s_length));
							pos = s.size();
						}
						else
						{
							s_length = new_pos - pos;
							str = vs2comlib::trim(s.substr((size_t)pos, (size_t)s_length));
							pos = new_pos + 2;
						}

						cmd_lines_src.push_back(vs2comlib::trim(str));
					}
				}
			}
		}
	}
	else
	{
		msg = "Nothing to do - missing command line parameters";
		router.empty_line();
	}

	// Parse global parameters
	if ((rc == vscdefs::RC_SUCCESS) & is_script)
	{
		vector<string> cmd_keys = vs2comlib::parse_ext(vs2comlib::trim(cmd_lines_src[0]), (string)" ");

		for (size_t i = 2; i < cmd_keys.size(); i++)
		{
			string s = vs2comlib::to_lower(cmd_keys[i]);
			
			if (is_script)
			{
				if (s == vscdefs::C_MODE_EVEN)
					even_g = true;
				else if (s == vscdefs::C_MODE_NOLIST)
					router.suppress(true);
				else if (s == vscdefs::PARM_LS)
					ls_g = true;
				else if (s.find(vscdefs::PARM_LOG, 0) == 0)
					router.set_log(get_cond_string(cmd_keys[i].erase(0, vscdefs::PARM_LOG.size())));
				else
				{
					if ((s.substr(0, 1) == "-") || (s.substr(0, 1) == "/"))
					{
						msg = "Invalid global key - '" + cmd_keys[i] + "'";
						rc = vscdefs::RC_FAILED;
						break;
					}
					else
						parm_values.push_back(cmd_keys[i]);				// Save script parameter
				}
			}
			else
				if (s.find(vscdefs::PARM_LOG, 0) == 0)
				{
					router.set_log(get_cond_string(cmd_keys[i].erase(0, vscdefs::PARM_LOG.size())));
				}
		}
	}


	if (rc != vscdefs::RC_SUCCESS)
		display_msg(vscdefs::DSPC_ERROR, -1, msg);

	return rc;
}

////////////////////////////////////////////////////////////////////////////
///// Skip comments                                                    /////
///// Parse PARM line of the script                                    /////
///// Validate against specified parameter							   /////
///// Print script lines if -ls is specified                           /////
///// Result in: cmd_lines_cln                                         /////
////////////////////////////////////////////////////////////////////////////
int prepare_command_lines()
{
	int seq_n = 0;															// Script line w/o empty and comments - to detect PARM is 1st line

	// If not script - parse 1st line
	if (!is_script)
	{
		cmd_lines_cln.push_back(vs2comlib::trim(cmd_lines_src[0]));
		cmd_line_cln_no.push_back(1);
	}
	else
	{
		if (ls_g)
			router.write_msg_ext("<SCRIPT " + script_file + ">");

		for (size_t i = 1; i < cmd_lines_src.size(); i++)
		{
			n_line++;
			if (ls_g)
				display_msg("", -1, "    Line#" + vs2comlib::pad_left(to_string(n_line), 3, (string)" ") + (string)": " + cmd_lines_src[i]);

			string str = cmd_lines_src[i];

			// Ignore all-line comments (starts from '*' or '-')
			if ((str.substr(0, 1) != "*") && (str.substr(0, 1) != "-"))
			{

				// Eliminate front tabs and spaces
				for (int j = 0; j < str.size(); j++)
				{
					if ((str.substr(j, 1) != "\t") && (str.substr(j, 1) != " "))
					{
						if (j > 0)
							str.erase(0, j);
						break;
					}
				}

				// Eliminate tail in-line comments
				for (int j = ((int)str.size() - 2); j >= 0; j--)
				{
					if (str.substr(j, 2) == "--")
					{
						str.erase(j, str.size() - j);
						break;
					}
				}

				//str = vs2comlib::to_lower(vs2comlib::trim(str));

				if (str != "")
				{
					seq_n++;

					vector<string> vxl = vs2comlib::parse_ext(str, " ");						// Parse command line as-is
					vector<string> vx = vs2comlib::parse_ext(vs2comlib::to_lower(str), " ");	// Parse command line to lowercase

					// Handle PARM line - must be the 1st in script
					if ((vx[0] == vscdefs::CMD_PARM) && (seq_n == 1))
					{ 

						for (size_t np = 1; np < vx.size(); np++)					// Create parm array
						{
							parm_names.push_back(vx[np]);
						}

						if (parm_values.size() > parm_names.size())
						{
							display_msg(vscdefs::DSPC_ERROR, -1, "The number of specified parameters exceeds the number of script parameters");
							rc = vscdefs::RC_FAILED;
							break;
						}

						// Check for duplicated names
						for (size_t nd = 0; nd < (parm_names.size() - 1); nd++)	
						{
							for (size_t nx = (nd + 1); nx < parm_names.size(); nx++)
							{
								if (parm_names[nd] == parm_names[nx])
								{
									display_msg(vscdefs::DSPC_ERROR, -1, "Duplicated parameter name - '" + parm_names[nx] + "'");
									rc = vscdefs::RC_FAILED;
									break;
								}
							}
						}
					}
					else
					{
						// Substitute parameters
						if (parm_names.size() > 0)
						{
							string stx = "";
							size_t ix = 0;

							while (ix < str.size())
							{
								string ch = str.substr(ix, 1);
								ix++;

								if (ch != "&")
									stx += ch;											// Single char, not var
								else
								{
									for (size_t jx = 0; jx < parm_names.size(); jx++)
									{
										size_t j_size = parm_names[jx].size();

										if (j_size <= (str.size() - ix))	// Check for var name
										{
											if (vs2comlib::to_lower(str.substr(ix, j_size)) == parm_names[jx])
											{
												// Conditions for var
												// 1. End of string; 2. ' ' after; 3. '.' after
												bool is_var = ((ix + j_size) == str.size());
												if (!is_var)
													is_var = ((str.substr(ix + j_size, 1) == " ") || (str.substr(ix + j_size, 1) == "."));

												if (is_var)											// Var is identified
												{
													ix += j_size;

													if (parm_values.size() > jx)
													{
														string cx = "";

														if (parm_values[jx] == vscdefs::PARM_PARM_HIDE)
															cx = vscdefs::PARM_KEY_HIDE;
														else if (parm_values[jx] == vscdefs::PARM_PARM_SHOW)
															cx = vscdefs::PARM_KEY_SHOW;
														else
															stx += parm_values[jx];

														if (cx != "")
															stx += get_input_value("Enter parameter '" + parm_names[jx] + "' value:", cx);
													}

													if (str.substr(ix, 1) == ".")
														ix++;

													break;
												}
											}
										}
									}
								}
							}

							if ((ls_g) && (str != stx))
							{
								display_msg("", -1, "    >>>>>" + vs2comlib::pad_left(to_string(n_line), 3, (string)" ") + (string)": " + stx);

								str = stx;
							}
						}

						cmd_lines_cln.push_back(str);
						cmd_line_cln_no.push_back(n_line);
					}
				}
			}
		}

		if (ls_g)
			router.write_msg_ext("<\\SCRIPT>");
	}

	return rc;
}

////////////////////////////////////////////////////////////////////////////
///// Parse and validate language (IF/ELSE, ...)                       /////
///// Result in: cmd_lines                                             /////
////////////////////////////////////////////////////////////////////////////
int validate_script_lng()
{
	for (size_t i = 0; i < cmd_lines_cln.size(); i++)
	{
		// Parse command line
		vector<string> vx = vs2comlib::parse_ext(vs2comlib::trim(cmd_lines_cln[i]), " ");
		
		string p0 = vs2comlib::to_lower(vx[0]);					// 1st parameter

		string p1 = "";
		if (vx.size() > 1)
			p1 = vs2comlib::to_lower(vx[1]);						// 2nd parameter;
		string p2 = "";

		////////////////// ON ////////////////
		if (p0 == vscdefs::CMD_ON)
		{ // ON
			bool on_ok = ((vx.size() == 2) || (vx.size() == 3));

			string cond = vscdefs::COND_ON_STOP;

			if (on_ok)
				on_ok = (p1 == vscdefs::CMD_ON_ERROR);

			if (on_ok)
			{
				if (vx.size() > 2)
				{
					p2 = vs2comlib::to_lower(vx[2]);				// 3rd parameter
					if (p2 == vscdefs::COND_ON_CONT)
						cond = vscdefs::COND_ON_CONT;
					else if (p2 != vscdefs::COND_ON_STOP)
						on_ok = false;
				}
			}

			if (on_ok)
			{
				cmd_lines.push_back(vx[0] + " " + vx[1] + " " + cond);
				cmd_line_no.push_back(cmd_line_cln_no[i]);
			}
			else
			{
				display_msg(vscdefs::DSPC_ERROR, cmd_line_cln_no[i], "Invalid ON command parameter");
				rc = vscdefs::RC_FAILED;
			}
		}
		////////////////// IF ////////////////
		else if (p0 == vscdefs::CMD_IF)
		{
			if ((vx.size() < 2) || ((p1 != vscdefs::COND_SUCCESS) && (p1 != vscdefs::COND_ERROR)))
			{
				display_msg(vscdefs::DSPC_ERROR, cmd_line_cln_no[i], "Invalid or missing IF command parameter");
				rc = vscdefs::RC_FAILED;
			}
			else
			{
				cmd_lines.push_back(vscdefs::CMD_IF + " " + vx[1]);			// IF + cond
				cmd_line_no.push_back(cmd_line_cln_no[i]);					// Line#

				if (vx.size() > 2)
				{
					string s = "";

					for (int j = 2; j < vx.size(); j++)
						s += vx[j] + " ";

					cmd_lines.push_back(vs2comlib::trim(s));
					cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#

					cmd_lines.push_back(vscdefs::CMD_END);
					cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
				}
			}
		}
		////////////////// ELSE ////////////////
		else if (p0 == vscdefs::CMD_ELSE)
		{
			cmd_lines.push_back(vscdefs::CMD_ELSE);								// ELSE
			cmd_line_no.push_back(cmd_line_cln_no[i]);					// Line#

			if (vx.size() > 1)
			{
				string s = "";

				for (int j = 1; j < vx.size(); j++)
					s += vx[j] + " ";

				cmd_lines.push_back(vs2comlib::trim(s));
				cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#

				cmd_lines.push_back(vscdefs::CMD_END);
				cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
			}
		}
		////////////////// STOP ////////////////
		else if (p0 == vscdefs::CMD_STOP)
		{
			if (vx.size() == 1)
			{
				cmd_lines.push_back(vx[0] + " " + vscdefs::COND_ERROR);
				cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
			}
			else if ((vx.size() > 1) && ((p1 == vscdefs::COND_ERROR) || (p1 == vscdefs::COND_SUCCESS)))
			{
				cmd_lines.push_back(vx[0] + " " + vx[1]);
				cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
			}
			else
			{
				display_msg(vscdefs::DSPC_ERROR, cmd_line_cln_no[i], "Invalid STOP command parameter");
				rc = vscdefs::RC_FAILED;
			}
		}
		////////////////// SET ////////////////
		else if (p0 == vscdefs::CMD_SET)
		{ 
			if ((vx.size() != 2) || (!((p1 == vscdefs::COND_SUCCESS) || (p1 == vscdefs::COND_ERROR))))
			{
				display_msg(vscdefs::DSPC_ERROR, cmd_line_cln_no[i], "Invalid SET command");
				rc = vscdefs::RC_FAILED;
			}
			else
			{
				cmd_lines.push_back(vx[0] + " " + vx[1]);
				cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
			}
		}
		////////////////// OTHER COMMANDS ////////////////
		else
		{
			cmd_lines.push_back(cmd_lines_cln[i]);
			cmd_line_no.push_back(cmd_line_cln_no[i]);				// Line#
		}

		if (rc == vscdefs::RC_FAILED)
			break;
	}

	return rc;
}

////////////////////////////////////////////////////////////////////////////
///// Preprocessor                                                     /////
///// Result in: defs                                                  /////
////////////////////////////////////////////////////////////////////////////
int preprocessor()
{
	

	for (int i = 0; i < 255; i++)				// Initial level array
	{
		if_lv_cond[i] = "";
		if_lv_op[i] = "";
	}

	for (size_t line_count = 0; line_count < cmd_lines.size(); line_count++)
	{
		def = new vscdefs::COMMAND_DEFINITION();
		def->CD_RAW = cmd_lines[line_count];

		// Handle global parms
		if (even_g)
			def->CD_BYTE_MODE |= vscdefs::MODE_EVEN;

		if (nolist_g)
			def->CD_BYTE_MODE |= vscdefs::MODE_NOLIST;

		vector<string> ca = vs2comlib::parse_ext(cmd_lines[line_count], " ");				// Command and argumenets

		string a0 = vs2comlib::to_lower(ca[0]);

		def->CD_COMMAND = a0;

		def->CD_CMDLINE = "";

		for (size_t n = 1; n < ca.size(); n++)
			def->CD_CMDLINE += ((n == 1) ? "" : " ") + ca[n];

		if (a0 == vscdefs::CMD_ON)
		{ // ON
			def->CD_ON = vs2comlib::to_lower(ca[1]);
			def->CD_ON_COND = vs2comlib::to_lower(ca[2]);
		}
		else if (a0 == vscdefs::CMD_STOP)
		{ // STOP
			def->CD_RC = (vs2comlib::to_lower(ca[1]) == vscdefs::COND_SUCCESS) ? 0 : 8;
		}
		else if (a0 == vscdefs::CMD_SET)
		{ // SET
			def->CD_IE_COND = vs2comlib::to_lower(ca[1]);
		}
		else if (a0 == vscdefs::CMD_IF)
		{ // IF 
			def->CD_IE_COND = vs2comlib::to_lower(ca[1]);
			def->CD_LEVEL = level;
			if_lv_cond[level] = def->CD_IE_COND;
			if_lv_op[level] = vscdefs::CMD_IF;
			level++;							// Increase level
		}
		else if (a0 == vscdefs::CMD_ELSE)
		{ // ELSE
			if (if_lv_op[level] != vscdefs::CMD_IF)
			{
				rc = vscdefs::RC_FAILED;
				display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "ELSE command mismatch");
			}
			else
			{
				def->CD_IE_COND = (if_lv_cond[level] == vscdefs::COND_SUCCESS) ? vscdefs::COND_ERROR : vscdefs::COND_SUCCESS;		// Reverse 'If' condition
				def->CD_LEVEL = level;
				if_lv_cond[level] = def->CD_IE_COND;
				if_lv_op[level] = vscdefs::CMD_ELSE;
				level++;						// Increase level
			}
		}
		else if (a0 == vscdefs::CMD_END)
		{ // END
			if (level == 0)
			{
				rc = vscdefs::RC_FAILED;
				display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Misplaced END command");
			}
			else
			{
				level--;						// Decrease level
				def->CD_LEVEL = level;
			}
		}
		else if (a0 == vscdefs::CMD_OS_COMMAND)
		{ // OS command
			// NONE
		}
		else if ((a0 == vscdefs::CMD_COPY) || (a0 == vscdefs::CMD_COPYX) || (a0 == vscdefs::CMD_LIST))
		{
			for (size_t i = 1; i < ca.size(); i++)
			{
				string ai = vs2comlib::to_lower(ca[i]);

				if (ai.substr(0, 1) == "/")
					ai = ai.replace(0, 1, "-");

				if (ai.substr(0, 1) == "-")
				{  
					if (ai == vscdefs::C_PARM_RECURSIVE)
						def->CD_BYTE_PARM |= vscdefs::PARM_RECURSIVE;
					else if (ai == vscdefs::C_PARM_ATTRIBUTES)
						def->CD_BYTE_PARM |= vscdefs::PARM_ATTRIBUTES;
					else if (ai == vscdefs::C_PARM_CRC)
						def->CD_BYTE_PARM |= vscdefs::PARM_CRC;
					else if (ai == vscdefs::C_PARM_ZERO_LEN)
						def->CD_BYTE_PARM |= vscdefs::PARM_ZERO_LEN;
					else if (ai == vscdefs::C_PARM_DIR_EMPTY)
						def->CD_BYTE_PARM |= vscdefs::PARM_DIR_EMPTY;
					else if (ai == vscdefs::C_PARM_SYSOBJ)
						def->CD_BYTE_PARM |= vscdefs::PARM_SYSOBJ;
					else if (ai == vscdefs::C_MODE_NOLIST)
						def->CD_BYTE_MODE |= vscdefs::MODE_NOLIST;
					else if (ai == vscdefs::C_MODE_EVEN)
						def->CD_BYTE_MODE |= vscdefs::MODE_EVEN;
					else if (ai == vscdefs::C_TARG_SAF)
						def->CD_BYTE_TARG |= vscdefs::TARG_MODE_SAF;
					else if (ai == vscdefs::PARM_CHK)
						def->CD_CHK = true;
					else if (ai == vscdefs::C_TARG_PURGE)
						def->CD_BYTE_TARG |= vscdefs::TARG_MODE_PURGE;
					else if (ai == vscdefs::C_TARG_MIRROR)
						def->CD_BYTE_TARG |= vscdefs::TARG_MODE_MIRROR;
					else if (ai == vscdefs::C_REPL_NONE)
						def->CD_BYTE_REPL |= vscdefs::REPL_NONE;
					else if (ai == vscdefs::C_REPL_ALL)
						def->CD_BYTE_REPL |= vscdefs::REPL_ALL;
					else
					{
						if (ai == vscdefs::PARM_VOL_S1)
							def->CD_VOL = 0;															// Use default volume size;
						else if (ai.find(vscdefs::PARM_VOL_L1, 0) == 0)										// Vol-value defined
						{
							ai.erase(0, vscdefs::PARM_VOL_L1.size());

							if (vs2comlib::is_numeric(ai))
								def->CD_VOL = vs2comlib::convert_string_to_long(ai);
							else
							{
								long long p = ai.find_last_not_of("0123456789");
								if ((p == 0) | (p != (ai.size() - 1)))
								{
									rc = vscdefs::RC_FAILED;
									display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Invalid VOL parameter");
								}
								else
								{
									def->CD_VOL = vs2comlib::convert_string_to_long(ai.substr(0, (size_t)p));
									ai.erase(0, (size_t)p);
									if (ai == "k")
										def->CD_VOL *= 1024;
									else if (ai == "m")
										def->CD_VOL *= 1048576;
									else if (ai == "g")
										def->CD_VOL *= 1073741824;
									else
									{
										rc = vscdefs::RC_FAILED;
										display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Invalid VOL parameter");
									}
								}
							}
							if ((rc == 0) & (def->CD_VOL < (vscdefs::CHUNK_SIZE * 2)))
								def->CD_VOL = vscdefs::CHUNK_SIZE * 2;
						}
						else if (ai.find(vscdefs::PARM_EXCL, 0) == 0)
						{
							ai.erase(0, vscdefs::PARM_EXCL.size());
							def->CD_EXCL = get_cond_string(ai);
						}
						else if (ai.find(vscdefs::C_PARM_COMPRESS, 0) == 0)
						{
							def->CD_BYTE_PARM |= vscdefs::PARM_COMPRESS;
							
							if (ai.find(vscdefs::C_PARM_COMPRESS1, 0) == 0)
							{
								ai.erase(0, vscdefs::C_PARM_COMPRESS1.size());
								def->CD_COMPRESS = get_cond_string(ai);
							}
							else
							{
								rc = vscdefs::RC_FAILED;
								display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Wrong parameter", ca[i]);
							}
						}
						else if (ai.find(vscdefs::PARM_LOG, 0) == 0)									// 'Log' cannot be in script 
						{
							if (is_script)
							{
								rc = vscdefs::RC_FAILED;
								display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "'LOG' parameter is not allowed inside the script");
							}
						}
						else
						{
							rc = vscdefs::RC_FAILED;
							display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Wrong parameter", ca[i]);
						}
					}
				}
				else
				{ // Source/target path
					if ((def->CD_SOURCE == "") || ((def->CD_TARGET == "") && (def->CD_COMMAND != vscdefs::CMD_LIST)))
					{
						string pwd = "";		// Password
						string pth = "";		// Path

						int dpos = vs2comlib::index_of_first(ca[i], vscdefs::P_PWD_DELIMITER);
						int dsize = (int)vscdefs::P_PWD_DELIMITER.size();

						if (dpos < 0)
							pth = ca[i];
						else
						{
							if ((int)(ca[i].size()) > (dpos + dsize))
							{
								pwd = ca[i].substr(0, dpos);
								pth = ca[i].substr((size_t)dpos + (size_t)dsize, ca[i].size() - dpos - dsize);
							}
							else
							{
								rc = vscdefs::RC_FAILED;
								display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Wrong parameter", ca[i]);
							}
						}

						if (def->CD_SOURCE == "")
						{
							def->CD_SOURCE = pth;
							def->CD_PR = pwd;
						}
						else
						{
							def->CD_TARGET = pth;
							def->CD_PW = pwd;
						}
					}
					else if (def->CD_INCL == "")		// Include list (3rd parameter)
						def->CD_INCL = get_cond_string(ai);
					else
					{
						rc = vscdefs::RC_FAILED;
						display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Ambiguous source/target/include list");
					}
				}

				if (rc != vscdefs::RC_SUCCESS)
					break;

			}

			// Validate parameters

			if (rc == vscdefs::RC_SUCCESS)
			{
				unsigned char bp, br, bt, bm = 0x00;

				if ((def->CD_COMMAND == vscdefs::CMD_COPY) || (def->CD_COMMAND == vscdefs::CMD_COPYX))
				{
					bp = def->CD_BYTE_PARM | vscdefs::ALLOWED_PARM_COPY;
					br = def->CD_BYTE_REPL | vscdefs::ALLOWED_REPL_COPY;
					bt = def->CD_BYTE_TARG | vscdefs::ALLOWED_TARG_COPY;
					bm = def->CD_BYTE_MODE | vscdefs::ALLOWED_MODE_COPY;
					if (
						(vscdefs::ALLOWED_PARM_COPY != bp) ||
						(vscdefs::ALLOWED_REPL_COPY != br) ||
						(vscdefs::ALLOWED_TARG_COPY != bt) ||
						(vscdefs::ALLOWED_MODE_COPY != bm) ||
						def->CD_CHK ||
						(((def->CD_BYTE_TARG & vscdefs::TARG_MODE_SAF) > 0) + ((def->CD_BYTE_TARG & vscdefs::TARG_MODE_PURGE) > 0) + ((def->CD_BYTE_TARG & vscdefs::TARG_MODE_MIRROR) > 0) > 1) ||
						(((def->CD_BYTE_REPL & vscdefs::REPL_NONE) > 0) + ((def->CD_BYTE_REPL & vscdefs::REPL_ALL) > 0) > 1)  ||
						(((def->CD_BYTE_PARM & vscdefs::PARM_CPX) != 0) && (((def->CD_BYTE_PARM & (vscdefs::NOT_APPLICABLE_CPX_PARM & 0xff)) != 0) || ((def->CD_BYTE_TARG & (vscdefs::NOT_APPLICABLE_FILE_TARG & 0xff)) != 0))) ||
						(((def->CD_BYTE_PARM & vscdefs::PARM_CRC) != 0) && ((def->CD_BYTE_PARM & vscdefs::PARM_COMPRESS) + (def->CD_BYTE_TARG & vscdefs::TARG_MODE_SAF) == 0))
						)
						rc = vscdefs::RC_FAILED;
				}
				else if (def->CD_COMMAND == vscdefs::CMD_LIST)
				{
					bp = def->CD_BYTE_PARM | vscdefs::ALLOWED_PARM_LIST;
					br = def->CD_BYTE_REPL | vscdefs::ALLOWED_REPL_LIST;
					bt = def->CD_BYTE_TARG | vscdefs::ALLOWED_TARG_LIST;
					bm = def->CD_BYTE_MODE | vscdefs::ALLOWED_MODE_LIST;
					if ((vscdefs::ALLOWED_PARM_LIST != bp) ||
						(vscdefs::ALLOWED_REPL_LIST != br) ||
						(vscdefs::ALLOWED_TARG_LIST != bt) ||
						(vscdefs::ALLOWED_MODE_LIST != bm) ||
						(def->CD_VOL != 0) ||
						(def->CD_PW != "")
						)
						rc = vscdefs::RC_FAILED;
				}

				if (rc != vscdefs::RC_SUCCESS)
				{
					display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Invalid/conflicting command parameter");
					break;
				}
			}
		}
		else
		{
			rc = vscdefs::RC_FAILED;
			display_msg(vscdefs::DSPC_ERROR, cmd_line_no[line_count], "Invalid command", ca[0]);
		}

		if (rc == vscdefs::RC_SUCCESS)
			defs.push_back(def);
		else
			break;
	}

	return rc;
}

////////////////////////////////////////////////////////////////////////////
///// Processor                                                        /////
///// Input - defs				                                       /////
////////////////////////////////////////////////////////////////////////////
int processor()
{
	string on_error_cond = vscdefs::COND_ON_STOP;			// Initially - stop on error by default
	bool stopped = false;
	def = nullptr;
	level = -1;										// If/Else level

	int* rc_level = new int[255];					// RC for if/else levels
	bool* if_run_level = new bool[255];				// true - 'if' ran for level; false - no. Check for 'else'


	string sstr = ""; // Space string
	for (int i = 0; i < 255; i++)
		sstr += vs2comlib::convert_char_to_string(&vscdefs::PAD_CHAR, 0, 1);


	for (int i = 0; i < 255; i++)					// Initial cleanul array
	{
		if_lv_cond[i] = "";
		rc_level[i] = vscdefs::RC_SUCCESS;
		if_run_level[i] = false;
	}

	string sps_ie = "";
	string sps_cmd = "";

	int i = 0;

	while ((i < defs.size()) && !stopped)
	{
		def = defs[i];

		sps_ie = (level < 0) ? "" : sstr.substr(0, ((size_t)level * 2) + 2);
		sps_cmd = (is_script ? ((level < 0) ? "" : sstr.substr(0, ((size_t)level * 2) + 2)) : "");

		///// ON /////
		if (def->CD_COMMAND == vscdefs::CMD_ON)
		{
			router.empty_line();

			router.write_msg_ext(sps_cmd + get_cmd() + " ERROR", vs2comlib::to_upper(def->CD_ON_COND));

			on_error_cond = def->CD_ON_COND;
		}
		///// IF /////
		else if (def->CD_COMMAND == vscdefs::CMD_IF)
		{
			level++;

			rc_level[level] = rc;

			router.empty_line();

			router.write_msg_ext(sps_ie + get_cmd(), vs2comlib::to_upper(def->CD_IE_COND));


			// Set level condition value
			if_run_level[level] =
				(((def->CD_IE_COND == vscdefs::COND_SUCCESS) && (rc_level[level] == 0)) ||
				((def->CD_IE_COND == vscdefs::COND_ERROR) && (rc_level[level] > 0)));

			// skip 'if' because of conditions
			if (!if_run_level[level])
			{ 
				while (!((defs[(size_t)i + 1]->CD_COMMAND == vscdefs::CMD_END) && (def->CD_LEVEL == defs[(size_t)i + 1]->CD_LEVEL)))
				{
					router.empty_line();

					router.write_msg_ext(sps_ie + "--" + get_cmd(defs[(size_t)i + 1]->CD_COMMAND), vs2comlib::to_upper(defs[(size_t)i + 1]->CD_CMDLINE));

					i++;
				}
			}
			rc = vscdefs::RC_SUCCESS;
		}
		///// ELSE /////
		else if (def->CD_COMMAND == vscdefs::CMD_ELSE)
		{
			level++;

			router.empty_line();

			router.write_msg_ext(sps_ie + get_cmd());


			// 'if' executed, skip 'else;
			if (if_run_level[level])
			{ 
				while ((defs[(size_t)i + 1]->CD_COMMAND != vscdefs::CMD_END) || (defs[(size_t)i + 1]->CD_LEVEL < def->CD_LEVEL))
				{
					router.empty_line();

					router.write_msg_ext(sps_ie + "--" + get_cmd(defs[(size_t)i + 1]->CD_COMMAND), vs2comlib::to_upper(defs[(size_t)i + 1]->CD_CMDLINE));
					i++;
				}
			}
		}
		///// END /////
		else if (def->CD_COMMAND == vscdefs::CMD_END)
		{
			level--;
			sps_ie = (level < 0) ? "" : sstr.substr(0, ((size_t)level * 2) + 2);
			sps_cmd = (is_script ? ((level < 0) ? "" : sstr.substr(0, ((size_t)level * 2) + 4)) : "");
			router.empty_line();

			router.write_msg_ext(sps_ie + get_cmd());

		}
		///// SET /////
		else if (def->CD_COMMAND == vscdefs::CMD_SET)
		{
			router.empty_line();

			router.write_msg_ext(sps_cmd + get_cmd(), vs2comlib::to_upper(def->CD_IE_COND));

			rc = (def->CD_IE_COND == vscdefs::COND_SUCCESS) ? vscdefs::RC_SUCCESS : vscdefs::RC_ERROR;
		}
		///// STOP /////
		else if (def->CD_COMMAND == vscdefs::CMD_STOP)
		{
			router.empty_line();

			router.write_msg_ext(sps_cmd + get_cmd(), vs2comlib::to_upper(def->CD_IE_COND));

			rc = (def->CD_IE_COND == vscdefs::COND_SUCCESS) ? vscdefs::RC_SUCCESS : vscdefs::RC_ERROR;

			stopped = true;
		}
		///// CHECK PREVIOUS RC /////
		else if ((rc == vscdefs::RC_ERROR) & (on_error_cond == vscdefs::COND_ON_STOP))
		{ 
			router.empty_line();

			router.write_msg_ext(sps_cmd + "EXECUTION TERMINATED");
	
			stopped = true;
		}
		///// OTHER /////
		else
		{
			router.empty_line();
						
			router.write_msg_ext(sps_cmd + get_cmd(), def->CD_CMDLINE);

			router.write_msg_ext("<" + get_cmd() + " LISTING>");

			string s = "";

			if (def->CD_COMMAND == vscdefs::CMD_OS_COMMAND)
			{
				rc = (system(def->CD_CMDLINE.c_str()) == 0) ? vscdefs::RC_SUCCESS : vscdefs::RC_ERROR;
			}
			else
			{

				def->CD_PR = get_input_value("Enter password for SOURCE data:", def->CD_PR);

				def->CD_PW = get_input_value("Enter password for TARGET data:", def->CD_PW);

				string rcs = "";

				def->CD_BYTE_PADL = (unsigned char)(sps_cmd.size() + 2);

				bool suppress_old = router.is_suppressed();

				if (def->CD_BYTE_MODE & vscdefs::MODE_NOLIST)
					router.suppress(true);

				if (def->CD_COMMAND == vscdefs::CMD_COPYX)
				{
					def->CD_BYTE_PARM |= vscdefs::PARM_CPX;
					def->CD_COMMAND = vscdefs::CMD_COPY;
				}

				def->CD_COMMAND_DSP = (def->CD_COMMAND == vscdefs::CMD_COPY) ? vscdefs::DSP_CPY : vscdefs::DSP_LST;

				rcs = cp(def);

				if (!suppress_old)
					router.suppress(false);

				if (rcs == vscdefs::MSGLEVEL_ABNORMAL)
					rc = vscdefs::RC_ABNORMAL;
				else if (rcs == vscdefs::MSGLEVEL_ERROR)
					rc = vscdefs::RC_ERROR;
			}
			
			int rc4 = rc / 4;
			router.write_msg_ext("<\\" + get_cmd() + " LISTING>");
			router.write_msg_ext(sps_cmd + get_cmd(), "ENDED", "RC = " + vscdefs::CP_RET_CODES[rc4]);


			if (rc > rc_max)
				rc_max = rc;

			if (rc > 8)
				stopped = true;
		}

		i++;
	}

	if (rc > rc_max)
		rc_max = rc;


	return rc;
}

/***** MAIN *****/

int main(int argn, char* args[])
{
	//tst();
	router.open();

	vs2iolib::set_font_color(vs2iolib::COLOR_WHITE);
	
	if (argn > 1)
	{
		for (int i = 1; i < argn; i++)
		{
			if (i > 1)
				command_line += " ";

			command_line += args[i];
		}

		cmd_lines_src.push_back(command_line);				// 1st line - always command line
	}

	router.write_msg_ext("VSCP "+ vscp_version, "STARTED", "<" + command_line + ">");
	router.empty_line();

	rc = parse_input(argn, args);

	if (rc == vscdefs::RC_SUCCESS)
		rc = prepare_command_lines();

	if (rc == vscdefs::RC_SUCCESS)
		rc = validate_script_lng();

	if (rc == vscdefs::RC_SUCCESS)
		rc = preprocessor();

	if (rc == vscdefs::RC_SUCCESS)
		rc = processor();

	router.empty_line();

	if (rc > rc_max)
		rc_max = rc;

	int rcm4 = rc_max / 4;
	//router.empty_line();

	router.write_msg_ext("VSCP ENDED", "MAX RC =", vscdefs::CP_RET_CODES[rcm4]);

	router.close();

	return rc_max;
}
