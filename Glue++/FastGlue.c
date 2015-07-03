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
	LifeBox* finalBox;
	LifeBox* maxBox;
	int blockX;
	int blockY;
	int xPlaceRight;
	int yPlaceDown;
	int maxIter;
	std::vector<TargetLocator*> wssVec;
	
	std::vector<LifeState*> initGliders;
	LifeState* initialBlock;
	
	SearchParams(int dist, int maxIt)
	{
		New();
		finalBox = NewBox(-30, -30, 29, 29);
		maxBox = NewBox(-dist, -dist, dist, dist);
		blockX = -3;
		blockY = -3;
		xPlaceRight = dist + 2;
		maxIter = maxIt;
		
		if(xPlaceRight % 2 == 1)
			xPlaceRight++;
			
		yPlaceDown = dist + 2;
		
		if(yPlaceDown % 2 == 1)
			yPlaceDown++;
		
		for(int y = -30; y <= xPlaceRight + yPlaceDown + 30; y++)
		{
			LifeState *gld;
			
			if(y < yPlaceDown)
				gld = NewState("3o$o$bo!", xPlaceRight, y);
			else
				gld = NewState("3o$o$bo!", xPlaceRight + yPlaceDown - y, yPlaceDown);
				
			initGliders.push_back(gld);
		}
		
		initialBlock = NewState("2o$2o!", blockX, blockY);
		
		New();
		
		wssVec.push_back(NewTargetLocator("obo$3bo$3bo$o2bo$b3o!"));
		wssVec.push_back(NewTargetLocator("b2o$b3o$ob2o$3o$bo!"));	
		
		wssVec.push_back(NewTargetLocator("bobo$o$o3bo$o$o2bo$3o!"));
		wssVec.push_back(NewTargetLocator("b2o$3o$3o$2obo$b3o$2bo!"));	

		wssVec.push_back(NewTargetLocator("bobo$4bo$o3bo$o3bo$4bo$bo2bo$2b3o!"));	
		wssVec.push_back(NewTargetLocator("b2o$b3o$b3o$b3o$ob2o$3o$bo!"));	

	}
	
	void EvolveToStabilization() const
	{
		for(int i = 0; i < maxIter; i++)
		{
			Capture(1);
			Run(2);
			if(AreEqual(1))
				break;
		}
	}

	bool EvolveToStabilization(int& wssIdx) const
	{
		wssIdx = -1;
		LifeBox* inner = maxBox;
		LifeBox* outer = finalBox;
		bool found = false;
		
		for(int i = 0; i < maxIter; i++)
		{
			Capture(1);
			Run(2);
			if(AreEqual(1))
				break;
			
			//if(i%3 == 0 || found)
			{
				for(int j = 0; j < wssVec.size(); j++)
				{
					if(ContainsLocator(wssVec[j]) == YES)
					{
						if(!found)
						{
							found = true;
						}
						else
						{	
							wssIdx = j;
							return false;
						}
					}
				}
			}				
			
			if(IsInside(outer) == NO)
				return false;
		}
		
		if(IsInside(inner) == NO)
			return false;
	
		if(GetPop() > 60)
			return false; 
						
		return true;
		
	}

	void GliderRange(int& min, int& max) const
	{
		int j = 0; 
		min = 1000;
		max = -1000;
		
		for(int i = GlobalState->max; i >= GlobalState->min; i--)
		{
			uint64_t curVal = GlobalState->state[i];

			if(curVal == 0)
			{
				j++;
				continue;
			}
				
			int curMax = (31 - __builtin_clzll(curVal)) + j;
			int curMin = (__builtin_ctzll(curVal) - 32) + j;
			
			if(min > curMin)
				min = curMin;
			
			if(max < curMax)
				max = curMax;
				
			j++;
		}
		
		min -= GlobalState->max - 32; 
		max -= GlobalState->max - 32; 
	
		min -= 8;
		max += 5;
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
	int wssIdx;
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
			
			int min;
			int max;
			PutNodeState(params, parentIndeces, dists, i, Hs);
			params.GliderRange(min, max);
			
			if(min + 30 + params.yPlaceDown < 0)
				min = -(30 + params.yPlaceDown);
			
			if(max + 30 + params.yPlaceDown > params.initGliders.size() - 1)
				max = params.initGliders.size() - 31 - params.yPlaceDown;
				
			if(max % 2 != 0)
				max--;
			
			if(min % 2 != 0)
				min++;
		
			Capture(0);
			
			for(int j = min; j <= max; j += 2)
			{
				New();
				PutState(0);
				PutState(params.initGliders[j + 30 + params.yPlaceDown]);
				
				if(params.EvolveToStabilization(wssIdx))
				{
					if(!searchOnly)
					{
						statesTempVec.push_back(GetHash());
							
						tempIndeces.push_back(i);
						temtDists.push_back(j);
					}
				}
				else if (wssIdx >= 0)
				{
					SearchResult* result = new SearchResult();
					result-> wssIdx = wssIdx;
					
					LoadParentVector(parentIndeces, dists, i, result->recipe);
					std::reverse(result->recipe.begin(), result->recipe.end());
					result->recipe.push_back(j);
					
					#pragma omp critical
					{
						results.push_back(result);
						
						if(results.size() % 500 == 0)
							SaveResults(std::string("results_") + std::to_string(results.size()) + std::string(".txt"), results);
					}
				}
			}
			
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
	omp_set_num_threads(5);
	
	std::vector<unsigned long> indeces;
	indeces.push_back(0);
	
	std::vector<signed char> dists;
	dists.push_back(0);
	
	int startDepth = 7;
	LoadSearchTree(std::string("data_depth") + std::to_string(startDepth), indeces, dists);
	
	std::vector<SearchResult*> results; 
	
	SearchParams params(20, 256);
	int prevLayerStartIndex = 0;
	int curLayerStartIndex = 0;
	int max = 11;
	
	for(int i = startDepth; i <= max; i++)
	{
		std::cout << "Depth " << i << std::endl;
		
		curLayerStartIndex = indeces.size();
		SlowSalvoSearch(prevLayerStartIndex, indeces, dists, params, results, i == max);
		prevLayerStartIndex = curLayerStartIndex;
		
		if(results.size() > 0)
		{
			std::string fname = std::string("results_depth") + std::to_string(i) + std::string(".txt");
			SaveResults(fname, results);
			std::cout << " Written " << results.size() << " results to file:" << fname << std::endl; 
		}
		
		if(i >= 4)
		{
			SaveSearchTree(std::string("data_depth") + std::to_string(i), indeces, dists);
		}
		
	}

	getchar();
}