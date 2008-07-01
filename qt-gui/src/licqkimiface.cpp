/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include "kdeversion.h"

// Qt includes
#include <qtextcodec.h>

// KDE includes
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

// Licq includes
#include "licq_user.h"

// KDE-Plugin includes
#include "usercodec.h"

// local includes
#include "licqkimiface.h"
#include "mainwin.h"
#include "gui-strings.h"

///////////////////////////////////////////////////////////////////////////////

LicqKIMIface::LicqKIMIface(const QCString& appId, QObject* parent, const char* name)
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
    
    FOR_EACH_USER_START(LOCK_R)
    QString id = pUser->IdString();
    if (!id.isEmpty())
    {
        QString kabcID = kabcIDForUser(id, pUser->PPID());
        if (!kabcID.isEmpty())
        {
/*            kdDebug() << pUser->GetAlias()
                      << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                      << "KABC ID=" << kabcID << endl;*/
            contactSet.insert(kabcID, true);
        }
    }
    FOR_EACH_USER_END

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
    ICQOwner* owner = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    bool offline = owner->StatusOffline();
    gUserManager.DropOwner(owner);

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
    
    FOR_EACH_USER_START(LOCK_R)
    if (!pUser->StatusOffline())
    {
        QString id = pUser->IdString();
        if (!id.isEmpty())
        {
            QString kabcID = kabcIDForUser(id, pUser->PPID());
            if (!kabcID.isEmpty())
            {
/*                kdDebug() << pUser->GetAlias()
                        << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                        << "KABC ID=" << kabcID << endl;*/
                contactSet.insert(kabcID, true);
            }
        }
    }
    FOR_EACH_USER_END

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

    QString name;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    QString id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
/*        kdDebug() << pUser->GetAlias()
                    << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                    << "KABC ID=" << kabcID << endl;*/
        QTextCodec* codec = UserCodec::codecForICQUser(pUser);
        QCString rawName = pUser->GetAlias();
        name = codec->toUnicode(rawName);
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    return name;
}

///////////////////////////////////////////////////////////////////////////////

QString LicqKIMIface::presenceString(const QString& uid)
{
    // check if we have a mapping, if we do, check user status string
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;
    
    if (licqID.isEmpty()) return QString::null;

    QString status;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    QString id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
/*        kdDebug() << pUser->GetAlias()
                    << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                    << "KABC ID=" << kabcID << endl;*/
        status = Strings::getStatus(pUser);
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    return status;
}

///////////////////////////////////////////////////////////////////////////////

int LicqKIMIface::presenceStatus(const QString& uid)
{
    // check if we have a mapping, if we do, check user status
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;
    
    if (licqID.isEmpty()) return 0; // unkown

    int status = 0; // unkown

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    QString id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
/*        kdDebug() << pUser->GetAlias()
                    << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                    << "KABC ID=" << kabcID << endl;*/
        switch (pUser->Status())
        {
            case ICQ_STATUS_OFFLINE:
                status = 1; // offline
                break;

            case ICQ_STATUS_ONLINE:
            case ICQ_STATUS_FREEFORCHAT:
                status = 4; // online
                break;

            case ICQ_STATUS_AWAY:
            case ICQ_STATUS_NA:
            case ICQ_STATUS_DND:
            case ICQ_STATUS_OCCUPIED:
                status = 3; // away
                break;

            default:
                status = 0; // unknown
                break;
        }
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    return status;
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

    QString kabcID;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    QString uin = pUser->IdString();
    if (!uin.isEmpty() && uin == contactId)
    {
        QString id = pUser->IdString();
        kabcID = kabcIDForUser(id, PPID);
/*        kdDebug() << pUser->GetAlias()
                    << ": Licq ID=(" << pUser->PPID() << ", " << id << ")"
                    << "KABC ID=" << kabcID << endl;*/
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

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

    QString id;
    unsigned long fullStatus = 0;
    bool found = false;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
        fullStatus = pUser->StatusFull();
        found = true;
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    if (!found) return QPixmap();

    return CMainWindow::iconForStatus(fullStatus, id.latin1(), PPID);
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

    QString id;
    bool found = false;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
        found = true;
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    if (!found) return;

    emit sendMessage(id.latin1(), PPID, message);
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::messageNewContact(const QString& contactId, const QString& protocol)
{
    // check input values
    if (contactId.isEmpty() || protocol.isEmpty()) return;

    // check if we know about the protocol
    unsigned long PPID = idForProtocol(protocol);
    if (PPID == 0) return;

    // check if user exists
    ICQUser* pUser = gUserManager.FetchUser(contactId.latin1(), PPID, LOCK_R);
    if (pUser != 0)
    {
        gUserManager.DropUser(pUser);

        emit sendMessage(contactId.latin1(), PPID, QString::null);
    }
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::chatWithContact(const QString& uid)
{
    // check if we have a mapping, if we do, look for user
    QPair<unsigned long, QString> licqUser = m_kabc2Licq[uid];
    unsigned long PPID = licqUser.first;
    QString licqID     = licqUser.second;
    
    if (licqID.isEmpty()) return;

    QString id;
    bool found = false;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
        found = true;
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    if (!found) return;

    emit sendChatRequest(id.latin1(), PPID);
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

    QString id;
    bool found = false;

    FOR_EACH_PROTO_USER_START(PPID, LOCK_R)
    id = pUser->IdString();
    if (!id.isEmpty() && id == licqID)
    {
        found = true;
        FOR_EACH_PROTO_USER_BREAK
    }
    FOR_EACH_PROTO_USER_END

    if (!found) return;

    emit sendFileTransfer(id.latin1(), PPID, sourceURL.path(), altFileName);
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

    // check if user already exists
    ICQUser* pUser = gUserManager.FetchUser(contactId.latin1(), PPID, LOCK_R);
    if (pUser != 0)
    {
        gUserManager.DropUser(pUser);
        return false;
    }

    emit addUser(contactId.latin1(), PPID);

    // assume the user was added
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void LicqKIMIface::addProtocol(const QString& name, unsigned long PPID)
{
    if (name.isEmpty()) return;

    // normalize name for KIMIface usage
    QString protocol = (name == "Licq" ? "ICQ" : name.upper());
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

#include "licqkimiface.moc"

#endif // USE_KDE
// End of file
