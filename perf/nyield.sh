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
    AVG=`echo "$AVG" | awk -F"." '{print "0"$1"."substr($2,1,6)}'`
    echo $AVG
}

echo "Test de performance de $1 pour un nombre de yield variant de 1 à 10000, pour 100 threads. Les résultats seront affichés dans le fichier $2."
echo "# Fichier généré automatiquement pour le test $1 avec le script nyield.sh" > $2
for i in `seq 1 10 10000`;
do
    echo "$i"
    TIME=`average 10 $1 100 $i-1`
    echo "$i $TIME" >> $2
done
echo "Fin"
