/* 
 * FILE: gzip.cpp
 * Created: Feb 22, 2018 Thu
 * @copyright 2016-20 Clarity Genomics BVBA
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>

#include "gzip.hpp"
#include "common.hpp"

Gzip::Gzip() 
	: gzipped(false), is_inflate(false), s_in(0), s_out(0), line_start(0)
{}

//Gzip::~Gzip() {
//	line_start = 0;
//	strm.zalloc = Z_NULL;
//	strm.zfree = Z_NULL;
//	strm.opaque = Z_NULL;
//	strm.avail_in = Z_NULL;
//	strm.next_in = Z_NULL;
//	strm.avail_out = Z_NULL;
//}

void Gzip::init(bool gzipped, bool is_inflate, size_t s_in, size_t s_out)
{
	this->gzipped = gzipped;
	this->is_inflate = is_inflate;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	int ret = 0;
	if (is_inflate) {
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit2(&strm, 47);
	}
	else {
		ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	}
	if (ret != Z_OK) {
		std::cerr << STAMP << "Error: " << ret << std::endl;
		exit(EXIT_FAILURE);;
	}

	strm.avail_out = 0;

	if (is_inflate) {
		this->s_in = s_in == 0 ? SIZE_16K : s_in; // IN buffer size
		this->s_out = s_out == 0 ? SIZE_32K : s_out; // OUT buffer size
	}
	else {
		this->s_in = s_in == 0 ? SIZE_32K : s_in;
		this->s_out = s_out == 0 ? SIZE_16K : s_out;
	}

	z_in.resize(this->s_in);
	z_out.resize(this->s_out);
	reset_buffers();
} // ~Gzip::init

/* 
 * return values: RL_OK (0) | RL_END (1)  | RL_ERR (-1)
 *
 * TODO: Make sure the stream is OK before calling this function.
 *       std::getline doesn't return error if the stream is not 
 *       readable/closed. It returns the same input it was passed.
 */
int Gzip::getline(std::ifstream & ifs, std::string & line)
{
	char* line_end = 0;
	int ret = RL_OK;

	line.clear();

	if (gzipped)
	{
		bool line_ready = false;
		for (; !line_ready; )
		{
			if (!line_start || !line_start[0])
			{
				ret = Gzip::inflatez(ifs); // inflate

				if (ret == Z_STREAM_END && strm.avail_out == SIZE_32K)
					return RL_END;

				if (ret < 0) 
					return RL_ERR;

				if (!line_start)
					line_start = (char*)z_out.data();
			}

			//line_end = strstr(line_start, "\n"); // returns 0 if '\n' not found
			line_end = std::find(line_start, (char*)&z_out[0] + SIZE_32K - strm.avail_out - 1, 10); // '\n'
			//line_end = std::find_if(line_start, (char*)&z_out[0] + OUT_SIZE - strm.avail_out - 1, [l = std::locale{}](auto ch) { return ch == 10; });
			//line_end = std::find_if(line_start, (char*)&z_out[0] + OUT_SIZE - strm.avail_out - 1, [l = std::locale{}](auto ch) { return std::isspace(ch, l); });
			if (line_end && line_end[0] == 10)
			{
				std::copy(line_start, line_end, std::back_inserter(line));

				if (line_end < (char*)&z_out[0] + SIZE_32K - strm.avail_out - 1) // check there is data after line_end
					line_start = line_end + 1; // skip '\n'
				else
				{
					line_start = 0; // no more data in OUT buffer - flag to inflate more
					strm.avail_out = 0; // mark OUT buffer as Full to reflush from the beginning [bug 61]
				}

				line_ready = true; // DEBUG: 
				
				//if (line == "@SRR1635864.196 196 length=101")
				//	std::cout << "HERE";
			}
			else
			{
				line_end = (char*)&z_out[0] + SIZE_32K - strm.avail_out; // end of data in out buffer
				std::copy(line_start, line_end, std::back_inserter(line));
				line_start = (strm.avail_out == 0) ? 0 : line_end;
				line_end = 0;
				line_ready = false;
			}
		} // ~for !line_ready
	}
	else // non-compressed file
	{
		if (ifs.eof()) return RL_END;

		std::getline(ifs, line);
		//if (ifs.fail()) return RL_ERR;
	}

	return RL_OK;
} // ~Gzip::getline

