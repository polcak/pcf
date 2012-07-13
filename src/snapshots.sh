#!/usr/bin/env bash

TIME=`date "+%Y-%m-%d-%H:%M"`

while [ 1 ]
do
	zip -r "graphs-$TIME.zip" graph/ log/ www/graph/
	echo "$TIME"
	sleep 300
done
