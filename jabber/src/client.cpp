/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2014 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "client.h"

#include "sessionmanager.h"
#include "user.h"
#include "vcard.h"

#include <gloox/connectionhttpproxy.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/disco.h>
#include <gloox/message.h>
#include <gloox/rostermanager.h>
#include <gloox/vcardupdate.h>

#include <licq/daemon.h>
#include <licq/logging/log.h>
#include <licq/licqversion.h>
#include <licq/thread/mutexlocker.h>

#define TRACE_FORMAT "Client::%s: "
#define TRACE_ARGS __func__
#include "debug.h"

using namespace LicqJabber;

using Licq::gDaemon;
using Licq::gLog;
using std::string;

static const time_t PING_TIMEOUT = 60;
Licq::Mutex Client::myGlooxMutex;

GlooxClient::GlooxClient(const gloox::JID& jid, const string& password) :
  gloox::Client(jid, password)
{
  // Empty
}

bool GlooxClient::checkStreamVersion(const string& version)
{
  // do not consider absence of the version as an error
  if (version.empty())
    return true;

  return gloox::Client::checkStreamVersion(version);
}

Client::Client(Licq::MainLoop& mainLoop, const Licq::UserId& ownerId,
    const string& username, const string& password, const string& host,
    int port, const string& resource, gloox::TLSPolicy tlsPolicy) :
  myMainLoop(mainLoop),
  myHandler(ownerId),
  mySessionManager(NULL),
  myJid(username + "/" + resource),
  myClient(myJid, password),
  myTcpClient(NULL),
  myRosterManager(myClient.rosterManager()),
  myVCardManager(&myClient)
{
  myClient.registerStanzaExtension(new gloox::VCardUpdate);
  myClient.addPresenceExtension(new gloox::VCardUpdate);

  myClient.registerConnectionListener(this);
  myRosterManager->registerRosterListener(this, false);
  myClient.logInstance().registerLogHandler(
      gloox::LogLevelDebug, gloox::LogAreaAll, this);

  mySessionManager = new SessionManager(myClient, myHandler);
  myClient.registerMessageSessionHandler(mySessionManager);

  myClient.disco()->setIdentity("client", "pc");
  myClient.disco()->setVersion("Licq", LICQ_VERSION_STRING);

  myClient.setTls(tlsPolicy);

  if (!gDaemon.proxyEnabled())
  {
    if (!host.empty())
      myClient.setServer(host);
    if (port > 0)
      myClient.setPort(port);
  }
  else if (gDaemon.proxyType() == Licq::Daemon::ProxyTypeHttp)
  {
    myTcpClient = new gloox::ConnectionTCPClient(
      myClient.logInstance(), gDaemon.proxyHost(), gDaemon.proxyPort());

    std::string server = myClient.server();
    if (!host.empty())
      server = host;

    gloox::ConnectionHTTPProxy* httpProxy =
      new gloox::ConnectionHTTPProxy(
        &myClient, myTcpClient, myClient.logInstance(),
        server, (port > 0 ? port : -1));

    httpProxy->setProxyAuth(gDaemon.proxyLogin(), gDaemon.proxyPasswd());

    myClient.setConnectionImpl(httpProxy);
  }
}

Client::~Client()
{
  myVCardManager.cancelVCardOperations(this);

  {
    // Lock is needed here to protect the calls to gnutls_global_init
    Licq::MutexLocker locker(myGlooxMutex);
    myClient.disconnect();
  }

  // Avoid memory leak in gloox
  myClient.removePresenceExtension(gloox::ExtVCardUpdate);

  delete mySessionManager;
}

int Client::getSocket()
{
  if (myTcpClient != NULL)
    return myTcpClient->socket();
  else
    return static_cast<gloox::ConnectionTCPClient*>(
      myClient.connectionImpl())->socket();
}

void Client::rawFileEvent(int /*fd*/, int /*revents*/)
{
  myClient.recv();
}

void Client::timeoutEvent(int /*id*/)
{
  myClient.whitespacePing();
}

void Client::setPassword(const string& password)
{
  myClient.setPassword(password);
}

bool Client::connect(unsigned status)
{
  // When having multiple accounts connecting at the same time, gloox can
  // SIGSEGV: TLSDefault::init returns false (probably thread race), making
  // ClientBase::getDefaultEncryption delete the TLSDefault instance which
  // deletes GnuTLSBase. The destructor for GnuTLSBase calls cleanup() which
  // does gnutls_bye(*m_session, ...). But m_session == NULL...
  Licq::MutexLocker locker(myGlooxMutex);

  changeStatus(status, false);
  myMainLoop.removeCallback(this);
  if (!myClient.connect(false))
    return false;

  // Register with mainloop for socket monitoring and whitespace ping
  myMainLoop.addRawFile(getSocket(), this);
  myMainLoop.addTimeout(PING_TIMEOUT*1000, this, 0, false);

  return true;
}

