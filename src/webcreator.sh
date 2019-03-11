#!/bin/bash

#args
if [ ! -d "$1" ]; then
	echo "Error: Wrong directory given"; exit 1
fi
if [ ! -f "$2" ]; then
	echo "Error: Wrong file given"; exit 1
fi
re='^[0-9]+$'
if ! [[ $3 =~ $re ]]; then
	echo "Error: Give Integer w"; exit 1
fi
if ! [[ $4 =~ $re ]]; then
	echo "Error: Give Integer p"; exit 1
fi
if ! [ `wc -l < $2` -gt 10000 ]; then
	echo "Error: Textfile lines are less than 10000"
fi
#chech if empty
if [ ! -z "$(ls -A $1)" ]; then 
   echo "Warning: directory is full, purging ..."
   
   #CAUTION WITH THAT
   rm -rf $1/*
   #DANGER ABOVE
fi

num=$(( ( RANDOM % 1000 )  + 100 ))
for ((i=0; i < $3; i++)) #for every dir
do
	mkdir "$1/site$i"
	for ((j=0; j < $4; j++)) #for every file
	do
		touch $1/site$i/page${i}_${num}.html
		let num=num+5 
	done
done

for ((i=0; i < $3; i++)) #for every dir
do
	echo "Creating website $i ..."
	a=($(ls $1/site$i/)) #files array
	for ((j=0; j < $4; j++)) #for every file
	do
		k=$(( ( RANDOM % ( `wc -l < $2` - 2001 ) )  + 1 ))
		m=$(( ( RANDOM % ( 1000 ) )  + 1000 ))
		echo " Creating page $1/site$i/${a[$j]} with $m lines starting at line $k"
		f=`expr $4 / 2`;let f=f+1;
		q=`expr $3 / 2`;let q=q+1;
		
		#f links
		fl=($(ls -I ${a[$j]} $1/site$i |sort -R |tail -$f))
		
		#q links
		ql=($(for folder in $1/*; do
				if ! [ "${folder}" == "$1/site$i" ]; then
				for fs in ${folder}/*; do
					echo ${fs}
				done
				fi
			done|sort -R |tail -$q))
		
		echo "<!DOCTYPE html>" > $1/site$i/${a[$j]}
		echo "<html>" >> $1/site$i/${a[$j]}
		echo "    <body>" >> $1/site$i/${a[$j]}
		
		sz=`expr $f + $q`
		offset=`expr $m / $sz`
		end=`expr $k + $m`
		for ((s=0; s < $f; s++))
		do
			nk=`expr $k + ${offset}`
			head -${nk} $2 | tail -${offset} >> $1/site$i/${a[$j]}
			echo " Adding link to $1/site$i/${fl[s]}"
			echo "    <br><a href=\"$1/site$i/${fl[s]}\">$1/site$i/${fl[s]}</a><br>" >> $1/site$i/${a[$j]}
			let k=k+${offset}
		done
		
		for ((s=0; s < $q; s++))
		do
			nk=`expr $k + ${offset}`
			head -${nk} $2 | tail -${offset} >> $1/site$i/${a[$j]}
			echo " Adding link to ${ql[s]}"
			echo "    <br><a href=\"${ql[s]}\">${ql[s]}</a><br>" >> $1/site$i/${a[$j]}
			let k=k+${offset}
		done
		
		while [ $k -lt ${end} ]
		do
			let k=k+1
			head -$k $2 | tail -1 >> $1/site$i/${a[$j]}
		done
		
		echo "    </body>" >> $1/site$i/${a[$j]}
		echo "</html>" >> $1/site$i/${a[$j]}
	done
done

#incoming links
for ((i=0; i < $3; i++)) #for every dir
do
	a=($(ls $1/site$i/)) #files array
	for ((j=0; j < $4; j++)) #for every file
	do
		flag=0
		for folder in $1/*; do
			for fs in ${folder}/*; do
				for link in `grep ^"    <br><a" ${fs} | cut -d '"' -f 2`; do
					if [ "${link}" == "$1/site$i/${a[$j]}" ]; then
						let flag=1
						break
					fi
				done
				if [ ${flag} -eq 1 ]; then 
					break
				fi
			done
			if [ ${flag} -eq 1 ]; then 
				break
			fi
		done
		
		if [ ${flag} -eq 0 ]; then
			echo "NOT all pages have at least one incoming link"
			exit 0
		fi
		
	done
	
done

echo "All pages have at least one incoming link"
echo "Done."
