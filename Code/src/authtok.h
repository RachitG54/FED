#ifndef SSEauthtok
#define SSEauthtok

#include "SSEincludes.h"
#include "envelope.h"

// We use a class now to support our serialize method.
namespace SSEtok {
	class authtok {
	private:
	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive & ar, const unsigned int version);
	public:
		vector<std::vector<uchar> > tokens;
		uint32_t attribute = 1;
		string enc_env;
		string rand_iv;
	    authtok();
	    authtok(vector<std::vector<uchar> > k, uint32_t a, string e, string r);
	    vector<std::vector<uchar> > gettokens();
	    uint32_t getattribute();
	    void readclass();
		MSGPACK_DEFINE(tokens,attribute,enc_env,rand_iv);
	};

	string sendstr(authtok A);
	authtok receiveclass(string &str);
}

#endif // SSEauthtok
