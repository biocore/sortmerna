/**
 * @file index.cpp
 * @brief Representation of indexed reference information.
 * @parblock
 * SortMeRNA - next-generation reads filter for metatranscriptomic or total RNA
 * @copyright 2012-16 Bonsai Bioinformatics Research Group
 * @copyright 2014-16 Knight Lab, Department of Pediatrics, UCSD, La Jolla
 *
 * SortMeRNA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SortMeRNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SortMeRNA.  If not, see <http://www.gnu.org/licenses/>.
 * @endparblock
 *
 * @contributors Jenya Kopylova, jenya.kopylov@gmail.com
 *               Laurent Noé, laurent.noe@lifl.fr
 *               Pierre Pericard, pierre.pericard@lifl.fr
 *               Daniel McDonald, wasade@gmail.com
 *               Mikaël Salson, mikael.salson@lifl.fr
 *               Hélène Touzet, helene.touzet@lifl.fr
 *               Rob Knight, robknight@ucsd.edu
 */


#include <deque>
#include <chrono>
#include <algorithm>
#include <locale>
#include <string>
#include <cstdint>
#include <fstream>
#include <ios>

#include "index.hpp"
#include "indexdb.hpp"
#include "paralleltraversal.hpp"
#include "references.hpp"
#include "refstats.hpp"


