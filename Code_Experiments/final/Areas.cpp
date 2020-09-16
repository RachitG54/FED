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
