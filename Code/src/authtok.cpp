#include "authtok.h"
namespace SSEtok {
	template<class Archive> void authtok::serialize(Archive & ar, const unsigned int version) {
	    ar & tokens;
		ar & attribute;
		ar & enc_env;
		ar & rand_iv;
	}
	authtok::authtok() = default;
	authtok::authtok(vector<std::vector<uchar> > t, uint32_t a, string e, string r) :
	    tokens(t), attribute(a), enc_env(e), rand_iv(r)
	{}

	void authtok::readclass() {
		for(uint32_t i = 0;i<tokens.size();i++) {
			for(uint32_t j = 0; j < tokens[i].size();j++) {
				printf("%02x",tokens[i][j]);
			}
			cout<<" ";
		}
		cout<<"\n"<<attribute<<"\n";
	}

	string sendstr(authtok A){
	    std::stringstream ss;
	    msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, A);
	    ss.write(sbuf.data(),sbuf.size());
	    std::string message = ss.str();

	    return message;
	}
	authtok receiveclass(string &str){
	    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	    msgpack::object obj = oh.get();
	    authtok A;
	    obj.convert(A);
	    return A;
	}
	vector<std::vector<uchar> > authtok::gettokens() {
		return tokens;
	}
	uint32_t authtok::getattribute() {
		return attribute;
	}
}
