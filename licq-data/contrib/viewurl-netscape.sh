#!/bin/sh
#
# A shell script to view a url using Netscape
# Checks to see if Netscape is running, and if so passes the URL
# to it, otherwise starts up a new copy.
# Set the NETSCAPE variable to where your netscape binary is or
# leave it as is if the binary is in your path.
# 
# Graham Roff, February 1999.
#

NETSCAPE=netscape

if [ -h ${HOME}/.netscape/lock ]; then
  ${NETSCAPE} -remote openURL\("$1"\,new_window\)
else
  ${NETSCAPE} "$1"
fi
