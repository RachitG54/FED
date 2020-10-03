/* ====================================================================
* Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
* Copyright (C) 2019  Rachit Garg Nishant Kumar

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.

* ====================================================================
*
*/
#include "headerUtils.h"

ApplicationAreas appArea;

int rowNumInputDb = -1, columnNumInputDb = -1, searchableAttr = -1;
vector<string> populationAreas;
vector<string> gender;
vector<int> caseControl;
vector< vector<int> > snpAttr;
vector< vector<string> > snpAttrHamming;

int ExtraAttributes = 2;

void parseInputDb(string inputDbFile){
	ifstream inp(inputDbFile.c_str());
	string line;
	getline(inp, line);
	vector<string> xx;
	boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
	assert(xx.size() == 3);
	rowNumInputDb = atoi(xx[0].c_str());
	columnNumInputDb = atoi(xx[1].c_str());
	searchableAttr = atoi(xx[2].c_str());

	for(int row = 0; row < rowNumInputDb; row++){
		xx.clear();
		getline(inp, line);
		boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
		int idx = 0;
		populationAreas.push_back(xx[idx++]);
		gender.push_back(xx[idx++]);
		if (appArea == ApplicationAreas::ChiSq){
			int data;
			if(xx[idx] == "Case") data = 1; else if(xx[idx] == "Control") data = 0; else {cerr<<("Error in case/control setting\n");exit(1);}
			idx++;
			caseControl.push_back(data);
		}
		if (appArea == ApplicationAreas::Hamming){
			vector<string> temp;
			for(int col = 0; col < columnNumInputDb - idx; col++){
				temp.push_back(xx[col + idx]);
			}
			snpAttrHamming.push_back(temp);
		}
		else{
			vector<int> temp;
			for(int col = 0; col < columnNumInputDb - idx; col++){
				temp.push_back(atoi(xx[col + idx].c_str()));
			}
			snpAttr.push_back(temp);
		}
	}
}

void findFilteredIndices(const vector<string>& query, vector<int>& filteredIndices){
	// assert(query.size() == 3);
	string givenPopulationArea = query[0];
	string givenSex = query[1];
	for(int i=0;i<rowNumInputDb;i++){
		if ((populationAreas[i] == givenPopulationArea) && (gender[i] == givenSex)){
			filteredIndices.pb(i);
		}
	}
}

void processMAFQuery(const vector<string>& query, const vector<int>& filteredIndices){
	int computableSnp = atoi(query[ExtraAttributes].c_str());
	int alleleRunningSum = 0;
	for(int ii=0, sz=filteredIndices.size(); ii<sz; ii++){
		int curIdx = filteredIndices[ii];
		alleleRunningSum += snpAttr[curIdx][computableSnp];
	}

	int totalNum = filteredIndices.size();
	if (totalNum == 0){
		cout<<-1<<endl;
	}
	// else if(1) {
	// 	cout<<alleleRunningSum<<"\n";
	// }
	else{

		float answerfinal = (min(alleleRunningSum,2*totalNum - alleleRunningSum))/(1.0*(2*totalNum));
		printf("[Ans]: %.5f\n",answerfinal);
		// cout<<"[Ans]: "<<(min(alleleRunningSum,2*totalNum - alleleRunningSum))/(1.0*(2*totalNum))<<endl;
	}
}

void processValueRetQuery(const vector<string>& query, const vector<int>& filteredIndices){
	int thisqsize = query.size() - ExtraAttributes;
	for(int jj=0;jj<thisqsize;jj++){
		int computableSnp = atoi(query[ExtraAttributes + jj].c_str());
		for(int ii=0, sz=filteredIndices.size(); ii<sz; ii++){
			int curIdx = filteredIndices[ii];
			cout<<"[Ans]: i: "<<ii<<", Position: "<<computableSnp<<", data "<<snpAttr[curIdx][computableSnp]<<"\n";
		}
	}

}

void processChiSqQuery(const vector<string>& query, const vector<int>& filteredIndices){
	int computableSnp = atoi(query[ExtraAttributes].c_str());
	int caseControlGroupCount[] = {0, 0};
	int caseControlGroupAlleleRunningSum[] = {0, 0};
	for(int ii=0, sz=filteredIndices.size(); ii<sz; ii++){
		int curIdx = filteredIndices[ii];
		// cerr<<caseControl[curIdx]<<" "<<curIdx<<"\n";
		caseControlGroupCount[caseControl[curIdx]]++;
		caseControlGroupAlleleRunningSum[caseControl[curIdx]] += snpAttr[curIdx][computableSnp];
		// cerr<<snpAttr[curIdx][computableSnp]<<"::";
	}
	int sumAlleleCC = caseControlGroupAlleleRunningSum[0] + caseControlGroupAlleleRunningSum[1];
	int sumNumCC = caseControlGroupCount[0] + caseControlGroupCount[1];
	long long val1 = caseControlGroupAlleleRunningSum[0]*caseControlGroupCount[1];
	long long val2 = caseControlGroupAlleleRunningSum[1]*caseControlGroupCount[0];
	long long weightedDiffCC = val1 - val2;

	// cerr << caseControlGroupCount[0] <<" "<<caseControlGroupCount[1] << " "<<sumAlleleCC << " "<< 2*sumNumCC - sumAlleleCC<<"\n";
	if ((caseControlGroupCount[0] == 0) || (caseControlGroupCount[1] == 0) || (sumAlleleCC == 0) || (((2*sumNumCC) - sumAlleleCC) == 0)) {
		cout<<-1<<endl;
		// cout<<-1<<" ";
		// if (caseControlGroupCount[0] == 0) cout<<"caseControlGroupCount[0]";
		// else if (caseControlGroupCount[1] == 0) cout<<"caseControlGroupCount[1]";
		// else if (sumAlleleCC == 0) cout<<"sumAlleleCC";
		// else cout<<"2*sumNumCC - sumAlleleCC";
		// cout<<endl;
	}
	else{
		long long num = 1;
		num = num*2*sumNumCC*weightedDiffCC*weightedDiffCC;
		long long denom = 1;
		denom = denom * (caseControlGroupCount[0]*caseControlGroupCount[1])*(sumAlleleCC*((2*sumNumCC) - sumAlleleCC));
		// printf("val1 %lld, val2 %lld, sumAlleleCC %lld, sumNumCC %lld, weightedDiffCC %lld\n",val1, val2, sumAlleleCC,sumNumCC,weightedDiffCC);
		cout<<"NUM: "<< num<< " DENOM: "<< denom<<"\n";
		double chiSq = num/(1.0*denom);
		float answerfinal = chiSq;

		printf("[Ans]: %.2f\n",answerfinal);
		// cout<<"[Ans]: "<<chiSq<<endl;
	}
}

