#!/bin/sh
#
# Brian Hunt ( bmh at canada.com ), December 1999
#
# set OPERA to your opera binary.
# set URLHOME to your default homepage.
#
# OPERA VERSION -> TRME

OPERA=opera
URLHOME='http://www.licq.org'

if [ "$1" = "" ] ; then
  URL=$URLHOME
else
  URL=$1
fi

${OPERA} -newpage "$URL" &
