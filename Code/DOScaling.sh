#!/bin/bash

NUMDOs=20000
multiDOProtocol="$1"
appArea="ValueRet"
# declare -a dataSizes=("20000" "40000" "60000" "80000" "100000")
# declare -a dataSizes=("400" "100" "200" "300" "50")
declare -a dataSizes=("400" "800" "1200" "1600" "2000")
declare -a snpSizes=("50" "50" "50" "50" "50")
numdb=1

collectStdOutFileName="logs/scaleDOCollect"'_'"$appArea"'_'"$multiDOProtocol"'_'"${NUMDOs}"
mergeStdOutFileName="logs/scaleDOMerge"'_'"$appArea"'_'"$multiDOProtocol"'_'"${NUMDOs}"
if [ "$multiDOProtocol" = "OPRF" ]; then 
	echo -e "Starting OPRF collect phase."
	for ((ii=0;ii<$NUMDOs;ii++)); do
		echo -e "**********Starting ii=$ii**********"
		echo -e "**********Starting ii=$ii**********" >> "$collectStdOutFileName"_Server.outp
		echo -e "**********Starting ii=$ii**********" >> "$collectStdOutFileName"_DO.outp
		./bin/SSE 7 "$appArea" collect >> "$collectStdOutFileName"_Server.outp &
		sleep 0.01
		./bin/SSE 6 "$appArea" ../Code_Experiments/final/data/split/"$ii.txt" "$ii" >> "$collectStdOutFileName"_DO.outp
		wait
	done
	echo -e "Starting merge phase."
	ct=0
	while [ $ct -lt $numdb ]
	do
		curDataSize=${dataSizes["$ct"]}
		echo -e "Starting merge of $curDataSize--->"
		./bin/SSE 7 "$appArea" merge "$curDataSize" > "$mergeStdOutFileName"'_'"$curDataSize"_Server.outp &
		sleep 1
		./bin/SSE 8 "$appArea" merge "$curDataSize" > "$mergeStdOutFileName"'_'"$curDataSize"_AuxServer.outp
		wait
		((ct++))
	done
else
	echo -e "Starting EqCheck Collect phase."
	for ((ii=0;ii<$NUMDOs;ii++)); do
		echo -e "**********Starting ii=$ii**********"
		echo -e "**********Starting ii=$ii**********" >> "$collectStdOutFileName"_DO.outp
		./bin/SSE 10 "$appArea" ../Code_Experiments/final/data/split/"$ii.txt" "$ii" >> "$collectStdOutFileName"_DO.outp
	done
	echo -e "Starting merge phase."
	ct=0
	while [ $ct -lt $numdb ]
	do
		curDataSize=${dataSizes["$ct"]}
		echo -e "Starting merge of $curDataSize--->"
		./bin/SSE 11 "$appArea" merge "$curDataSize" > "$mergeStdOutFileName"'_'"$curDataSize"_Server.outp &
		sleep 1
		./bin/SSE 12 "$appArea" merge "$curDataSize" > "$mergeStdOutFileName"'_'"$curDataSize"_AuxServer.outp
		wait
		((ct++))
	done
fi


