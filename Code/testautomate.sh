
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


declare -a dataSizesSmall=("400" "800" "1200" "1600" "2000")

declare -a dataSizes=("5000" "10000" "15000" "20000" "25000")
# declare -a snpSizes=("50" "50" "50" "50" "50")
# declare -a appAreas=("Hamming" "ChiSq" "ValueRet" "MAF" "AddHom")
# declare -a dataSizes=("2000000")
# declare -a dataSizes=("20000" "40000" "60000" "80000" "100000")
# declare -a dataSizes=("5000" "10000" "15000" "20000" "25000")
# declare -a snpSizes=("200" "200" "200" "200" "200")
declare -a snpSizes=("50" "50" "50" "50" "50")

declare -a appAreas=("ValueRet" "MAF" "ChiSq" "Hamming" "AddHom")

numdb=5
numAppAreas=3

runOnSmallDb=true
if [ "$1" == "Big" ];
then 
	runOnSmallDb=false
else
	runOnSmallDb=true
fi

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
		logFilePrefix="./logs/SingleDO/log/log_"$fileSuffix""
		relevantLogFilePrefix="./logs/SingleDO/relevantlog/log_"$fileSuffix""
		errorLogFilePrefix="./logs/SingleDO/errorlog/log_"$fileSuffix""

		echo -e "Starting Setup"
		 if [ "$curAppArea" == "AddHom" ];
		then
			./bin/SSE 0 "$curAppArea" ../Code_Experiments/final/data/output_"$curDataSize"_"$curSnpSize"_MAF0.txt >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup
		else
			./bin/SSE 0 "$curAppArea" ../Code_Experiments/final/data/output_"$fileSuffix".txt >"$logFilePrefix"_setup 2>"$errorLogFilePrefix"_setup
		fi
		cat "$logFilePrefix"_setup | grep -E '^(\[Final SE\]|\[Final IP\]|\[Final ValueRet\]|\[Final Hamm\])' > "$relevantLogFilePrefix"_setup

		date

		echo -e "Starting other entities"
		./bin/SSE 1 "$curAppArea" >"$logFilePrefix"_Server 2>"$errorLogFilePrefix"_Server &
		sleep 3
		./bin/SSE 2 "$curAppArea" >"$logFilePrefix"_AuxServer 2>"$errorLogFilePrefix"_AuxServer &
		sleep 3
		if [ "$curAppArea" == "AddHom" ];
		then
			./bin/SSE 3 "$curAppArea" ../Code_Experiments/final/queries_MAF.txt >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		else
			./bin/SSE 3 "$curAppArea" ../Code_Experiments/final/queries_"$curAppArea".txt >"$logFilePrefix"_Client 2>"$errorLogFilePrefix"_Client
		fi

		wait

		echo -e "All entities done"
		cat "$logFilePrefix"_Server | grep -E '(\[Server SE|\[Server DCFE|\[Final OblivC|\[Final Hamm|\[Final Data)' > "$relevantLogFilePrefix"_Server
		cat "$logFilePrefix"_AuxServer | grep -E '(\[Authority SE|\[Authority DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_AuxServer
		cat "$logFilePrefix"_Client | grep -E '(\[Client SE|\[Client DCFE|\[Final OblivC|\[Final Hamm)' > "$relevantLogFilePrefix"_Client

		echo -e "\n##############DONE##############\n"
		((ct++))
	done
	((appAreasct++))
done
