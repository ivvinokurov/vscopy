#include "pch.h"
#include "vs2trans.h"
#include "vs2comlib.h"
#include "vs2iolib.h"
//#include "vs2iom.h"
#include "vs2iof.h"



vs2trans::vs2trans(const string &ta_file, const string &key)
{
	p_path = vs2comlib::trim(ta_file);
	p_mode = MODE_NONE;
	p_key = key;
}

vs2trans::vs2trans()
{
	p_path = "";
	p_mode = MODE_NONE;
}

////////////////////////////////////////////////////////////////
////////////////  TRANSACTION LOG MODE METHODS /////////////////
////////////////////////////////////////////////////////////////

// Begin transaction
void vs2trans::Begin()
{
	if (p_mode != MODE_NONE)
		throw exception("Transaction::begin - already in the roll mode or started");

	if (IO.file_exists())
		throw exception("Transaction::begin  - previous transaction is not completed or rolled back (transaction log is not removed)");

	IO.open(p_path, vs2iolib::FILE_MODE_CREATE, p_key);
	p_current_pos = 0;
	p_mode = MODE_STARTED;
}

void vs2trans::Write(const long long id, const long long address, const unsigned char * data, const long long length)
{
	if (p_mode != MODE_STARTED)
		throw exception("Transaction::write - not in the log mode");

	IO.write_long(id);
	IO.write_long(address);
	IO.write_long(length);
	IO.write(length, (unsigned char*)data);
	IO.write_long(length + 24 + 8);
	IO.flush();
}


void vs2trans::Commit()
{
	if (p_mode != MODE_STARTED)
		throw exception("Transaction::commit - not in the log mode");

	IO.close();
	IO.rmfile();

	p_mode = MODE_NONE;
}


////////////////////////////////////////////////////////////////
////////////////  ROLLBACK MODE METHODS ////////////////////////
////////////////////////////////////////////////////////////////

void vs2trans::Open()
{
	if (p_mode != MODE_NONE)
		throw exception("Transaction::open - already in the roll mode or started");

	if (!IO.file_exists())
		p_eof = true;
	else
	{
		p_eof = false;
		IO.open(p_path, vs2iolib::FILE_MODE_OPEN, p_key);
		if (vs2iolib::get_file_length(p_path) == 0)
			p_eof = true;
		else
			p_current_pos = IO.get_length();			// Point to the end initially
	}

	p_mode = MODE_ROLL;
}

bool vs2trans::Read()
{
	if (p_mode != MODE_ROLL)
		throw "Transaction::read - not in the roll mode";

	if (p_eof)
	{
		this->T_id = -1;
		this->T_address = -1;
		this->T_length = -1;
		this->T_data = nullptr;
		return false;
	}
	else
	{
		IO.set_position(p_current_pos - 8);
		long long ln = IO.read_long();					// Length
		
		p_current_pos -= ln;

		IO.set_position(p_current_pos);

		this->T_id = IO.read_long();
		this->T_address = IO.read_long();
		this->T_length = IO.read_long();

		if (this->buf_length > 0)
		{
			if (this->T_length > this->buf_length)
			{
				delete(this->T_data);
				this->T_data = (unsigned char *)malloc((size_t)this->T_length);
				this->buf_length = this->T_length;
			}
		}
		else
		{
			this->T_data = (unsigned char *)malloc((size_t)this->T_length);
			this->buf_length = this->T_length;
		}
		
		IO.read(this->T_length, this->T_data);

		if (p_current_pos == 0)
			p_eof = true;

		return true;
	}
}


void vs2trans::Close()
{
	if (p_mode != MODE_ROLL)
		throw exception("Transaction::close - not in the roll mode");

		IO.close();
		IO.rmfile();

	p_mode = MODE_NONE;

	if (this->buf_length > 0)
		free (this->T_data);
}

	////////////////////////////////////////////////////////////////
	////////////////  OTHER METHODS ////////////////////////////////
	////////////////////////////////////////////////////////////////

bool vs2trans::Started() const
{
	return (p_mode == MODE_STARTED);
}

bool vs2trans::RollMode() const
{
	return (p_mode == MODE_ROLL);
}


bool vs2trans::Pending() 
{
	return (vs2iolib::get_file_length(p_path) > 0);
}

/////////////////////////////////////////////////
/////////////// GET RECORD FIELDS ///////////////
/////////////////////////////////////////////////

long long vs2trans::get_T_id()
{
	return this->T_id;
}

long long vs2trans::get_T_address()
{
	return this->T_address;
}

long long vs2trans::get_T_length()
{
	return this->T_length;
}

void vs2trans::get_T_data(unsigned char * data)
{
	vs2comlib::copy_char_array(data, 0, this->T_data, 0, this->T_length);
}

