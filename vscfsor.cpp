#include "stdafx.h"
#include "stdio.h"
#include "string"
#include "vs2comlib.h"
#include "vs2iolib.h"
#include "vs2iof.h"
#include "vscrout.h"
#include "vscfsor.h"
#include "vs2cmx.h"
#include "vscdefs.h"

using namespace std;

///////////////////////////////////////////////////////
//////////////// PUBLIC MEMBERS ///////////////////////
///////////////////////////////////////////////////////

vs2cmx		R_xc;											// Decompressor

// Open FSO-R object
string vscfsor::Open(vscrout * router, vscdefs::COMMAND_DEFINITION * command)
{
	string rc = "";
	cmd = command;

	// Save input parameters
	R_router = router;
	R_parm = &command->CD_BYTE_PARM;
	R_key = command->CD_PR;
	R_even = ((*(R_parm + vscdefs::BYTE_MODE) & vscdefs::MODE_EVEN) != 0);			// Even
	R_empty = ((*(R_parm + vscdefs::BYTE_PARM) & vscdefs::PARM_DIR_EMPTY) != 0);		// Empty
	R_cpx = ((*(R_parm + vscdefs::BYTE_PARM) & vscdefs::PARM_CPX) != 0);			// CPX

	R_incl = command->CD_INCL;
	R_excl = command->CD_EXCL;
	R_source = command->CD_SOURCE;

	R_buffer = (unsigned char*)malloc(vscdefs::CHUNK_SIZE * 10);					// Create buffer
	R_w_buffer = (unsigned char*)malloc(vscdefs::CHUNK_SIZE * 10);					// Create work buffer

	////////////////////////////////////////
	//--------- Determine source ---------//
	////////////////////////////////////////
	if (vs2iolib::directory_exists(R_source))
		R_file = false;
	else
	{
		R_file = true;

		if (!vs2iolib::file_exists(R_source))
		{
			if (IsFileExt(R_source, vscdefs::SEC_FILE_EXT) || (IsFileExt(R_source, vscdefs::SAF_FILE_EXT)))
				rc = vscdefs::MSGLEVEL_ERROR;
			else
			{
				if (vs2iolib::file_exists(R_source + "." + vscdefs::SAF_FILE_EXT))
					R_source += ("." + vscdefs::SAF_FILE_EXT);
				else if (vs2iolib::file_exists(R_source + "." + vscdefs::SEC_FILE_EXT))
					R_source += ("." + vscdefs::SEC_FILE_EXT);
				else
					rc = vscdefs::MSGLEVEL_ERROR;
			}
		}
	}

	if (rc != "")
	{
		R_router->write_msg_error(vscdefs::DSPC_ERROR, "Input file/directory is not found - ", R_source);
		return rc;
	}

	// If input is SAF file and NOT CPX - saf
	if (R_file && IsFileExt(R_source, vscdefs::SAF_FILE_EXT))
	{
		if (!R_cpx)
		{
			R_saf = true;
			command->CD_SAF_I = R_saf;			// Save SAF indicator
			R_file = false;
		}
	}

	//--------- Prepare FILE source ---------//
	if (R_file)
	{ // File input
		// Check for non-file parameters
		if (((*(R_parm + vscdefs::BYTE_PARM) & (vscdefs::NOT_APPLICABLE_FILE_PARM & 0xff)) != 0) ||
			((*(R_parm + vscdefs::BYTE_TARGET) & (vscdefs::NOT_APPLICABLE_FILE_TARG & 0xff)) != 0))
		{
			router->write_msg_error(vscdefs::DSPC_ERROR, "Unaplicable parameter(s) for file input");
			return vscdefs::MSGLEVEL_ABNORMAL;
		}

		vscdefs::DIR_INFO* di = new vscdefs::DIR_INFO();

		di->name = vs2iolib::get_file_path(R_source);
		di->level = 0;
		di->attrs = vs2iolib::get_fs_object_attributes(di->name);
		R_dir_info.push_back(di);
	}
	else
	{
		if (R_saf)
		{ // SAF INPUT
			unsigned char _resv[64] = { 0x00 };

			//R_saf_io = new vliof(R_source, vliof::FILE_MODE_OPEN, R_key);
			R_saf_io = new vs2iof();
			R_saf_file_info.io = R_saf_io;									// I/O object
			R_saf_file_info.buffer = R_buffer;
			R_last_file_info = &R_saf_file_info;

			// Check length validity
			if (vs2iolib::get_file_length(R_source) < (SAF_HEADER_SIZE + CONTROL_FIELD_SIZE))
			{
				router->write_msg_error(vscdefs::DSPC_ERROR, "Invalid source SAF file length", R_source);
				return vscdefs::MSGLEVEL_ERROR;
			}

			if (R_saf_io->open(R_source, vs2iolib::FILE_MODE_OPEN, R_key) != "")
			{
				R_router->write_msg_ext(vscdefs::DSPC_ERROR, "Open input SAF file error", R_saf_io->error_msg);
				return vscdefs::MSGLEVEL_ABNORMAL;
			}

			R_saf_io->begin_crc(vs2iolib::CRC_READ);

			if ((read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS) || (R_hdr.type != vscdefs::DATA_SAF))
			{
				file_damaged();
				return vscdefs::MSGLEVEL_ERROR;
			}

			R_encrypted = ((R_hdr.mask & vscdefs::MASK_ENCRYPT) != 0);			// If input SAF file is encrypted

			if ((R_hdr.mask & vscdefs::MASK_CRC) == 0)
				R_saf_io->end_crc(vs2iolib::CRC_READ);								// No CRC in the source file

			R_saf_total_size = R_saf_io->get_length();							// File length

			R_saf_core_name = vs2iolib::get_file_name_wo_ext(R_source);			// Core file name - with path but w/o ext

			if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
			{
				file_damaged();
				return vscdefs::MSGLEVEL_ERROR;
			}

			// R_HDR AFTER 'OPEN' - AFTER SAF HEADER
		}
		else
		{ // NON-SAF INPUT
			build_dir_tree(0, R_source, vs2iolib::get_fs_object_attributes(R_source));
		}
	}

	R_opened = true;

	return "";
}

