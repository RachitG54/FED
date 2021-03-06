
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