bool Client::isConnected()
{
  return myClient.authed();
}

void Client::changeStatus(unsigned status, bool notifyHandler)
{
  // Must reset status to avoid sending the old status message
  myClient.presence().resetStatus();

  string msg = myHandler.getStatusMessage(status);
  myClient.setPresence(statusToPresence(status), 0, msg);
  if (notifyHandler)
    myHandler.onChangeStatus(status);
}

void Client::getVCard(const string& user)
{
  myVCardManager.fetchVCard(gloox::JID(user), this);
}

void Client::setOwnerVCard(const UserToVCard& wrapper)
{
  myPendingPhotoHash = wrapper.pictureSha1();

  gloox::VCard* card = wrapper.createVCard();
  myVCardManager.storeVCard(card, this);
}

void Client::addUser(const string& user, const gloox::StringList& groupNames,
                     bool notify)
{
  if (notify)
    myRosterManager->subscribe(gloox::JID(user), user, groupNames);
  else
    myRosterManager->add(gloox::JID(user), user, groupNames);
}

void Client::changeUserGroups(
    const string& user, const gloox::StringList& groups)
{
  gloox::RosterItem* item = myRosterManager->getRosterItem(gloox::JID(user));
  if (item != NULL)
  {
    item->setGroups(groups);
    myRosterManager->synchronize();
  }
}

void Client::removeUser(const string& user)
{
  myRosterManager->remove(gloox::JID(user));
}

void Client::renameUser(const string& user, const string& newName)
{
  gloox::RosterItem* item = myRosterManager->getRosterItem(gloox::JID(user));
  if (item != NULL)
  {
    item->setName(newName);
    myRosterManager->synchronize();
  }
}

void Client::grantAuthorization(const string& user)
{
  myRosterManager->ackSubscriptionRequest(gloox::JID(user), true);
}

void Client::refuseAuthorization(const string& user)
{
  myRosterManager->ackSubscriptionRequest(gloox::JID(user), false);
}

void Client::requestAuthorization(const string& user, const string& msg)
{
  gloox::Subscription subscription(
      gloox::Subscription::Subscribe, gloox::JID(user), msg);
  myClient.send(subscription);
}

void Client::onConnect()
{
  gloox::ConnectionBase* conn = myClient.connectionImpl();
  myHandler.onConnect(conn->localInterface(), conn->localPort(),
                      presenceToStatus(myClient.presence().subtype()));

  // Fetch the current vCard from the server
  myVCardManager.fetchVCard(myClient.jid().bareJID(), this);
}

bool Client::onTLSConnect(const gloox::CertInfo& /*info*/)
{
  return true;
}

void Client::onDisconnect(gloox::ConnectionError error)
{
  // Socket no longer open, stop monitoring it
  myMainLoop.removeCallback(this);

  bool authError = false;

  switch (error)
  {
    case gloox::ConnNoError:
      break;
    case gloox::ConnStreamError:
      gLog.error("stream error (%d): %s", myClient.streamError(),
          myClient.streamErrorText().c_str());
      break;
    case gloox::ConnStreamVersionError:
      gLog.error("incoming stream version not supported");
      break;
    case gloox::ConnStreamClosed:
      gLog.error("connection closed by the server");
      break;
    case gloox::ConnProxyAuthRequired:
    case gloox::ConnProxyAuthFailed:
    case gloox::ConnProxyNoSupportedAuth:
      gLog.error("proxy authentication failed");
      authError = true;
      break;
    case gloox::ConnIoError:
      gLog.error("connection I/O error");
      break;
    case gloox::ConnParseError:
      gLog.error("XML parse error");
      break;
    case gloox::ConnConnectionRefused:
      gLog.error("server refused connection");
      break;
    case gloox::ConnDnsError:
      gLog.error("could not resolve server hostname");
      break;
    case gloox::ConnOutOfMemory:
      gLog.error("out of memory");
      break;
    case gloox::ConnNoSupportedAuth:
      gLog.error("no supported authentication mechanism");
      break;
    case gloox::ConnTlsFailed:
      gLog.error("TLS veification failed");
      break;
    case gloox::ConnTlsNotAvailable:
      gLog.error("TLS not available");
      break;
    case gloox::ConnCompressionFailed:
      gLog.error("compression error");
      break;
    case gloox::ConnAuthenticationFailed:
      gLog.error("authentication failed (error %d)", myClient.authError());
      authError = true;
      break;
    case gloox::ConnUserDisconnected:
      break;
    case gloox::ConnNotConnected:
      break;
  }
  myHandler.onDisconnect(authError);
}