void Index::load(uint32_t idx_num, uint32_t idx_part, Runopts & opts, Refstats & refstats)
{
	// STEP 1: load the kmer 'count' variables (dbname.kmer.dat)
	std::string idxfile = opts.indexfiles[idx_num].second + ".kmer_" + std::to_string(idx_part) + ".dat";
	std::ifstream inkmer(idxfile, std::ios::in | std::ios::binary);

	if (!inkmer.good())
	{
		fprintf(stderr, "\n  ERROR: The index '%s' does not exist.\n", idxfile.c_str()); // (char*)
		fprintf(stderr, "  Make sure you have constructed your index using the command `indexdb'. See `indexdb -h' for help.\n\n");
		exit(EXIT_FAILURE);
	}

	uint32_t limit = 1 << refstats.lnwin[idx_num];

	for (uint32_t i = 0; i < limit && !inkmer.eof(); i++)
	{
		lookup_tbl.push_back(kmer());
		inkmer.read(reinterpret_cast<char*>(&(lookup_tbl[i].count)), sizeof(uint32_t));
	}
	inkmer.close();

	// STEP 2: load the burst tries ( bursttrief.dat, bursttrier.dat )
	std::string btriefile = opts.indexfiles[idx_num].second + ".bursttrie_" + std::to_string(idx_part) + ".dat";
	std::ifstream btrie(btriefile, std::ios::in | std::ios::binary);
	if (!btrie.good())
	{
		fprintf(stderr, "\n  ERROR: The index '%s' does not exist.\n", btriefile.c_str()); // (char*)
		fprintf(stderr, "  Make sure you have constructed your index using the command `indexdb'. See `indexdb -h' for help.\n\n");
		exit(EXIT_FAILURE);
	}

	// loop through all 9-mers
	for (uint32_t i = 0; i < limit && !btrie.eof(); i++)
	{
		uint32_t sizeoftries[2] = { 0 };
		char *dst = NULL; // ptr to block of memory for two mini-burst tries

		// the size of both mini-burst tries
		for (int j = 0; j < 2; j++)
		{
			btrie.read(reinterpret_cast<char*>(&sizeoftries[j]), sizeof(uint32_t));
		}

		// allocate contiguous memory for both mini-burst tries if they exist
		if (lookup_tbl[i].count != 0)
		{
			dst = new char[(sizeoftries[0] + sizeoftries[1])]();
			if (dst == NULL)
			{
				fprintf(stderr, "  ERROR: failed to allocate memory for mini-burst tries\n");
				exit(EXIT_FAILURE);
			}
			// load 2 burst tries per 9-mer
			for (int j = 0; j < 2; j++)
			{
				// mini-burst trie exists
				if (sizeoftries[j] != 0)
				{
					NodeElement newnode[4]; // create a root trie node
					// copy the root trie node into the beginning of burst trie array
					memcpy(dst, &newnode[0], sizeof(NodeElement) * 4);
					memset(dst, 0, sizeof(NodeElement) * 4);
					if (j == 0) lookup_tbl[i].trie_F = (NodeElement*)dst;
					else lookup_tbl[i].trie_R = (NodeElement*)dst;
					// queue to store the trie nodes as we create them
					std::deque<NodeElement*> nodes;
					nodes.push_back((NodeElement*)dst);
					((NodeElement *&)dst) += 4;
					// queue to store the flags of node elements given in the binary file
					std::deque<char> flags;
					// read the first trie node
					for (int i = 0; i < 4; i++)
					{
						char tmp;
						btrie.read(reinterpret_cast<char*>(&tmp), sizeof(char));
						flags.push_back(tmp);
					}
					// build the mini-burst trie
					while (!nodes.empty())
					{
						// ptr to traverse each trie node
						NodeElement* node = nodes.front();
						// trie node elements
						for (int i = 0; i < 4; i++)
						{
							unsigned char flag = flags.front();
							// what does the node element point to
							switch (flag)
							{
								// set values to 0
							case 0:
							{
								node->flag = 0;
								node->size = 0;
								node->nodetype.trie = NULL;
							}
							break;
							// trie node
							case 1:
							{
								// read the trie node
								for (int i = 0; i < 4; i++)
								{
									char tmp;
									btrie.read(reinterpret_cast<char*>(&tmp), sizeof(char));
									flags.push_back(tmp);
								}
								node->flag = 1;
								node->size = 0;
								NodeElement newnode[4];
								memcpy((NodeElement*)dst, &newnode[0], sizeof(NodeElement) * 4);
								nodes.push_back((NodeElement*)dst);
								node->nodetype.trie = (NodeElement*)dst;
								((NodeElement *&)dst) += 4;
							}
							break;
							// bucket
							case 2:
							{
								uint32_t sizeofbucket = 0;
								// read the bucket info
								btrie.read(reinterpret_cast<char*>(&sizeofbucket), sizeof(uint32_t));

								char* bucket = new char[sizeofbucket]();
								if (bucket == NULL)
								{
									fprintf(stderr, "\n  %sERROR%s: failed to allocate memory for allocate bucket (paralleltraversal.cpp)\n", RED, COLOFF);
									exit(EXIT_FAILURE);
								}
								btrie.read(reinterpret_cast<char*>(bucket), sizeofbucket);
								// copy the bucket into the burst trie array
								memcpy((void*)dst, (void*)bucket, sizeofbucket);
								delete[] bucket;
								bucket = NULL;
								// assign pointers from trie node to the bucket
								node->flag = flag;
								node->nodetype.bucket = dst;
								node->size = sizeofbucket;
								dst = ((char *)dst) + sizeofbucket;
							}
							break;
							// ?
							default:
							{
								fprintf(stderr, "\n  %sERROR%s: flag is set to %d (load_index)\n", RED, COLOFF, flag);
								exit(EXIT_FAILURE);
							}
							break;
							}
							flags.pop_front();
							node++;
						}//~loop through 4 node elements in a trie node 
						nodes.pop_front();
					}//~while !nodes.empty()
				}//~if mini-burst trie exists
				else
				{
					if (j == 0) lookup_tbl[i].trie_F = NULL;
					else lookup_tbl[i].trie_R = NULL;
				}
			}//~for both mini-burst tries
		}//~if ( sizeoftries != 0 )
		else
		{
			lookup_tbl[i].trie_F = NULL;
			lookup_tbl[i].trie_R = NULL;
		}
	}//~for all 9-mers in the look-up table
	btrie.close();

	// STEP 3: load the position reference tables (pos.dat)
	std::string posfile = opts.indexfiles[idx_num].second + ".pos_" + std::to_string(idx_part) + ".dat";
	std::ifstream inreff(posfile, std::ios::in | std::ios::binary);

	if (!inreff.good())
	{
		fprintf(stderr, "\n  ERROR: The database name '%s' does not exist.\n\n", posfile.c_str());
		exit(EXIT_FAILURE);
	}

	uint32_t size = 0;
	inreff.read(reinterpret_cast<char*>(&number_elements), sizeof(uint32_t));
	positions_tbl.reserve(number_elements); // = new kmer_origin[number_elements]();

	if (positions_tbl.capacity() == 0)
	{
		fprintf(stderr, "  ERROR: could not allocate memory for positions_tbl (main(), paralleltraversal.cpp)\n");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < number_elements; i++)
	{
		/* the number of positions */
		positions_tbl.push_back(kmer_origin());
		inreff.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		positions_tbl[i].size = size;
		/* the sequence seq_pos array */
		positions_tbl[i].arr = new seq_pos[size]();
		if (positions_tbl[i].arr == NULL)
		{
			fprintf(stderr, "  ERROR: could not allocate memory for positions_tbl (paralleltraversal.cpp)\n");
			exit(EXIT_FAILURE);
		}
		inreff.read(reinterpret_cast<char*>(positions_tbl[i].arr), sizeof(seq_pos)*size);
	}

	inreff.close();
	index_num = idx_num;
	part = idx_part;
} // ~Index::load

void Index::clear()
{
	// lookup_tbl
	for (int i = 0; i < lookup_tbl.size(); i++)
	{
		if (lookup_tbl[i].trie_F != NULL)
		{
			delete[] lookup_tbl[i].trie_F;
			lookup_tbl[i].trie_F = NULL;
			lookup_tbl[i].trie_R = NULL;
		}
		if (lookup_tbl[i].trie_R != NULL)
		{
			delete[] lookup_tbl[i].trie_R;
			lookup_tbl[i].trie_R = NULL;
		}
	}
	lookup_tbl.clear();

	// positions_tbl
	for (int i = 0; i < positions_tbl.size(); i++)
	{
		if (positions_tbl[i].arr != NULL)
		{
			delete[] positions_tbl[i].arr;
			positions_tbl[i].arr = NULL;
		}
	}
	positions_tbl.clear();
} // ~Index::clear