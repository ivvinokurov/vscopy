#pragma once
#include <vector>

class vs2crypto final
{

private:
	static const unsigned char byte_e[256];

	static const unsigned char byte_d[256];

	static const unsigned char byte_e2[256];

	static const unsigned char byte_d2[256];

	static const unsigned char byte_s[1024];

	static const long long byte_s_len = 1024;

public:

	/// <summary>
	/// Encrypt byte array
	/// </summary>
	static void encrypt(unsigned char * target_data, unsigned char * source_data, long long shift, long long length, unsigned char * key, long long keylen);

	/// <summary>
	/// Decrypt byte array
	/// </summary>
	static void decrypt(unsigned char * target_data, unsigned char * source_data, long long shift, long long length, unsigned char * key, long long keylen);
};