/*
 * Called from getline
 * reads chunks of compressed data into IN buffer 'Gzip::z_in' and uncomresses them into OUT buffer 'Gzip::z_out'
 */
int Gzip::inflatez(std::ifstream & ifs)
{
	int ret;

	// break out when either 
	// - error occurs, or
	// - IN buffer empty AND end of stream reached, or
	// - OUT buffer full OR OUT buffer not full AND IN buffer empty
	for (;;)
	{
		if (strm.avail_in == 0 && !ifs.eof()) // IN buffer empty -> get more compressed data from file
		{
			std::fill(z_in.begin(), z_in.end(), 0); // reset buffer to 0
			ifs.read((char*)z_in.data(), SIZE_16K); // read into IN buffer
			if (!ifs.eof() && ifs.fail())
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}

			strm.avail_in = ifs.gcount(); // number of chars read into IN buffer. see above
			strm.next_in = z_in.data(); // point gzip to the start of the IN buffer
		}

		if (strm.avail_in == 0 && ifs.eof()) // in buffer empty and end of file -> end of processing
		{
			if (strm.avail_out < SIZE_32K)
				strm.avail_out = SIZE_32K;

			ret = inflateEnd(&strm); // free up the resources

			if (ret != Z_STREAM_END)
			{
				std::cout << STAMP << "xINFO: inflateEnd status is " << ret << std::endl;
			}

			return Z_STREAM_END;
		}

		if (strm.avail_out == 0) // out buffer is full - reset
		{
			std::fill(z_out.begin(), z_out.end(), 0); // reset buffer to 0
			strm.avail_out = SIZE_32K;
			strm.next_out = z_out.data(); // point gzip to the start of the OUT buffer
		}

		ret = inflate(&strm, Z_NO_FLUSH); //  Z_NO_FLUSH Z_SYNC_FLUSH Z_BLOCK
		assert(ret != Z_STREAM_ERROR);
		switch (ret)
		{
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR; /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			return ret;
		case Z_STREAM_END:
			break;
		}

		if (strm.avail_out == 0 || (strm.avail_out < SIZE_32K && strm.avail_in == 0))
		//                 |_ OUT buffer is Full   |_ OUT buffer not empty     |_ IN buffer is empty
			break;
	} // for(;;)

	return ret;// == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
} // ~Gzip::inflatez

int Gzip::zip_ss(std::istream& ist, std::ofstream& ofs) 
{
	int ret, flush;
	unsigned have; // number of data bytes in OUT buffer

	// compress until end of file
	for (;;flush != Z_FINISH) {
		ist.read((char*)z_in.data(), s_in); // read into IN buffer
		strm.avail_in = ist.gcount();  // number of chars read into IN buffer
		if (!ist.eof() && ist.fail()) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = ist.eof() ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = z_in.data(); // point gzip to the start of the IN buffer

		// deflate input until output buffer not full, finish
		// compression if end of input reached
		for (;;strm.avail_out == 0) {
			strm.avail_out = s_out;
			strm.next_out = z_out.data();
			ret = deflate(&strm, flush);
			assert(ret != Z_STREAM_ERROR);
			have = s_out - strm.avail_out;
			ofs.write(reinterpret_cast<char*>(z_out.data()), have);
			if (ofs.bad() || ofs.fail()) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		}
		assert(strm.avail_in == 0);
	}
	assert(ret == Z_STREAM_END);

	(void)deflateEnd(&strm); // clean up
	return Z_OK;
} // ~Gzip::zip_ss

void Gzip::reset_buffers() 
{
	std::fill(z_in.begin(), z_in.end(), 0); // 0 fill IN buffer
	std::fill(z_out.begin(), z_out.end(), 0); // 0 fill OUT buffer
}