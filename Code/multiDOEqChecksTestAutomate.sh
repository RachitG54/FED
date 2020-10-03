
# ====================================================================
# Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
# Copyright (C) 2019  Rachit Garg Nishant Kumar

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# ====================================================================
#
#
#!/bin/bash


declare -a dataSizes=("40000" "80000" "100000")
# declare -a dataSizes=("5000" "10000" "15000" "20000" "25000")
# declare -a dataSizes=("500" "1000" "1500" "2000" "2500")

declare -a dataSizesSmall=("400" "100" "200" "300" "50")

declare -a snpSizes=("50" "50" "50" "50" "50")
# declare -a snpSizes=("200" "200" "200" "200" "200")

declare -a appAreas=("MAF" "ChiSq" "Hamming" "AddHom")
# declare -a appAreas=("MAF" "ChiSq" "ValueRet" "Hamming")

numdb=1
numAppAreas=1

runOnSmallDb=true
if [ "$1" == "Big" ];
then 
	runOnSmallDb=false
else
	runOnSmallDb=true
fi

echo -e "###############Running for multi DO###############"

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
		echo -e "\n************STARTING************\n"
		date

		echo -e "CurAppArea, CurDataSize, CurSnpSize = "$curAppArea" "$curDataSize" "$curSnpSize""
		
		fileSuffix="$curDataSize"_"$curSnpSize"_"$curAppArea"0
		logFilePrefix="./logs/MultiDO/EqCheck/log/mDOEq_log_"$fileSuffix""
		relevantLogFilePrefix="./logs/MultiDO/EqCheck/relevantlog/mDOEq_log_"$fileSuffix""
		errorLogFilePrefix="./logs/MultiDO/EqCheck/errorlog/mDOEq_log_"$fileSuffix""

		echo -e "---------->Starting DO for eq check phase"
		echo -e "--->Starting DO"
		if [ "$curAppArea" == "AddHom" ];
		then
			curGenomeFileName=output_"$curDataSize"_"$curSnpSize"_MAF0.txt
		else
			curGenomeFileName=output_"$fileSuffix".txt
		fi
		./bin/SSE 10 "$curAppArea" ../Code_Experiments/final/data/"$curGenomeFileName" 0 >"$logFilePrefix"_collect_DO 2>"$errorLogFilePrefix"_collect_DO

		echo -e "---------->Starting merge stage"
		echo -e "--->Starting server"
		./bin/SSE 11 "$curAppArea" merge 1 >"$logFilePrefix"_merge_Server 2>"$errorLogFilePrefix"_merge_Server &

		echo -e "--->Starting AuxServer"
		./bin/SSE 12 "$curAppArea" merge 1 >"$logFilePrefix"_merge_AuxServer 2>"$errorLogFilePrefix"_merge_AuxServer

		wait

		date

		echo -e "---------->Starting normal init phase"
		./bin/SSE 0 "$curAppArea" ../Code_Experiments/final/data/"$curGenomeFileName" 2 >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup

		echo -e "---------->Starting Query Phase"
		echo -e "--->Starting Server"
		./bin/SSE 1 "$curAppArea" 2 >"$logFilePrefix"_Server 2>"$errorLogFilePrefix"_Server &
		sleep 3
		echo -e "--->Starting AuxServer"
		./bin/SSE 2 "$curAppArea" 2 >"$logFilePrefix"_AuxServer 2>"$errorLogFilePrefix"_AuxServer &
		sleep 3
		echo -e "--->Starting Client"
		if [ "$curAppArea" == "AddHom" ];
		then
			./bin/SSE 3 "$curAppArea" ../Code_Experiments/final/queries_MAF.txt 2 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		else
			./bin/SSE 3 "$curAppArea" ../Code_Experiments/final/queries_"$curAppArea".txt 2 >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
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

		echo -e "\n##############DONE##############\n"
		((ct++))
	done
	((appAreasct++))
done
