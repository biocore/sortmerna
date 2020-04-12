#pragma once
/* 
 * FILE: gzip.hpp
 * Created: Feb 22, 2018 Thu
 */

#include <vector>

#include "zlib.h"

constexpr auto SIZE_32K = 32768U; // IN or OUT buffer size
constexpr auto SIZE_16K = 16384U; // IN or OUT buffer size
constexpr auto RL_OK = 0;
constexpr auto RL_END = 1;
constexpr auto RL_ERR = -1;

class Gzip
{
public:
	Gzip();
	//~Gzip();
	void init(bool gzipped = false, bool is_inflate = false, size_t s_in = 0, size_t s_out = 0);
	int getline(std::ifstream & ifs, std::string & line);
	int zip_ss(std::istream& ist, std::ofstream& ofs); // zip stream to stream
	void reset_buffers();

private:
	bool gzipped;
	bool is_inflate; // if True -> perform inflation, else -> deflation
	// zlib related
	char* line_start; // pointer to the start of a line within the 'z_out' buffer
	z_stream strm; // stream control structure. Holds stream in/out buffers (byte arrays), sizes, positions etc.
	unsigned s_in;
	unsigned s_out;
	std::vector<unsigned char> z_in; // IN buffer for compressed data
	std::vector<unsigned char> z_out; // OUT buffer for decompressed data

private:
	int inflatez(std::ifstream & ifs); // 'z' in the name to distinguish from zlib.inflate
};