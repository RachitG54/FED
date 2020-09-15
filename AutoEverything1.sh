#!/bin/bash

# declare -a dataSizes=("5000" "10000" "15000" "20000" "25000")
declare -a dataSizes=("20000" "40000" "60000" "80000" "100000")
# declare -a dataSizes=("10000000")
declare -a dataSizesSmall=("400" "800" "1200" "1600" "2000")
declare -a snpSizes=("50" "50" "50" "50" "50")
declare -a appAreas=("ValueRet" "MAF" "ChiSq" "Hamming")

locfilter=100
numdb=5
numAppAreas=1
numq=10
randomOrConstantQ=constant #constant or random

gendata=true
remlogs=false
runSetup=true
runSingleDO=true
runOPRF=true
runEQCheck=true
checkcorrect=true

Totalwd=$(pwd)
Codewd=$Totalwd/Code
CodeExpwd=$Totalwd/Code_Experiments/final

rundir=$2
tcpport=$3

if [ "$#" -lt 3 ]; then
    echo "Illegal number of parameters"
    exit
fi

echo "Creating Dummy directories for experiments"
mkdir -p $rundir/src/keys $rundir/src/DB $rundir/src/EDB $rundir/data/split $rundir/ideallogs $rundir/reallogs
cd $rundir
cd reallogs
mkdir -p SingleDO MultiDO
cd SingleDO
mkdir -p correctnesslog errorlog log relevantlog 
cd ../MultiDO
mkdir -p OPRF EqCheck
cd OPRF
mkdir -p correctnesslog errorlog log relevantlog 
cd ../EqCheck
mkdir -p correctnesslog errorlog log relevantlog 
cd $rundir

runOnSmallDb=true
if [ "$1" == "Big" ];
then 
	runOnSmallDb=false
else
	runOnSmallDb=true
fi

echo -e "*******Simulating Network*******"

# $Totalwd/simulateWAN.sh

