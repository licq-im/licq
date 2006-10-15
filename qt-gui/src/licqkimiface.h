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

#ifndef LICQKIMIFACE_H
#define LICQKIMIFACE_H

// Qt includes
#include <qmap.h>
#include <qobject.h>
#include <qpair.h>

// KDE includes
#include "kimiface.h"

/**
 * Licq implementation of the generic DCOP interface for KDE instant messenger applications
 */
class LicqKIMIface : public QObject, public KIMIface
{
    Q_OBJECT
public:

    LicqKIMIface(const QCString& appid, QObject* parent = 0, const char* name = 0);
    virtual ~LicqKIMIface();

    /**
     * Obtain a list of IM contacts entries.
     * @return a list of KABC uids.
     */
    virtual QStringList allContacts();
    
    /**
     * Obtain a list of IM contacts who are currently reachable.
     * @return a list of KABC uids who can receive a message, even if offline.
     */
    virtual QStringList reachableContacts();
    
    /**
     * Obtain a list of IM contacts who are currently online.
     * @return a list of KABC uids who are online with unspecified presence.
     */
    virtual QStringList onlineContacts();
    
    /**
     * Obtain a list of IM contacts who may receive file transfers.
     * @return a list of KABC uids capable of file transfer.
     */
    virtual QStringList fileTransferContacts();

    /**
     * Confirm if a given KABC uid is known to us.
     * @param uid the KABC uid you are interested in.
     * @return whether the program knows of this KABC uid.
     */
    virtual bool isPresent(const QString& uid);
    
    /**
     * Obtain the IM app's idea of the contact's display name
     * Useful if KABC lookups may be too slow
     * @param uid the KABC uid you are interested in.
     * @return The corresponding display name.
     */
    virtual QString displayName(const QString& uid);
    
    /**
     * Obtain the IM presence as a i18ned string for the specified contact
     * @param uid the KABC uid you want the presence for.
     * @return the i18ned string describing presence.
     */
    virtual QString presenceString(const QString& uid);
    
    /**
     * Obtain the IM presence as a number (see KIMIface) for the specified contact
     * @param uid the KABC uid you want the presence for.
     * @return a numeric representation of presence - currently one of
     * 0 (Unknown), 1 (Offline), 2 (Connecting), 3 (Away), 4 (Online)
     */
    virtual int presenceStatus(const QString& uid);
    
    /**
     * Indicate if a given contact can receive files
     * @param uid the KABC uid you are interested in.
     * @return Whether the specified addressee can receive files.
     */
    virtual bool canReceiveFiles(const QString& uid);
    
    /**
     * Some media are unidirectional (eg, sending SMS via a web interface).
     * This refers to the contact's ability to respond as defined by the medium,
     * not by their presence.
     * Someone may appear offline (SMS has no presence) to you but in fact be able to respond.
     * @param uid the KABC uid you are interested in.
     * @return Whether the specified addressee can respond.
     */
    virtual bool canRespond(const QString& uid);
    
    /**
     * Get the KABC uid corresponding to the supplied IM address
     * @param contactId the protocol specific identifier for the contact,
     * eg UIN for ICQ, screenname for AIM, nick for IRC.
     * @param protocol the protocol, eg one of "AIMProtocol", "MSNProtocol", "ICQProtocol"
     * @return a KABC uid or null if none found/protocol not supported
     */
    virtual QString locate(const QString& contactId, const QString& protocol);
    
    /**
     * Obtain the icon representing IM presence for the specified contact
     * @param uid the KABC uid you want the presence for.
     * @return a pixmap representing the uid's presence.
     */
    virtual QPixmap icon(const QString& uid);
    
    /**
     * Get the supplied contact's current context (home, work, or any).
     * @param uid the KABC uid you want the context for.
     * @return A QString describing the context, or null if not supported.
     */
    virtual QString context(const QString& uid);

    /**
     * Discover what protocols the application supports
     * @return the set of protocols that the application supports
     */
    virtual QStringList protocols();

    /**
     * Send a single message to the specified contact
     * Any response will be handled by the IM client as a normal
     * conversation.
     * @param uid the KABC uid you want to send to.
     * @param message the message to send.
     */
    virtual void messageContact(const QString& uid, const QString& message);

    /**
     * Open a chat or message for a contact not yet listed in allContacts().
     * @param contactId the protocol specific identifier, see locate() or addContact().
     * @param protocol the protocol name, see locate() or addContact().
     */
    virtual void messageNewContact(const QString& contactId, const QString& protocol);

    /**
     * Start a chat session with the specified contact
     * @param uid the KABC uid you want to chat with.
     */
    virtual void chatWithContact(const QString& uid);

