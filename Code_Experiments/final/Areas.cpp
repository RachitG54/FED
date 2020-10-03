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
#include<iostream>
#include<string>

using namespace std;
int main(int argc, char* argv[]) {
	if (argc!=2) {
		cerr<<"Enter number of locations.\n";
		exit(1);
	}

	int tot=stoi(string(argv[1]));

	for(int i = 0; i<tot;i++) {
		int x = i%26;
		int y1=i/26;
		int y = y1%26;
		int z = y1/26;
		string s = "AAA";
		s[0] = (char)(s[0]+z);
		s[1] = (char)(s[1]+y);
		s[2] = (char)(s[2]+x);
		cout<<s<<"\n";
	}
	return 0;
}
