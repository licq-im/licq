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
   char *name(void);
   unsigned short port(void);
   bool retry(void);
   void retrying(void);
   void ok (void);
protected:
   char nameVal[64];
   unsigned short portVal, retries;
friend class ICQRemoteServers;
};


class ICQRemoteServers
{
public:
   ICQRemoteServers(void);
   void addServer(char *theName, unsigned short thePort);
   unsigned short numServers(void);
   RemoteServer *current(void);
   void next(void);
   void setServer(unsigned short newCurrent);
//protected:
   vector <RemoteServer *> servers;
   short currentServerNum;
};


#endif
