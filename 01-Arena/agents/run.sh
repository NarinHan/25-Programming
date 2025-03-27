#!/bin/bash

./gen

AGENT1="./test1"
AGENT2="./test2"
MAP_FILE="map.txt"

for turn in {1..42}; do
    PLAYER=$(head -n 1 "$MAP_FILE")
    if [[ $PLAYER -eq 1 ]]; then
	echo "Turn $turn: Agent 1" 
        $AGENT1
    else
	echo "Turn $turn: Agent 2"
        $AGENT2
    fi
done
