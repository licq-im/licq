#!/bin/sh
#
# Script to replace any occurence of NULL in a header or source file
# by a 0, thus allowing buggy c++ implementations to compile code
# containing NULL's.
#
# Graham Roff, Feb/99
#

# Make sure this is what the user wants to do 
echo "This script will convert all NULL references to 0's."
read -e -p "Are you sure you want to proceed [y/N] ? " ANSWER
if [ ${ANSWER}x != 'Yx' -a ${ANSWER}x != 'yx' ]; then
  echo "Exiting."
  exit 1
fi

# Do it
echo Converting NULL's to 0's for old compilers...
for INF in `ls *.h *.cpp`; do
	echo -n "Parsing $INF..."
	sed -e s/NULL/0/g < ${INF} > ${INF}.sed 
	mv -f ${INF}.sed ${INF}
	echo "done";
done
echo "Finished."

