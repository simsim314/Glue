#include "LifeAPI.h"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include<omp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class RecipeNode 
{
public:
	RecipeNode* parent;
	short dist;
	
	RecipeNode()
	{
		parent = NULL;
	}
	
	RecipeNode(int d)
	{
		parent = NULL;
		dist = d;
	}
	
	RecipeNode* CreateLeaf(short dist)
	{
		RecipeNode *newNode = new RecipeNode(dist);
		newNode->parent = this;
		return newNode;
	}
	
	void LoadParentVector(std::vector<int>& Hs)
	{
		Hs.clear();
		RecipeNode* curNode = this;
		
		while(curNode != NULL)
		{
			Hs.push_back(curNode->dist);
			curNode = curNode->parent;
		}
	}

};

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
		
		for(int i = 0; i < maxIter; i++)
		{
			Capture(1);
			Run(2);
			if(AreEqual(1))
				break;
			
			for(int j = 0; j < wssVec.size(); j++)
			{
				if(ContainsLocator(wssVec[j]) == YES)
				{
					wssIdx = j;
					return false;
				}
			}
			
			if(IsInside(outer) == NO)
				return false;
		}
		
		if(IsInside(inner) == NO)
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

void PutNodeState(const SearchParams& params, RecipeNode* node, std::vector<int>& Hs)
{
	if(node == NULL)
		Hs.clear();
	else
		node->LoadParentVector(Hs);
		
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

void FlushTempDS(std::vector<RecipeNode*>& tempCurLayer, std::vector<uint64_t> &statesTempVec, std::unordered_set<uint64_t>& existingStates, std::vector<RecipeNode*> &curLayer)
{
	for(int j = 0; j < tempCurLayer.size(); j++)
	{
		uint64_t hash = statesTempVec[j];
		RecipeNode* node = tempCurLayer[j];
		
		if(existingStates.find(hash) == existingStates.end())
		{
			existingStates.insert(hash);
			curLayer.push_back(node);
		}
		else
		{
			delete node;
		}
	}
				
	tempCurLayer.clear();
	statesTempVec.clear();
}

void SlowSalvoSearch(const std::vector<RecipeNode*>& prevLayer, std::vector<RecipeNode*> &curLayer, const SearchParams& params, std::vector<SearchResult*>& results)
{
	std::unordered_set<uint64_t> existingStates;
	int total = 0; 
	
	#pragma omp parallel shared(existingStates), shared(prevLayer), shared(curLayer), shared(params), shared(results), shared(total)
	{
	
		std::vector<RecipeNode*> tempCurLayer;
		std::vector<uint64_t> statesTempVec;
		std::vector<int> Hs;
		int wssIdx; 
		
		int reportStep = prevLayer.size() / 10;
		
		#pragma omp for
		for(int i=0; i < prevLayer.size(); ++i)
		{
			#pragma omp atomic
			total++;
			
			if(reportStep > 1 && total % reportStep == 0)
				printf("%d%% of %d\n", 10 * total / reportStep, prevLayer.size());
				
			if(tempCurLayer.size() > 1000)
			{
				#pragma omp critical
				FlushTempDS(tempCurLayer, statesTempVec, existingStates, curLayer);
			}
			
			RecipeNode* curNode = prevLayer[i];
			int min;
			int max;
			PutNodeState(params, curNode, Hs);
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
					statesTempVec.push_back(GetHash());
					tempCurLayer.push_back(curNode->CreateLeaf(j));
				}
				else if (wssIdx >= 0)
				{
					SearchResult* result = new SearchResult();
					result-> wssIdx = wssIdx;
					
					curNode->LoadParentVector(result->recipe);
					std::reverse(result->recipe.begin(), result->recipe.end());
					result->recipe.push_back(j);
					
					#pragma omp critical
					results.push_back(result);
				}
			}
			
		}
		
		#pragma omp critical
		FlushTempDS(tempCurLayer, statesTempVec, existingStates, curLayer);
	}
}

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

int main()
{
	//omp_set_num_threads(1);
	
	std::vector<RecipeNode*> prevLayer;
	prevLayer.push_back(NULL);
	
	std::vector<RecipeNode*> curLayer;
	std::vector<SearchResult*> results; 
	
	SearchParams params(20, 256);
	
	for(int i = 0;; i++)
	{
		std::cout << "Depth " << i + 1 << std::endl;
		
		SlowSalvoSearch(prevLayer, curLayer, params, results);
		
		prevLayer = curLayer;
		curLayer.clear();
		
		
		if(results.size() > 0)
		{
			std::string fname = std::string("results_") + std::to_string(i) + std::string(".txt");
			std::ofstream myfile;
			myfile.open(fname);
			
			for(int j = 0; j < results.size(); j++)
				myfile << ToString(results[j]->recipe) << std::endl;
			
			myfile.close();
			
			std::cout << " Written " << results.size() << " results to file:" << fname << std::endl; 
		}
		
	}

	getchar();
}