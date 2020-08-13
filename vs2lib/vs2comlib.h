#pragma once
#include <string>
#include <vector>
#include <windows.h>

using namespace std;

class vs2comlib final
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// DATA CONVERSION //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////// BIT STEAM OPERATIONS ////////////////////////

	// Set bits
	// Write bit data from the 'source' starting at bit 0 to 'target' starting at bit 'pos'
	// Parms:
	// src - source data (bits starts from 0)
	// trg - target data (bits starts from 'pos')
	// pos - target bit index (0..n)
	// len - number of bits
	static void bs_set_bits(unsigned char* src, unsigned char* trg, unsigned long int pos, unsigned long int len);

	// Get bits
	// Write bit data from the 'source' starting at bit 'pos' to 'target' starting at bit 0
	// Parms:
	// src - source data (bits starts from 'pos')
	// trg - target data (bits starts from 0)
	// pos - source bit index (0..n)
	// len - number of bits
	static void bs_get_bits(unsigned char* src, unsigned char* trg, unsigned long int pos, unsigned long int len);

	// Copy bits
	// Write bit data from the 'source' starting at bit 'src_pos' to 'target' starting at bit trg_pos
	// Parms:
	// src - source data
	// trg - target data
	// src_pos - source bit index (0..n)
	// trg_pos - target bit index (0..n)
	// len - number of bits
	static void bs_copy_bits(unsigned char* src, unsigned char* trg, unsigned long int src_pos, unsigned long int trg_pos, unsigned long int len);

	// Set bit value
	static void bs_set(unsigned char* trg, unsigned long int pos, bool value);

	// Get bit value
	static bool bs_get(unsigned char* src, unsigned long int pos);

	// Compare bit values
	static bool bs_compare(unsigned char* src1, unsigned char* src2, unsigned long int pos1, unsigned long int pos2, unsigned long int len);

	// Get integer value
	static unsigned long long bs_get_int(unsigned char* src, unsigned long int pos, unsigned long int len);

	//////////////////////// TO HEX CONVERSIONS //////////////////////////


	/// <summary>
	/// Convert char array to hex representation
	/// </summary>
	static void convert_char_to_hex_char(unsigned const char* source, const int length, unsigned char* target);



	/// <summary>
	/// Convert short to hex representation
	/// </summary>
	static std::string convert_short_to_hex_string(short value);

	/// <summary>
	/// Convert int to hex representation
	/// </summary>
	static std::string convert_int_to_hex_string(int value);

	/// <summary>
	/// Convert long to hex representation
	/// </summary>
	static std::string convert_long_to_hex_string(long long value);

	/// <summary>
	/// Convert string to hex representation
	/// </summary>
	static std::string convert_string_to_hex_string(const std::string& value);

	/// <summary>
	/// Convert wchar to hex representation string
	/// </summary>
	static std::string convert_wchar_to_hex_string(const wchar_t* value, long long length);

	/// <summary>
	/// Convert wchar to wtring
	/// </summary>
	static std::wstring convert_wchar_to_wstring(wchar_t* value, long long index, long long length);

	/// <summary>
	/// Convert wchar to char
	/// Length - wchar size (number of wchars)
	/// </summary>
	static void convert_wchar_to_char(wchar_t* source, long long length, unsigned char* target);

	/// <summary>
	/// Convert wstring to wchar
	/// </summary>
	static void convert_wstring_to_wchar(const wstring& value, wchar_t* b);

	/// <summary>
	/// Convert wstring to char array
	/// </summary>
	static void convert_wstring_to_char(const wstring& value, unsigned char* b);


	//////////////// TO BYTE ARRAY CONVERSIONS //////////////////////////

	// Convert long to byte array
	// High-to-low (sort order)
	static void convert_long_to_char(const long long n, unsigned char* b);
	// As Is in memory
	static void convert_long_to_char_asis(const long long n, unsigned char* b);

	/// Convert int to byte array
	// High-to-low (sort order)
	static void convert_int_to_char(const int value, unsigned char* b);
	// As Is in memory
	static void convert_int_to_char_asis(const int value, unsigned char* b);

	// Convert short to byte array
	// High-to-low (sort order)
	static void convert_short_to_char(short int value, unsigned char* b);
	// As Is in memory
	static void convert_short_to_char_asis(short int value, unsigned char* b);

	/// <summary>
	/// Convert string to byte array
	/// </summary>
	static void convert_string_to_char(const std::string& value, unsigned char* b);


	//////////////// FROM BYTE ARRAY CONVERSIONS //////////////////////////

	// Convert byte array to long
	// High-to-low (sort order)
	static long long convert_char_to_long(const unsigned char* value);
	// As Is in memory
	static long long convert_char_to_long_asis(const unsigned char* value);

	// Convert byte array to int
	// High-to-low (sort order)
	static int convert_char_to_int(const unsigned char* value);
	// As Is in memory
	static int convert_char_to_int_asis(const unsigned char* value);


	// Convert byte array to short
	// High-to-low (sort order)
	static short convert_char_to_short(const unsigned char* value);
	// As Is in memory
	static short convert_char_to_short_asis(const unsigned char* value);

	/// Convert byte array to string
	static std::string convert_char_to_string(const unsigned char* value, long long index = 0, long long length = 0);

	// Convert byte array to hex string
	static std::string convert_char_to_hex_string(const unsigned char* value, long long index = 0, long long length = 0);

	//////////////// SIMPLE TYPES CONVERSIONS (TO/FROM STRING) //////////////////////////

	/// <summary>
	/// Convert string to long
	/// </summary>
	static long long convert_string_to_long(const std::string& value);

	/// <summary>
	/// Convert string to int
	/// </summary>
	static int convert_string_to_int(const std::string& value);

	/// <summary>
	/// Convert string to float
	/// </summary>
	static float convert_string_to_float(const std::string& value);

	/// <summary>
	/// Convert string to double
	/// </summary>
	static double convert_string_to_double(const std::string& value);

	//////////////////////////////////////////////////////////////////////
	////////////////////////    Operations    ////////////////////////////
	//////////////////////////////////////////////////////////////////////
	/// <summary>
	/// Parse string with delimiters to the string array
	/// </summary>
	static std::vector<std::string> parse(const std::string& value, const std::string& delimiters = "/");

	// Parse string extended
	static vector<string> parse_ext(const string& value, const string& delimiters = "/");

	/// <summary>
	/// Compare to strings, 2nd is a pattern and can include wildcards '*' and '?'
	/// </summary>
	static bool compare(const std::string& pattern, const std::string& value);

	/// <summary>
	/// Compare char[] keys
	/// </summary>
	static int compare_keys(unsigned char* x, int keylenx, unsigned char* y, int keyleny, bool partial = false);

	/// <summary>
	/// Copy sub-array of bytes
	/// </summary>
	static void copy_char_array(unsigned char* target, long long target_offset, unsigned char* source, long long source_offset, long long length);

	/// <summary>
	/// Check if string represents a numeric value
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	static bool is_numeric(const string& value);

	// Find first index of the substring
	static int index_of_first(string input, string substring);

	// Find last index of the substring
	static int index_of_last(string input, string substring);

	//////////////////////////////////////////////////////////////////////
	//////////////////////// Basic functions  ////////////////////////////
	//////////////////////////////////////////////////////////////////////

	// Convert to Lowercase
	static string to_lower(const std::string& str);

	// Convert to Uppercase
	static string to_upper(const std::string& str);

	// Pad left by chars
	static string pad_left(const string& str, const unsigned int len, const string sym);

	// Pad right by chars
	static string pad_right(const string& str, const unsigned int len, const string sym);

	// Trim all spaces
	static string trim(const std::string& str);

	// Get current date-time representation
	static string current_datetime();

	// Get current time representation
	static string current_time();

	// Return formatted size: bytes, Kb, Mb, Gb, Tb
	static string formatted_size(long long sz);

	// Return formatted number with delimiters
	static string formatted_number(long long sz);

	//////////////////////////////////////////////////////////////////////
	////////////////////////////// SORT //////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	static void sort32(long int* items, long int* refs, long int count, bool asc = true);
	static void sort64(long long* items, long long* refs, long long count, bool asc = true);
	static long long find64(long long* data, long long left, long long size, long long value);
	static long long find64ge(long long* data, long long size, long long value);



private:
	static void q_sort32(long int* items, long int* refs, long int left, long int right, bool asc);
	static void q_sort64(long long* items, long long* refs, long long left, long long right, bool asc);

};
