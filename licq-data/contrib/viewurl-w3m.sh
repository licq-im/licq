#!/bin/sh
#
# A shell script to view a url using w3m and xterm
# Set the XTERM variable to where your xterm binary is and 
# the W3M variable to where your w3m binary is or
# leave them as is if the binaries are in your path.
# 
# Graham Roff, October 1999.
#

XTERM=xterm
W3M=w3m

${XTERM} -title "Licq URL View" -e ${W3M} "$1"
