
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
#include "cliquery.h"
namespace SSEquery {
	template<class Archive> void cliquery::serialize(Archive & ar, const unsigned int version) {
	    ar & keywords;
		ar & operators;
		ar & notkeywords;
		ar & attribute;
	}
	cliquery::cliquery() = default;
	cliquery::cliquery(vector<string> k,vector<bool> o, vector<bool> n, uint32_t a) :
	    keywords(k), operators(o), notkeywords(n), attribute(a)
	{}

	void cliquery::readclass() {
		cout<<"( ";
		for(uint32_t i = 0;i<keywords.size();i++) {
			cout<<keywords[i]<<" ";
			//if(notkeywords[i]) cout<<"NOT ";
			if(i!=keywords.size()-1) {cout<< "AND "; /*if(operators[i]) cout<<" AND "; else cout<<" OR ";*/}
		}
		cout<<")";
		cout<<"\n"<<attribute<<"\n";
	}
	string sendstr(cliquery Q){
	    std::stringstream ss;
	    boost::archive::text_oarchive oa(ss);
	    oa << Q;
	    std::string message = ss.str();
	    cout << "Sending "<<message.size()<<" bytes of query"<<"\n";
	    return message;
	}
	cliquery receiveclass(string &str){
	    cout << "Receiving "<<str.size()<<" bytes of query"<<"\n";
	    stringstream ss;
	    ss<<str;
	    boost::archive::text_iarchive ia{ss};
	    cliquery Q;
	    ia >> Q;
	    return Q;
	}
}
