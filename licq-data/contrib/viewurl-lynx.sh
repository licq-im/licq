#!/bin/sh
#
# A shell script to view a url using lynx and xterm
# Set the XTERM variable to where your xterm binary is and 
# the LYNX variable to where your lynx binary is or
# leave them as is if the binaries are in your path.
# 
# Graham Roff, February 1999.
#

XTERM=xterm
LYNX=lynx

${XTERM} -title "Licq URL View" -e ${LYNX} "$1"
