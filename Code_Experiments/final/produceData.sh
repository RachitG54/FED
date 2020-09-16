#!/bin/bash

# declare -a dataSizes=("1000" "2000" "5000" "10000" "15000")
declare -a dataSizes=("5000" "10000" "15000" "20000" "25000")
# declare -a dataSizes=("20000" "40000" "60000" "80000" "100000")
# declare -a dataSizes=("2000000")
# declare -a dataSizes=("500" "1000" "1500" "2000" "2500")
declare -a dataSizesSmall=("400" "800" "1200" "1600" "2000")
#declare -a snpSizes=("1000" "1000" "1000" "1000" "1000")
# declare -a snpSizes=("10" "10" "10" "10" "10")
declare -a snpSizes=("50" "50" "50" "50" "50")
# declare -a snpSizes=("200" "200" "200" "200" "200")
declare -a appAreas=("ValueRet" "MAF" "ChiSq" "Hamming")

numdb=5
numAppAreas=3
numq=10
randomOrConstantQ=constant #constant or random

runOnSmallDb=true
if [ "$1" == "Big" ];
then 
	runOnSmallDb=false
else
	runOnSmallDb=true
fi

echo -e "*******Running on small db = "$runOnSmallDb""

appAreasct=0
while [ $appAreasct -lt $numAppAreas ]
do
	curAppArea=${appAreas["$appAreasct"]}
	ct=0
	while [ $ct -lt $numdb ]
	do
		if [ "$runOnSmallDb" = true ];
		then
			curDataSize=${dataSizesSmall["$ct"]}
		else
			curDataSize=${dataSizes["$ct"]}
		fi
		curSnpSize=${snpSizes["$ct"]}
		echo "MaxSampleSize="$curDataSize"" > config.txt
		echo "MaxNumSNPs="$curSnpSize"" >> config.txt
		./generate_data_random config.txt "$curAppArea" ./data/output_"$curDataSize"_"$curSnpSize"_"$curAppArea" 1
		((ct++))
	done
	./generate_query "$numq" config.txt "$curAppArea" "$randomOrConstantQ" > queries_"$curAppArea".txt
	((appAreasct++))
done
