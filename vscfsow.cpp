#include "stdafx.h"
#include "stdio.h"
#include "string"
#include "time.h"
#include "vscfsow.h"
#include "vscrout.h"
#include "vs2comlib.h"
#include "vscfsor.h"
#include "vs2crc32.h"
#include "vs2cmx.h"
#include "vs2iolib.h"
#include "vs2iof.h"

using namespace std;

// Open FSO-W object
vs2cmx				W_cmx;

string vscfsow::Open(vscrout* router, vscdefs::COMMAND_DEFINITION* command, int attrs)
{
	/************************************** Prepare input parameters *********************************/
	cmd = command;

	W_router = router;
	W_parm = &command->CD_BYTE_PARM;

	string dest = W_name = W_root = command->CD_TARGET;											// Save input 'target_data' parameter	-	used for root

	if (cmd->CD_COMMAND == vscdefs::CMD_COPY)
	{
		W_encrypted = (cmd->CD_PW.size() > 0);
		W_is_crc = ((*(W_parm + vscdefs::BYTE_PARM) & vscdefs::PARM_CRC) != 0);
		W_key = command->CD_PW;
		W_buffer = (unsigned char*)malloc(vscdefs::CHUNK_SIZE * 10);					// Create buffer
		W_w_buffer = (unsigned char*)malloc(vscdefs::CHUNK_SIZE * 10);					// Create work buffer

		W_saf = ((*(W_parm + vscdefs::BYTE_TARGET) & vscdefs::TARG_MODE_SAF) != 0);		// Output is SAF(true) or no(false)
		cmd->CD_SAF_O = W_saf;															// Save SAF indicator

		W_empty = ((*(W_parm + vscdefs::BYTE_PARM) & vscdefs::PARM_DIR_EMPTY) != 0);	// Empty

		// Calculate SAF segment size
		W_saf_segment_size = (command->CD_VOL == 0) ? SAF_DEFAULT_VOLUME_SIZE : command->CD_VOL;
		if (W_saf_segment_size < (vscdefs::CHUNK_SIZE * 2))
			W_saf_segment_size = vscdefs::CHUNK_SIZE * 2;

		W_crc = 0;																		// CRC for out SAF

		if (W_is_crc)
			W_file_mask |= vscdefs::MASK_CRC;

		if (W_encrypted)
			W_file_mask |= vscdefs::MASK_ENCRYPT;

		/**************************************************************************************************/
		/************************************** Pre-Processing ********************************************/
		/**************************************************************************************************/

		// Output is SAF file
		if (W_saf)
		{
			W_saf_file_name = dest;								// Directory indicator

			if (vs2iolib::directory_exists(dest))
			{
				W_saf_dest_type = DEST_DIR;
			}
			else if (vs2iolib::directory_exists(vs2iolib::get_file_path(dest)))
			{
				W_saf_dest_type = DEST_FILE;
				if (!is_file_ext(dest, vscdefs::SAF_FILE_EXT))
					W_saf_file_name += ("." + vscdefs::SAF_FILE_EXT);
			}
			else
			{
				W_router->write_msg_error(vscdefs::DSPC_ERROR, "Invalid output path for SAF file", dest);
				return vscdefs::MSGLEVEL_ERROR;
			}

			W_name = W_saf_file_name;
		}
		else
		{ // Output is directory
			// Check if trg dir exists 
			if (dest != "")
			{
				if (!vs2iolib::directory_exists(dest))
				{
					if (!vs2iolib::mkdir(dest))
					{
						W_router->write_msg_error(vscdefs::DSPC_ERROR, "Error when creating output directory", dest);
						return vscdefs::MSGLEVEL_ERROR;
					}
					else
					{
						if (attrs >= 0)
							vs2iolib::set_fs_object_attributes(dest, attrs);
					}
				}
				W_name = dest;
			}
		}

		W_root = dest;
		W_root_attrs = attrs;
	}

	W_opened = true;

	return "";
}

