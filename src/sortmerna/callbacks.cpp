/* 
 * FILE: callbacks.cpp
 * Created: Jan 04, 2018 Thu
 */

#include <string>
#include <cstdint>
#include <sstream>

#include "read.hpp"
#include "references.hpp"
#include "options.hpp"
#include "readstats.hpp"
#include "index.hpp"
#include "output.hpp"


 // called on each read
void reportsJob(
	std::vector<Read> & reads, /* one or two (if paired) reads */
	Runopts & opts,
	References & refs,
	Refstats & refstats,
	Output & output
)
{
	// only needs one loop through all read, no reference file dependency
	if (opts.fastxout && refs.num == 0 && refs.part == 0) 
	{
		output.report_fasta(opts, reads);
	}

	// only needs one loop through all read, no reference file dependency
	if (opts.de_novo_otu && refs.num == 0 && refs.part == 0) {
		output.report_denovo(opts, reads);
	}

	for (Read read : reads)
	{
		if (opts.blastout)
		{
			output.report_blast(opts, refstats, refs, read);
		}

		if (opts.samout)
		{
			output.report_sam(opts, refs, read);
		}
	} // ~for reads
} // ~reportsJob

/* 
 * Calculate:
 *     readstats.total_reads_mapped_cov 
 *     readstats.otu_map
 *     //read.hit_denovo see TODO in the function body
 */
void computeStats(Read & read, Readstats & readstats, References & refs, Runopts & opts)
{
	// OTU-map: index of alignment holding maximum SW score
	uint32_t index_max_score = read.hits_align_info.max_index;
	// loop all the alignments of this read
	for (uint32_t p = 0; p < read.hits_align_info.alignv.size(); ++p)
	{
		// continue loop if the reference sequence in this alignment
		// belongs to the database section currently loaded into RAM
		if ((read.hits_align_info.alignv[p].index_num == refs.num) && (read.hits_align_info.alignv[p].part == refs.part))
		{
			// get the edit distance between reference and read
			uint32_t id = 0;
			uint32_t mismatches = 0;
			uint32_t gaps = 0;

			read.calcMismatchGapId(refs, p, mismatches, gaps, id);

			int32_t align_len = abs(read.hits_align_info.alignv[p].read_end1 + 1 - read.hits_align_info.alignv[p].read_begin1);
			int32_t total_pos = mismatches + gaps + id;
			std::stringstream ss;
			ss.precision(3);
			ss << (double)id / total_pos << ' ' << (double)align_len / read.hits_align_info.alignv[p].readlen;
			double align_id_round = 0.0;
			double align_cov_round = 0.0;
			ss >> align_id_round >> align_cov_round;

			// alignment with the highest SW score passed
			// %id and %coverage thresholds
			if ((p == index_max_score) &&
				(align_id_round >= opts.align_id) &&
				(align_cov_round >= opts.align_cov))
			{
				// increment number of reads passing identity
				// and coverage threshold
				readstats.increment_total_reads_mapped_cov(); // thread safe

				// fill OTU map with highest-scoring alignment for the read
				if (opts.otumapout)
				{
					// reference sequence identifier for mapped read
					std::string refhead = refs.buffer[read.hits_align_info.alignv[p].ref_seq].header;
					std::string ref_seq_str = refhead.substr(0, refhead.find(' '));

					// read identifier
					std::string read_seq_str = read.header.substr(0, read.header.find(' '));
					readstats.pushOtuMap(ref_seq_str, read_seq_str); // thread safe
				}

				// TODO: this check is already performed during alignment (paralleltraversalJob and compute_lis_alignment). Is it necessary here?
				// do not output read for de novo OTU construction it passed the %id/coverage thresholds
				//if (opts.de_novo_otu && read.hit_denovo) read.hit_denovo = !read.hit_denovo; // flip
			}
		}//~if alignment at current database and index part loaded in RAM
	}//~for all alignments
} // ~computeStats