// Get root name
string vscfsor::GetName()
{
	return R_source;
}

// Close FSO-R object
string vscfsor::Close()
{
	string rc = "";

	if (!R_opened)
		return "";

	if (R_saf)
	{
		if (rc == "")
			rc = check_saf(SAF_CHECK_MODE_TAIL);
		
			R_saf_io->close();
	}
	else
	{
		// Files cleanup
		for (size_t i = 0; i < R_file_info.size(); i++)	// Delete all file info objects
			delete R_file_info[i];

		R_file_info.clear();							// Clear file array

		// Dirs cleanup
		for (size_t i = 0; i < R_dir_info.size(); i++)	// Delete all dir info objects
			delete R_dir_info[i];

		R_dir_info.clear();								// Clear dir array
	}
	delete R_buffer;								// Free file I/O buffer
	R_buffer = nullptr;

	delete R_w_buffer;								// Free work I/O buffer
	R_w_buffer = nullptr;

	R_opened = false;

	return rc;
}


// Get next directory
vscdefs::DIR_INFO* vscfsor::GetNextDir()
{
	R_last_file_info = nullptr;

	return R_saf ? get_next_dir_saf() : get_next_dir_nonsaf();
}

// Go to next non-FILE record type
string vscfsor::bypass_saf_files()
{
	// Skip all files to next non-FILE record
	while (R_hdr.type == vscdefs::DATA_FIL)
	{
		if (R_saf_file_info.mode != vscdefs::FILE_MODE_OPENED)
		{
			short lfn = R_saf_io->read_short();						// Read file name length
			string fn = R_saf_io->read_string((long long)lfn);		// Read file name

			R_saf_io->read(24, R_buffer);							// Skip timestamp(8) + filelen(8) + attrs(4) + reserve(4)
		}
		else
			R_saf_file_info.mode = vscdefs::FILE_MODE_UNDEFINED;

		R_saf_file_info.rc = skip_saf_file(true);					// Skip file

		if (R_saf_file_info.rc == "")
		{
			if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
			{
				R_saf_file_info.rc = vscdefs::MSGLEVEL_ERROR;
				break;
			}
		}
		else
			break;
	}

	return R_saf_file_info.rc;
}

// Get next SAF directory
vscdefs::DIR_INFO* vscfsor::get_next_dir_saf()
{
	unsigned short skip_level = 0x7fff;									// Level where skip set (skip all lower)

	R_saf_dir_info.level = -1;											// 'Undefined'
	R_saf_dir_info.name = "";

	// Skip all files to next dir record
	R_saf_dir_info.rc = bypass_saf_files();

	while ((R_saf_dir_info.rc == "") && (R_hdr.type == vscdefs::DATA_DIR))
	{
		R_saf_dir_info.level = R_hdr.level;								// Read level
		R_saf_dir_info.attrs = R_hdr.attrs;								// Read attrs
		R_saf_dir_info.name = R_hdr.name;								// Read dir name

		if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
			file_damaged();
		else
		{
			if (R_saf_dir_info.level == 0)
				R_dir_stack.push_back(vscdefs::SAF_MSG_PREFIX + R_saf_dir_info.name);
			else
			{
				if (R_saf_dir_info.level <= R_dir_level)
				{
					R_dir_stack.erase(R_dir_stack.begin() + R_saf_dir_info.level, R_dir_stack.end());
					//R_dir_comp.erase(R_dir_comp.begin() + R_saf_dir_info.level, R_dir_comp.end());
				}
				R_dir_stack.push_back(R_saf_dir_info.name);
			}

			R_dir_level = R_saf_dir_info.level;

			// Not skipped
			if (R_saf_dir_info.level <= skip_level)
			{
				if (check_name_pattern(R_saf_dir_info.name, R_incl, R_excl, true))
				{
					cmd->CD_STAT_DIRS++;

					string s = R_dir_stack[0];
					for (size_t i = 1; i < R_dir_stack.size(); i++)
						s += ("\\" + R_dir_stack[i]);

					cmd->CD_SOURCE_DIR = s;

					skip_level = 0x7fff;
					return &R_saf_dir_info;
				}
				else
				{
					skip_level = R_saf_dir_info.level;
					cmd->CD_STAT_DIRS_SKIPPED++;
				}
			}
			else
				cmd->CD_STAT_DIRS_SKIPPED++;

			if (R_hdr.type == vscdefs::DATA_FIL)
				R_saf_dir_info.rc = bypass_saf_files();
		}
	}

	return (R_saf_dir_info.rc == "") ? nullptr : &R_saf_dir_info;
}