    /**
     * Send the file to the contact
     * @param uid the KABC uid you are sending to.
     * @param sourceURL a @see KURL to send.
     * @param altFileName an alternate filename describing the file or description
     * @param fileSize file size in bytes
     */
    virtual void sendFile(const QString& uid, const KURL& sourceURL,
                          const QString& altFileName = QString::null,
                          uint fileSize = 0);

    /**
     * Add a contact to the contact list
     * @param contactId the protocol specific identifier for the contact
     * eg UIN for ICQ, screenname for AIM, nick for IRC.
     * @param protocol the protocol, eg one of "AIMProtocol", "MSNProtocol", "ICQProtocol", ...
     * @return whether the adding succeeded. False may signal already present,
     * protocol not supported, or add operation not supported.
     */
    virtual bool addContact(const QString& contactId, const QString& protocol);

// public API of the interface for use by Licq
public:
    /**
     * Maps an Licq user identifier (IdString) to a KABC uid.
     * @param szId the Licq user identifier.
     * @param PPID the Licq protocol identifier
     * @return the KABC identifier of the given contact or QString::null.
     */
    inline QString kabcIDForUser(const char* szId, unsigned long PPID) const
    {
        if (szId == 0) return QString::null;

        QString id = szId;
        return kabcIDForUser(id, PPID);
    }

    /**
     * Sets a KABC uid for a given Licq user identifier.
     * @param szId the Licq user identifier.
     * @param PPID the Licq protocol identifier
     * @param kabcID the KABC uid for the given user. If empty clears the mapping
     * in both directions.
     */
    inline void setKABCIDForUser(const char* szId, unsigned long PPID, const QString& kabcID)
    {
        if (szId == 0) return;
        
        QString id = szId;
        setKABCIDForUser(id, PPID, kabcID);
    }

    /**
     * Tells the DCOP handler about a new protocol being available to Licq.
     * @param name the protocol name
     * @param PPID the numerical protocol ID
     */
    void addProtocol(const QString& name, unsigned long PPID);

    /**
     * Tells the DCOP handler to remove information about a protocol.
     * Will write all available data to its presistant file and
     * reload the information for the remaining protocols.
     * @param PPID the numerical protocol ID
     */
    void removeProtocol(unsigned long PPID);

    /**
     * Tells the DCOP handler about a change of a user's status.
     * @param szId the Licq user identifier
     * @param PPID the Licq protocol identifier
     */
    void userStatusChanged(const char* szId, unsigned long PPID);
    
signals:
    /**
     * Asks for sending of a message to a given Licq user.
     * @param id the Licq user identifier (i.e. IdString, szId).
     * @param PPID the Licq protocol ID.
     * @param message the message to send.
     */
    void sendMessage(const char* id, unsigned long PPID, const QString& message);
    
    /**
     * Asks for sending of a file transfer request to a given Licq user.
     * @param id the Licq user identifier (i.e. IdString, szId).
     * @param PPID the Licq protocol ID.
     * @param filename the file to send.
     * @param description am optional decription of the file
     */
    void sendFileTransfer(const char* id, unsigned long PPID,
                          const QString& filename, const QString& description);
                          
    /**
     * Asks for sending of a chat request/invitation to a given Licq user.
     * @param id the Licq user identifier (i.e. IdString, szId).
     * @param PPID the Licq protocol ID.
     */
    void sendChatRequest(const char* id, unsigned long PPID);

    /**
     * Asks for adding a user to the contact list
     * @param szId the protocol specific user ID, for example UIN string in ICQ
     * @param PPID the numerical protocol identifier
     */
    void addUser(const char* szId, unsigned long PPID);
    
private:
    /**
     * Loads the ID mapping file which knows the mapping from Licq IDs to
     * KABC IDs.
     * The mapping file is stored in the user's kde data directory in the
     * subdirectory licq and is a simple INI-style config file with pairs
     * licqid=kabcid, one config group per protocol
     */
    void loadIDMapping(const QString& protocol);

    /**
     * Saves the ID mapping to its file. Keys with empty values, i.e.
     * Licq IDs with empty KABC IDs, are skipped.
     */
    void saveIDMapping();

    /**
     * Helper method for mapping protocol strings to Licq's protocol numbers.
     */
    inline unsigned long idForProtocol(const QString& protocol)
    {
        if (protocol.isEmpty()) return 0;

        return m_protoName2ID[protocol];
    }

    /**
     * Helper method.
     */
    QString kabcIDForUser(const QString& licqID, unsigned long PPID) const;
    
    /**
     * Helper method.
     */
    void setKABCIDForUser(const QString& licqID, unsigned long PPID, const QString& kabcID);
    
private:
    QCString m_dcopAppID;

    QMap<unsigned long, QMap<QString, QString> > m_licq2KABC;
    QMap<QString, QPair<unsigned long, QString> > m_kabc2Licq;
    
    QMap<QString, unsigned long> m_protoName2ID;
};

#endif // LICQKIMIFACE_H

// End of file
