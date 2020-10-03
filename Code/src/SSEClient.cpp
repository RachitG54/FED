
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
#include "SSEClient.h"
#include <unistd.h>
#include <sstream>
#include <fstream>

SSEClient::SSEClient() {
	group.setDH();
	cout<<"group is setup\n";
}

void SSEClient::request(char* host, char* port1,char* port2) {
	tcpclient authcli;
	tcpclient servcli;

	socket_ptr sock1 = authcli.initialize(host,port1);
	socket_ptr sock2 = servcli.initialize(host,port2);

	vector<string> allLines;
	getAllLinesFromFile(pathQuery, allLines);

	int searchableAttr, numQ;

	vector<string> xx;
	boost::split(xx, allLines[0], boost::is_any_of(" \t"), boost::token_compress_on);

	numQ = atoi(xx[0].c_str());
	searchableAttr = atoi(xx[1].c_str());
	cout<<"Numnber of queries are "<<numQ<<"\n";
	int colnum;

	bool controlvar = true;

	for(int iclient=1; iclient < (int)allLines.size(); iclient++) {
		cout<<"\n****************Moving to next query****************\n"<<endl;
		// cout<<"Sleeping for 2 seconds before next query."<<endl;
		// usleep(2*1000*1000);

		authcli.getAndResetTotalBytesRead();
		authcli.getAndResetTotalBytesWritten();
		authcli.getAndResetTotalTimeSpentInMicroSec();
		servcli.getAndResetTotalBytesRead();
		servcli.getAndResetTotalBytesWritten();
		servcli.getAndResetTotalTimeSpentInMicroSec();

		cout<<"Done with reset of read and written byte and timing vars."<<endl;



		vector<string> curAttr;
		boost::split(curAttr, allLines[iclient], boost::is_any_of(" \t"), boost::token_compress_on);

		colnum = curAttr.size();

		std::vector<string> keywords;

		REP(j,0,searchableAttr-1) {
			keywords.pb(curAttr[j]);
		}


		if(protocol == 2) {
			if(controlvar) {
				iclient--;
				controlvar = false;
				keywords.pb("Case");
			}
			else {
				controlvar = true;
				keywords.pb("Control");
			}
		}

		SSEtime myTtlCtTimer;

		if(DO == 1) {
			cout<<"Calculating OPRF's\n";
			OPRF F;
			vector<string> tmpkw(keywords);
			BIGNUM** rinverses = (BIGNUM**)malloc(sizeof(BIGNUM*)*keywords.size());
			REP(i,0,tmpkw.size()-1) {
				keywords[i].clear();
				cout<<tmpkw[i]<<"::\n";
				F.evaluateP1((const uchar*)tmpkw[i].c_str(),tmpkw[i].size(),&rinverses[i],keywords[i]);
			}

			{
				stringstream ss;
				msgpack::sbuffer sbuf;
			    msgpack::pack(sbuf, keywords);
			    ss.write(sbuf.data(),sbuf.size());
			    string s = ss.str();
			    servcli.writewithsize(sock2,s);
			}
			tmpkw.clear();
			{
				string str; servcli.readstructure(sock2,str);
	            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	            oh.get().convert(tmpkw);
			}


			REP(i,0,tmpkw.size()-1) {
				keywords[i].clear();
				F.fin_evaluateP1(tmpkw[i],rinverses[i],keywords[i]);
			}


		}
		else if(DO==2) {
			cout<<"Calculating EQ Check's\n.";
			MDSSE MDinst;
			vector<string> gamma(keywords.size());
			vector<string> delta(keywords.size());
			int kwsize = MDinst.kwsize;
			REP(i,0,keywords.size()-1) {
				uchar* alpha = (uchar*)malloc(sizeof(uchar)*kwsize);
			    {
			    	int rc = RAND_bytes(alpha, kwsize);
			    	if(rc!=1) { exit(1); }
			    }
			    gamma[i].resize(kwsize,0);
			    delta[i].resize(kwsize,0);

		    	REP(s,0,keywords[i].size()-1) {
		    		gamma[i][s] = keywords[i][s];
		    	}
	    		keywords[i].clear();
		    	REP(s,0,kwsize-1) {
		    		delta[i][s] = alpha[s];
		    		gamma[i][s] = (char)((uchar)delta[i][s] ^ (uchar)gamma[i][s]);
		    	}
		    	free(alpha);
			}
			keywords.clear();

			{
				stringstream ss;
				msgpack::sbuffer sbuf;
			    msgpack::pack(sbuf, gamma);
			    ss.write(sbuf.data(),sbuf.size());
			    string s = ss.str();
			    servcli.writewithsize(sock2,s);
			}

			{
				stringstream ss;
				msgpack::sbuffer sbuf;
			    msgpack::pack(sbuf, delta);
			    ss.write(sbuf.data(),sbuf.size());
			    string s = ss.str();
			    authcli.writewithsize(sock1,s);
			}

			{
				string str; authcli.readstructure(sock1,str);
	            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	            oh.get().convert(keywords);
			}

		}
		SSEtime totquerytime;
		uint32_t wordcount = keywords.size();
		if(wordcount == 0) {
			cout<<"Nothing to Query\n";
			exit(1);
		}

		cout<<"Assuming right now that there are and between keywords and no boolean nots\n";
		SSEquery::cliquery Q(keywords,std::vector<bool>(),std::vector<bool>(),1);
		string message = SSEquery::sendstr(Q);
		authcli.writewithsize(sock1,message);

		{
			myTtlCtTimer.donetime();
			uint64_t seTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t seNetworkTimeAuth = authcli.getTotalTimeSpentInMicroSec();
			uint64_t seNetworkTimeServer = servcli.getTotalTimeSpentInMicroSec();
			uint64_t seActualTime = seTotalTimeTaken - seNetworkTimeAuth - seNetworkTimeServer;
			assert(seActualTime > 0);
			cout<<"[Client SE Time]: Total time = "<<seTotalTimeTaken<<", network auth = "<<seNetworkTimeAuth<<", network serv = "<<seNetworkTimeServer<<endl;
			cout<<"[Client SE Time]: Actual time = "<<seActualTime<<endl;
		}

		cout<<"[Client SE comm]: With authority, Read = "<<authcli.getTotalBytesRead()<<", Written = "<<authcli.getTotalBytesWritten()<<endl;
		cout<<"[Client SE comm]: With server, Read = "<<servcli.getTotalBytesRead()<<", Written = "<<servcli.getTotalBytesWritten()<<endl;

		cout<<"Starting DCFE part\n";

		authcli.getAndResetTotalBytesRead();
		authcli.getAndResetTotalBytesWritten();
		authcli.getAndResetTotalTimeSpentInMicroSec();
		servcli.getAndResetTotalBytesRead();
		servcli.getAndResetTotalBytesWritten();
		servcli.getAndResetTotalTimeSpentInMicroSec();

		myTtlCtTimer.resetTime();
		myTtlCtTimer.starttime();

		if(protocol == 0) {
			int attr = stoi(curAttr[searchableAttr]);
			{
				string attrstr = to_string(attr);
				servcli.writewithsize(sock2,attrstr);
			}

			SSEtime timer;
			string xoredstr; authcli.readstructure(sock1,xoredstr);
			timer.donetime();
		    cout << "[ValueRet:] [Comm:] AuxServer-Client takes ";
			timer.printtime(0);
		    cout << "[ValueRet:] [Comm:] "<<xoredstr.size()<<" bytes from server"<<"\n";

			int sz;
			int noofchars;
			string DatatoC;
			vi positions;
			{
				SSEtime timer;
				string str; servcli.readstructure(sock2,str);
				timer.donetime();
		    cout << "[ValueRet:] [Comm:] Server-Client takes ";
				timer.printtime(0);

		    cout << "[ValueRet:] [Comm:] "<<str.size()<<" bytes from server"<<"\n";
        msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        // now starts streaming deserialization.
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(sz);
        pac.next(oh);
        oh.get().convert(noofchars);
        pac.next(oh);
        oh.get().convert(DatatoC);
        pac.next(oh);
        oh.get().convert(positions);
			}
			Xvalret XData;
			string findata = XData.compclient(xoredstr,DatatoC,sz,noofchars,positions);

			totquerytime.donetime();
			cout<<"[ValueRet:] Query takes ";
			totquerytime.printtime(0);
		}
		else if(protocol == 1) {
			int attr = stoi(curAttr[searchableAttr]);
			{
				string attrstr = to_string(attr);
				servcli.writewithsize(sock2,attrstr);
			}

			IP XDataIP;

			long long delta;
			long long gamma;
			{
		 		string str; servcli.readstructure(sock2,str);
       	cout<<"Recieved "<<str.size()<<" bytes from server, outputting data\n";
        msgpack::unpacker pac;
				pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(gamma);
			}

			{
				SSEtime timer;
				string str; authcli.readstructure(sock1,str);
				timer.donetime();
			    cout << "[MAF:] [Comm:] AuxServer-Client takes ";
			    timer.printtime(0);
			    cout << "[MAF:] [Comm:] "<<str.size()<<" bytes from AuxServer"<<"\n";
		        msgpack::unpacker pac;
		        pac.reserve_buffer(str.size());
		        memcpy(pac.buffer(), str.data(), str.size());
		        pac.buffer_consumed(str.size());
		        msgpack::object_handle oh;
		        pac.next(oh);
		        oh.get().convert(delta);
			}


			long long ans = gamma - delta;
			double precision = 100000.0;

			float answerfinal = ans/precision;
			printf("[Ans]: %.5f\n",answerfinal);
			totquerytime.donetime();
			cout<<"[MAF:] Query takes ";
			totquerytime.printtime(0);
		}
		else if(protocol == 2) {
			int attr = stoi(curAttr[searchableAttr]);
			{
				string attrstr = to_string(attr);
				servcli.writewithsize(sock2,attrstr);
			}

			IP XDataIP;
			if(controlvar) {
				long long share1, share2;
				{
					SSEtime timer;
					string str; authcli.readstructure(sock1,str);
					timer.donetime();
			    cout << "[ChiSq:] [Comm:] AuxServer-Client takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<str.size()<<" bytes from AuxServer"<<"\n";
	        msgpack::unpacker pac;
	        pac.reserve_buffer(str.size());
	        memcpy(pac.buffer(), str.data(), str.size());
	        pac.buffer_consumed(str.size());
	        // now starts streaming deserialization.
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(share2);
				}

				{
					SSEtime timer;
					string str; servcli.readstructure(sock2,str);
					timer.donetime();
			    cout << "[ChiSq:] [Comm:] Server-Client takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<str.size()<<" bytes from Server"<<"\n";
	        msgpack::unpacker pac;
	        pac.reserve_buffer(str.size());
	        memcpy(pac.buffer(), str.data(), str.size());
	        pac.buffer_consumed(str.size());
	        // now starts streaming deserialization.
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(share1);
				}
				long long ans = share1 - share2;
				// cout<<share1<<" "
				double precision = 100.0;

				float answerfinal = ans/precision;
				printf("[Ans]: %.2f\n",answerfinal);

				totquerytime.donetime();
				totquerytime.printtime(0);
			}
		}
		else if(protocol == 3) {
			vector<string> q;
			cout<<colnum<<"\n";
			REP(attr,searchableAttr,colnum-1) {
				q.pb(curAttr[attr]);
			}

			{
		    stringstream ss;
		    msgpack::sbuffer sbuf;
		    msgpack::pack(sbuf, q);
				cout<<"Sending "<<sbuf.size()<<" bytes of query from client to auxserver\n";
				ss.write(sbuf.data(),sbuf.size());
				string message = ss.str();
				authcli.writewithsize(sock1,message);
			}
 		}
		else if(protocol == 4) {

			int attr = stoi(curAttr[searchableAttr]);
			{
				string attrstr = to_string(attr);
				servcli.writewithsize(sock2,attrstr);
			}

			Xaddpaillier XDatapaillier;
			uint32_t y,z;
			{
				string str; servcli.readstructure(sock2,str);
        cout<<"Recieved "<<str.size()<<" bytes from server, outputting data\n";
        msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        // now starts streaming deserialization.
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(y);
			}
			cout<<"y is "<<y<<"\n";
			{
				string str; authcli.readstructure(sock1,str);
        cout<<"Recieved "<<str.size()<<" bytes from authority, outputting data\n";
        msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        // now starts streaming deserialization.
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(z);
			}
			cout<<"z is "<<z<<"\n";
			uint32_t anspail = XDatapaillier.compclient(z,y);
			cout<<"[Ans]: "<<anspail<<"\n";
 		}

		{
			myTtlCtTimer.donetime();
			uint64_t netTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t netNetworkTimeAuth = authcli.getTotalTimeSpentInMicroSec();
			uint64_t netNetworkTimeServer = servcli.getTotalTimeSpentInMicroSec();
			uint64_t netActualTime = netTotalTimeTaken - netNetworkTimeAuth - netNetworkTimeServer;
			assert(netActualTime > 0);
			cout<<"[Client DCFE Time]: Total time = "<<netTotalTimeTaken<<", network auth = "<<netNetworkTimeAuth<<", network serv = "<<netNetworkTimeServer<<endl;
			cout<<"[Client DCFE Time]: Actual time = "<<netActualTime<<endl;
		}

		cout<<"[Client DCFE comm]: With authority, Read = "<<authcli.getTotalBytesRead()<<", Written = "<<authcli.getTotalBytesWritten()<<endl;
		cout<<"[Client DCFE comm]: With server, Read = "<<servcli.getTotalBytesRead()<<", Written = "<<servcli.getTotalBytesWritten()<<endl;

		cout<<"Query is completed\n";
  }
}
SSEClient::~SSEClient() {
}
