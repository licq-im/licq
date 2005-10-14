#!/bin/sh

BASE=$HOME/varjat/.licq
echo "Licq 1.3.2 Upgrade Script"
echo
echo "This script will upgrade licq's config files to be compatible"
echo "with version 1.3.2. We will use the licq config files located"
echo "in ${BASE}.  It will permamently change these files."
echo "!!! IT IS WISE TO MAKE A BACKUP BEFORE CONTINUING !!!"
echo "Continue (y/N)?"
read CONTINUE

if [ "$CONTINUE" != "y" -a "$CONTINUE" != "Y" ]; then
  echo "Aborting."
  exit
fi

echo "Updating user files..."

cd ${BASE}/users
for i in  *.Licq; do
  ENCODING=`grep Encoding $i | sed "s/^.*[ ]*=[ ]*\(.*$\)/\1/1"`
  grep "Alias = " $i > $i.nick
  if [ -z "$ENCODING" ]; then
    NEWALIAS=`iconv -t "UTF-8" $i.nick`
  else
    REALENCODING=`echo "${ENCODING}" | sed "s/ //g"`
    NEWALIAS=`iconv -f "$REALENCODING" -t "UTF-8" $i.nick`
  fi

  awk '/Alias =/ { printf "%s\n", "'"$NEWALIAS"'" } !/Alias =/ { print $0 }' $i > $i.temp
  mv $i.temp $i
  rm $i.nick
done

echo "Done"
