// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_remoteserver.h"
#include "licq_constants.h"

ICQRemoteServers::ICQRemoteServers()
{
   currentServerNum = -1;
}


unsigned short ICQRemoteServers::numServers()
{
   return(servers.size());
}


void ICQRemoteServers::addServer(char *theName, unsigned short thePort)
{
   servers.push_back(new RemoteServer(theName, thePort));
}


RemoteServer *ICQRemoteServers::current()
{
   return (currentServerNum >= 0 ? servers[currentServerNum] : NULL);
}


void ICQRemoteServers::next()
{
   if (servers.size() == 0) return;
   currentServerNum++;
   if (currentServerNum >= (short)servers.size()) currentServerNum = 0;
}


void ICQRemoteServers::setServer(unsigned short newCurrent)
{
   if (newCurrent <= servers.size()) currentServerNum = newCurrent - 1;
}





//-----RemoteServer::constructor-------------------------------------------------------------------
RemoteServer::RemoteServer(char *theName, unsigned short thePort)
// called when first constructing our known servers
{
   strcpy(nameVal, theName);
   portVal = thePort;
   retries = 0;
}  


//----RemoteServer::data retrieval functions-------------------------------------------------------
char *RemoteServer::name()             { return(nameVal); }
unsigned short RemoteServer::port()    { return(portVal); }

//-----RemoteServer::retry functions---------------------------------------------------------------
bool RemoteServer::retry()     { return (retries < MAX_SERVER_RETRIES); }
void RemoteServer::retrying()  { retries++; }
void RemoteServer::ok()        { retries = 0; }

