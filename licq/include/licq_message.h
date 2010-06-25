#ifndef MESSAGE_H
#define MESSAGE_H

#include "licq/userevents.h"

const unsigned long E_LICQxVER      = Licq::UserEvent::FlagLicqVerMask;
const unsigned long E_DIRECT        = Licq::UserEvent::FlagDirect;
const unsigned long E_MULTIxREC     = Licq::UserEvent::FlagMultiRec;
const unsigned long E_URGENT        = Licq::UserEvent::FlagUrgent;
const unsigned long E_CANCELLED     = Licq::UserEvent::FlagCancelled;
const unsigned long E_ENCRYPTED     = Licq::UserEvent::FlagEncrypted;
const unsigned long E_UNKNOWN       = Licq::UserEvent::FlagUnknown;

typedef Licq::UserEvent CUserEvent;
typedef Licq::EventMsg CEventMsg;
typedef Licq::EventFile CEventFile;
typedef Licq::EventUrl CEventUrl;
typedef Licq::EventChat CEventChat;
typedef Licq::EventAdded CEventAdded;
typedef Licq::EventAuthRequest CEventAuthRequest;
typedef Licq::EventAuthGranted CEventAuthGranted;
typedef Licq::EventAuthRefused CEventAuthRefused;
typedef Licq::EventWebPanel CEventWebPanel;
typedef Licq::EventEmailPager CEventEmailPager;
typedef Licq::EventContactList::Contact CContact;
typedef Licq::EventContactList::ContactList ContactList;
typedef Licq::EventContactList CEventContactList;
typedef Licq::EventSms CEventSms;
typedef Licq::EventServerMessage CEventServerMessage;
typedef Licq::EventEmailAlert CEventEmailAlert;
typedef Licq::EventPlugin CEventPlugin;
typedef Licq::EventUnknownSysMsg CEventUnknownSysMsg;

#endif
