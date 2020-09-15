#ifndef SSEcliquery
#define SSEcliquery


#include "SSEincludes.h"

// We use a class now to support our serialize method.
namespace SSEquery {
	class cliquery {
	private:
	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive & ar, const unsigned int version);
	public:
		vector<string> keywords;
		vector<bool> operators;
		vector<bool> notkeywords;
		uint32_t attribute = 1;
	    cliquery();
	    cliquery(vector<string> k,vector<bool> o, vector<bool> n, uint32_t a);

	    void readclass();
	};

	string sendstr(cliquery Q);
	cliquery receiveclass(string &str);
}

#endif // SSEcliquery


