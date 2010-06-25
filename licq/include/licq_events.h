#ifndef ICQEVENT_H
#define ICQEVENT_H

#include "licq/event.h"

namespace Licq
{
typedef std::list<std::string> StringList;
}

typedef Licq::ExtendedData CExtendedAck;
typedef Licq::SearchData CSearchAck;
typedef Licq::Event LicqEvent;
typedef Licq::Event ICQEvent;

const unsigned short SA_OFFLINE = Licq::SearchData::StatusOffline;
const unsigned short SA_ONLINE = Licq::SearchData::StatusOnline;
const unsigned short SA_DISABLED = Licq::SearchData::StatusDisabled;

typedef Licq::Event::ConnectType ConnectType;
#define CONNECT_SERVER Licq::Event::ConnectServer
#define CONNECT_USER Licq::Event::ConnectUser
#define CONNECT_NONE Licq::Event::ConnectNone

typedef Licq::Event::ResultType EventResult;
#define EVENT_ACKED Licq::Event::ResultAcked
#define EVENT_SUCCESS Licq::Event::ResultSuccess
#define EVENT_FAILED Licq::Event::ResultFailed
#define EVENT_TIMEDOUT Licq::Event::ResultTimedout
#define EVENT_ERROR Licq::Event::ResultError
#define EVENT_CANCELLED Licq::Event::ResultCancelled

#endif
