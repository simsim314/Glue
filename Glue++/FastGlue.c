#include "LifeAPI.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include<omp.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


void LoadParentVector(const std::vector<unsigned long>& parentIndexes, const std::vector<signed char>& dists, int idx, std::vector<int>& Hs)
{
	Hs.clear();
	int curIdx = idx;
	
	while(curIdx > 0)
	{
		Hs.push_back(dists[curIdx]);
		curIdx = parentIndexes[curIdx];
	}
}

class SearchParams
{
public:
	LifeBox* maxBox;
	int blockX;
	int blockY;

	std::vector<LifeState*> initGliders;
	LifeState* initialBlock;
	
	SearchParams(int maxIt, int block_x, int block_y)
	{
		maxBox = NewBox(-30, -30, 25, 25);
		blockX = block_x;
		blockY = block_y;

		gld = NewState("3o$o$bo!", 29, 29);
		initGliders.push_back(gld);
		initialBlock = NewState("2o$2o!", blockX, blockY);
	}
};

void PutNodeState(const SearchParams& params, const std::vector<unsigned long>& parentIndexes, const std::vector<signed char>& dists, int idx, std::vector<int>& Hs)
{
	LoadParentVector(parentIndexes, dists, idx, Hs);
		
	New();
	
	PutState(params.initialBlock, params.blockX, params.blockY);
	
	for(int i = Hs.size() - 1; i >= 0; i--)
	{
		PutState(params.initGliders[Hs[i] + 30 + params.yPlaceDown]);
		params.EvolveToStabilization();
	}
}

class SearchResult
{
public:
	std::vector<int> recipe;
};

std::string ToString(const std::vector<int>& v)
{
	std::stringstream ss;
	for(size_t i = 0; i < v.size(); ++i)
	{
	  if(i != 0)
		ss << ",";
		
	  ss << v[i];
	}
	return  ss.str();
}

void SaveResults(const std::string& fname, const std::vector<SearchResult*>& results)
{
	std::ofstream myfile;
	myfile.open(fname);
	
	for(int j = 0; j < results.size(); j++)
		myfile << ToString(results[j]->recipe) << std::endl;
	
	myfile.close();
}
/*
void SaveSearchTree(std::string fname, const std::vector<unsigned long>& parentIndeces, const std::vector<signed char>& dists)
{
	std::ofstream parentFILE(fname + std::string("_indices.txt"), std::ios::out);
	
	parentFILE << (int)parentIndeces.size() << "\n";
	
	for(std::vector<unsigned long>::const_iterator i = parentIndeces.begin(); i != parentIndeces.end(); ++i) 
		parentFILE << *i << "\n";
	
	parentFILE.close();
	
	std::ofstream distsFILE(fname + std::string("_dists.txt"), std::ios::out);
	
	for(std::vector<signed char>::const_iterator i = dists.begin(); i != dists.end(); ++i) 
		distsFILE << (int)*i << "\n";
	
	distsFILE.close();
	
}

void LoadSearchTree(std::string fname, std::vector<unsigned long>& parentIndeces, std::vector<signed char>& dists)
{
	parentIndeces.clear();
	dists.clear();
	
	std::ifstream parentFILE(fname + std::string("_indices.txt"), std::ios::out | std::ofstream::binary);
	std::ifstream distsFILE(fname + std::string("_dists.txt"), std::ios::out | std::ofstream::binary);

	int size;

	parentFILE >> size;
	
	for(int i = 0; i < size; i++)
	{
		int pval; 
		int dval;
		
		parentFILE >> pval;
		parentIndeces.push_back(pval);
		
		distsFILE >> dval;
		dists.push_back((signed char)dval);
	}
	
	parentFILE.close();
	distsFILE.close();
	
}
*/
void FlushTempDS(std::vector<unsigned long>& tempParentIndeces, std::vector<signed char>& tempDists, std::vector<uint64_t> &statesTempVec, std::unordered_map<uint64_t, int>& existingStates, std::vector<unsigned long>& parentIndeces, std::vector<signed char>& dists)
{
	for(int j = 0; j < tempParentIndeces.size(); j++)
	{
		uint64_t hash = statesTempVec[j];

		auto pairIter = existingStates.find(hash);

		if(pairIter == existingStates.end())
		{
			existingStates[hash] = 1;
			parentIndeces.push_back(tempParentIndeces[j]);
			dists.push_back(tempDists[j]);
		}
		else
		{
			pairIter->second++;
		}
	}

	if(existingStates.size() > 2000000)
	{
		static std::vector<uint64_t> vals;

		for (const auto& elem: existingStates) 
		{	
			if(elem.second >= 4)
			{
				vals.push_back(elem.first);
			}
		}

		existingStates.clear();

		for(int i = 0; i < vals.size(); i++)
			existingStates[vals[i]] = 0;
		
		vals.clear();
	}

	tempParentIndeces.clear();
	tempDists.clear();
	statesTempVec.clear();
}

