#include "pch.h"
#include "vs2cmx.h"
#include "vs2comlib.h"


using namespace std;

const unsigned char vs2cmx::XCMP_CHUNK_SG[2] = { 0xE7, 0x56 };	// Chunk compression signature
//const unsigned long long vs2cmx::XCMP_CHUNK_SG_POS = 0;					// +00(02) Signature position

const unsigned long long XCMP_DESCRIPTOR_POS = 2;		// +02(02) Descriptor position (reserved)
const unsigned long long XCMP_DECOMPRESSED_POS = 4;		// +04(04) Position of the decompressed block length (bytes)
const unsigned long long XCMP_COMPRESSED_POS = 8;		// +08(04) Position of the compressed block length (bytes)


// Sliding dictionary size and hash table's size 
// Some combinations of HASHBITS and THRESHOLD values will not work
// correctly because of the way this program hashes strings
static const unsigned int DICTBITS = 14;// 13;
static const unsigned int HASHBITS = 12;//  10;
static const unsigned int DICTSIZE = (1 << DICTBITS);
static const unsigned int HASHSIZE = (1 << HASHBITS);

// Minimum match length & maximum match length
static const unsigned int THRESHOLD = 3;//2;
static const unsigned int MATCHBITS = 8;// 4;
static const unsigned int MAXMATCH = ((1 << MATCHBITS) + THRESHOLD - 1);

// Dictionary plus MAXMATCH extra chars for string comparisions
unsigned char dict[DICTSIZE + MAXMATCH];

// hashtable & link list table
unsigned int hash_t[HASHSIZE];
unsigned int nextlink[DICTSIZE + 1];

const unsigned int masks[17] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535 };

// Ratio vs. speed constant
// The larger this constant, the better the compression
static const unsigned int MAXCOMPARES = 75;

// Unused entry flag
static const unsigned int NIL = 0xFFFF;

/* bits per symbol- normally 8 for general purpose compression */
static const unsigned int CHARBITS = 8;


// # of bits to shift after each XOR hash/
// This constant must be high enough so that only THRESHOLD + 1
// characters are in the hash accumulator at one time
static const unsigned int SHIFTBITS = ((HASHBITS + THRESHOLD) / (THRESHOLD + 1));

// Sector size constants
static const unsigned int SECTORBIT = 10;									// The number of sector length bits
static const unsigned int SECTORLEN = (1 << SECTORBIT);						// Sector length
static const unsigned int SECTORAND = ((0xFFFF << SECTORBIT) & 0xFFFF);		// Mask to suppress right <SECTORBIT> bits
unsigned int bitsin;
static unsigned int matchlength;
static unsigned int matchpos;
static unsigned int bitbuf;

// Source parms
unsigned char * src;				// Source data pointer
unsigned char * trg;				// Target data pointer
unsigned long int src_len = 0;		// Source length 
unsigned long int src_pos = 0;		// Source current position 

// Bit counter
unsigned long int bit_cnt = 0;

// Main compressor
unsigned long int vs2cmx::Compress(const unsigned char * source, unsigned char * target, const unsigned long int length)
{
	// Save parms
	src = (unsigned char *)source;
	trg = target;
	src_len = length;

	// Source current position
	src_pos = 0;

	// Initial bit counter
	bit_cnt = XCMP_HEADER_SIZE * 8;

	// Create initial header
	memset(trg, 0, XCMP_HEADER_SIZE);
	vs2comlib::copy_char_array(trg, XCMP_CHUNK_SG_POS, (unsigned char *)XCMP_CHUNK_SG, 0, 2); // Save signature
	vs2comlib::convert_int_to_char(length, target + XCMP_DECOMPRESSED_POS);

	unsigned int dictpos, deleteflag, sectorlen;
	unsigned long bytescompressed;

	// Init hash_t, nextlink
	for (unsigned long int i = 0; i < HASHSIZE; i++)
		hash_t[i] = NIL;					// 0xffff
	nextlink[DICTSIZE] = NIL;				// 0xffff


	dictpos = deleteflag = 0;

	bytescompressed = 0;

	while (true)
	{
		// Delete old data from dictionary 
		if (deleteflag) 
			delete_data(dictpos);

		// Grab more data to compress 
		if ((sectorlen = load_dict(dictpos)) == 0) 
			break;

		// Hash the data 
		hash_data(dictpos, sectorlen);

		// Find dictionary matches
		dict_search(dictpos, sectorlen);

		bytescompressed += sectorlen;

		dictpos += SECTORLEN;

		// Wrap back to beginning of dictionary when its full
		if (dictpos == DICTSIZE)
		{
			dictpos = 0;
			deleteflag = 1;						// OK to delete now 
		}
	}

	unsigned long int ln = (bit_cnt << (sizeof(bit_cnt) * 8 - 3)) ? bit_cnt / 8 + 1 : bit_cnt / 8;

	vs2comlib::convert_int_to_char(ln, target + XCMP_COMPRESSED_POS);
	
	return ln;
}

