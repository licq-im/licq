#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>


class RemoteServer
{
public:
   RemoteServer(char *theName, unsigned short thePort);
   char *name();
   unsigned short port();
   bool retry();
   void retrying();
   void ok ();
protected:
   char nameVal[64];
   unsigned short portVal, retries;
friend class ICQRemoteServers;
};


class ICQRemoteServers
{
public:
   ICQRemoteServers();
   void addServer(char *theName, unsigned short thePort);
   unsigned short numServers();
   RemoteServer *current();
   void next();
   void setServer(unsigned short newCurrent);
//protected:
   vector <RemoteServer *> servers;
   short currentServerNum;
};


#endif
