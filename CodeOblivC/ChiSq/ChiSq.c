#include<stdio.h>
#include<stdlib.h>
#include<obliv.h>
#include <time.h>

#include"ChiSq.h"
#include"util.h"


int main(int argc,char *argv[])
{

  if(argc<3) {
    fprintf(stderr,"Usage: %s <1(party)> <port> <--|server address> <numberCase> <numberControl> <totalNumCase> <totalNumControl> <messagedim>\n",argv[0]);
    fprintf(stderr,"Usage: %s <2(party)> <port> <--|server address> <numberCase> <numberControl>\n",argv[0]);
    return 1;
  }

  int party = atoi(argv[1]);

  if(party == 1) {   
    if(argc<9) { 
      fprintf(stderr,"Usage: %s <1|2(party)> <port> <--|server address> <numberCase> <numberControl> <totalNumCase> <totalNumControl> <messagedim>\n",argv[0]);
      return 1;
    } 
  }
  else if(party==2) {
    if(argc<6) { 
      fprintf(stderr,"Usage: %s <1|2(party)> <port> <--|server address> <numberCase> <numberControl>\n",argv[0]);
      return 1;
    }
  }
  else {
    fprintf(stderr,"Usage: %s <1(party)> <port> <--|server address> <numberCase> <numberControl> <totalNumCase> <totalNumControl> <messagedim>\n",argv[0]);
    fprintf(stderr,"Usage: %s <2(party)> <port> <--|server address> <numberCase> <numberControl>\n",argv[0]);
    return 1;
  }

  //Need T.size()-totalnum, N (modulus), so basically need message dim
  
  srand(time(NULL));

  ProtocolDesc pd;
  protocolIO io;

  long ret = strtoul(argv[4], NULL, 10);
  io.numberCase = ret;
  ret = strtoul(argv[5], NULL, 10);
  io.numberControl = ret;

  long N;
  long totalNumCase,totalNumControl;
  if(party == 1) {
    totalNumCase = strtoul(argv[6],NULL,10);
    totalNumControl = strtoul(argv[7],NULL,10);
    long messagedim = strtoul(argv[8],NULL,10);
    N = 1 << messagedim;
  }
  else {
    N = 0;
    totalNumCase = 0;
    totalNumControl = 0;
  }

  io.N = N;
  io.totalNumCase = totalNumCase;
  io.totalNumControl = totalNumControl;

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
    { 
      // fprintf(stderr,"TCP connect failed\n");
      exit(1);
    }
  }
  
  setCurrentParty(&pd,party);
  execYaoProtocol(&pd,ChiSq,&io);
  // char filename[10];

  // if(party==1) filename = "P1Out";
  // else filename = "P2out";
  // if(party==1) {
  //   printf("%lld\n",io.output1);
  // }
  // else if(party==2) {
  //   printf("%lld\n",io.output2); 
  // }
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

  // if(party==2) {
  //   // printf("%lld\n",io.num);
  //   // printf("%lld\n",io.denom);

  //   double finalans = (io.output*1.0)/precision;
  //   // if(io.denom == 0) {
  //   //   finalans = -1;
  //   // }
  //   // else {
  //   //   finalans = finalans/(io.denom); 
  //   // }
  //   printf("Final answer is %lf\n",finalans);
  // }
  // fprintf(stderr,"Result: %d\n",io.res);
  cleanupProtocol(&pd);

  printf("[Final OblivC] : Bytes sent = %d\n", tcp2PBytesSent(&pd));

  return 0;
}
