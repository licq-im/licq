#!/bin/sh
#
# A shell script to view a url using lynx and xterm
# Set the XTERM variable to where your xterm binary is and 
# the LYNX variable to where your lynx binary is or
# leave them as is if the binaries are in your path.
# 
# Graham Roff, February 1999.
#

NCFTP=ncftp
XTERM=xterm

${XTERM} -title "Licq URL View" -e ${NCFTP} "$1"
