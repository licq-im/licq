#include "licq/contactlist/group.h"

#include <cstdio> // snprintf

#include "licq_file.h"
#include <licq/types.h>

using std::map;
using std::string;

using Licq::Group;

Group::Group(int id, const string& name)
  : myId(id),
    myName(name),
    mySortIndex(0)
{
  char strId[8];
  snprintf(strId, 7, "%u", myId);
  strId[7] = '\0';

  myMutex.setName(strId);
}

Group::~Group()
{
}

void Group::save(CIniFile& file, int num) const
{
  char key[MAX_KEYxNAME_LEN];

  sprintf(key, "Group%d.id", num);
  file.WriteNum(key, myId);

  sprintf(key, "Group%d.name", num);
  file.writeString(key, myName);

  sprintf(key, "Group%d.Sorting", num);
  file.WriteNum(key, mySortIndex);

  map<unsigned long, unsigned long>::const_iterator i;
  for (i = myServerIds.begin(); i != myServerIds.end(); ++i)
  {
    char pidstr[5];
    Licq::protocolId_toStr(pidstr, i->first);
    sprintf(key, "Group%d.ServerId.%s", num, pidstr);
    file.WriteNum(key, i->second);
  }
}

unsigned long Group::serverId(unsigned long protocolId) const
{
  map<unsigned long, unsigned long>::const_iterator iter;
  iter = myServerIds.find(protocolId);
  if (iter == myServerIds.end())
    return 0;
  return iter->second;
}

void Group::setServerId(unsigned long protocolId, unsigned long serverId)
{
  myServerIds[protocolId] = serverId;
}


bool Licq::compare_groups(const Group* first, const Group* second)
{
  return first->sortIndex() < second->sortIndex();
}