# cd Code_Experiments/final
echo -e "*******Setting config parameter in new directory*******"
$CodeExpwd/Areas $locfilter > PossibleAreas.txt
cp $CodeExpwd/PossibleSexes.txt PossibleSexes.txt
cp $Codewd/paths.config $rundir
cp $Codewd/src/keys/* $rundir/src/keys

echo -e "*******Starting experiments now*******"

appAreasct=0

while [ $appAreasct -lt $numAppAreas ] && $gendata
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
		$CodeExpwd/generate_data_random config.txt "$curAppArea" ./data/output_"$curDataSize"_"$curSnpSize"_"$curAppArea" 1
		((ct++))
	done

	$CodeExpwd/generate_query "$numq" config.txt "$curAppArea" "$randomOrConstantQ" > queries_"$curAppArea".txt

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
		
		# echo -e "./compute_stats ./data/output_"$curDataSize"_"$curSnpSize"_"$curAppArea"0.txt queries_"$curAppArea".txt "$curAppArea"" 
		$CodeExpwd/compute_stats ./data/output_"$curDataSize"_"$curSnpSize"_"$curAppArea"0.txt queries_"$curAppArea".txt "$curAppArea" | grep "\[Ans\]: " > ideallogs/output_"$curDataSize"_"$curSnpSize"_"$curAppArea"0
                ((ct++))
        done	

	((appAreasct++))
done

if $remlogs;
then
	rm ideallogs/*
fi

appAreasct=0

while [ $appAreasct -lt $numAppAreas ] && $runSingleDO
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
		echo -e "\n************STARTING************\n"
		date

		echo -e "CurAppArea, CurDataSize, CurSnpSize = "$curAppArea" "$curDataSize" "$curSnpSize""
		
		fileSuffix="$curDataSize"_"$curSnpSize"_"$curAppArea"0
		logFilePrefix="reallogs/SingleDO/log/log_"$fileSuffix""
		relevantLogFilePrefix="reallogs/SingleDO/relevantlog/log_"$fileSuffix""
		errorLogFilePrefix="reallogs/SingleDO/errorlog/log_"$fileSuffix""
		correctLogFilePrefix="reallogs/SingleDO/correctnesslog/log_"$fileSuffix""
		
		if $runSetup;
		then
			echo -e "Starting Setup"
			if [ "$curAppArea" == "AddHom" ];
			then
				$Codewd/bin/SSE $tcpport 0 "$curAppArea" ./data/output_"$curDataSize"_"$curSnpSize"_MAF0.txt >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup
			else
				$Codewd/bin/SSE $tcpport 0 "$curAppArea" ./data/output_"$fileSuffix".txt >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup
			fi
			cat "$logFilePrefix"_setup | grep -E '^(\[Final SE\]|\[Final IP\]|\[Final ValueRet\]|\[Final Hamm\])' > "$relevantLogFilePrefix"_setup
		fi

		date

		echo -e "Starting other entities"
		$Codewd/bin/SSE $tcpport 1 "$curAppArea" >"$logFilePrefix"_Server 2>"$errorLogFilePrefix"_Server &
		sleep 3
		$Codewd/bin/SSE $tcpport 2 "$curAppArea" >"$logFilePrefix"_AuxServer 2>"$errorLogFilePrefix"_AuxServer &
		sleep 3
		if [ "$curAppArea" == "AddHom" ];
		then
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_MAF.txt >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		else
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_"$curAppArea".txt >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		fi

		wait

		echo -e "All entities done"
		cat "$logFilePrefix"_Server | grep -E '(\[Server SE|\[Server DCFE|\[Final OblivC|\[Final Hamm|\[Final Data)' > "$relevantLogFilePrefix"_Server
		cat "$logFilePrefix"_AuxServer | grep -E '(\[Authority SE|\[Authority DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_AuxServer
		cat "$logFilePrefix"_Client | grep -E '(\[Client SE|\[Client DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_Client
		if [ "$curAppArea" == "Hamming" ];
                then
			logtoread="Server"
                else
			logtoread="Client"
                fi

		cat "$logFilePrefix"_"$logtoread" | grep "\[Ans\]: " > "$correctLogFilePrefix"

		echo -e "\n##############DONE##############\n"
		((ct++))
	done
	((appAreasct++))
done

appAreasct=0

while [ $appAreasct -lt $numAppAreas ] && $runOPRF 
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
		echo -e "\n************STARTING************\n"
		date

		echo -e "CurAppArea, CurDataSize, CurSnpSize = "$curAppArea" "$curDataSize" "$curSnpSize""

		fileSuffix="$curDataSize"_"$curSnpSize"_"$curAppArea"0
		logFilePrefix="reallogs/MultiDO/OPRF/log/mDOOPRF_log_"$fileSuffix""
		relevantLogFilePrefix="reallogs/MultiDO/OPRF/relevantlog/mDOOPRF_log_"$fileSuffix""
		errorLogFilePrefix="reallogs/MultiDO/OPRF/errorlog/mDOOPRF_log_"$fileSuffix""
		correctLogFilePrefix="reallogs/MultiDO/OPRF/correctnesslog/log_"$fileSuffix""

		echo -e "---------->Starting collect phase"
		echo -e "--->Starting server"
		$Codewd/bin/SSE $tcpport 7 "$curAppArea" collect >"$logFilePrefix"_collect_Server 2>"$errorLogFilePrefix"_collect_Server &
		
		sleep 3
		
		echo -e "--->Starting DO"
		if [ "$curAppArea" == "AddHom" ];
		then
			curGenomeFileName=output_"$curDataSize"_"$curSnpSize"_MAF0.txt
		else
			curGenomeFileName=output_"$fileSuffix".txt
		fi
		$Codewd/bin/SSE $tcpport 6 "$curAppArea" ./data/"$curGenomeFileName" 0 >"$logFilePrefix"_collect_DO 2>"$errorLogFilePrefix"_collect_DO

		wait

		echo -e "---------->Starting merge stage"
		echo -e "--->Starting server"
		$Codewd/bin/SSE $tcpport 7 "$curAppArea" merge 1 >"$logFilePrefix"_merge_Server 2>"$errorLogFilePrefix"_merge_Server &
		
		sleep 3
		
		echo -e "--->Starting AuxServer"
		$Codewd/bin/SSE $tcpport 8 "$curAppArea" merge 1 >"$logFilePrefix"_merge_AuxServer 2>"$errorLogFilePrefix"_merge_AuxServer

		wait

		date

		echo -e "---------->Starting normal init phase"
		$Codewd/bin/SSE $tcpport 0 "$curAppArea" ./data/"$curGenomeFileName" 1 >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup

		echo -e "---------->Starting Query Phase"
		echo -e "--->Starting Server"
		$Codewd/bin/SSE $tcpport 1 "$curAppArea" 1 >"$logFilePrefix"_Server 2>"$errorLogFilePrefix"_Server &
		sleep 3
		echo -e "--->Starting AuxServer"
		$Codewd/bin/SSE $tcpport 2 "$curAppArea" 1 >"$logFilePrefix"_AuxServer 2>"$errorLogFilePrefix"_AuxServer &
		sleep 3
		echo -e "--->Starting Client"
		if [ "$curAppArea" == "AddHom" ];
		then
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_MAF.txt 1 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		else
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_"$curAppArea".txt 1 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		fi

		wait

		echo -e "All entities done. Finding and dumping relevant logs."
		cat "$logFilePrefix"_collect_Server | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_collect_Server
		cat "$logFilePrefix"_collect_DO | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_collect_DO

		cat "$logFilePrefix"_merge_Server | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_merge_Server
		cat "$logFilePrefix"_merge_AuxServer | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_merge_AuxServer

		cat "$logFilePrefix"_setup | grep -E '^(\[Final SE\]|\[Final IP\]|\[Final ValueRet\]|\[Final Hamm\])' > "$relevantLogFilePrefix"_setup

		cat "$logFilePrefix"_Server | grep -E '(\[Server SE|\[Server DCFE|\[Final OblivC|\[Final Hamm|\[Final Data)' > "$relevantLogFilePrefix"_Server
		cat "$logFilePrefix"_AuxServer | grep -E '(\[Authority SE|\[Authority DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_AuxServer
		cat "$logFilePrefix"_Client | grep -E '(\[Client SE|\[Client DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_Client
		if [ "$curAppArea" == "Hamming" ];
                then
			logtoread="Server"
                else
			logtoread="Client"
                fi

		cat "$logFilePrefix"_"$logtoread" | grep "\[Ans\]: " > "$correctLogFilePrefix"
		
		echo -e "\n##############DONE##############\n"
		((ct++))
	done
	((appAreasct++))
done

appAreasct=0

while [ $appAreasct -lt $numAppAreas ] && $runEQCheck
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
		echo -e "\n************STARTING************\n"
		date

		echo -e "CurAppArea, CurDataSize, CurSnpSize = "$curAppArea" "$curDataSize" "$curSnpSize""
		
		fileSuffix="$curDataSize"_"$curSnpSize"_"$curAppArea"0
		logFilePrefix="reallogs/MultiDO/EqCheck/log/mDOEq_log_"$fileSuffix""
		relevantLogFilePrefix="reallogs/MultiDO/EqCheck/relevantlog/mDOEq_log_"$fileSuffix""
		errorLogFilePrefix="reallogs/MultiDO/EqCheck/errorlog/mDOEq_log_"$fileSuffix""
		correctLogFilePrefix="reallogs/MultiDO/EqCheck/correctnesslog/log_"$fileSuffix""

		echo -e "---------->Starting DO for eq check phase"
		echo -e "--->Starting DO"
		if [ "$curAppArea" == "AddHom" ];
		then
			curGenomeFileName=output_"$curDataSize"_"$curSnpSize"_MAF0.txt
		else
			curGenomeFileName=output_"$fileSuffix".txt
		fi
		$Codewd/bin/SSE $tcpport 10 "$curAppArea" ./data/"$curGenomeFileName" 0 >"$logFilePrefix"_collect_DO 2>"$errorLogFilePrefix"_collect_DO

		echo -e "---------->Starting merge stage"
		echo -e "--->Starting server"
		$Codewd/bin/SSE $tcpport 11 "$curAppArea" merge 1 >"$logFilePrefix"_merge_Server 2>"$errorLogFilePrefix"_merge_Server &
		
		sleep 3
		
		echo -e "--->Starting AuxServer"
		$Codewd/bin/SSE $tcpport 12 "$curAppArea" merge 1 >"$logFilePrefix"_merge_AuxServer 2>"$errorLogFilePrefix"_merge_AuxServer

		wait

		date

		echo -e "---------->Starting normal init phase"
		$Codewd/bin/SSE $tcpport 0 "$curAppArea" ./data/"$curGenomeFileName" 2 >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup

		echo -e "---------->Starting Query Phase"
		echo -e "--->Starting Server"
		$Codewd/bin/SSE $tcpport 1 "$curAppArea" 2 >"$logFilePrefix"_Server 2>"$errorLogFilePrefix"_Server &
		sleep 3
		echo -e "--->Starting AuxServer"
		$Codewd/bin/SSE $tcpport 2 "$curAppArea" 2 >"$logFilePrefix"_AuxServer 2>"$errorLogFilePrefix"_AuxServer &
		sleep 3
		echo -e "--->Starting Client"
		if [ "$curAppArea" == "AddHom" ];
		then
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_MAF.txt 2 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		else
			$Codewd/bin/SSE $tcpport 3 "$curAppArea" queries_"$curAppArea".txt 2 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		fi

		wait

		echo -e "All entities done. Finding and dumping relevant logs."
		cat "$logFilePrefix"_collect_DO | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_collect_DO

		cat "$logFilePrefix"_merge_Server | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_merge_Server
		cat "$logFilePrefix"_merge_AuxServer | grep -E '(\[Output Comm:\]|\[Output Time:\])' > "$relevantLogFilePrefix"_merge_AuxServer

		cat "$logFilePrefix"_setup | grep -E '^(\[Final SE\]|\[Final IP\]|\[Final ValueRet\]|\[Final Hamm\])' > "$relevantLogFilePrefix"_setup

		cat "$logFilePrefix"_Server | grep -E '(\[Server SE|\[Server DCFE|\[Final OblivC|\[Final Hamm|\[Final Data)' > "$relevantLogFilePrefix"_Server
		cat "$logFilePrefix"_AuxServer | grep -E '(\[Authority SE|\[Authority DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_AuxServer
		cat "$logFilePrefix"_Client | grep -E '(\[Client SE|\[Client DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_Client
		if [ "$curAppArea" == "Hamming" ];
                then
			logtoread="Server"
                else
			logtoread="Client"
                fi

		cat "$logFilePrefix"_"$logtoread" | grep "\[Ans\]: " > "$correctLogFilePrefix"

		echo -e "\n##############DONE##############\n"
		((ct++))
	done
	((appAreasct++))
done


echo -e "*******Evaluation Complete. Fixing network bandwidth and latency.*******"

# $Totalwd/fixnetworksimulate.sh

echo -e "*******Checking Computation now*******"

python3 $Codewd/CheckForAnyErrorsInLogFiles.py reallogs/SingleDO/errorlog
python3 $Codewd/CheckForAnyErrorsInLogFiles.py reallogs/MultiDO/OPRF/errorlog
python3 $Codewd/CheckForAnyErrorsInLogFiles.py reallogs/MultiDO/EqCheck/errorlog

appAreasct=0

while [ $appAreasct -lt $numAppAreas ] && $checkcorrect
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
		echo -e "\n************STARTING CHECKING************\n"
		date

		echo -e "CurAppArea, CurDataSize, CurSnpSize = "$curAppArea" "$curDataSize" "$curSnpSize""
		
		fileSuffix="$curDataSize"_"$curSnpSize"_"$curAppArea"0
		
		correctLogFile1="reallogs/SingleDO/correctnesslog/log_"$fileSuffix""
		correctLogFile2="reallogs/MultiDO/OPRF/correctnesslog/log_"$fileSuffix""
		correctLogFile3="reallogs/MultiDO/EqCheck/correctnesslog/log_"$fileSuffix""

		correctLogFileIdeal="ideallogs/output_"$fileSuffix""
		
		echo -e "SingleDO Correctness"

		if cmp -s "$correctLogFile1" "$correctLogFileIdeal" ; then
   			echo "CORRECTNESS PASSED"
		else
   			echo "CHECK IF VALUES ONLY DIFFER BY THEIR PRECISION"
		fi

		echo -e "MultiDO OPRF Correctness"

		if cmp -s "$correctLogFile2" "$correctLogFileIdeal" ; then
   			echo "CORRECTNESS PASSED"
		else
   			echo "CHECK IF VALUES ONLY DIFFER BY THEIR PRECISION"
		fi

		echo -e "MultiDO EqCheck Correctness"

		if cmp -s "$correctLogFile3" "$correctLogFileIdeal" ; then
   			echo "CORRECTNESS PASSED"
		else
   			echo "CHECK IF VALUES ONLY DIFFER BY THEIR PRECISION"
		fi
		
		((ct++))	
        done	

	((appAreasct++))
done

