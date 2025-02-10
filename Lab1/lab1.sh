#!/bin/bash

rand() {
    SEED=$(date +%s) 
    echo $SEED
}

while true; do
    WIDTH=$(tput cols)
    HEIGHT=$(tput lines)
    
    if (( WIDTH < 8 )); then
        X=0
    else
        X=$(( $(rand) % (WIDTH - 8) ))  
    fi
    
    Y=$(( $(rand) % HEIGHT ))          
    
    clear
    tput cup $Y $X                    
    date +"%H:%M:%S"                  
    
    sleep 1                            
done