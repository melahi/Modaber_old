#!/bin/bash

#In the name of God



mkdir -p "ModaberResults"
mkdir -p "colinResults"
mkdir -p "ffResults"


Domain=( 'DepotsNum' 'ZenoNum' );
for (( i = 1; i <= 20; i++)) {
	for (( j = 0; j < ${#Domain[*]} ; j++)) {
		timeout 5m ./runModaber.sh "./numeric/${Domain[$j]}.pddl" "./numeric/${Domain[$j]}Problem$i" > "ModaberResults/${Domain[$j]}Problem$i.output" 2>&1
		if [ -f solution ]; then
			mv solution "./ModaberResults/${Domain[$j]}Problem$i.solution"
		fi
#		timeout 5m ./colin-clp "./numeric/${Domain[$j]}.pddl" "./numeric/${Domain[$j]}Problem$i" > "colinResults/${Domain[$j]}Problem$i.output" 2>&1
#		timeout 5m ./ff -o "./numeric/${Domain[$j]}.pddl" -f "./numeric/${Domain[$j]}Problem$i" > "ffResults/${Domain[$j]}Problem$i.output" 2>&1
	}
}
