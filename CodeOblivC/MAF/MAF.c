#include<stdio.h>
#include<obliv.h>

#include"MAF.h"
#include"util.h"


int main(int argc,char *argv[])
{

  if(argc<3) {
    fprintf(stderr,"Usage: %s <1(party)> <port> <--|server address> <number> <totalNum> <messagedim>\n",argv[0]);
    fprintf(stderr,"Usage: %s <2(party)> <port> <--|server address> <number>\n",argv[0]);
    return 1;
  }

  int party = atoi(argv[1]);

  if(party == 1) {   
    if(argc<7) { 
      fprintf(stderr,"Usage: %s <1|2(party)> <port> <--|server address> <number> <totalNum> <messagedim>\n",argv[0]);
      return 1;
    } 
  }
  else if(party==2) {
    if(argc<5) { 
      fprintf(stderr,"Usage: %s <1|2(party)> <port> <--|server address> <number>\n",argv[0]);
      return 1;
    }
  }
  else {
    fprintf(stderr,"Usage: %s <1(party)> <port> <--|server address> <number> <messagedim> <totalNum>\n",argv[0]);
    fprintf(stderr,"Usage: %s <2(party)> <port> <--|server address> <number>\n",argv[0]);
    return 1;
  }

  //Need T.size()-totalnum, N (modulus), so basically need message dim

  ProtocolDesc pd;
  protocolIO io;


  long ret = strtoul(argv[4], NULL, 10);
  io.number = ret;
  long N;
  long totalNum;
  if(party == 1) {
    long messagedim = strtoul(argv[5],NULL,10);
    N = 1 << messagedim;
    totalNum = strtoul(argv[6],NULL,10); 
  }
  else {
    N = 0;
    totalNum = 0;
  }

  io.N = N;
  io.totalNum = totalNum;

  //protocolUseStdio(&pd);
  const char* remote_host = (strcmp(argv[3],"--")==0?NULL:argv[3]);
  //ocTestUtilTcpOrDie(&pd,remote_host,argv[2]);
  if(!remote_host)
  { if(protocolAcceptTcp2PProfiled(&pd,argv[2])!=0)
    { fprintf(stderr,"TCP accept failed\n");
      exit(1);
    }
  }
  else
  {
    if(protocolConnectTcp2PProfiled(&pd,remote_host,argv[2])!=0) 
    { fprintf(stderr,"TCP connect failed\n");
      exit(1);
    }
  }

  setCurrentParty(&pd,party);
  execYaoProtocol(&pd,MAF,&io);
  
  if(party==1) {
    printf("%lld\n",io.output1);
  }
  else if(party==2) {
    printf("%lld\n",io.output2); 
  }
  FILE* fp;

  if(party==1) 
    fp = fopen("./src/EDB/P1Out","w");
  else 
    fp = fopen("./src/EDB/P2Out","w");
  if(party==1) {
    fprintf(fp,"%lld\n",io.output1);
    // printf("Share is %lld\n",io.output1);
  }
  else if(party==2) {
    fprintf(fp,"%lld\n",io.output2);
    // printf("Share is %lld\n",io.output2); 
  }
  // fprintf(io.output, "%lld\n");
  fclose(fp);

  // fprintf(stderr,"Result: %d\n",io.res);
  cleanupProtocol(&pd);
  
  printf("[Final OblivC] : Bytes sent = %d\n", tcp2PBytesSent(&pd));

  return 0;
}
