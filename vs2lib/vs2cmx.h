#pragma once

//#include "vlfclib.h"

using namespace std;

class vs2cmx final
{
public:
	static const unsigned char		XCMP_CHUNK_SG[2];			// Chunk compression signature
	static const unsigned long long XCMP_CHUNK_SG_POS = 0;		// +00(02) Signature position
	static const unsigned long int	XCMP_HEADER_SIZE = 12;		// Header size


	// Main compressor
	unsigned long int Compress(const unsigned char * source, unsigned char * target, const unsigned long int length);

	// Main decompressor
	unsigned long int Decompress(const unsigned char * source, unsigned char * target);

	// Return compressed length
	static unsigned long int GetCompressedLength(const unsigned char * source);

	// Return decompressed length
	static unsigned long int GetDecompressedLength(const unsigned char * source);

	// Check compression signature
	static bool CheckSG(const unsigned char * source);

private:
	
	// Delete data from the dictionary search structures 
	// this is only done when the number of bytes to be
	// compressed exceeds the dictionary's size 
	void delete_data(const unsigned int dictpos);

	// Load dictionary with characters from the input stream
	unsigned int load_dict(const unsigned int dictpos);

	// Hash data just entered into dictionary
	// XOR hashing is used here, but practically any hash function will work
	void hash_data(const unsigned int dictpos, const unsigned int bytestodo);

	// Find dictionary matches for characters in current sector
	void dict_search(const unsigned int dictpos, const unsigned int bytestodo);

	// Find match for string at position dictpos 
	// This search code finds the longest AND closest
	// match for the string at dictpos
	void find_match(const unsigned int dictpos, const unsigned int startlen);

	// Writes multiple bit codes to the output stream 
	//void send_bits(const unsigned int bits, const unsigned int numbits);

	// Send a match to the output stream
	void send_match(const unsigned int matchlen, const unsigned int matchdistance);

	// Send one character (or literal) to the output stream
	void send_char(const unsigned int character);


};