// Close FSO-W object
string vscfsow::Close()
{
	if (!W_opened) 
		return "";

	string rc = "";

	if (cmd->CD_COMMAND == vscdefs::CMD_COPY)
	{
		CloseFile();													// Close current file if opened

		if (rc == "")
			rc = write_dirs_stack(WDS_FLUSH);

		// OUTPUT = SAF
		if (W_saf)
		{
			if (rc == "")
			{
				write_header(vscdefs::DATA_EOF, nullptr, &W_saf_io);
				//W_saf_io.write_int(-1, vscdefs::DATA_EOF);
				W_saf_io.write_long((unsigned long long)(W_is_crc ? W_saf_io.end_crc(vs2iolib::CRC_WRITE) : W_saf_io.get_position() + 8));	// Write crc/length
				W_saf_io.close();
			}
			else
			{
				W_saf_io.close();
				W_saf_io.rmfile();
			}
		}

		// Cleanup tree work arrays
		W_stack_save.clear();
		W_stack_dir.clear();
		W_stack_dir_src.clear();
		W_stack_attrs.clear();
	}

	W_opened = false;

	return rc;
}

// Get root name
string vscfsow::GetName()
{
	return W_name;
}

// Set next directory
string vscfsow::OpenDir(string name, short int level, int attrs)
{
	string rc = "";
	unsigned char _resv[64] = { 0x00 };								// Reserve data
	string r_dir = W_root;

	CloseFile();													// Close current file if opened

	/** ACTIONS DEPENDING ON LEVEL **/
	if ((level) == 0)
	{ // Top level (always must be)
		W_stack_dir.push_back((W_saf ||(W_root == "")) ? name : W_root + "\\" + name);
		W_stack_save.push_back(false);
		W_stack_attrs.push_back(attrs);
		W_stack_dir_src.push_back(cmd->CD_SOURCE_DIR);

		if (W_saf && (cmd->CD_COMMAND == vscdefs::CMD_COPY))
		{
			if (W_saf_dest_type == DEST_DIR)
				W_saf_file_name += ("\\" + vs2iolib::get_file_name(W_stack_dir[0]) + "." + vscdefs::SAF_FILE_EXT);

			// Cleanup old SAF volumes (is exist)
			W_saf_core_name = vs2iolib::get_file_name_wo_ext(W_saf_file_name);

			vector<string> vf = vs2iolib::get_file_list(W_saf_core_name + "_V?????" + "." + vscdefs::SAF_FILE_EXT, true);

			for (unsigned int i = 0; i < vf.size(); i++)
			{
				string fn = vs2iolib::get_file_path(W_saf_core_name) + vf[i];
				vs2iolib::rmfile(fn);
			}

			vf.clear();

			// Create SAF VLIOF
			//W_saf_io.set(W_saf_file_name, vs2iolib::FILE_MODE_CREATE, W_key);

			// Open out SAF
			if (W_saf_io.open(W_saf_file_name, vs2iolib::FILE_MODE_CREATE, W_key) != "")
			{
				W_router->write_msg_ext(vscdefs::DSPC_ERROR, "Open input SAF file error", W_saf_io.error_msg);
				return vscdefs::MSGLEVEL_ABNORMAL;
			}

			if (W_is_crc)
				W_saf_io.begin_crc(vs2iolib::CRC_WRITE);
			
			// Write SAF header
			W_hdr.mask = W_file_mask;										// Mask
			W_hdr.name = W_saf_file_name;									// File name

			write_header(vscdefs::DATA_SAF, &W_hdr, &W_saf_io);
		}
	}
	else
	{
		/** New level - UP or EQUAL**/
		if ((level) <= W_level)														// Up
		{
			if (cmd->CD_COMMAND == vscdefs::CMD_COPY)
				rc = write_dirs_stack(WDS_FLUSH);

			if (rc != "")
				return rc;


			r_dir = W_stack_dir[((long long)level - 1)] + "\\" + name;

			// Erase current level
			W_stack_dir.erase(W_stack_dir.begin() + level, W_stack_dir.end());
			W_stack_dir_src.erase(W_stack_dir_src.begin() + level, W_stack_dir_src.end());
			W_stack_save.erase(W_stack_save.begin() + level, W_stack_save.end());
			W_stack_attrs.erase(W_stack_attrs.begin() + level, W_stack_attrs.end());
		}
		/** New level - DOWN **/
		else if ((level) > W_level)
		{
			r_dir = W_stack_dir[W_level] + "\\" + name;
		}

		W_stack_dir.push_back(W_saf ? name : r_dir);
		W_stack_save.push_back(false);
		W_stack_attrs.push_back(attrs);
		W_stack_dir_src.push_back(cmd->CD_SOURCE_DIR);
	}

	W_level = level;

	return "";
}

