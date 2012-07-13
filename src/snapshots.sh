#!/usr/bin/env bash


while [ 1 ]
do
	TIME=`date "+%Y-%m-%d-%H:%M"`
	zip -r "graphs-$TIME.zip" graph/ log/ www/graph/
	echo "$TIME"
	sleep 300
done
