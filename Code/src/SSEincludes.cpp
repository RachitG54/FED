#include "SSEincludes.h"

//These are machine specific configurations - read from a config file and populate (done first thing in main)
string pathjustgarble, pathSSE, pathoblivc, pathGenome, pathQuery;


int protocol = 0;
int DO = 0;
int DO_id = 0;
int networkports = 0;
ll MDDObytes = 0;
ll MDClntbytes = 0;
ll MDSrvrbytes = 0;
ll MDAuxSrvrbytes = 0;

ll MDDObytesSE = 0;
ll MDClntbytesSE = 0;
ll MDSrvrbytesSE = 0;
ll MDAuxSrvrbytesSE = 0;
int compressornot = 0;

void handleErrors() {
  ERR_print_errors_fp(stderr);
  abort();
}
// Read hexadecimally stored keys
uchar* readkeys(uint32_t &keylen, string keyname) {
	uchar* key;
	FILE* fp;
	string fname = "./src/keys/"+keyname;
    fp = fopen(fname.c_str(),"r");
    fscanf(fp, "%u\n",&keylen);
    key = (uchar*)malloc(sizeof(uchar)*keylen);
    REP(i,0,keylen-1) {
    	fscanf(fp, "%02x", &key[i]);
    }
    fclose(fp);
    return key;
}
// Store and read XSet, used in implementation of SSE
void storeset(set<BIGNUM*> s, string name) {
	FILE* fp;
	string fname = "./src/EDB/"+name;
    fp = fopen(fname.c_str(),"w");
    tr(s,it) {
    	BN_print_fp(fp,*it);
    	fprintf(fp, "\n");
    }
    fclose(fp);
}
void storeset2(set<BIGNUM*> s, string name,uint32_t size) {
    FILE* fp;
    string fname = "./src/EDB/"+name;
    fp = fopen(fname.c_str(),"w");
    fprintf(fp, "%d\n", size);
    fprintf(fp, "%d\n", s.size());
    tr(s,it) {
        uchar* str = (uchar*) malloc(sizeof(uchar)*size);
        if(0 == BN_bn2bin(*it, str)) handleErrors();
        REP(i,0,size-1) {
            fprintf(fp, "%c", str[i]);
        }
        free(str);
    }
    fclose(fp);
}

set<string> readset2(string name) {
    FILE* fp;
    string fname = "./src/EDB/"+name;
    fp = fopen(fname.c_str(),"r");
    set<string> XSet;
    uint32_t size;
    fscanf(fp,"%d\n",&size);
    int numberofelements;
    fscanf(fp,"%d\n",&numberofelements);
    REP(i,0,numberofelements-1) {
        string comp = "";comp.resize(size);
        REP(j,0,size-1) {
            char temp;
            fscanf(fp,"%c",&temp);
            comp[j] = temp;
        }
        XSet.insert(comp);
    }
    fclose(fp);
    return XSet;
}

set<string> readset(string name) {
    ifstream fp;
    string fname = "./src/EDB/"+name;
    fp.open(fname,ios::in);
    string temp;
    set<string> XSet;
    while(fp>>temp) {
        BIGNUM* t = BN_new();
        if(0==BN_hex2bn(&t, temp.c_str())) handleErrors();
        uint32_t len = BN_num_bytes(t);
        uchar* str = (uchar*) malloc(sizeof(uchar)*len);
        if(0 == BN_bn2bin(t, str)) handleErrors();
        string comp; comp.resize(len);
        REP(i,0,len-1) comp[i] = str[i];
        XSet.insert(comp);
        free(str);
        BN_free(t);
    }
    fp.close();
    return XSet;
}
//For reading of size included messagepack items
msgpack::object_handle readobject(string name) {
    FILE* fp;
    fp = fopen(name.c_str(),"r");
    int size; fscanf(fp,"%d\n",&size);
    char* buftemp = (char*)malloc(sizeof(char)*size);
    REP(i,0,size-1) fscanf(fp,"%c",&buftemp[i]);
    fclose(fp);
    msgpack::object_handle oh = msgpack::unpack(buftemp, size);
    free(buftemp);
    return oh;
}
// Reading file contents, optimize it here
int filelength(string filename) {
    ifstream inp(filename);
    inp.seekg (0, inp.end);
    return inp.tellg();
}