// Verify the file name - shall not be the same as input (non-SAF, with path)
bool vscfsow::VerifyFileName(string name, string in_file_name)
{
	return (W_saf || (cmd->CD_COMMAND != vscdefs::CMD_COPY))? true : (vs2comlib::to_lower(name) != vs2comlib::to_lower(W_stack_dir[W_level] + "\\" + name));
}



// Set next file in directory
string vscfsow::OpenFile(string name, int attrs, long long length, long long timestamp, string* condition, bool compress)
{
	string rc = "";
	CloseFile();													// Close current file if opened
	W_file_attrs = attrs;
	W_file_timestamp = timestamp;
	W_compressed = compress;

	rc = write_dirs_stack(WDS_FORCE);

	if (rc != "")
		return rc;

	long long out_timestamp = 0;

	*condition = vscdefs::DSPA_ADDED;

	W_hdr.name = name;									// Name
	W_hdr.attrs = attrs;								// Attributes
	W_hdr.timestamp = timestamp;						// Timestamp
	W_hdr.original_length = length;						// Original length
	W_hdr.mask = W_file_mask;							// Mask


	string fn = vs2comlib::to_lower(is_file_ext(name, vscdefs::SEC_FILE_EXT) ? vs2iolib::get_file_name_wo_ext(name) : name);							// File name

	if (W_compressed)
		W_hdr.mask |= vscdefs::MASK_COMPRESS;

	if (W_saf)
		write_header(vscdefs::DATA_FIL, &W_hdr, &W_saf_io);
	else
	{
		string file_name = W_stack_dir[W_level] + "\\" + (is_file_ext(name, vscdefs::SEC_FILE_EXT) ? vs2iolib::get_file_name_wo_ext(name) : name);							// File name

		if (is_sec_mode())
			file_name += ("." + vscdefs::SEC_FILE_EXT);									// Add '.vspk' for target name if compressed

		out_timestamp = vs2iolib::get_last_write_time(file_name);							// Out file timestamp (0 - no file)

		if (out_timestamp != 0)
		{
			if (*(W_parm + vscdefs::BYTE_REPLACE) == vscdefs::REPL_ALL)
				*condition = vscdefs::DSPA_REPLACED;
			else
			{
				if ((*(W_parm + vscdefs::BYTE_REPLACE) == vscdefs::REPL_NONE) || ((out_timestamp >= W_file_timestamp)))
				{
					*condition = vscdefs::DSPA_SKIPPED;
					return "";
				}
				else
					*condition = vscdefs::DSPA_REPLACED;
			}
		}

		if (W_file_io.open(file_name, vs2iolib::FILE_MODE_CREATE, W_key) != "")
		{
			W_router->write_msg_error(vscdefs::DSPC_ERROR, "Open output file error", W_file_io.error_msg);
			return vscdefs::MSGLEVEL_ERROR;
		}

		// COMPRESS/ENCRYPT
		if (is_sec_mode())
		{
			W_hdr.name = file_name;									// Name

			write_header(vscdefs::DATA_FIL, &W_hdr, &W_file_io);

			if (is_crc_mode())
				W_file_io.begin_crc(vs2iolib::CRC_WRITE);						// begin CRC if required
		}
	}

	W_file_opened = true;

	return "";
}

