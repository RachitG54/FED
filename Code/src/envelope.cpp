#include "envelope.h"
namespace SSEenv {
	template<class Archive> void envelope::serialize(Archive & ar, const unsigned int version) {
		ar & attribute;
		ar & stag;
		ar & rhos;
	}
	envelope::envelope() = default;
	envelope::envelope(uint32_t a, vector<uchar> s, vector< vector<uchar> > r) :
	    attribute(a), stag(s), rhos(r)
	{}

	void envelope::readclass() {
		cout<<attribute<<"\n";
		REP(i,0,stag.size()-1) printf("%02x",stag[i]);
		cout<<"\n";
		for(uint32_t i = 0;i<rhos.size();i++) {
			for(uint32_t j = 0; j < rhos[i].size();j++) {
				printf("%02x",rhos[i][j]);
			}
			cout<<" ";
		}
		cout<<"\n";
	}

	string sendstr(envelope E){
	    std::stringstream ss;
	    msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, E);
	    ss.write(sbuf.data(),sbuf.size());
	    std::string message = ss.str();

	    return message;
	}
	envelope receiveclass(string &str){
	    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	    msgpack::object obj = oh.get();
	    envelope E;
	    obj.convert(E);
	    return E;
	}

	vector<std::vector<uchar> > envelope::getrhos() {
		return rhos;
	}
	uint32_t envelope::getattribute() {
		return attribute;
	}
}