// Get next non-SAF directory
vscdefs::DIR_INFO* vscfsor::get_next_dir_nonsaf()
{
	if (R_last_file_info != nullptr)
	{
		if (R_last_file_info->mode == vscdefs::FILE_MODE_OPENED)
		{
			R_last_file_info->io->close();
			R_last_file_info->mode = vscdefs::FILE_MODE_UNDEFINED;
		}
	}

	// Files cleanup
	for (size_t i = 0; i < R_file_info.size(); i++)	// Delete all file info objects
		delete R_file_info[i];
	
	R_file_info.clear();							// Clear file array
	R_file_index = -1;								// Reset file index

	// Find dir info
	if (R_dir_index < (int)R_dir_info.size())		// Increase index if not eof
		R_dir_index++;

	if (R_dir_index == R_dir_info.size())		// Eof
		return nullptr;
	else
	{
		cmd->CD_SOURCE_DIR = R_dir_info[R_dir_index]->name;
		cmd->CD_STAT_DIRS++;
		R_dir_info[R_dir_index]->rc = "";
		return R_dir_info[R_dir_index];
	}
}

// Get next file in directory
vscdefs::FILE_INFO* vscfsor::GetNextFile()
{
	// get_next_file_... returns: nullptr - eod; otherwise fileinfo with rc: "" - success; error msg - error
	R_last_file_info = R_saf ? vscfsor::get_next_file_saf() : vscfsor::get_next_file_nonsaf();

	if (R_last_file_info != nullptr)
		if (R_last_file_info->rc == "")
			cmd->CD_SOURCE_FILE = vs2iolib::get_file_name(R_last_file_info->name);

	return R_last_file_info;
}

// Get next SAF file
// Input R_hdr.type:
// If R_hdr.type == EOD - read next descriptor
// Read FIL, otherwise return nullptr (end of files)
// Read 1st descriptor after FIL
vscdefs::FILE_INFO* vscfsor::get_next_file_saf()
{
	// Skip file if opened
	if (R_saf_file_info.mode == vscdefs::FILE_MODE_OPENED)
	{
		R_saf_file_info.rc = skip_saf_file();

		if (R_saf_file_info.rc != "")
			return &R_saf_file_info;
	}

	R_saf_file_info.mode = vscdefs::FILE_MODE_UNDEFINED;

	if (R_hdr.type == vscdefs::DATA_EOD)
		if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
		{
			R_saf_file_info.rc = vscdefs::MSGLEVEL_ERROR;
			
			return &R_saf_file_info;
		}

	if (R_hdr.type == vscdefs::DATA_FIL)
		while ((R_hdr.type == vscdefs::DATA_FIL) && (R_saf_file_info.rc == ""))
		{
			if (check_name_pattern(R_hdr.name, R_incl, R_excl, false))
			{
				R_saf_file_info.name = R_hdr.name;									// Name
				R_saf_file_info.timestamp = R_hdr.timestamp;						// Timestamp
				R_saf_file_info.original_length = R_hdr.original_length;			// Original length
				R_saf_file_info.attrs = R_hdr.attrs;								// Attrs
				R_saf_file_info.mode = vscdefs::FILE_MODE_OPENED;					// Set opened state
				
				R_compressed = ((R_hdr.mask & vscdefs::MASK_COMPRESS) != 0);		// If input SAF file is compressed

				R_last_file_info = &R_saf_file_info;

				if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
					file_damaged();

				return &R_saf_file_info;
			}
			else
			{
				cmd->CD_STAT_FILES_SKIPPED++;
				cmd->CD_STAT_FILES_SKIPPED_SIZE += R_saf_file_info.original_length;
				R_saf_file_info.rc = skip_saf_file();								// Skip file

				if (R_saf_file_info.rc == "")
					if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
						file_damaged();

				if (R_saf_file_info.rc != "")
					return &R_saf_file_info;
			}
		}

	return nullptr;											// No file found in the directory
}

