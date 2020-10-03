
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
