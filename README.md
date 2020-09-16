This is the implementation for -  A Practical Model for Collaborative Databases: Securely Mixing, Searching and Computing: publication https://eprint.iacr.org/2019/1262 at ESORICS 2020.

We have four codebases in folders Code, Codegarble, CodeOblivC, Code_Experiments.

Code contains the code base with implementations of FED by modifying the implementation of MC-OXT by Jarecki et al and our CED protocols.

Codegarble contains the code base with justgarbled. This is forked from https://github.com/irdan/justGarble.

CodeOblivC contains the code base with OblivC. You need to download OblivC separately to run this.

Code_Experiments contains the codes for generating the genomic datasets that we used for our evaluation.

Also included with the source code is the additive paillier library found here: http://acsc.cs.utexas.edu/libpaillier/

Pre-requisite libraries
Boost, msgpack, openssl, oblivc, gmp library.

Please see a brief tutorial on installing everything in InstallTutorial.md.

To compile the code, after installing the pre-requisite libraries do the following steps.
1) Ensure that the path to paths.config are set correctly.
2) Run make.

This implicitly runs:
a) make on the Code/src folder. This generates a binary in Code/bin called SSE.
b) make on Codegrable.
c) make on CodeOblivC.

Test with appropriate scripts AutoEverything. They are a complete automation of the entire codebase and experimentation. To test manually, following is the README for running Code.

###########################################################################

<protocol = MAF/Chisq/ValueRet/Hamming/AddHom>

For Data Owners, run: ./bin/SSE 0 <protocol> <path_to_genomic data>

For Servers, run: ./bin/SSE 1 <protocol>
For AuxServers, run: ./bin/SSE 2 <protocol>
For Clients, run: ./bin/SSE 3 <protocol> <path_to_query_file>

Note that to setup a proper channel, the server must be run, then AuxServer and followed by client.

For Multiple Data Owner protocol:

OPRF

Collect Stage (Collecting OPRF keywords)
For Server, run: ./bin/SSE 7 <protocol> <collect>
For DO with id i, run: ./bin/SSE 6 <protocol> <genomePath> <id>

Merge Stage (Combining data)
For Server, run: ./bin/SSE 7 <protocol> <merge> <totalnoofDOs>
For AuxServer, run: ./bin/SSE 8 <protocol> <merge> <totalnoofDOs>

Equality Checks
For DO with id i, run: ./bin/SSE 10 <protocol> <genomePath> <id>

Merge Stage (Combining data)
For Server, run: ./bin/SSE 11 <protocol> <merge> <totalnoofDOs>
For AuxServer, run: ./bin/SSE 12 <protocol> <merge> <totalnoofDOs>


Note the order for queries
For running the query phase, add <1/2> at the end of the call for singleDO
1 for OPRF
2 for Equality Checks

For Data Owners, run: ./bin/SSE 0 <protocol> <path_to_genomic data> <1/2>

For Servers, run: ./bin/SSE 1 <protocol> <1/2>
For AuxServers, run: ./bin/SSE 2 <protocol> <1/2>
For Clients, run: ./bin/SSE 3 <protocol> <path_to_query_file> <1/2>

You can also look at scripts testAutomate for testing single data-owner protocols and multiDOtestautomate for multiple-data owner versions.

- Run the protocol
	- There are 3 script files for running protocols under the code folder:
		- testautomate.sh : for singleDO
		- multiDOtestautomate.sh : for multiDO OPRF
		- multiDOEqChecksTestAutomate.sh : for multiDO Eq checks

		- Set the same parameters as set in produceData.sh in these files and run in the same manner :
			- ./testautomate.sh Big : will create log files in log, errorlog, relevantlogs
			- ./multiDOtestautomate.sh Big : will create log files in mDOlog, mDOerrorlog, mDOrelevantLogs
			- ./multiDOEqChecksTestAutomate.sh Big : will create log files in mDOlog, mDOerrorlog, mDOrelevantLogs
		- After runnign any scripts copy the required log folders in a separate place so that future make cleanfull don't erase the same.

The Code folder implicitly calls the executables in Codegarble and CodeOblivC.

###########################################################################
For creating the data for experiments in Code_Experiments folder.

For running scripts to generate raw data for experiments:
- Produce the dataset on which the protocols will run
	- Use produceData.sh under securedb/src/Code_Experiments/final folder to generate the data
	- The data is generated in a folder named data in that folder.
	- Run mkdir -p data, to create the folder.
	- The script can be run in 2 modes - ./produceData.sh Big or ./produceData.sh Small  [NOTE : The big and small keywords are case-sensitive]
	- When run with Big parameter the filesizes under the variable name dataSizes are used -- when run with Small the fileSizes under variable name dataSizesSmall are used.
	- Set the dataSizes and dataSizesSmall to apt values as required.
	- Set the snpSizes and appAreas also as required
	- There are 2 more paras in the file - numdb and numAppAreas : If the numdb = 1, only the first db size is run. Same goes for numAppAreas.
	- After setting the apt paras in the file produceData.sh, run the same as:
		./produceData.sh Small or ./produceData.sh Big
