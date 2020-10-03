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
#include <fstream>

ll MaxNumPpl = 500;
ll MaxNumSNPs = 10;
int ExtraAttributes = 2;

vector<string> PopulationAreas;
vector<string> PossibleSexes;

void FillInPossibleAttributeValues(){
	string possibleAreasFileName = "PossibleAreas.txt";
	string possibleSexesFileName = "PossibleSexes.txt";
	getAllLinesFromFile(possibleAreasFileName, PopulationAreas);
	getAllLinesFromFile(possibleSexesFileName, PossibleSexes);
}

void outputComputableAttr(ll& numOfSNPs, ofstream& myfile, ApplicationAreas appArea){
	for(int j=0;j<numOfSNPs;j++){
		if ((appArea == ApplicationAreas::MAF) || (appArea == ApplicationAreas::ChiSq)){
			myfile<<(rand()%3)<<"\t";
		}
		else if (appArea == ApplicationAreas::ValueRet){
			myfile<<(rand()%4)<<"\t";
		}
		else if (appArea == ApplicationAreas::Hamming){
			//For hamming
			int x = rand()%3;
			if (x==0) myfile<<"001";
			else if (x==1) myfile<<"010";
			else if (x==2) myfile<<"100";
			else assert(false);
			myfile<<"\t";
		}
		else{
			cerr<<"Unrecognized app area."<<endl;
			throw;
		}
	}
}

int main(int argc, char** argv){
	ios_base::sync_with_stdio(false);

	string configFileName = argv[1];
	string applicationName = argv[2];
	string outputFilePrefix = argv[3];
	int numOfDOs = atoi(argv[4]);
	if (numOfDOs < 1){
		cerr<<"Got wrong DOs."<<endl;
		exit(1);
	}

	FillInPossibleAttributeValues();
	parseAndFillConfigParas(configFileName, MaxNumPpl, MaxNumSNPs);
	cerr<<"Found the config parameters : "<<"MaxNumPpl = "<<MaxNumPpl<<", MaxNumSNPs = "<<MaxNumSNPs<<endl;

	ApplicationAreas appArea = parseApplicationArea(applicationName);
	cerr<<"Found the application area. AppArea = "<<appArea<<endl;

	if (appArea == ApplicationAreas::ChiSq){
		ExtraAttributes = 3; //One extra for case-control
	}

	ll numOfSNPs = MaxNumSNPs;
	ll numCols = numOfSNPs + ExtraAttributes;
	ll allComb = PopulationAreas.size()*PossibleSexes.size();
	if (appArea == ApplicationAreas::ChiSq){
		allComb *= 2;
	}
	ll numRows = MaxNumPpl + allComb;

	srand(time(NULL));
	int possiblePopulationsLen = PopulationAreas.size();
	int possibleSexesLen = PossibleSexes.size();

	/*
	* (numRows * numCols) is the (row size * column size) = (num of ppl * num of attr)
	* Attributes are : Population, Sex, SNP data
	*/

	// cout<<numRows<<" "<<numCols<<" "<<ExtraAttributes<<"\n";

	for(int donum=0;donum<numOfDOs;donum++){
		string outfilename = outputFilePrefix + to_string(donum) + ".txt";
		ofstream myfile;
		myfile.open(outfilename);

		myfile<<numRows<<" "<<numCols<<" "<<ExtraAttributes<<"\n";

		for(int i=0;i<PopulationAreas.size();i++){
			for(int j=0;j<PossibleSexes.size();j++){
				if (appArea == ApplicationAreas::ChiSq){
					for(int k=0;k<2;k++){
						myfile<<PopulationAreas[i]<<"\t"<<PossibleSexes[j]<<"\t"<<(k&1 ? "Case" : "Control")<<"\t";
						outputComputableAttr(numOfSNPs, myfile, appArea);
						myfile<<"\n";
					}
				}
				else{
					myfile<<PopulationAreas[i]<<"\t"<<PossibleSexes[j]<<"\t";
					outputComputableAttr(numOfSNPs, myfile, appArea);
					myfile<<"\n";
				}
			}
		}

		for(int i=0;i<numRows-allComb;i++){
			//NOTE : If adding a new attribute, also update the const variable ExtraAttributes defined at the top.

			//Add populationAreas and sex as filtering attributes
			myfile<<PopulationAreas[rand()%possiblePopulationsLen]<<"\t"<<PossibleSexes[rand()%possibleSexesLen]<<"\t";
			if (appArea == ApplicationAreas::ChiSq){
				//Add case/control as a boolean attribute
				int num = rand()%2;
				if(num==1) myfile<<"Case\t";
				else myfile<<"Control\t";
			}

			//Now add other computable attributes for SNPs
			outputComputableAttr(numOfSNPs, myfile, appArea);
			myfile<<"\n";
		}
		myfile.close();
	}
}