// Get next non-SAF file
vscdefs::FILE_INFO* vscfsor::get_next_file_nonsaf()
{
	if (R_last_file_info != nullptr)
	{
		if (R_last_file_info->mode == vscdefs::FILE_MODE_OPENED)
		{
			R_last_file_info->io->close();
			R_last_file_info->mode = vscdefs::FILE_MODE_UNDEFINED;
		}
	}

	if ((R_dir_index < 0) || (R_dir_index >= (int)R_dir_info.size()))
		return nullptr;							// No dir is set

	if (R_file_index < 0)
	{
		if (R_file)
		{
			vscdefs::FILE_INFO* fi = new vscdefs::FILE_INFO();
			fi->name = R_source;															// Full name (incl path)
			fi->timestamp = vs2iolib::get_last_write_time(fi->name);						// Timestamp
			fi->file_length = fi->original_length = vs2iolib::get_file_length(fi->name);	// File length
			fi->io = new vs2iof();															// Set I/O parameters
			fi->buffer = R_buffer;															// Set buffer address
			R_file_info.push_back(fi);
		}
		else
		{
			vector<vs2iolib::FSO_INFO *> vf = vs2iolib::get_file_list_info(R_dir_info[R_dir_index]->name, "*", ((*R_parm & vscdefs::PARM_SYSOBJ) != 0));
			if (vf.size() == 0)
				return nullptr;

			for (size_t i = 0; i < vf.size(); i++)
			{
				string full_name = R_dir_info[R_dir_index]->name + "\\" + vf[i]->name;

				if (check_name_pattern(vf[i]->name, R_incl, R_excl, false))
				{
					vscdefs::FILE_INFO* fi = new vscdefs::FILE_INFO();
					fi->name = full_name;										// Full name (incl path)
					fi->timestamp = vf[i]->time_write;							// Timestamp
					fi->file_length = fi->original_length = vf[i]->size;		// File length
					fi->io = new vs2iof();	// Set I/O parameters
					fi->buffer = R_buffer;										// Set buffer address
					R_file_info.push_back(fi);
				}
				else
				{
					cmd->CD_STAT_FILES_SKIPPED++;
					cmd->CD_STAT_FILES_SKIPPED_SIZE += vs2iolib::get_file_length(full_name);
				}

				delete vf[i];												// Delete FSO_INFO object
			}

			if (R_file_info.size() == 0)
				return nullptr;
		}
	}

	R_file_index++;

	if (R_file_index >= (int)R_file_info.size())
		return nullptr;

	R_last_file_info = R_file_info[R_file_index];

	R_last_file_info->mode = vscdefs::FILE_MODE_OPENED;									// Set opened state

	R_last_file_info->rc = "";

	return R_last_file_info;
}

