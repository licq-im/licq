#!/bin/bash
# This requires
# Arguments = '$a' $i
# PassMessage = 1
# PassMessage is 1 so we could read the user message.
# Arguments = '$a' (getting user Alias, make sure you use ' ') $i ( getting user IP address)
# Writen by Elius - www.buga-designs.com - ICQ: 124650632



# This line store the message the user sent to var MSG.
MSG=`tee`

# Checks for the user input and display the data according to it.
if [ "$MSG" = "1" ]; then
fortune SP
else
        if [ "$MSG" = "2" ]; then
        fortune
        else
                if [ "$MSG" = "3" ]; then
                cpu_temp=$(sensors | tail -n 7 | head -1 | cut -d "+" -f 2 | cut -d " " -f 1)
                mb_temp=$(sensors | tail -n 8 | head -1 | cut -d "+" -f 2 | cut -d " " -f 1)
                echo "CPU : $cpu_temp"
                echo "Chipset : $mb_temp"
                else
                        if [ "$MSG" = "4" ]; then
                        ping_time=$(ping -c 1 $2 | head -2 | cut -d "=" -f 4,9 | tail -n 1)
                        echo "Ping takes $ping_time"
                        else
                                echo " Hi $1"
                                echo " 1 - south park fortune"
                                echo " 2 - fortune"
                                echo " 3 - System temperature"
                        echo " 4 - Ping"
                        echo " Any other char - menu"
                        fi
                fi
        fi
fi