// Delete data from the dictionary search structures 
// this is only done when the number of bytes to be
// compressed exceeds the dictionary's size 
void vs2cmx::delete_data(const unsigned int dictpos)
{

	register unsigned int i, j;

	j = dictpos;

	// Delete all references to the sector being deleted 

	for (i = 0; i < DICTSIZE; i++)
		if ((nextlink[i] & SECTORAND) == j)
			nextlink[i] = NIL;		// SECTORAND- mask: 1111 1100 0000 0000 (10 bits)

	for (i = 0; i < HASHSIZE; i++)
		if ((hash_t[i] & SECTORAND) == j)
			hash_t[i] = NIL;

}

// Load dictionary with characters from the input stream
unsigned int vs2cmx::load_dict(const unsigned int dictpos)
{
	register unsigned int i = 0, j = 0;

	if (src_pos == src_len)
		return 0;																	// EOD

	i = ((src_len - src_pos) > SECTORLEN) ? SECTORLEN : src_len - src_pos;			// Calculate # of chars
	
	vs2comlib::copy_char_array(&dict[dictpos], 0, src, src_pos, i);					// Copy chars

	src_pos += i;

	//if ((i = fread(&dict[dictpos], sizeof(char), SECTORLEN, infile)) == EOF)
	//{
	//	printf("\nerror reading from input file");
	//	exit(EXIT_FAILURE);
	//}

	// Since the dictionary is a ring buffer, copy the characters at
	// the very start of the dictionary to the end
	if (dictpos == 0)
	{
		for (j = 0; j < MAXMATCH; j++) dict[j + DICTSIZE] = dict[j];
	}

	return i;
}

// Hash data just entered into dictionary
// XOR hashing is used here, but practically any hash function will work
void vs2cmx::hash_data(const unsigned int dictpos, const unsigned int bytestodo)
{
	register unsigned int i, j, k;

	if (bytestodo <= THRESHOLD)   // not enough bytes in sector for match? 
		for (i = 0; i < bytestodo; i++) nextlink[dictpos + i] = NIL;
	else
	{
		// Matches can't cross sector boundries 
		for (i = bytestodo - THRESHOLD; i < bytestodo; i++)
			nextlink[dictpos + i] = NIL;

		j = (((unsigned int)dict[dictpos]) << SHIFTBITS) ^ dict[dictpos + 1];

		k = dictpos + bytestodo - THRESHOLD;  // Calculate end of sector 

		for (i = dictpos; i < k; i++)
		{
			nextlink[i] = hash_t[j = (((j << SHIFTBITS) & (HASHSIZE - 1)) ^ dict[i + THRESHOLD])];
			hash_t[j] = i;
		}
	}
}


// Find dictionary matches for characters in current sector
void vs2cmx::dict_search(const unsigned int dictpos, const unsigned int bytestodo)
{
	register unsigned int i, j;
	// Greedy search loop (fast)

	i = dictpos; j = bytestodo;

	while (j) // Loop while there are still characters left to be compressed
	{
		find_match(i, THRESHOLD);

		if (matchlength > j) matchlength = j;		// Clamp matchlength

		if (matchlength > THRESHOLD)				// Valid match?
		{
			send_match(matchlength, (i - matchpos) & (DICTSIZE - 1));
			i += matchlength;
			j -= matchlength;
		}
		else
		{
			send_char(dict[i++]);
			j--;
		}
	}
}