// Read file data chunk (uncompess and decode if necessary)
// Entry point R_hdr -next block after FIL
// Return nullptr - error 
vscdefs::FILE_INFO* vscfsor::GetData()
{
	long long ln = 0;

	if ((R_last_file_info == nullptr) ||
		((R_last_file_info->mode == vscdefs::FILE_MODE_CLOSED) || (R_last_file_info->mode == vscdefs::FILE_MODE_UNDEFINED)))
	{
		R_router->write_msg_error(vscdefs::DSPC_ERROR, "GetData: Internal error - read after EOF or file descriptor is undefined");
		if (R_last_file_info == nullptr)
			R_last_file_info = &err_info;

		R_last_file_info->rc = vscdefs::MSGLEVEL_ERROR;

		return R_last_file_info;
	}

	/////// SAF ///////
	if (R_saf)
	{
		if (R_hdr.type == vscdefs::DATA_EOV)							// End Of Volume?
		{
			R_last_file_info->rc = switch_saf_volume(false);

			if (R_last_file_info->rc != "")
				return R_last_file_info;
		}

		if (R_hdr.type == vscdefs::DATA_EOD)
		{
			R_last_file_info->mode = vscdefs::FILE_MODE_CLOSED;	// EOF
			R_last_file_info->length = 0;						// Chunk length

			if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
				file_damaged();
		}
		else
		{
			if (R_hdr.type > vscdefs::DATA_MAX_CHUNK)
				file_damaged();
			else
			{
				ln = (long long)R_hdr.type;

				if (R_compressed)
				{
					R_last_file_info->io->read(ln, R_w_buffer);

					if (!vs2cmx::CheckSG(R_w_buffer))
						file_damaged();
					else
					{
						unsigned long cp_length = (unsigned long int)vs2cmx::GetCompressedLength(R_w_buffer);// Extract compressed chunk length

						vs2cmx xc;

						ln = xc.Decompress(R_w_buffer, R_buffer);											// Decompress data
					}
				}
				else
					R_last_file_info->io->read(ln, R_last_file_info->buffer);

				R_last_file_info->length = ln;

				// Read next descriptor
				if (R_last_file_info->rc == "")
					if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
						file_damaged();
			}
		}

		if (R_last_file_info->rc != "")
			return R_last_file_info;
	}
	else
	{
		/////// NON-SAF ///////
		// Open file if not opened yet
		R_compressed = false;

		if (!R_last_file_info->io->opened())
		{
			if (R_last_file_info->io->open(R_last_file_info->name, vs2iolib::FILE_MODE_OPEN, this->R_key) != "")
			{
				R_router->write_msg_error("ERROR", "Open input file error", R_last_file_info->io->error_msg);
				R_last_file_info->rc = vscdefs::MSGLEVEL_ERROR;
				return R_last_file_info;
			}

			R_last_file_info->io->begin_crc(vs2iolib::CRC_READ);								// Begin CRC

			R_last_file_info->file_length = R_last_file_info->io->get_length();				// Total file length

			/* If ext is 'vsec' - consider compression/encryption */
			if (!R_cpx && (IsFileExt(R_last_file_info->name, vscdefs::SEC_FILE_EXT)))
			{
				/* Read VSEC file header */
				if (read_header(&R_hdr, R_last_file_info->io) != vscdefs::RC_SUCCESS)
				{
					file_damaged();
					return R_last_file_info;
				}

				R_compressed = ((R_hdr.mask & vscdefs::MASK_COMPRESS) != 0);					// Check for compression

				R_last_file_info->original_length = R_hdr.original_length;						// Original file uncompressed length (8)
				R_last_file_info->attrs = R_hdr.attrs;											// Read file attributes (4)


				if ((R_hdr.mask & vscdefs::MASK_CRC) == 0)
					R_last_file_info->io->end_crc(vs2iolib::CRC_READ);								// End CRC - not required

				/* Read next: chunk length or record type (EOD) */
				if (read_header(&R_hdr, R_last_file_info->io) != vscdefs::RC_SUCCESS)
				{
					file_damaged();
					return R_last_file_info;
				}

				R_last_file_info->position = R_last_file_info->io->get_position();				// Current position
			}
			/* Plain file */
			else
			{
				R_last_file_info->attrs = R_last_file_info->io->get_fs_object_attributes();			// Get file attributes
				R_last_file_info->original_length = R_last_file_info->file_length;
				R_last_file_info->position = 0;
			}
		}

		/* READ FILE */
		if (is_sec_mode())
		{
			/* Check for EOD */
			if (R_hdr.type == vscdefs::DATA_EOD)
			{
				// Check CRC/File length
				unsigned long long new_check_field = R_last_file_info->io->is_crc(vs2iolib::CRC_READ) ? R_last_file_info->io->end_crc(vs2iolib::CRC_READ) : R_last_file_info->file_length;

				unsigned long long file_check_field = R_last_file_info->io->read_long();		// Read source control field

				if (file_check_field != new_check_field)
				{
					R_router->write_msg_error("ERROR:", "Invalid " + (string)(R_last_file_info->io->is_crc(vs2iolib::CRC_READ) ? "CRC" : "length") + " of the source file", R_last_file_info->name);
					R_last_file_info->rc = vscdefs::MSGLEVEL_ERROR;
				}

				R_last_file_info->mode = vscdefs::FILE_MODE_CLOSED;		// EOF
				R_last_file_info->length = R_last_file_info->position = 0;
				R_last_file_info->io->close();
				return R_last_file_info;
			}

			R_last_file_info->length = (long long)R_hdr.type;								// Length to read

			/* READ COMPRESSED FILE */
			if (R_compressed)
			{
				R_last_file_info->io->read(R_last_file_info->length, R_w_buffer);		// Read data

				if (!vs2cmx::CheckSG(R_w_buffer))
				{
					file_damaged();
					return R_last_file_info;
				}
				
				R_last_file_info->length = R_xc.Decompress(R_w_buffer, R_buffer);				// Decompress data
			}
			else
				R_last_file_info->io->read(R_last_file_info->length, R_buffer);			// Plain read

			/* Read next chunk length or EOD */
			if (read_header(&R_hdr, R_last_file_info->io) != vscdefs::RC_SUCCESS)
			{
				file_damaged();
				return R_last_file_info;
			}

			R_last_file_info->position = R_last_file_info ->io->get_position();				// Set new position
		}
		/* READ PLAIN FILE */
		else
		{
			long long rest = R_last_file_info->file_length - R_last_file_info->position;

			if (rest > 0)
			{
				ln = (rest > vscdefs::CHUNK_SIZE) ? vscdefs::CHUNK_SIZE : rest;		// Length to read
				R_last_file_info->io->read(ln, R_buffer);
				R_last_file_info->position += ln;
				R_last_file_info->length = ln;
			}
			else
			{
				/* End of file - close */
				R_last_file_info->mode = vscdefs::FILE_MODE_CLOSED;		// EOF
				R_last_file_info->length = R_last_file_info->file_length = R_last_file_info->position = 0;
				R_last_file_info->io->close();
			}
		}
	}

	if (R_last_file_info->rc != "")
	{
		R_router->write_msg_error(vscdefs::DSPC_ERROR, "The source file is damaged or invalid encryption key", R_last_file_info->name);
		R_last_file_info->io->close();
		R_last_file_info->mode = vscdefs::FILE_MODE_CLOSED;
	}

	return R_last_file_info;
}

