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
#include <sstream>

vector<string> PopulationAreas;
vector<string> PossibleSexes;
ll maxNumPpl = -1;
ll maxNumSNPs = -1;

void FillInPossibleAttributeValues(){
	string possibleAreasFileName = "PossibleAreas.txt";
	string possibleSexesFileName = "PossibleSexes.txt";
	getAllLinesFromFile(possibleAreasFileName, PopulationAreas);
	getAllLinesFromFile(possibleSexesFileName, PossibleSexes);
}

int main(int argc, char** argv){
	ios_base::sync_with_stdio(false);
	int numQ = atoi(argv[1]);
	string configFileName = argv[2];
	string applicationName = argv[3];
	string randomOrConstantQ = argv[4];

	if (randomOrConstantQ != "random" && randomOrConstantQ != "constant"){
		cout<<"The last parameter expected is either random or constant."<<endl;
		exit(-1);
	}

	FillInPossibleAttributeValues();
	int possiblePopulationsLen = PopulationAreas.size();
	int possibleSexesLen = PossibleSexes.size();
	cerr<<"Read in possible attribute values. possiblePopulationsLen = "<<possiblePopulationsLen<<", possibleSexesLen = "<<possibleSexesLen<<endl;

	parseAndFillConfigParas(configFileName, maxNumPpl, maxNumSNPs);
	cerr<<"Found the config parameters : "<<"MaxNumPpl = "<<maxNumPpl<<", MaxNumSNPs = "<<maxNumSNPs<<endl;

	ApplicationAreas appArea = parseApplicationArea(applicationName);
	cerr<<"Found the application area. AppArea = "<<appArea<<endl;

	srand(time(NULL));

	int searchableattr = 2;
	cout<<numQ<<" "<<searchableattr<<"\n";
	if (randomOrConstantQ == "constant"){
		//generate one constant query and use the same one throughout
		stringstream ss;
		ss<<PopulationAreas[rand()%possiblePopulationsLen]<<"\t"<<PossibleSexes[rand()%possibleSexesLen]<<"\t";
		if ((appArea == ApplicationAreas::MAF) || (appArea == ApplicationAreas::ChiSq) || (appArea == ApplicationAreas::ValueRet)){
			ss<<(rand()%maxNumSNPs);
		}
		else if (appArea == ApplicationAreas::Hamming){
			for(int j=0; j<maxNumSNPs; j++){
				int randint = (rand()%3);
				if (randint == 0) ss<<"001";
				else if (randint == 1) ss<<"010";
				else if (randint == 2) ss<<"100";
				else assert(false);
				ss<<"\t";
			}
		}
		else assert(false);
		string line = ss.str();
		for(int ii=0;ii<numQ;ii++){
			cout<<line<<"\n";
		}
	}
	else{
		for(int i=0; i<numQ; i++){
			cout<<PopulationAreas[rand()%possiblePopulationsLen]<<"\t"<<PossibleSexes[rand()%possibleSexesLen]<<"\t";
			if ((appArea == ApplicationAreas::MAF) || (appArea == ApplicationAreas::ChiSq) || (appArea == ApplicationAreas::ValueRet)){
				cout<<(rand()%maxNumSNPs);
			}
			else if (appArea == ApplicationAreas::Hamming){
				for(int j=0; j<maxNumSNPs; j++){
					int randint = (rand()%3);
					if (randint == 0) cout<<"001";
					else if (randint == 1) cout<<"010";
					else if (randint == 2) cout<<"100";
					else assert(false);
					cout<<"\t";
				}
			}
			else assert(false);
			cout<<"\n";
		}
	}
}
