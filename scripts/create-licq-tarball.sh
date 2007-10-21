#!/bin/bash
#
# Script to create a source tarball from Licq's svn repository.
# 
# Author: Erik Johansson <erik@ejohansson.se>
# Latest version: http://svn.licq.org/svn/trunk/scripts/
# 
# Usage: ./create-licq-tarball.sh [tardir]
#        [tardir] - optional directory to place tarball in;
#                   defaults to current directory

# Where to save the tarball(s) (default: first argument or current dir)
TARDIR="${1:-.}"

# What to name the tarball (REV is replaced with the svn revision)
# Note: Don't add .tar.gz or .tar.bz2
TARNAME="licq-1.3.5~rREV"
#TARNAME="licq-1.3.5-rc2"

# Archives to create (1 = create; 0 = don't create)
CREATE_GZ=1
CREATE_BZ2=0

# Sign archives using default GPG key (1 = sign; 0 = don't sign)
SIGN=0

# What to name the directory in the tarball (REV is replaced with the svn revision)
DIRNAME="${TARNAME}"

# What revision to export
LICQREV="HEAD"

# Ownership of files in tarball
OWNER="root"
GROUP="root"

# Licq repository
REPO="http://svn.licq.org/svn"

### END SETTINGS ###

#------------------------------------------------------------------#
# Echos "$1 failed" or "failed" and then exits.
#------------------------------------------------------------------#
function failed()
{
   if [ -z $1 ]; then
      echo "failed"
   else
      echo "$1 failed"
   fi
   exit 1
}

SVNREV=$(svn info -r"${LICQREV}" "${REPO}" | grep "^Revision:" | awk '{print $2}') || failed

TMPDIR=$(mktemp -d) || failed "mktemp -d"
DIRNAME="${DIRNAME//REV/$SVNREV}"
TARNAME="${TARNAME//REV/$SVNREV}"
LICQDIR="${TMPDIR}/${DIRNAME}"

function exit_if_exists()
{
   if [ -e "$1" ]; then
      echo "$1 already exists"
      exit 1
   fi
}

function svnexport()
{
   echo -n "Exporting $1 (r${SVNREV})... "
   svn export --ignore-externals -r"${SVNREV}" -q "${REPO}/$1" "${LICQDIR}/$2" || failed
   echo "done"
}

function makecvs()
{
   echo "Running make -f $1/Makefile.cvs"
   make -C "${LICQDIR}/$1" -f "${LICQDIR}/$1/Makefile.cvs" > /dev/null 2>&1 || failed
   rm -rf "${LICQDIR}/$1/autom4te.cache"
   rm -f "${LICQDIR}/$1/Makefile.cvs"
}

function cleanup()
{
   echo "Removing ${TMPDIR}"
   rm -rf "${TMPDIR}"
}

test ${CREATE_GZ}  -ne 0 && exit_if_exists "${TARDIR}/${TARNAME}.tar.gz"
test ${CREATE_BZ2} -ne 0 && exit_if_exists "${TARDIR}/${TARNAME}.tar.bz2"

svnexport "trunk/licq" ""
svnexport "trunk/admin" "admin"

makecvs "."

for plugin in auto-reply console email msn osd qt-gui rms; do
   svnexport "trunk/${plugin}" "plugins/${plugin}"
   makecvs "plugins/${plugin}"
done

rm -f "${LICQDIR}/admin/Makefile.common"

for plugin in licqweb; do
   svnexport "trunk/${plugin}" "plugins/${plugin}"
done

#svnexport "branches/qt-gui_qt4" "plugins/qt4-gui"

echo "Creating tarball ${TARNAME}.tar"
tar --owner "${OWNER}" --group "${GROUP}" -C "${TMPDIR}" -cf "${TMPDIR}/${TARNAME}.tar" "${DIRNAME}" || failed

function sign()
{
   if [ ${SIGN} -ne 0 ]; then
      echo "Signing $1"
      gpg --sign --armor --detach-sign --output "$1.sign" "$1" || failed "signing"
   fi
}

if [ ${CREATE_GZ} -ne 0 ]; then
   echo -n "Creating ${TARDIR}/${TARNAME}.tar.gz... "
   cp "${TMPDIR}/${TARNAME}.tar" "${TARDIR}" || failed "copy tarball"
   gzip --best "${TARDIR}/${TARNAME}.tar" || failed
   echo "done"

   sign "${TARDIR}/${TARNAME}.tar.gz"
fi

if [ ${CREATE_BZ2} -ne 0 ]; then
   echo -n "Creating ${TARDIR}/${TARNAME}.tar.bz2... "
   cp "${TMPDIR}/${TARNAME}.tar" "${TARDIR}" || failed "copy tarball"
   bzip2 --best "${TARDIR}/${TARNAME}.tar" || failed
   echo "done"

   sign "${TARDIR}/${TARNAME}.tar.bz2"
fi

cleanup