// Write file data chunk
// Return "" - ok, otherwise error 
string vscfsow::WriteData(unsigned char* data, long long length)
{
	if (W_saf)
	{ // Write SAF data
		// If SAF and segmented - check if need to switch to new volume
		if (W_saf_segment_size > 0)
		{
			unsigned long long pos = W_saf_io.get_position();

			if ((W_saf_io.get_position() + length + 8) > W_saf_segment_size)
			{
				write_header(vscdefs::DATA_EOV, nullptr, &W_saf_io);
				//W_saf_io.write_int(-1, vscdefs::DATA_EOV);													// Write EOV mark

				if (W_is_crc)
					W_saf_io.write_long((unsigned long long)W_saf_io.end_crc(vs2iolib::CRC_WRITE));		// Write crc
				else
					W_saf_io.write_long((unsigned long long)(W_saf_io.get_position() + 8));				// Write file length

				W_saf_io.close();

				// Open new segment
				string io_name = (W_saf_core_name + "_V" + vs2comlib::pad_left(to_string(++W_saf_segment_numb), 5, (string)"0") + "." + vscdefs::SAF_FILE_EXT);

				W_saf_io.open(io_name);

				if (W_is_crc)
					W_saf_io.begin_crc(vs2iolib::CRC_WRITE);
			}
		}

		if (W_compressed)
		{
			unsigned long int new_length_compressed = W_cmx.Compress(data, W_buffer, (unsigned long)length);
			W_saf_io.write_int(new_length_compressed);							// Write data length (SAF)
			W_saf_io.write((long long)new_length_compressed, W_buffer);
		}
		else
		{
			W_saf_io.write_int((int)length);											// Write data length (SAF)
			W_saf_io.write(length, data);
		}
	}
	else
	{ // Write non-SAF data
		if (is_sec_mode())
		{
			if (W_compressed)
			{
				unsigned long int new_length_compressed = W_cmx.Compress(data, W_buffer, (unsigned long)length);
				W_file_io.write_int(new_length_compressed);							// Write data length (SAF)
				W_file_io.write((long long)new_length_compressed, W_buffer);
			}
			else
			{
				W_file_io.write_int((int)length);											// Write data length (SAF)
				W_file_io.write(length, data);
			}
		}
		else
			W_file_io.write(length, data);
	}

	return "";
}

// Get full dir path from stack
string vscfsow::get_relative_path()
{
	string s = "";
	string s2 = "";
	size_t l = 0;
	size_t n = 0;

	if (W_saf)
	{
		for (size_t i = 1; i < W_stack_dir.size(); i++)
			s += ("\\" + W_stack_dir[i]);
	}
	else
	{
		s2 = (cmd->CD_SAF_I) ? vs2iolib::get_file_path(W_stack_dir[0]) : W_stack_dir[0];

		l = s2.size();
		n = W_stack_dir.size() - 1;
		s = W_stack_dir[n].substr(l, W_stack_dir[n].size() - l);
	}

	return s;
}

// Create directory hierarchy (Non-SAF); Write dirs to SAF file (SAF)
// Parameters:
// mode = flush - write all non-saved dirs (only if Empty)
// mode = force - force write directory hierarchys (even if Empty)
string vscfsow::write_dirs_stack(const bool mode)
{
		// Create directory hierarchy (Non-SAF); Write dirs to SAF file (SAF)
		for (unsigned int i = 0; i < W_stack_dir.size(); i++)
		{
			bool save_dir = false;
			
			if (!W_stack_save[i])
			{
				if (mode == WDS_FLUSH)
				{
					if (!W_empty)
					{
						W_router->write_msg_ext(vscdefs::DSP_CPY, vscdefs::DSPA_SKIPPED + vscdefs::DSPT_DIR, W_stack_dir_src[i] + "{EMPTY}");
						cmd->CD_STAT_DIRS_SKIPPED++;
					}
					else
						save_dir = true;
				}
				else
					save_dir = true;

				if (save_dir)
				{
					cmd->CD_STAT_DIRS++;
					if (W_saf)
					{
						W_router->write_msg_ext(vscdefs::DSP_CPY, vscdefs::DSPA_ADDED + vscdefs::DSPT_DIR, W_stack_dir_src[i]);

						W_hdr.name = W_stack_dir[i];									// Name
						W_hdr.attrs = W_stack_attrs[i];									// Attributes
						W_hdr.level = (short)i;											// Level
						W_hdr.timestamp = time(0);										// Timestamp

						write_header(vscdefs::DATA_DIR, &W_hdr, &W_saf_io);
					}
					else
					{
						if (!vs2iolib::directory_exists(W_stack_dir[i]))
						{
							W_router->write_msg_ext(vscdefs::DSP_CPY, vscdefs::DSPA_ADDED + vscdefs::DSPT_DIR, W_stack_dir_src[i]);

							if (!vs2iolib::mkdir(W_stack_dir[i]))
							{
								W_router->write_msg_error(vscdefs::DSPC_ERROR, "Cannot create directory", W_stack_dir[i]);
								return vscdefs::MSGLEVEL_ABNORMAL;
							}

							if ((*(W_parm + vscdefs::BYTE_PARM) & (vscdefs::PARM_ATTRIBUTES)) > 0)
								vs2iolib::set_fs_object_attributes(W_stack_dir[i], W_stack_attrs[i]);
						}
					}

					W_stack_save[i] = true;
				}
			}
		}

	return "";
}

