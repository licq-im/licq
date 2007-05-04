#!/bin/bash
# By Juan. (juam on #licq)
#
# !!! BACKUP YOUR LICQ FOLDER (~/.licq) JUST IN CASE!!!
#

BACK_SUFIX="~"

function change_val()
{ 	local back;

	back="$1$BACK_SUFIX"
	vars=$2
	cp "$1" "$back"
	awk  '
		$1 ~ /^('$vars')/ {
			print $1 " = 0"
		} 
		$1 !~ /^('$vars')/ {
			print
		}
	' < "$back" > "$1" 

}


function check()
{	
	if [[  ! -a $1 ]]; then
		echo "$1: not found. exiting."
		exit 1
	fi
}

case $# in
	0) 	echo "Usage: $0 licq-dir"
        	exit 1
esac

for root in $@; do

	echo "changing users from $root/users/\*.Licq"
	check "$root/users"
	for i in "$root"/users/*.Licq; do
		change_val "$i" "SID|GSID|InvisibleSID|VisibleSID"
	done

	echo "changing $root/owner.Licq"
	check "$root/owner.Licq"
	change_val "$root/owner.Licq" "SSTime|SSCount"

	echo "changing $root/licq.conf"
	check "$root/licq.conf"
	t=`grep NumOfGroups "$root/licq.conf" |
	   cut -d'=' -f2 | 
	   awk '
	   {	for(i=1;i<=$1;i++)
	   	{	if( i == 1)
	   			c = "";
	   		else 
	   			c = "|";
	   		ba = sprintf("%s%sGroup%d.id",ba,c,i);
	   	}
	   	print ba
	   }'`
	change_val "$root"/licq.conf "$t"

done
