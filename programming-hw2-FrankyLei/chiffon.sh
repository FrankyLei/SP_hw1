#!/bin/bash


#bash chiffon.sh -m [n_hosts] -n [n_players] -l [lucky_number]
if [ "$#" != "6" ]; then
	echo "[error] arg < 6 , arg == $#"
	exit 0
fi

#---------get argvs -----------

helpFunction()
{
   echo ""
   echo "Usage: $0 -a parameterA -b parameterB -c parameterC"
   echo -e "\t-m Description of what is parameterA"
   echo -e "\t-n Description of what is parameterB"
   echo -e "\t-l Description of what is parameterC"
   exit 1 # Exit script after printing help
}

while getopts "m:n:l:" opt
do
   case "$opt" in
      m ) parameterA="$OPTARG" ;;
      n ) parameterB="$OPTARG" ;;
      l ) parameterC="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$parameterA" ] || [ -z "$parameterB" ] || [ -z "$parameterC" ]
then
   echo "Some or all of the parameters are empty";
   helpFunction
fi

# Begin script in case all parameters are correct
#echo "pra 1 $parameterA"
#echo "pra 2 $parameterB"
#echo "pra 3 $parameterC"


n_host=$parameterA
if [[ "$parameterA" -lt 1 ]] || [[ "$parameterA" -gt 10 ]] ; then 
    echo "[error] n_host val isn't corret , n_host = $parameterA"
fi

n_players=$parameterB

if [[ "$parameterB" -lt 8 ]] || [[ "$parameterB" -gt 12 ]]; then 
    echo "[error] n_player val isn't corret , n_player = $parameterB"
fi

lucky_number=$parameterC
if [[ "$parameterC" -lt 0 ]] || [[ "$parameterC" -gt 1000 ]]; then 
    echo "[error] lucky_number val isn't corret , number = $parameterC"
fi
#--------end get argvs ---------

# Prepare FIFO files for all M hosts.
# 0 for read from host, > 0 for write to host 

for (( i=0; i<=n_host; i++ ))
do
	mkfifo ./fifo_${i}.tmp
done

#Generate combinations of N  players and assign to hosts via FIFO.
#Collect scores from hosts and calculate the final result.

for ((i=1; i<=n_host; i++))
do
    #echo "./host -m ${i} -d 0 -l ${lucky_number}"
    ./host -m ${i} -d 0 -l ${lucky_number} & # background !!!
done

exec 3<> "./fifo_0.tmp"

for (( i=1; i<=n_host; i++ ))	
do
	index=$((i + 3))
	exec {index}<> "./fifo_${i}.tmp"
done

#echo "[shell] after fifo_i.tmp"


mapID=()
doneComb=0
combNum=0

# generate C(N, 8)

hostStatus=()
score=()

for (( i=1; i<=n_host; i++ ))
do
	hostStatus[${i}]=true
	score=[${i}]=$((0))
done

#echo "[shell] init host status"

for (( i=1; i<=8; i++ ))
do
	A[i]=${i}
    #echo ${A[i]}
	comb="${comb} ${i}"
    #echo ${comb} 

	if [ "${i}" -eq  "8" ]; then
		for (( j=1; j<=n_host; j++))
		do
			if [ "${hostStatus[$[j]]}" = true ]; then
				#echo ${comb}
				echo ${comb} > "fifo_${j}.tmp"
				hostStatus[$[j]]=false
				break
			fi
		done
		comb=""
		combNum=$((combNum + 1))
	fi
done

#echo "[shell] someting"

while [ "${A[1]}" -lt "$((n_players - 7))" ]
do	
	for (( i=8; i>=1; i--))
	do
		#echo "ho"
		if [ "${A[i]}" -lt "$((n_players - 8 + i))" ]; then
			
			A[i]=$((A[i] + 1))
			p=i	
			break
		fi
	done

	for (( i=p+1; i<=8; i++))
	do

		A[i]=$((A[$((i - 1))] + 1))
	done

	for (( i=1; i<=8; i++))
	do
		comb="${comb} ${A[i]}"

		#echo ${comb}
		#echo ${i}
		if [ "${i}" -eq  "8" ]; then

			#echo "here"
			for (( j=1; j<=n_host; j++ ))
			do
				#if [ "${hostStatus[$[j]]}" = true ]; then
					#echo ${comb}
					echo ${comb} > "fifo_${j}.tmp"
					hostStatus[${j}]=false
					break
				
			done

			if [ "${j}" -eq "$((n_host + 1))" ]; then	


				read < "./fifo_0.tmp" doneKey	

				#echo ${comb} > "fifo_${mapID[${doneKey}]}.tmp"
				hostStatus[${doneKey}] = true

				for (( i=1; i<=8; i++ ))
				do
					read < "./fifo_0.tmp" data

					#echo ${data}

					k=0 
					id=0
					rank=0
					for subdata in $data
					do
						if [ "${k}" -eq "0" ]; then
							id=${subdata}	
						else
							rank=${subdata}
						fi
						k=$((k+1))
					done
		
					score[${id}]=$((score[${id}] + rank))
				done
				doneComb=$((doneComb + 1))
			fi
			comb=""
			combNum=$((combNum + 1))
		fi
	done
done

#echo "[shell] 1 someting"
#echo ${combNum}

for (( doneComb; doneComb<combNum; doneComb++))
do
	read< "./fifo_0.tmp" doneKey
	#echo ${doneKey}	

	for (( j=1; j<=8; j++ ))
	do 
		read < "./fifo_0.tmp" data
		#echo ${data}

		k=0 
		id=0
		rank=0
		for subdata in ${data}
		do
			#echo ${subdata}
			if [ "${k}" -eq "0" ]; then
				id=${subdata}	
			else
				rank=${subdata}
				#echo ${rank}
			fi
			k=$((k+1))
		done
		
		score[${id}]=$((score[${id}] + ${rank}))
		#echo ${score[${id}]}
	done
done	

#echo "[shell] 2 someting"

# 3.Send -1s to all hosts for close hosts
for (( i=1; i<=n_host; i++ ))
do
	echo "-1 -1 -1 -1 -1 -1 -1 -1" > "fifo_${i}.tmp"
done

# 4.print the final scores ordered by plyer id

#echo "-------------score----------------"
for (( i=1; i<=n_players; i++ ))
do
	#echo "hi"
	echo "${i} ${score[${i}]}"
done
#echo "end scoring "

# 5.Wait for all forked process
wait


# Remove FIFO , exit
rm -f fifo_*.tmp
exit 0
