#!/bin/bash

function capture_time {
    # Start Timestamp
    STARTTIME=`date +%s.%N`
    
    # Commands here (eg: TCP connect test or something useful)
    $1 $2 $3 >> /dev/null
    
    # End timestamp
    ENDTIME=`date +%s.%N`
    
    # Convert nanoseconds to milliseconds
    # crudely by taking first 3 decimal places
    TIMEDIFF=`echo "$ENDTIME - $STARTTIME" | bc | awk -F"." '{print $1"."substr($2,1,6)}'`
    echo $TIMEDIFF
}

function average {
    SUM=0
    for i in `seq 1 $1`;
    do
	TIMEDIFF=`capture_time $2 $3 $4`
	SUM=`echo "$SUM + $TIMEDIFF" | bc | awk -F"." '{print $1"."substr($2,1,6)}'`
    done
    AVG=`bc -l <<< "$SUM/$1"`
    AVG=`echo "$AVG" | awk -F"." '{print $1"."substr($2,1,6)}'`
    echo $AVG
}

echo "Test de performance de $1 en cours..."
echo "Moyenne d'exécution sur 1000 exécutions : `average 1000 $1` secondes"