void Client::handleItemAdded(const gloox::JID& jid)
{
  TRACE("%s", jid.full().c_str());

  gloox::RosterItem* item = myRosterManager->getRosterItem(jid);
  addRosterItem(*item);
}

void Client::handleItemSubscribed(const gloox::JID& jid)
{
  TRACE("%s", jid.full().c_str());

  gLog.info("Now authorized for %s", jid.bare().c_str());
}

void Client::handleItemRemoved(const gloox::JID& jid)
{
  TRACE("%s", jid.full().c_str());

  myHandler.onUserRemoved(jid.bare());
}

void Client::handleItemUpdated(const gloox::JID& jid)
{
  TRACE("%s", jid.full().c_str());

  gloox::RosterItem* item = myRosterManager->getRosterItem(jid);
  addRosterItem(*item);
}

void Client::handleItemUnsubscribed(const gloox::JID& jid)
{
  TRACE("%s", jid.full().c_str());

  gLog.info("No longer authorized for %s", jid.bare().c_str());
}

void Client::handleRoster(const gloox::Roster& roster)
{
  TRACE();

  std::set<string> jidlist;
  gloox::Roster::const_iterator it;

  for (it = roster.begin(); it != roster.end(); ++it)
  {
    if (addRosterItem(*it->second))
      jidlist.insert(it->first);
  }

  myHandler.onRosterReceived(jidlist);
}

void Client::handleRosterPresence(const gloox::RosterItem& item,
                                  const string& /*resource*/,
                                  gloox::Presence::PresenceType presence,
                                  const string& msg)
{
  using namespace gloox;

  TRACE("%s %d", item.jid().c_str(), presence);

  std::string photoHash;

  const RosterItem::ResourceMap& resources = item.resources();
  for (RosterItem::ResourceMap::const_iterator resource = resources.begin();
       photoHash.empty() && resource != resources.end(); ++resource)
  {
    const StanzaExtensionList& extensions = resource->second->extensions();
    for (StanzaExtensionList::const_iterator it = extensions.begin();
         photoHash.empty() && it != extensions.end(); ++it)
    {
      if ((*it)->extensionType() == gloox::ExtVCardUpdate)
      {
        const VCardUpdate* vCardUpdate = dynamic_cast<const VCardUpdate*>(*it);
        if (vCardUpdate != NULL)
          photoHash = vCardUpdate->hash();
      }
    }
  }

  myHandler.onUserStatusChange(
      JID(item.jid()).bare(), presenceToStatus(presence), msg, photoHash);
}

void Client::handleSelfPresence(const gloox::RosterItem& /*item*/,
                                const string& /*resource*/,
                                gloox::Presence::PresenceType /*presence*/,
                                const string& /*msg*/)
{
  TRACE();
}

bool Client::handleSubscriptionRequest(
    const gloox::JID& jid, const string& msg)
{
  TRACE();

  myHandler.onUserAuthorizationRequest(jid.bare(), msg);
  return false; // Ignored by gloox
}

bool Client::handleUnsubscriptionRequest(
    const gloox::JID& /*jid*/, const string& /*msg*/)
{
  TRACE();

  return false; // Ignored by gloox
}

void Client::handleNonrosterPresence(const gloox::Presence& /*presence*/)
{
  TRACE();
}

void Client::handleRosterError(const gloox::IQ& /*iq*/)
{
  TRACE();
}

void Client::handleLog(gloox::LogLevel level, gloox::LogArea area,
                       const string& message)
{
  const char* areaStr = "Area ???";
  switch (area)
  {
    case gloox::LogAreaClassParser:
      areaStr = "Parser";
      break;
    case gloox::LogAreaClassConnectionTCPBase:
      areaStr = "TCP base";
      break;
    case gloox::LogAreaClassClient:
      areaStr = "Client";
      break;
    case gloox::LogAreaClassClientbase:
      areaStr = "Client base";
      break;
    case gloox::LogAreaClassComponent:
      areaStr = "Component";
      break;
    case gloox::LogAreaClassDns:
      areaStr = "DNS";
      break;
    case gloox::LogAreaClassConnectionHTTPProxy:
      areaStr = "HTTP proxy";
      break;
    case gloox::LogAreaClassConnectionSOCKS5Proxy:
      areaStr = "SOCKS5 proxy";
      break;
    case gloox::LogAreaClassConnectionTCPClient:
      areaStr = "TCP client";
      break;
    case gloox::LogAreaClassConnectionTCPServer:
      areaStr = "TCP server";
      break;
    case gloox::LogAreaClassS5BManager:
      areaStr = "SOCKS5";
      break;
    case gloox::LogAreaClassSOCKS5Bytestream:
      areaStr = "SOCKS5 bytestream";
      break;
    case gloox::LogAreaClassConnectionBOSH:
      areaStr = "BOSH";
      break;
    case gloox::LogAreaClassConnectionTLS:
      areaStr = "TLS";
      break;
    case gloox::LogAreaXmlIncoming:
      areaStr = "XML in";
      break;
    case gloox::LogAreaXmlOutgoing:
      areaStr = "XML out";
      break;
    case gloox::LogAreaUser:
      areaStr = "User";
      break;
    case gloox::LogAreaAllClasses:
    case gloox::LogAreaAll:
      areaStr = "All";
      break;
  }

  switch (level)
  {
    case gloox::LogLevelDebug:
      gLog.debug("[%s] %s", areaStr, message.c_str());
      break;
    default:
    case gloox::LogLevelWarning:
      gLog.warning("[%s] %s", areaStr, message.c_str());
      break;
    case gloox::LogLevelError:
      gLog.error("[%s] %s", areaStr, message.c_str());
      break;
  }
}