void SlowSalvoSearch(int prevLayerStartIndex, std::vector<unsigned long>& parentIndeces, std::vector<signed char>& dists, const SearchParams& params, std::vector<SearchResult*>& results, bool searchOnly)
{
	std::unordered_map<uint64_t, int> existingStates;
	int total = 0; 
	int last = parentIndeces.size();
	std::vector<unsigned long> newParentIndeces;
	std::vector<signed char> newDists;
	
	#pragma omp parallel shared(searchOnly), shared(existingStates), shared(newDists), shared(newParentIndeces), shared(parentIndeces), shared(dists), shared(params), shared(results), shared(total)
	{
	
		std::vector<unsigned long> tempIndeces;
		std::vector<signed char> temtDists;
		std::vector<uint64_t> statesTempVec;
		std::vector<int> Hs;
		int wssIdx; 
		
		int reportStep = (last - prevLayerStartIndex) / 25;
		
		#pragma omp for
		for(int i= prevLayerStartIndex; i < last; ++i)
		{
			#pragma omp atomic
			total++;
			
			if(reportStep > 1 && total % reportStep == 0)
				printf("%d%% of %d, %d\n", 4 * total / reportStep, last - prevLayerStartIndex, newParentIndeces.size());
			
			if(!searchOnly && tempIndeces.size() > 4000)
			{
				#pragma omp critical
				FlushTempDS(tempIndeces, temtDists, statesTempVec, existingStates, newParentIndeces, newDists);
			}
			
			//Need to have an array of latest states and just place or not place a glider 
			/*SearchResult* result = new SearchResult();
			result-> wssIdx = wssIdx;
			
			LoadParentVector(parentIndeces, dists, i, result->recipe);
			std::reverse(result->recipe.begin(), result->recipe.end());
			result->recipe.push_back(j);
			
			#pragma omp critical
			{
				results.push_back(result);
				
				if(results.size() % 500 == 0)
					SaveResults(std::string("results_") + std::to_string(results.size()) + std::string(".txt"), results);
			}*/
		}
		
		if(!searchOnly)
		{
			#pragma omp critical
			FlushTempDS(tempIndeces, temtDists, statesTempVec, existingStates, newParentIndeces, newDists);
		}
	}
	
	for(int i = 0; i < newParentIndeces.size(); i++)
	{
		parentIndeces.push_back(newParentIndeces[i]);
		dists.push_back(newDists[i]);
	}
}

int main()
{
	omp_set_num_threads(6);
	New();
	
	std::vector<unsigned long> indeces;
	indeces.push_back(1);
	
	std::vector<signed char> dists;
	dists.push_back(0);
	
	int startDepth = 0;
	//LoadSearchTree(std::string("data_depth") + std::to_string(startDepth), indeces, dists);
	
	std::vector<SearchResult*> results; 

	SearchParams params(20);
	int curLayerStartIndex = 0;
	int max = 1;
	
	for(int i = startDepth; i <= max; i++)
	{
		std::cout << "Depth " << i << std::endl;
		SlowSalvoSearch(prevLayerStartIndex, indeces, dists, params, results, i == max);
		
		if(results.size() > 0)
		{
			std::string fname = std::string("P30_results_depth") + std::to_string(i) + std::string(".txt");
			SaveResults(fname, results);
			std::cout << " Written " << results.size() << " results to file:" << fname << std::endl; 
		}
		
		//if(i >= 4)
		{
			//SaveSearchTree(std::string("data_depth") + std::to_string(i), indeces, dists);
		}	
	}
	getchar();
}