void get_file_contents(const char *filename, string &contents)
{
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp)
  {
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return;
  }
  throw(errno);
}
// Conversion functions helpful in TSet implementation
uint32_t toInt(uchar* bytes) {
    return (uint32_t)(((unsigned char)bytes[0] << 24) | ((unsigned char)bytes[1] << 16) |((unsigned char)bytes[2] << 8)|(unsigned char)bytes[3]);
}

uchar* tochar(uint32_t value, uint32_t len) {
    uchar* buffer = (uchar*)malloc(sizeof(uchar)*len);
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;
    REP(i,4,len-1) buffer[i] = 0;
    return buffer;
}

uint64_t toBigInt(uchar* bytes, uint32_t len) {
    uint64_t no = 0;
    REP(i,0,len-1) {
        ll temp = bytes[len-1-i];
        temp = temp << (8*i);
        no = no | temp;
    }
    return no;
}
uint64_t toBigInt(vector<uchar> bytes, uint32_t len) {
    uint64_t no = 0;
    REP(i,0,len-1) {
        no = no | (bytes[len-1-i] << (8*i));
    }
    return no;
}
// Compression and decompression functions, changes yet to be tested
int compress_vector(string &message) {
    if(compressornot == 0) {
        return F_OK;
    }

    string uncompsize = std::to_string(message.size());
    uncompsize.resize(constlen,0);

    unsigned long source_length = message.size();
    uLongf destination_length = compressBound(source_length);

    char *destination_data = (char *) malloc(destination_length);
    if (destination_data == nullptr) {
        return Z_MEM_ERROR;
    }

    Bytef *source_data = (Bytef *) message.data();
    int return_value = compress2((Bytef *) destination_data, &destination_length, source_data, source_length, Z_BEST_COMPRESSION);

    message.assign(destination_data,destination_length);
    message.resize(destination_length+constlen);

    REP(i,0,constlen-1) message[destination_length+i] = uncompsize[i];
    free(destination_data);
    return return_value;
}

int decompress_vector(string &message) {
    if(compressornot == 0) {
        return F_OK;
    }

    ll size;
    stringstream ss;
    const char* sdata = message.data();
    ss.write((sdata+message.size()-constlen), constlen);
    ss >> size;

    uLongf destination_length = (uLongf)size;

    char *destination_data = (char *) malloc(destination_length);
    if (destination_data == nullptr) {
        return Z_MEM_ERROR;
    }

    Bytef *source_data = (Bytef *) (sdata);
    uint32_t sourcesize = message.size()-constlen;
    int return_value = uncompress((Bytef *) destination_data, &destination_length, source_data, sourcesize);

    message.assign(destination_data,destination_length);

    free(destination_data);
    return return_value;
}

uchar** readlabels(int &n,int &labelsize) {
    FILE*fp;
    string path = "labels";
    fp = fopen(path.c_str(),"r");
    uchar **labels = (uchar**)malloc(sizeof(uchar*)*(2*n));
    for(int i = 0; i < 2*n; i++) {
        labels[i] = (uchar*)malloc(sizeof(uchar)*labelsize);
        for(int j = 0; j < labelsize;j++) {
            char tempchar;
            fscanf(fp, "%c", &tempchar);
            labels[i][j] = (uchar)tempchar;
        }
    }
    fclose(fp);
    return labels;
}

//Parsing of file for experiments and user reading
string Usertrim(const std::string& str, const std::string& whitespace = " \t\n\r") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}
void getAllLinesFromFile(string fileName, vector<string>& output){
    ifstream inp(fileName.c_str());
    string line;
    while(getline(inp,line)){
        output.pb(Usertrim(line));
    }
}
