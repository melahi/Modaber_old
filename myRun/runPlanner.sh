#!/bin/bash

#In the name of God

Planner="SimpleModaber"

Domain=( 'Depots' 'ZenoTravel' 'DriverLog');
DomainFile=( 'DepotsNum.pddl' 'zenonumeric.pddl' 'driverlogNumeric.pddl');

for (( j = 0; j < ${#Domain[*]} ; j++)) {
	mkdir -p "${Planner}Results/${Domain[$j]}"
}

for (( i = 1; i <= 20; i++)) {
#Depots domain has 22 problems but we just try for first 20 problems of it; so we should try for other 2 problem later!
	for (( j = 0; j < ${#Domain[*]} ; j++)) {
		TheDomainFile="../../Problem/ipc2002/Tests1/${Domain[$j]}/Numeric/${DomainFile[$j]}"
		TheProblemFile="../../Problem/ipc2002/Tests1/${Domain[$j]}/Numeric/pfile$i"
		echo "Planner \"$Planner\" try to solve: $TheDomainFile $TheProblemFile"   
		timeout 30m ./runModaber.sh "$TheDomainFile" "$TheProblemFile" > "${Planner}Results/${Domain[$j]}/pfile$i.output" 2>&1
		if [ -f solution ]; then
			mv solution "${Planner}Results/${Domain[$j]}/pfile$i.solution"
		fi
	}
}