// Close current file
void vscfsow::CloseFile()
{
	if (W_file_opened && (cmd->CD_COMMAND == vscdefs::CMD_COPY))
	{
		if (W_saf)
			write_header(vscdefs::DATA_EOD, nullptr, &W_saf_io);
			//W_saf_io.write_int(-1, vscdefs::DATA_EOD);									// Write EOD (SAF)
		else
		{
			if (is_sec_mode())
			{
				write_header(vscdefs::DATA_EOD, nullptr, &W_file_io);
				//W_file_io.write_int(-1, vscdefs::DATA_EOD);									// Write EOD

				W_file_io.write_long((long long)(is_crc_mode() ? (W_file_io.end_crc(vs2iolib::CRC_WRITE)) : (W_file_io.get_length() + 8)));	// Write at tail crc/length
			}

			W_file_io.close();
		}

		W_file_opened = false;
	}
}


// Check parm for compress mode
bool vscfsow::is_sec_mode()
{
	return (W_compressed || W_encrypted);
}

// Check for file extension (e.g. '.vspk')
bool vscfsow::is_file_ext(const string name, const string ext)
{
	if (name.size() > ext.size())
		if (name.substr(name.size() - ext.size() - 1, ext.size() + 1) == ("." + ext))
			return true;

	return false;
}

// Check parm for CRC mode
bool vscfsow::is_crc_mode()
{
	return ((*(W_parm + vscdefs::BYTE_PARM) & vscdefs::PARM_CRC) != 0);
}

// Write file header for SAF/SEC
void vscfsow::write_header(unsigned int type, vscdefs::FILE_HEADER* hdr, vs2iof* io)
{
	io->write_int(type);

	if ((type == vscdefs::DATA_EOD) || (type == vscdefs::DATA_EOV) || (type == vscdefs::DATA_EOF))
		return;
		

	if (type == vscdefs::DATA_SAF)
		io->write(7, (unsigned char*)vscdefs::SG_SAF);
	else if (type == vscdefs::DATA_FIL)
		io->write(7, (unsigned char*)vscdefs::SG_FILE);
	else
		io->write(7, (unsigned char*)vscdefs::SG_DIR);
		
	io->write(1, &hdr->mask);

	// Timestamp
	io->write_long((type != vscdefs::DATA_SAF)? hdr->timestamp : 0);

	// Length
	io->write_long((type == vscdefs::DATA_FIL)? hdr->original_length : 0);

	// Attrs
	io->write_int((type != vscdefs::DATA_SAF)? hdr->attrs : 0);

	// Level
	io->write_short((type == vscdefs::DATA_DIR)? hdr->level : 0);

	io->write_long(0);											// Reserve
	io->write_short((short)hdr->name.size());					// Name length

	io->write_string(hdr->name);
}

// Indicator if source is SAF file or no
bool vscfsow::IsSAF()
{
	return W_saf;
}

// Check file name for compression requirements
bool vscfsow::check_name_compress(const string nm, const string cond, const bool is_dir)
{
	bool ret_file = true;
	bool ret_dir = true;

	string name = vs2comlib::to_lower(nm);

	vector<string> v_incl = vs2comlib::parse(vs2comlib::to_lower(cond), "/,;");

	for (unsigned int i = 0; i < v_incl.size(); i++)
		if (v_incl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
			ret_dir = false;
		else
			ret_file = false;

	for (unsigned int i = 0; i < v_incl.size(); i++)
	{
		if (v_incl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
		{
			if (is_dir)
			{
				v_incl[i].erase(0, 1);

				if (vs2comlib::compare(v_incl[i], name))
				{
					ret_dir = true;
					break;
				}
			}
		}
		else
		{
			if (!is_dir)
			{
				if (vs2comlib::compare(v_incl[i], name))
				{
					ret_file = true;
					break;
				}
			}
		}
	}

	v_incl.clear();

	return (is_dir) ? ret_dir : ret_file;
}
