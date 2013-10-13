/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
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
#ifndef LICQDBUS_DBUSINTERFACE_H
#define LICQDBUS_DBUSINTERFACE_H

#include <string>

#include <boost/noncopyable.hpp>
#include <licq/macro.h>

struct DBusMessage;
struct DBusMessageIter;

namespace Licq
{
class MainLoop;
}

namespace LicqDbus
{

/**
 * Callback functions for D-Bus events
 */
class DbusCallback
{
public:
  /**
   * Callback when connection to messages bus is established
   */
  virtual void dbusConnected();

  /**
   * Callback when connection to message bus is lost or terminated
   */
  virtual void dbusDisconnected();

  /**
   * Callback for incoming method calls
   *
   * @param path Object path message was sent to
   * @param iface Interface message was sent to
   * @param member Method invoked
   * @param msgref Message reference for sending reply
   * @param argref Argument reference for call DbusInterface::getMessageParam
   * @param fmt Message format or NULL if no arguments
   * @return One of DbusInterface::ErrorType
   */
  virtual int dbusMethod(const char* path, const char* iface, const char* member,
      DBusMessage* msgref, DBusMessageIter* argref, const char* fmt);

  /**
   * Callback for incoming signals
   *
   * @param path Object path signal was sent to
   * @param iface Interface signal was sent to
   * @param member Signal name
   * @param msgref Message reference for sending reply
   * @param argref Argument reference for call DbusInterface::getMessageParam
   * @param fmt Message format or NULL if no arguments
   */
  virtual void dbusSignal(const char* path, const char* iface, const char* member,
      DBusMessage* msgref, DBusMessageIter* argref, const char* fmt);

  /**
   * Callback to return introspect data
   *
   * @param path Object path to return data for
   * @return Introspect data excluding outer layer and standard parts
   */
  virtual std::string dbusIntrospect(const char* path) = 0;
};


/**
 * Generic D-Bus interface for usage with Licq::MainLoop
 */
class DbusInterface : private boost::noncopyable
{
public:
  /**
   * Encode characters not allowed in object paths parts
   *
   * @param s Unencoded object path part
   * @return String with invalid characters replaced
   */
  static std::string encodeObjectPathPart(const std::string& s);

  /**
   * Decode object path part into original string
   *
   * @param s Encoded object path part
   * @return String with encoded characters restored
   */
  static std::string decodeObjectPathPart(const std::string& s);

  /**
   * Constructor
   *
   * @param mainLoop Mainloop to service D-Bus
   */
  DbusInterface(Licq::MainLoop& mainLoop, DbusCallback* callback);

  /**
   * Destructor
   */
  ~DbusInterface();

  /**
   * Connect to D-Bus
   *
   * @return True on success
   */
  bool connect();

  /**
   * Disconnect from D-Bus
   */
  void disconnect();

  /**
   * Block until all outgoing messages are sent
   */
  void flush();

  /**
   * Request a name on the message bus
   *
   * @param name to requst
   * @return True if name was obtained
   */
  bool requestName(const std::string& name);

  /**
   * Get next message string parameter
   *
   * @param argref Message reference supplied to callback
   * @param ret Pointer to return parameter in (const char** or int*)
   */
  void getNextMessageParamValue(DBusMessageIter* argref, void* ret);

  /**
   * Send a signal
   *
   * @param path Object name
   * @param iface Interface name
   * @param name Signal name
   * @param fmt Format of parameters or NULL for none
   */
  void sendSignal(const std::string& path, const std::string& iface,
      const std::string& name, const char* fmt, ...);

  /**
   * Send a reply to a method call
   *
   * @param msgref Message to reply to
   * @param fmt Format of parameters or NULL for none
   */
  void sendReply(DBusMessage* msgref, const char* fmt, ...);

  enum ErrorType
  {
    /* Special value for callback return if sendReply or sendError was called directly */
    MethodReplied = 0,

    /* Standard D-Bus errors */
    ErrorFailed,
    ErrorInvalidArgs,
    ErrorInvalidSignature,
    ErrorNotSupported,
    ErrorUnknownInterface,
    ErrorUnknownMethod,
    ErrorUnknownObject,
  };

  /**
   * Send an error reply
   *
   * @param msgref Message to reply to
   * @param type Error type
   * @param text Error text or NULL if none
   */
  void sendError(DBusMessage* msgref, int type, const char* text);

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace LicqDbus

#endif
