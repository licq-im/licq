#!/bin/sh
#
# Jon Keating <jon@licq.org>
#
# Script to upgrade old versions of Licq installations to the new format
# that was introduced in 1.3.0. 

BASE=${HOME}/.licq
LICQCONF=${BASE}/licq.conf
TMPLICQCONF=${BASE}/licq.conf.tmp
CONF=${BASE}/users.conf

if [ ! -d "$BASE" ]; then
  echo "upgrade-1.3.0.sh"
  echo "Upgrades an older Licq base directory to the structure used since 1.2.7."
  exit
fi

echo "Licq 1.3 Format Upgrade Script"

echo ""
echo "Did you backup $BASE?  If not you may lose all your contacts!"
echo "Continue? (y/n)"
read CONT
if [ "$CONT" != "y" -a "$CONT" != "Y" ]; then
  echo "Aborting."
  exit
fi

echo ""
echo "Updating licq.conf file..."
cd $BASE

sed -e 's/Version = 1027/Version = 1028/g' $LICQCONF > $TMPLICQCONF && mv $TMPLICQCONF $LICQCONF
UIN=`cat $BASE/owner.uin | grep Uin | cut -d " " -f 3`
echo "[owners]" >> $LICQCONF
echo "NumOfOwners = 1" >> $LICQCONF
echo "Owner1.Id = $UIN" >> $LICQCONF
echo "Owner1.PPID = Licq" >> $LICQCONF

echo ""
echo "Updating owner file..."
mv owner.uin owner.Licq

echo "Updating user files..."
cd users
for i in `ls *.uin | cut -d . -f 1`; do
   mv $i.uin $i.Licq
done

echo "Updating users.conf file..."
echo "[users]" > $CONF
echo "NumOfUsers = `ls *.Licq | wc -l`" >> $CONF
ls *.Licq | awk 'BEGIN { i = 1 } { printf "User%d = %d.Licq\n", i++, $1}' >> $CONF

echo "Updating history files..."
cd ../history
for i in `ls *.history | cut -d . -f 1`; do
   mv $i.history $i.Licq.history
done

for i in `ls *.removed | cut -d . -f 1`; do
   mv $i.history.removed $i.Licq.history.removed
done

echo "Upgrade completed."
