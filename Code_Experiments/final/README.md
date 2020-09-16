```
- Run make
- Use ./generate_data_random to generate random data for MAF, ChiSq, valueret or hamming.
	- Usage : ./generate_data_random <configFileName> <applicationName> <outputFilePrefix> <num of DOs>
	- For single DO, sample usage : 
		- ./generate_data_random config.txt MAF output 1
		- This will produce 1 file with the prefix output. Had the numOfDOs been > output, the files would be named output0, output1 etc.
- Use generate_query to generate random queries.
	- Run as : 
		./generate_query <number of queries to generate> <config file> <applicationName>
	- Sample usage : 
		./generate_query 10 config.txt MAF > queriesMAF.txt
- Once data and queries are generated, use compute_stats.cpp to find the required stats.
	- Usage : 
		./compute_stats <inputDataFile> <queryFile> <appArea>
	- Sample usage : 
		./compute_stats outputMAF.txt queries.txt MAF > ansMAF.txt
		./compute_stats outputValueRet.txt queries.txt ValueRet > ansValueRet.txt
		./compute_stats outputChiSq.txt queries.txt ChiSq > ansChiSq.txt
- If you see a -1 in the answer, that means the denominator was 0 in the calculation and so the value is not defined.
- Edit the config file to change the parameters.
```