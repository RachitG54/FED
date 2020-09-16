#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#define pb push_back
#define mp make_pair
#define ll long long

using namespace std;

enum ApplicationAreas {MAF, ValueRet, ChiSq, Hamming};

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t\n\r")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void getAllLinesFromFile(string fileName, vector<string>& output){
	ifstream inp(fileName.c_str());
	string line;
	while(getline(inp,line)){
		output.pb(trim(line));
	}
}

void parseAndFillConfigParas(string fileName, ll& maxNumPeople, ll& maxNumSnps){
    ifstream inp(fileName.c_str());
    string line;
    while(getline(inp,line)){
        vector<string> xx;
        boost::split(xx, line, boost::is_any_of(" \t="), boost::token_compress_on);
        if (xx[0] == "MaxSampleSize"){
            maxNumPeople = stoll(xx[1]);
        }
        else if (xx[0] == "MaxNumSNPs"){
            maxNumSnps = stoll(xx[1]);
        }
        else{
            cerr<<"Something wrong with config file. Cannot parse."<<endl;
            throw;  
        }
    }
}

ApplicationAreas parseApplicationArea(string application){
    ApplicationAreas xx;
    if (application == "MAF"){
        xx = ApplicationAreas::MAF;
    }
    else if (application == "ValueRet"){
        xx = ApplicationAreas::ValueRet;
    }
    else if (application == "ChiSq"){
        xx = ApplicationAreas::ChiSq;
    }
    else if (application == "Hamming"){
        xx = ApplicationAreas::Hamming;
    }
    else{
        cerr<<"Unrecognized application area. Exiting."<<endl;
        throw;
    }
    return xx;
}