// Find match for string at position dictpos 
// This search code finds the longest AND closest
// match for the string at dictpos
void vs2cmx::find_match(const unsigned int dictpos, const unsigned int startlen)
{
	register unsigned int i, j, k;
	unsigned char l;


	i = dictpos; matchlength = startlen; k = MAXCOMPARES;
	l = dict[dictpos + matchlength];

	do
	{
		if ((i = nextlink[i]) == NIL) return;   // Get next string in list

		if (dict[i + matchlength] == l)			// Possible larger match? 
		{
			for (j = 0; j < MAXMATCH; j++)		// Compare strings
				if (dict[dictpos + j] != dict[i + j]) break;

			if (j > matchlength)				// Found larger match? 
			{
				matchlength = j;
				matchpos = i;
				if (matchlength == MAXMATCH) 
					return;						// Exit if largest possible match

				l = dict[dictpos + matchlength];
			}
		}
	} while (--k);								// Keep on trying until we run out of chances
}

// Main decompressor
unsigned long int vs2cmx::Decompress(const unsigned char * source, unsigned char * target)
{
	register unsigned long int i, j, k;
	
	unsigned long int decompressed_length = vs2cmx::GetDecompressedLength(source);

	src = (unsigned char *)source;
	trg = (unsigned char *)target;

	// Initial bit counter
	bit_cnt = XCMP_HEADER_SIZE * 8;
	i = 0;
	while (i < decompressed_length)
	{
		int QQ = 0;
		unsigned long int nbits = 0;

		if (!vs2comlib::bs_get(src, bit_cnt))   // Character or match?
		{
			vs2comlib::bs_copy_bits(src, trg, bit_cnt + 1, i * 8, CHARBITS);
			k = 1;							// 1 byte 
			nbits = 9;
		}
		else
		{
			unsigned char b[2] = { 0 };

			// Get match length from input stream
			k = (unsigned long int)vs2comlib::bs_get_int(src, bit_cnt + 1, MATCHBITS) + THRESHOLD + 1;

			// Get match position from input stream 
			vs2comlib::bs_copy_bits(src, b, bit_cnt + 1 + MATCHBITS, 16 - DICTBITS, DICTBITS);
			unsigned long int offset = (unsigned long int)vs2comlib::convert_char_to_short(b);                   // vs2comlib::bs_get_int(src, bit_cnt + 1 + MATCHBITS, DICTBITS);
			j = (unsigned long int)(i - offset);
			
			vs2comlib::copy_char_array(trg, i, trg, j, k);

			nbits = MATCHBITS + DICTBITS + 1;
		}

		i += k;
		bit_cnt += nbits;
	}

	return decompressed_length;
}

// Send a match to the output stream
void vs2cmx::send_match(const unsigned int matchlen, const unsigned int matchdistance)
{
	unsigned char b[2] = { 0 };

	vs2comlib::convert_short_to_char(matchdistance, b);

	vs2comlib::bs_set(trg, bit_cnt, true);

	unsigned int n = matchlen - (THRESHOLD + 1);

	vs2comlib::bs_copy_bits((unsigned char *)&n, trg, 0, bit_cnt + 1, MATCHBITS);

	vs2comlib::bs_copy_bits(b, trg, 16 - DICTBITS, bit_cnt + 1 + MATCHBITS, DICTBITS);

	bit_cnt += (1 + MATCHBITS + DICTBITS);
}

// Send one character (or literal) to the output stream
void vs2cmx::send_char(const unsigned int character)
{
	vs2comlib::bs_set(trg, bit_cnt, false);

	vs2comlib::bs_copy_bits((unsigned char *)&character, trg, 0, bit_cnt + 1, CHARBITS);

	bit_cnt += 9;
}

// Return compressed length
unsigned long int vs2cmx::GetCompressedLength(const unsigned char * source)
{
	return vs2comlib::convert_char_to_int(source + XCMP_COMPRESSED_POS);
}

// Return decompressed length
unsigned long int vs2cmx::GetDecompressedLength(const unsigned char * source)
{
	return vs2comlib::convert_char_to_int(source + XCMP_DECOMPRESSED_POS);
}

// Check compression signature
bool vs2cmx::CheckSG(const unsigned char * source)
{
	return (vs2comlib::compare_keys((unsigned char *)source, 2, (unsigned char *)vs2cmx::XCMP_CHUNK_SG, 2, false) == 0);
}