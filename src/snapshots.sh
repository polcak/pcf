#!/usr/bin/env bash

rm -f graph/* log/* www/graph/*

while [ 1 ]
do
	TIME=`date "+%Y-%m-%d-%H-%M"`
	tar cvf "graphs-$TIME.tar" graph/ log/ www/graph/
	echo "$TIME"
	sleep 300
done
