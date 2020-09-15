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
