#!/bin/sh
#
# Brian Hunt ( bmh at canada.com ), December 1999
#
# set NETSCAPE to your netscape binary.
# set URLHOME to your default homepage.
# --
# (): Open new window, at:
# 	(1) specified link
#	(2) home page given in URLHOME
# Conditions of execution:
# Netscape lock file exists (~/.netscape/lock):
#	(1) netscape running
# 	-> Open new window
#	(2) netscape not running
#		(as per `ps` , which echos netscape's pids)
#	  delete ~/.netscape/lock 
#	-> Open new window
# Netscape lockfile does not exist;
#	-> Open new window.
#

NETSCAPE=netscape
URLHOME='http://www.linux.org'

if [ "$1" = "" ] ; then
  URL=$URLHOME
else
  URL=$1
fi

if [ -h ${HOME}/.netscape/lock ]; then
  if ps -e | grep netscape > /dev/null ; then
     ${NETSCAPE} -remote openURL\("$URL"\,new_window\) &
  else
     rm ${HOME}/.netscape/lock
     ${NETSCAPE} "$URL" &
  fi
else
  ${NETSCAPE} "$URL" &
fi


