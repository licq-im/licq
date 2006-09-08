#!/bin/sh
#
# by Nicos Gollan <gtdev@spearhead.de>, 2003-01-31
#
# Used to open URLs in Mozilla, using Mozillas 'remote' function
# (see http://www.mozilla.org/unix/remote.html) to avoid opening
# new browser windows all over the place. Default is to open the
# requested site in a new tab. Configuration can be done via
# environment variables.
#
# Recognized environment variables:
#  - VURL_MOZILLA
#    Your mozilla binary. Defaults to 'mozilla'
#  - VURL_HOME
#    Your homepage. Defaults to 'www.licq.org'. This page is
#    opened if this script is called without parameters
#  - VURL_OPENMODE
#    Method of opening the URL.
#      "new-tab" - opens URL in a new tab (default)
#      "new-window" - opens URL in a new window
#      "current" - opens URL in the current window
#

if [ -z "${VURL_MOZILLA}" ]; then
  MOZILLA=firefox
else
  MOZILLA=${VURL_MOZILLA}
fi

if [ -z "${VURL_HOME}" ]; then
  URLHOME='http://www.licq.org'
else
  URLHOME="${VURL_HOME}" 
fi

if [ -z "${VURL_OPENMODE}" ]; then
  OPENMODE=new-tab
else
  OPENMODE="${VURL_OPENMODE}"
fi

if [ "$1" = "" ] ; then
  URL=$URLHOME
else
  URL=$1
fi

if [ "${OPENMODE}" = "current" ]; then
  OPENSTRING="${URL}"
else
  OPENSTRING="${URL},${OPENMODE}"
fi

# If mozilla is already running, open the URL as new tab,
# otherwise launch a fresh copy of mozilla.
((${MOZILLA} -remote openurl\("${OPENSTRING}"\) ) ||  (${MOZILLA} "${URL}") & ) > /dev/null 2>&1
