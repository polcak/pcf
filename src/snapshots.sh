#!/usr/bin/env bash

rm -f graph/{javascript,tcp,icmp}/* log/{javascript,tcp,icmp}/* www/graph/{javascript,tcp,icmp}/*

while [ 1 ]
do
	TIME=`date "+%Y-%m-%d-%H-%M"`
	tar cvf "graphs-$TIME.tar" graph/ log/ www/graph/ www/data
	echo "$TIME"
	sleep 300
done