void Client::handleVCard(const gloox::JID& jid, const gloox::VCard* vcard)
{
  TRACE();

  if (vcard != NULL)
  {
    VCardToUser user(vcard);
    myHandler.onUserInfo(jid.bare(), user);

    if (jid.bare() == myClient.jid().bare())
      broadcastPhotoHash(user.pictureSha1());
  }
}

void Client::handleVCardResult(gloox::VCardHandler::VCardContext context,
                               const gloox::JID& jid, gloox::StanzaError error)
{
  TRACE();

  if (error != gloox::StanzaErrorUndefined)
  {
    gLog.warning("%s vCard for user %s failed with error %u",
        context == gloox::VCardHandler::StoreVCard ? "Storing" : "Fetching",
        jid ? jid.bare().c_str() : myClient.jid().bare().c_str(), error);
  }

  if (!jid && context == gloox::VCardHandler::StoreVCard)
  {
    if (error == gloox::StanzaErrorUndefined)
      broadcastPhotoHash(myPendingPhotoHash);
    else
      broadcastPhotoHash(boost::none);

    myPendingPhotoHash = boost::none;
  }
}

void Client::broadcastPhotoHash(const boost::optional<std::string>& hash)
{
  TRACE();

  if (hash)
  {
    if (hash->empty())
    {
      // Bug in gloox: if the hash is empty then VCardUpdate will not generate
      // a empty photo tag as it should.
      gloox::VCardUpdate card("dummy");

      gloox::Tag* tag = card.tag();
      tag->removeChild("photo");
      new gloox::Tag(tag, "photo");

      myClient.addPresenceExtension(new gloox::VCardUpdate(tag));
      delete tag;
    }
    else
      myClient.addPresenceExtension(new gloox::VCardUpdate(*hash));
  }
  else
  {
    myClient.addPresenceExtension(new gloox::VCardUpdate);
  }

  myClient.setPresence();
}

bool Client::addRosterItem(const gloox::RosterItem& item)
{
  // Filter out the states where the contact should not be on our list
  if (item.subscription() == gloox::S10nNoneIn
      || item.subscription() == gloox::S10nFrom)
    return false;

  // States where we have sent a subscription request that hasn't be answered
  bool awaitAuth = item.subscription() == gloox::S10nNoneOut
      || item.subscription() == gloox::S10nNoneOutIn
      || item.subscription() == gloox::S10nFromOut;

  myHandler.onUserAdded(item.jid(), item.name(), item.groups(), awaitAuth);
  return true;
}

unsigned Client::presenceToStatus(gloox::Presence::PresenceType presence)
{
  switch (presence)
  {
    case gloox::Presence::Invalid:
    case gloox::Presence::Probe:
    case gloox::Presence::Error:
    case gloox::Presence::Unavailable:
      return User::OfflineStatus;

    case gloox::Presence::Chat:
      return User::OnlineStatus | User::FreeForChatStatus;

    case gloox::Presence::Away:
      return User::OnlineStatus | User::AwayStatus;

    case gloox::Presence::DND:
      return User::OnlineStatus | User::DoNotDisturbStatus;

    case gloox::Presence::XA:
      return User::OnlineStatus | User::NotAvailableStatus;

    case gloox::Presence::Available:
    default:
      return User::OnlineStatus;
  }
}

gloox::Presence::PresenceType Client::statusToPresence(unsigned status)
{
  if (status == User::OfflineStatus)
    return gloox::Presence::Unavailable;

  if (status & User::AwayStatus)
    return gloox::Presence::Away;

  if (status & User::NotAvailableStatus)
    return gloox::Presence::XA;

  if (status & (User::OccupiedStatus | User::DoNotDisturbStatus))
    return gloox::Presence::DND;

  if (status & User::FreeForChatStatus)
    return gloox::Presence::Chat;

  return gloox::Presence::Available;
}
