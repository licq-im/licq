#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "remoteserver.h"
#include "constants.h"

ICQRemoteServers::ICQRemoteServers(void)
{
   currentServerNum = -1;
}


unsigned short ICQRemoteServers::numServers(void)
{
   return(servers.size());
}


void ICQRemoteServers::addServer(char *theName, unsigned short thePort)
{
   servers.push_back(new RemoteServer(theName, thePort));
}


RemoteServer *ICQRemoteServers::current(void)
{
   return (currentServerNum >= 0 ? servers[currentServerNum] : NULL);
}


void ICQRemoteServers::next(void)
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
char *RemoteServer::name(void)             { return(nameVal); }
unsigned short RemoteServer::port(void)    { return(portVal); }

//-----RemoteServer::retry functions---------------------------------------------------------------
bool RemoteServer::retry(void)     { return (retries < MAX_SERVER_RETRIES); }
void RemoteServer::retrying(void)  { retries++; }
void RemoteServer::ok(void)        { retries = 0; }