// Indicator if source is SAF file or no
bool vscfsor::IsSAF()
{
	return R_saf;
}

// Indicator if source is file
bool vscfsor::IsFile()
{
	return R_file;
}

///////////////////////////////////////////////////////
//////////////// PRIVATE MEMBERS //////////////////////
///////////////////////////////////////////////////////

// Check for file extension
bool vscfsor::IsFileExt(const string name, const string ext)
{
	if (name.size() > ext.size())
		if (name.substr(name.size() - ext.size() - 1, ext.size() + 1) == ("." + ext))
			return true;
	return false;
}

// Check SAF file length/crc if -chk parameter is specified
// Modes:
// 0 - check whole file: -chk is specified   (file is not opened yet)
// 1 - check file tail: end of file handling (file is opened)
string vscfsor::check_saf(const int mode)
{
	string rc = "";
	string save_name = R_saf_io->name;				// Save 1st volume name

	unsigned long int r_type = 0;

	//------ All --------------------------------------------
	if (mode == SAF_CHECK_MODE_ALL)
	{
		r_type = R_saf_io->read_int();				// Read 1st record type (always must be DIR)

		if (r_type != vscdefs::DATA_DIR)
		{
			R_router->write_msg_error(vscdefs::DSPC_ERROR, "Missing root folder in the source SAF file (record type) at pos ", to_string((R_saf_io->get_position() - 4)));
			return vscdefs::MSGLEVEL_ERROR;
		}

		while (r_type != vscdefs::DATA_EOF)
		{
			if (r_type == vscdefs::DATA_DIR)
			{
				short lv = R_saf_io->read_short();							// Read level
				int attrs = R_saf_io->read_int();							// Read attrs
				short name_len = R_saf_io->read_short();					// Read name length
				string dir_name = R_saf_io->read_string(name_len);			// Read name
			}
			else if (r_type == vscdefs::DATA_FIL)
			{
				unsigned long long file_len = 0;
				unsigned short int file_name_len = R_saf_io->read_short();

				string file_name = R_saf_io->read_string(file_name_len);

				long long l = R_saf_io->read_long();						// Timestamp
				l = R_saf_io->read_long();									// Original file length
				int attrs = R_saf_io->read_int();							// Attrs

				rc = skip_saf_file();										// Skip file

				if (rc != "")
					return rc;
			}
			else
			{
				R_router->write_msg_error("ERROR:", "Invalid structure of the source SAF file (record type) at pos", to_string((R_saf_io->get_position() - 4)));

				return vscdefs::MSGLEVEL_ERROR;
			}

			r_type = R_saf_io->read_int();		// Read type
		}
	}

	//------ All and Tail -----------------------------------
	unsigned long long new_control = 0;
	unsigned long long old_control = 0;

	if (R_saf_io->is_crc(vs2iolib::CRC_READ))
		new_control = R_saf_io->end_crc(vs2iolib::CRC_READ);									// Get CRC
	else
		new_control = R_saf_io->get_length(); //R_saf_total_size;														// Get length

	old_control = R_saf_io->read_long();													// Read old CRC/length

	if (old_control != new_control)
	{
		R_router->write_msg_error("ERROR:", "Invalid " + (string)(R_saf_io->is_crc(vs2iolib::CRC_READ) ? "CRC" : "size") + " of the source SAF file", R_saf_io->name);

		rc = vscdefs::MSGLEVEL_ERROR;
	}
	
	return rc;
}

// Skip input SAF file data
//		On-call file position is: after file header
//		Ret file position is: after the DATA_EOD
string vscfsor::skip_saf_file(const bool skip_check)
{
	while ((R_hdr.type != vscdefs::DATA_EOD) && R_saf_file_info.rc == "")
	{
		if (R_hdr.type == vscdefs::DATA_FIL)
		{
			if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
				file_damaged();
		}
		else if (R_hdr.type == vscdefs::DATA_EOV)
			R_saf_file_info.rc = vscfsor::switch_saf_volume(false);
		else
		{
			if (R_saf_io->is_crc(vs2iolib::CRC_READ) & (!skip_check))
				R_saf_io->read((long long)R_hdr.type, R_buffer);						// If CRC - read data
			else
				R_saf_io->set_position(R_saf_io->get_position() + (long long)R_hdr.type);	// Otherwise shift position

			if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
				file_damaged();
		}
	}

	return R_saf_file_info.rc;
}