inline int chardiff(char x1, char x2){
	assert(x1 == '0' || x1 == '1');
	assert(x2 == '0' || x2 == '1');
	return abs(x1-x2);
}

int hammingDist(string s1, string s2){
	assert(s1.length() == 3);
	assert(s2.length() == 3);
	int totalDiff = 0;
	for(int i=0;i<3;i++){
		char x1 = s1[i];
		char x2 = s2[i];
		totalDiff += chardiff(x1,x2);
	}
	return totalDiff;
}

void processHammingQuery(const vector<string>& query, const vector<int>& filteredIndices){
	int numSNPs = columnNumInputDb - ExtraAttributes;
	int minHamming = 1e9;
	// int minHammingIdx = -1;

	// for(int j=0;j<numSNPs;j++){
	// 	cout<<query[extraAttr+j]<<"::";
	// }
	// cout<<"\n";

	int completeham = 0;
	for(int ii=0,sz=filteredIndices.size();ii<sz;ii++){
		int i = filteredIndices[ii];
		int totalHammingDist = 0;

		for(int j=0;j<numSNPs;j++){
			string s1 = snpAttrHamming[i][j];
			string s2 = query[ExtraAttributes + j];
			// if(s1 == "001") cout<<2<<"::";
			// else if(s1=="010") cout<<1<<"::";
			// else cout<<0<<"::";
			// cout<<s1<<"::";
			totalHammingDist += hammingDist(s1,s2);
		}
		// cout<<"\n";
		// cerr<<"Found the hamming distance between "<<i<<"-th row of input and the given query."<< "Hamming distance = "<<totalHammingDist<<endl;

		completeham += totalHammingDist;
		if (totalHammingDist < minHamming){
			minHamming = totalHammingDist;
			// minHammingIdx = i;
		}
	}
	cout<<"[Ans]: "<<completeham<<"\n";
	cout<<"Found the minimum hamming distance as "<<minHamming<<"\n\n";
}

void processQuery(const vector<string>& query){
	vector<int> filteredIndices;
	findFilteredIndices(query, filteredIndices);
	switch(appArea){
		case MAF:
			processMAFQuery(query, filteredIndices);
			break;
		case ValueRet:
			processValueRetQuery(query, filteredIndices);
			break;
		case ChiSq:
			processChiSqQuery(query, filteredIndices);
			break;
		case Hamming:
			processHammingQuery(query, filteredIndices);
			break;
		default:
			cerr<<"Unrecognized app area. Exit."<<endl;
			throw;
	}
}

void processQueryFile(string fileName){
	vector<string> allLines;
	getAllLinesFromFile(fileName, allLines);

	vector<string> xx;
	boost::split(xx, allLines[0], boost::is_any_of(" \t"), boost::token_compress_on);
	// int numQ = atoi(xx[0].c_str());
	searchableAttr = atoi(xx[1].c_str());

	for(int i=1; i < (int)allLines.size(); i++){
		vector<string> curAttr;
		boost::split(curAttr, allLines[i], boost::is_any_of(" \t"), boost::token_compress_on);
		// cout<<allLines[i]<<"\n";
		processQuery(curAttr);
	}
}

int main(int argc, char** argv){
	ios_base::sync_with_stdio(false);
	string inputDb = argv[1];
	string queryFile = argv[2];
	string appAreaStr = argv[3];
	appArea = parseApplicationArea(appAreaStr);
	cerr<<"Set the appArea done. AppArea = "<<appArea<<endl;

	parseInputDb(inputDb);
	cerr<<"Parsed input db. RowNumFound = "<<rowNumInputDb<<", columnNumInputDb = "<<columnNumInputDb<<", populationAreas.size = "<<
			populationAreas.size()<<", gender.size() = "<<gender.size()<<", caseControl.size() = "<<caseControl.size()<<", snpAttr.size() = "<<snpAttr.size()<<endl;

	processQueryFile(queryFile);
}
