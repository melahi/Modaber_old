#!/bin/bash

#In the name of God




Planner="LiftedRangedModaber"
Planner="LiftedSMTModaber"
Planner="POPF"

Planner="PreferenceMyFP"
Planner="PreferenceMyFP_PG"
Planner="MyFP_PG_Preprocessing"

Domain=( 'market' 'Satellite' 'Depots' 'ZenoTravel' 'DriverLog' 'Rovers');
DomainFile=( 'domain.pddl' 'metricSat.pddl' 'DepotsNum.pddl' 'zenonumeric.pddl' 'driverlogNumeric.pddl' 'NumRover.pddl');

Domain=( 'market' 'Satellite' );
DomainFile=( 'domain.pddl' 'metricSat.pddl' );


Domain=( 'pegsol-strips' 'elevators-numeric' 'transport-numeric' 'elevators-strips' 'woodworking-numeric');

#Set maximum memory limit
#MaximumMemoryLimit=$((1 * 1024 * 1024))
#ulimit -v $MaximumMemoryLimit

for (( j = 0; j < ${#Domain[*]} ; j++)) {
	mkdir -p "${Planner}Results/${Domain[$j]}"
}

for (( i = 1; i <= 30; i++)) {
	for (( j = 0; j < ${#Domain[*]} ; j++)) {
		
		FileNumber=`printf p%02d $i`;
		TheDomainFile="../../Problem/demo-instances/preferences/${Domain[$j]}/${FileNumber}-domain.pddl"
		TheProblemFile="../../Problem/demo-instances/preferences/${Domain[$j]}/${FileNumber}.pddl"
		echo "Planner \"$Planner\" try to solve: $TheDomainFile $TheProblemFile"   
#		timeout 30m ./runModaber.sh "$TheDomainFile" "$TheProblemFile" "${FileNumber}.sol" > "${Planner}Results/${Domain[$j]}/pfile$i.output" 2>&1
		./runModaber.sh "$TheDomainFile" "$TheProblemFile" "${FileNumber}.sol" > "${Planner}Results/${Domain[$j]}/pfile$i.output" 2>&1
#		timeout 30m ./optic-clp -N "$TheDomainFile" "$TheProblemFile" > "${Planner}Results/${Domain[$j]}/pfile$i.output" 2>&1
		if [ -f "${FileNumber}.sol.1" ]; then
			mv ${FileNumber}.sol.* "${Planner}Results/${Domain[$j]}/"
		fi
	}
}
