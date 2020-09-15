#ifndef SSEenvelope
#define SSEenvelope

#include "SSEincludes.h"

namespace SSEenv {
	class envelope {
	private:
	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive & ar, const unsigned int version);
	public:
		uint32_t attribute = 1;
		vector<uchar> stag;
		vector< vector<uchar> > rhos;
	    envelope();
	    envelope(uint32_t a, vector<uchar> s, vector< vector<uchar> > r);
	    uint32_t getattribute();
	    vector<std::vector<uchar> > getrhos();
	    void readclass();
		MSGPACK_DEFINE(attribute,stag,rhos);
	};

	string sendstr(envelope E);
	envelope receiveclass(string &str);
}

#endif // SSEenvelope


