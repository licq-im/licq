#!/bin/sh
#
# Script to tag a Licq release.
#
# Copyright (c) 2007 Erik Johansson <erijo@licq.org>
#
# Distributed under the terms of the GNU GPL version 2.
#

### SETTINGS ###

# Set to the URL you use to check out Licq
REPO="svn+ssh://licq"

### END SETTINGS ###
function usage()
{
    echo "Usage: $0 -v version -r rev"
    echo "  -h, --help  This message"
    echo "  -v          Name the tag licq-<version> (e.g. 1.3.5-rc3)"
    echo "  -r          Revision to tag (e.g. 5656)"
}

args=$(getopt -n "$0" -o h,v:,r: -l help -- $*)
if [ $? -ne 0 ]; then
    echo ""
    usage
    exit 1
fi

set -- $args
while [ $# -gt 0 ]; do
    case $1 in
	-h|--help) usage; exit 0 ;;
	-v) VERSION="licq-$(eval echo $2)"; shift ;;
	-r) REV=$(eval echo $2); shift ;;
	--) ;;
	*) echo "$0: unknown option '$1'"; exit 1 ;;
    esac
    shift
done

if [ -z "$VERSION" -o -z "$REV" ]; then
    echo "$0: Missing argument"
    echo "Try \`$0 --help' for more information"
    exit 2
fi

TMPDIR=$(mktemp -d) || exit 1
TAGDIR="${TMPDIR}/${VERSION}"

function cleanup
{
  echo "Removing ${TMPDIR}"
  rm -rf "${TMPDIR}"
}

function failed
{
  echo "failed"
  cleanup
  exit 1
}

echo -n "Getting tags dir..."
svn co -N -q "${REPO}/tags" "${TMPDIR}" || failed
echo "done"

function svncp
{
  echo -n "Copying $1 to $2..."
  svn cp -r$REV -q "${REPO}/trunk/$1" "${TAGDIR}/$2" || failed
  echo "done"
}

svncp licq /

# Remove the checked out externals
for dir in $(svn pg svn:externals "${TAGDIR}" | awk '{print $1}'); do
  if [ -d "${TAGDIR}/${dir}" ]; then
    echo -n "Removing external ${dir}..."
    rm -rf "${TAGDIR}/${dir}" || failed
    echo "done"
  fi
done

# Remove the externals property
echo -n "Removing svn:externals..."
svn pd -q svn:externals "${TAGDIR}" || failed
echo "done"

# Add admin and all plugins
svncp admin admin
for plugin in auto-reply console email licqweb msn osd qt-gui rms; do
  svncp $plugin plugins/$plugin
done

function separator
{
  echo ""
  echo "###########################################################################"
  echo ""
}

# Create the commit message
msg="${TMPDIR}/svn-commit.tmp"
echo "Tagged ${VERSION} from trunk r${REV}." > "$msg"
separator

echo "svn status:"
svn st -q "${TMPDIR}"
separator

echo "svn diff:"
svn diff "${TMPDIR}"
separator

echo "Commit message:"
cat "$msg"
separator

echo -n "Enter yes to commit: "
read ans
if [ "$ans" == "yes" ]; then
  svn ci -F "$msg" "${TMPDIR}"
  cleanup
else
  echo "OK, commit ${TMPDIR} manually then."
  echo "I left the commit message in ${msg}."
fi