// Switch input SAF file volume
// Entry R_hdr position: EOV
string vscfsor::switch_saf_volume(bool skip_check)
{
	bool c = R_saf_io->is_crc(vs2iolib::CRC_READ);

	if (!skip_check)
	{
		// Get crc/file length for control
		long long new_cv = c ? R_saf_io->end_crc(vs2iolib::CRC_READ) : R_saf_io->get_position() + 8;
		
		// Read length/crc
		long long old_cv = R_saf_io->read_long();											
		
		if (new_cv != old_cv)
		{
			R_router->write_msg_error("ERROR:", "Invalid " + (string)(c ? "CRC" : "size") + " of the source SAF file", R_saf_io->name);

			R_saf_io->close();

			R_saf_file_info.rc = vscdefs::MSGLEVEL_ERROR;

			return R_saf_file_info.rc;
		}
	}

	// Open new segment
	string s = (R_saf_core_name + "_V" + vs2comlib::pad_left(to_string(++R_saf_segment_number), 5, (string)"0") + "." + vscdefs::SAF_FILE_EXT);

	if (!vs2iolib::file_exists(s))
	{
		R_router->write_msg_error("ERROR:", "The source SAF file volume is not found", s);

		R_saf_file_info.rc = vscdefs::MSGLEVEL_ERROR;

		return R_saf_file_info.rc;
	}

	R_saf_io->close();
	R_saf_io->open(s);
	R_saf_total_size += R_saf_io->get_length();

	if (c)
		R_saf_io->begin_crc(vs2iolib::CRC_READ);

	if (read_header(&R_hdr, R_saf_io) != vscdefs::RC_SUCCESS)
	{
		file_damaged();
		return vscdefs::MSGLEVEL_ERROR;
	}

	return "";
}

// Build tree for cpdir_gnr
void vscfsor::build_dir_tree(unsigned short int level, const string& root, const int root_attrs)
{
	vscdefs::DIR_INFO* di = new vscdefs::DIR_INFO();

	if (level == 0)
	{
		di->name = root;
		di->level = level;
		di->attrs = root_attrs;
		R_dir_info.push_back(di);

		if ((*R_parm & vscdefs::PARM_RECURSIVE) == 0)
			return;
	}
	else
	{
		if (check_name_pattern(vs2iolib::get_file_name(root), R_incl, R_excl, true))
		{
			di->name = root;
			di->level = level;
			di->attrs = vs2iolib::get_fs_object_attributes(root);
			R_dir_info.push_back(di);
		}
		else
		{
			R_router->write_msg_ext(vscdefs::DSP_CPY, vscdefs::DSPA_SKIPPED + vscdefs::DSPT_DIR, root);
			cmd->CD_STAT_DIRS_SKIPPED++;
			delete di;
			return;
		}
	}

	vector<vs2iolib::FSO_INFO *> dirs = vs2iolib::get_directory_list_info(root, "*", ((*R_parm & vscdefs::PARM_SYSOBJ) != 0));		// Get the list of sub-dirs

	// Recursively process child dirs 
	for (unsigned int i = 0; i < dirs.size(); i++)
	{
		build_dir_tree(level + 1, root + "\\" + dirs[i]->name, dirs[i]->attrs);
		
		delete dirs[i];
	}

	dirs.clear();
}

