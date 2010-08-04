/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
 * Copyright (C) 2006-2010 Licq developers
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

#include "licqkimiface.h"

#include "config.h"

#include <boost/foreach.hpp>

// Qt includes
#include <QByteArray>
#include <QPixmap>
#include <QTextCodec>

// KDE includes
#include <kdebug.h>
#include "kdeversion.h"
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

// Licq includes
#include <licq/contactlist/usermanager.h>

// local includes
#include "helpers/usercodec.h"
#include "mainwin.h"

using namespace LicqQtGui;

///////////////////////////////////////////////////////////////////////////////

LicqKIMIface::LicqKIMIface(const QByteArray& appId, QObject* parent, const char* name)
    : DCOPObject("KIMIface"), QObject(parent, name),
      m_dcopAppID(appId)
{
}

///////////////////////////////////////////////////////////////////////////////

LicqKIMIface::~LicqKIMIface()
{
    saveIDMapping();
}

///////////////////////////////////////////////////////////////////////////////

QStringList LicqKIMIface::allContacts()
{
    // workaround for missing QSet
    QMap<QString, bool> contactSet;

  Licq::UserListGuard userList;
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    QString id = user->accountId().c_str();
    QString kabcID = kabcIDForUser(id, user->protocolId());
        if (!kabcID.isEmpty())
        {
/*            kdDebug() << user->getAlias().c_str()
                      << ": Licq ID=(" << user->protocolId() << ", " << id << ")"
                      << "KABC ID=" << kabcID << endl;*/
            contactSet.insert(kabcID, true);
        }
  }

    QStringList contacts;
    QMap<QString, bool>::const_iterator it    = contactSet.begin();
    QMap<QString, bool>::const_iterator endIt = contactSet.end();
    for (; it != endIt; ++it)
    {
        contacts << it.key();
    }

    return contacts;
}

///////////////////////////////////////////////////////////////////////////////

QStringList LicqKIMIface::reachableContacts()
{
  bool offline;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    offline = o->StatusOffline();
  }

    if (offline)
        return QStringList();
    else
        return allContacts();
}

///////////////////////////////////////////////////////////////////////////////

QStringList LicqKIMIface::onlineContacts()
{
    // workaround for missing QSet
    QMap<QString, bool> contactSet;

  Licq::UserListGuard userList;
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    Licq::UserReadGuard u(user);
    if (u->StatusOffline())
      continue;

    QString id = u->accountId().c_str();
    QString kabcID = kabcIDForUser(id, u->protocolId());
            if (!kabcID.isEmpty())
            {
/*                kdDebug() << u->getAlias().c_str()
                        << ": Licq ID=(" << u->protocolId() << ", " << id << ")"
                        << "KABC ID=" << kabcID << endl;*/
                contactSet.insert(kabcID, true);
            }
  }

    QStringList contacts;
    QMap<QString, bool>::const_iterator it    = contactSet.begin();
    QMap<QString, bool>::const_iterator endIt = contactSet.end();
    for (; it != endIt; ++it)
    {
        contacts << it.key();
    }

    return contacts;
}

///////////////////////////////////////////////////////////////////////////////

QStringList LicqKIMIface::fileTransferContacts()
{
    return onlineContacts();
}

///////////////////////////////////////////////////////////////////////////////