// Check file name for include/exclude lists
bool vscfsor::check_compress_pattern(const string nm, const string incl, const bool is_dir)
{
	bool ret = false;

	string name = vs2comlib::to_lower(nm);

	vector<string> v_incl = vs2comlib::parse(vs2comlib::to_lower(incl), "/,;");

	for (unsigned int i = 0; i < v_incl.size(); i++)
	{
		if (v_incl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
		{
			if (is_dir)
			{
				string s = v_incl[i].erase(0, 1);

				if (vs2comlib::compare(s, name))
				{
					ret = true;
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
					ret = true;
					break;
				}
			}
		}
	}

	v_incl.clear();

	return ret;
}

/**/
// Check file name for include/exclude lists
bool vscfsor::check_name_pattern(const string nm, const string incl, const string excl, const bool is_dir)
{
	bool ret_file = true;
	bool ret_dir = true;

	string name = vs2comlib::to_lower(nm);

	vector<string> v_incl = vs2comlib::parse(vs2comlib::to_lower(incl), "/,;");

	for (unsigned int i = 0; i < v_incl.size(); i++)
		if (v_incl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
			ret_dir = false;
		else
			ret_file = false;

	vector<string> v_excl = vs2comlib::parse(vs2comlib::to_lower(excl), "/,;");

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

	if (ret_dir | ret_file)
		for (unsigned int i = 0; i < v_excl.size(); i++)
		{
			if (v_excl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
			{
				if (is_dir)
				{
					v_excl[i].erase(0, 1);

					if (vs2comlib::compare(v_excl[i], name))
					{
						ret_dir = false;
						break;
					}
				}
			}
			else
			{
				if (!is_dir)
				{
					if (vs2comlib::compare(v_excl[i], name))
					{
						ret_file = false;
						break;
					}
				}
			}
		}

	v_incl.clear();
	v_excl.clear();

	return (is_dir) ? ret_dir : ret_file;
}

/**/

/**
// Check file name for include/exclude lists
bool vscfsor::check_name(const string& nm, const bool is_dir)
{
	bool ret_file = true;
	bool ret_dir = true;

	string name = vs2comlib::to_lower(nm);

	vector<string> v_incl = vs2comlib::parse(vs2comlib::to_lower(R_incl), "/,;");

	for (unsigned int i = 0; i < v_incl.size(); i++)
		if (v_incl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
			ret_dir = false;
		else
			ret_file = false;

	vector<string> v_excl = vs2comlib::parse(vs2comlib::to_lower(R_excl), "/,;");

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

	if (ret_dir | ret_file)
		for (unsigned int i = 0; i < v_excl.size(); i++)
		{
			if (v_excl[i].substr(0, 1) == vscdefs::NAME_PREFIX_DIR)
			{
				if (is_dir)
				{
					v_excl[i].erase(0, 1);

					if (vs2comlib::compare(v_excl[i], name))
					{
						ret_dir = false;
						break;
					}
				}
			}
			else
			{
				if (!is_dir)
				{
					if (vs2comlib::compare(v_excl[i], name))
					{
						ret_file = false;
						break;
					}
				}
			}
		}

	v_incl.clear();
	v_excl.clear();

	return (is_dir) ? ret_dir : ret_file;
}
**/

// Read file header for SAF/SEC
// Return: 0 - ok or 8 - error
unsigned long int vscfsor::read_header(vscdefs::FILE_HEADER* hdr, vs2iof* io)
{
	hdr->type = io->read_int();				// Read type

	if ((hdr->type == vscdefs::DATA_EOF) || (hdr->type == vscdefs::DATA_EOD) || (hdr->type == vscdefs::DATA_EOV) || (hdr->type <= vscdefs::DATA_MAX_CHUNK))
		return vscdefs::RC_SUCCESS;

	io->read(7, hdr->sg);					// Read signature

	if (hdr->type == vscdefs::DATA_SAF)
	{
		if (vs2comlib::compare_keys(hdr->sg, 7, (unsigned char*)vscdefs::SG_SAF, 7) != 0)
			return vscdefs::RC_ERROR;
	}
	else if (hdr->type == vscdefs::DATA_FIL)
	{
		if (vs2comlib::compare_keys(hdr->sg, 7, (unsigned char*)vscdefs::SG_FILE, 7) != 0)
			return vscdefs::RC_ERROR;
	}
	else if (hdr->type == vscdefs::DATA_DIR)
	{
		if (vs2comlib::compare_keys(hdr->sg, 7, (unsigned char*)vscdefs::SG_DIR, 7) != 0)
			return vscdefs::RC_ERROR;
	}
	else
		return vscdefs::RC_ERROR;

	io->read(1, &hdr->mask);					// Read mask

	hdr->timestamp = io->read_long();			// Read timestamp

	hdr->original_length = io->read_long();		// Read length

	hdr->attrs = io->read_int();				// Read attrs

	hdr->level = io->read_short();				// Read level

	long long tmp_res = io->read_long();		// Read reserve

	tmp_res = (long long)io->read_short();		// Read name length

	hdr->name = io->read_string(tmp_res);		// Read name

	return vscdefs::RC_SUCCESS;
}

// Display error and close SAF file
void vscfsor::file_damaged()
{
	R_router->write_msg_error("ERROR:", "The file is damaged or invalid/missing encryption key - ", R_last_file_info->io->name, "Position around " + to_string(R_last_file_info->io->get_position()));

	R_last_file_info->io->close();

	R_last_file_info->mode = vscdefs::FILE_MODE_CLOSED;	

	R_last_file_info->rc = vscdefs::MSGLEVEL_ERROR;
}

// Check parm for compress mode
bool vscfsor::is_sec_mode()
{
	return (R_compressed || R_encrypted);
}

// Indicator if compression for target data
bool vscfsor::IsComressionRequired(string name, bool is_dir, short level)
{
	if ((cmd->CD_BYTE_PARM & vscdefs::PARM_COMPRESS) == 0)
		return false;																			// Compression is not required

	if (cmd->CD_COMPRESS == "")
		return true;

	if (is_dir)
	{
		if (RW_compress_level >= 0)
		{
			if (level > RW_compress_level)
				return true;
			else
				RW_compress_level = -1;
		}

		//Check dir
		string dir = vs2iolib::get_file_name(name);												// Get last level dir name


		if (check_compress_pattern(dir, cmd->CD_COMPRESS, true))
		{
			RW_compress_level = level;
			return true;
		}

		return false;
	}
	else
		return check_compress_pattern(vs2iolib::get_file_name(name), cmd->CD_COMPRESS, false);

}