bool LicqKIMIface::isPresent(const QString& uid)
{
    // check if we have a mapping, if we do, check if user still exists
    if (m_kabc2Licq.find(uid) == m_kabc2Licq.end()) return false;

    QStringList all = allContacts();

    return all.find(uid) != all.end();
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::displayName(const QString& uid)
{
    // check if we have a mapping, if we do, check user alias
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return QString::null;

  Licq::UserId userId(licqID.accountId().c_str(), PPID);
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return QString;

/*        kdDebug() << u->getAlias().c_str()
                    << ": Licq ID=(" << u->protocolId() << ", " << licqID << ")"
                    << "KABC ID=" << kabcID << endl;*/
  const QTextCodec* codec = UserCodec::codecForUser(*u);
  return codec->toUnicode(u->getAlias().c_str());
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::presenceString(const QString& uid)
{
    // check if we have a mapping, if we do, check user status string
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return QString::null;

  Licq::UserId userId(licqID.accountId().c_str(), PPID);
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return QString;

/*        kdDebug() << u->getAlias().c_str()
                    << ": Licq ID=(" << u->protocolId() << ", " << licqID << ")"
                    << "KABC ID=" << kabcID << endl;*/
  return u->statusString().c_str();
}

///////////////////////////////////////////////////////////////////////////////

int LicqKIMIface::presenceStatus(const QString& uid)
{
    // check if we have a mapping, if we do, check user status
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return 0; // unkown

  Licq::UserId userId(licqID.accountId().c_str(), PPID);
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return QString;

/*        kdDebug() << u->getAlias().c_str()
                    << ": Licq ID=(" << u->protocolId() << ", " << licqID << ")"
                    << "KABC ID=" << kabcID << endl;*/

  if (u->status() == Licq::User::OfflineStatus)
    return 1; // offline
  if (u->status() & Licq::User::AwayStatuses)
    return 3; // away
  return 4; // online
}

///////////////////////////////////////////////////////////////////////////////

bool LicqKIMIface::canReceiveFiles(const QString& uid)
{
    QStringList fileContacts = fileTransferContacts();

    return fileContacts.find(uid) != fileContacts.end();
}

///////////////////////////////////////////////////////////////////////////////

bool LicqKIMIface::canRespond(const QString& uid)
{
    QStringList onlines = onlineContacts();

    return onlines.find(uid) != onlines.end();
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::locate(const QString& contactId, const QString& protocol)
{
    unsigned long PPID = idForProtocol(protocol);

    if (PPID == 0) return QString::null;

  Licq::UserId userId(contactId.accountId().c_str(), PPID);
  if (!Licq::gUserManager.userExists(userId))
    return QString;

  QString kabcID = kabcIDForUser(id, PPID);
/*        kdDebug() << u->getAlias().c_str()
                    << ": Licq ID=(" << u->protocolId() << ", " << contactId << ")"
                    << "KABC ID=" << kabcID << endl;*/

    return kabcID;
}

///////////////////////////////////////////////////////////////////////////////

QPixmap LicqKIMIface::icon(const QString& uid)
{
    // check if we have a mapping, if we do, look for user
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return QPixmap();

  Licq::UserId userId(licqID.toLatin1().data(), PPID);
  unsigned status = 0;

  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return QPixmap();
    status = u->status();
  }

  return CMainWindow::iconForStatus(status, userId);
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::context(const QString& uid)
{
    // context not supported
    Q_UNUSED(uid);
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QStringList LicqKIMIface::protocols()
{
    QStringList result;

    QMap<QString, unsigned long>::const_iterator it    = m_protoName2ID.begin();
    QMap<QString, unsigned long>::const_iterator endIt = m_protoName2ID.end();
    for(; it != endIt; ++it)
    {
        result << it.key();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::messageContact(const QString& uid, const QString& message)
{
    // check if we have a mapping, if we do, look for user
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return;

  Licq::UserId userId(licqID.toLatin1().data(), PPID);
  if (!Licq::gUserManager.userExists(userId))
    return;

  emit sendMessage(licqID.latin1(), PPID, message);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::messageNewContact(const QString& contactId, const QString& protocol)
{
    // check input values
    if (contactId.isEmpty() || protocol.isEmpty()) return;

    // check if we know about the protocol
    unsigned long PPID = idForProtocol(protocol);
    if (PPID == 0) return;

  Licq::UserId userId(contactId.toLatin1().data(), PPID);

    // check if user exists
  if (Licq::gUserManager.userExists(userId))
        emit sendMessage(contactId.latin1(), PPID, QString::null);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::chatWithContact(const QString& uid)
{
    // check if we have a mapping, if we do, look for user
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return;

  Licq::UserId userId(licqID.toLatin1().data(), PPID);
  if (!Licq::gUserManager.userExists(userId))
    return;

  emit sendChatRequest(licqID.latin1(), PPID);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::sendFile(const QString& uid, const KURL& sourceURL,
                            const QString& altFileName, uint fileSize)
{
    Q_UNUSED(fileSize)

    if (!sourceURL.isLocalFile()) return;

    // check if we have a mapping, if we do, look for user
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;

    if (licqID.isEmpty()) return;

  Licq::UserId userId(licqID.toLatin1().data(), PPID);
  if (!Licq::gUserManager.userExists(userId))
    return;

  emit sendFileTransfer(licqID.latin1(), PPID, sourceURL.path(), altFileName);
}

///////////////////////////////////////////////////////////////////////////////

bool LicqKIMIface::addContact(const QString& contactId,
                              const QString& protocol)
{
    // check input values
    if (contactId.isEmpty() || protocol.isEmpty()) return false;

    // check if we know about the protocol
    unsigned long PPID = idForProtocol(protocol);
    if (PPID == 0) return false;

  Licq::UserId userId(contactId.toLatin1().data(), PPID);

    // check if user already exists
  if (Licq::gUsermanager.userExists(userId))
        return false;

  emit addUser(userId);

    // assume the user was added
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::addProtocol(const QString& name, unsigned long PPID)
{
    if (name.isEmpty()) return;

    // normalize name for KIMIface usage
  QString protocol = name.upper();
    protocol.append("Protocol");

    // set mapping of name to PPID
    m_protoName2ID[protocol] = PPID;

    // load user ID mappings
    loadIDMapping(protocol);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::removeProtocol(unsigned long PPID)
{
    // write current mapping
    saveIDMapping();

    // clear mappings for protocol
    QMap<QString, QString> map = m_licq2KABC[PPID];
    map.clear();
    m_licq2KABC[PPID] = map;

    // clear reverse mapping
    m_kabc2Licq.clear();

    // iterate over the protocols and load the remaining mappings
    QMap<QString, unsigned long>::const_iterator protoIt    = m_protoName2ID.begin();
    QMap<QString, unsigned long>::const_iterator protoEndIt = m_protoName2ID.end();

    QString protocol;
    for (; protoIt != protoEndIt; ++protoIt)
    {
        if (protoIt.data() == PPID)
            protocol = protoIt.key();
        else
            loadIDMapping(protoIt.key());
    }

    m_protoName2ID[protocol] = 0;
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::userStatusChanged(const char* szId, unsigned long PPID)
{
    if (szId == 0) return;

    QString kabcID = kabcIDForUser(szId, PPID);
    if (kabcID.isEmpty()) return; // no mapping

    int status = presenceStatus(kabcID);

    // trigger DCOP signal
    contactPresenceChanged(kabcID, m_dcopAppID, status);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::loadIDMapping(const QString& protocol)
{
//    kdDebug() << "LicqKIMIface::loadIDMapping(" << protocol << ")" << endl;

    QString dataDir = locateLocal("data", "licq/idmapping");

    KSimpleConfig mappingFile(dataDir);

    QMap<QString, QString> map = mappingFile.entryMap(protocol);

    QMap<QString, QString>::const_iterator it    = map.begin();
    QMap<QString, QString>::const_iterator endIt = map.end();

    for(; it != endIt; ++it)
    {
        setKABCIDForUser(it.key(), idForProtocol(protocol), it.data());
    }
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::saveIDMapping()
{
//    kdDebug() << "LicqKIMIface::saveIDMapping()" << endl;

    QString dataDir = locateLocal("data", "licq/idmapping");

    KSimpleConfig mappingFile(dataDir);

    QMap<QString, unsigned long>::const_iterator protoIt    = m_protoName2ID.begin();
    QMap<QString, unsigned long>::const_iterator protoEndIt = m_protoName2ID.end();

    for (; protoIt != protoEndIt; ++protoIt)
    {
        // only protocols with valid PPID
        if (protoIt.data() == 0) continue;

        mappingFile.setGroup(protoIt.key());

        const QMap<QString, QString>& map = m_licq2KABC[protoIt.data()];
        QMap<QString, QString>::const_iterator it    = map.begin();
        QMap<QString, QString>::const_iterator endIt = map.end();

        for(; it != endIt; ++it)
        {
            if (!it.data().isEmpty())
            {
                mappingFile.writeEntry(it.key(), it.data());
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::kabcIDForUser(const QString& licqID, unsigned long PPID) const
{
    if (licqID.isEmpty()) return QString::null;

    return (m_licq2KABC[PPID])[licqID];
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::setKABCIDForUser(const QString& licqID, unsigned long PPID,
                                    const QString& kabcID)
{
    if (licqID.isEmpty()) return;

    if (!kabcID.isEmpty())
        m_kabc2Licq[kabcID] = qMakePair(PPID, licqID);

    (m_licq2KABC[PPID])[licqID] = kabcID;
}
