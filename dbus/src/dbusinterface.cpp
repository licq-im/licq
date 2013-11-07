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

#include "dbusinterface.h"

#include <boost/shared_ptr.hpp>
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <dbus/dbus.h>
#include <list>
#include <map>
#include <poll.h>

#include <licq/logging/log.h>
#include <licq/mainloop.h>

namespace LicqDbus
{

/**
 * Object to handle a DBusWatch in the main loop
 */
class WatchHandler : public Licq::MainLoopCallback
{
public:
  typedef boost::shared_ptr<WatchHandler> Ptr;

  WatchHandler(int id, Licq::MainLoop& mainLoop, DBusConnection* conn, DBusWatch* watch);
  ~WatchHandler();
  void toggle();

private:
  // From MainLoopCallback
  void rawFileEvent(int id, int fd, int revents);

  int myId;
  Licq::MainLoop& myMainLoop;
  DBusConnection* myConn;
  DBusWatch* myWatch;
};


/**
 * Object to handle a DBusTimeout in the main loop
 */
class TimeoutHandler : public Licq::MainLoopCallback
{
public:
  typedef boost::shared_ptr<TimeoutHandler> Ptr;

  TimeoutHandler(int id, Licq::MainLoop& mainLoop, DBusConnection* conn, DBusTimeout* timeout);
  ~TimeoutHandler();
  void toggle();

private:
  // From MainLoopCallback
  void timeoutEvent(int id);

  int myId;
  Licq::MainLoop& myMainLoop;
  DBusConnection* myConn;
  DBusTimeout* myTimeout;
};


typedef std::map<DBusWatch*,WatchHandler::Ptr> WatchMap;
typedef std::map<DBusTimeout*,TimeoutHandler::Ptr> TimeoutMap;


/**
 * DbusInterface internals
 */
class DbusInterface::Private
{
public:
  Private(DbusInterface* interface, Licq::MainLoop& mainLoop, DbusCallback* callback, bool systemBus)
    : myInterface(interface), myMainLoop(mainLoop), myCallback(callback), mySystemBus(systemBus)
  { /* Empty */ }

  // Callbacks from libdbus
  static dbus_bool_t addWatchCb(DBusWatch* watch, void* data);
  static void removeWatchCb(DBusWatch* watch, void* data);
  static void toggleWatchCb(DBusWatch* watch, void* data);
  static dbus_bool_t addTimeoutCb(DBusTimeout* timeout, void* data);
  static void removeTimeoutCb(DBusTimeout* timeout, void* data);
  static void toggleTimeoutCb(DBusTimeout* timeout, void* data);
  static DBusHandlerResult handleMessage(DBusConnection* connection, DBusMessage* message, void* data);

  static bool addMsgArgs(DBusMessageIter* args, const char* fmt, va_list ap);

  DbusInterface* myInterface;
  Licq::MainLoop& myMainLoop;
  DbusCallback* myCallback;
  DBusConnection* myConn;
  WatchMap myWatches;
  TimeoutMap myTimeouts;
  int myNextWatchId;
  int myNextTimeoutId;
  bool mySystemBus;
};

} // namespace LicqDbus


using namespace LicqDbus;


void DbusCallback::dbusConnected()
{ /* Empty */ }

void DbusCallback::dbusDisconnected()
{ /* Empty */ }

int DbusCallback::dbusMethod(const char* path, const char* iface, const char* member,
      DBusMessage* /*msgref*/, DBusMessageIter* /*argref*/, const char* /*fmt*/)
{
  Licq::gLog.info("Unhandled D-Bus method call '%s' to '%s' interface '%s'", member, path, iface);
  return DbusInterface::ErrorUnknownInterface;
}

void DbusCallback::dbusSignal(const char* path, const char* iface, const char* member,
      DBusMessage* /*msgref*/, DBusMessageIter* /*argref*/, const char* /*fmt*/)
{
  Licq::gLog.info("Unhandled D-Bus signal call '%s' to '%s' interface '%s'", member, path, iface);
}


WatchHandler::WatchHandler(int id, Licq::MainLoop& mainLoop, DBusConnection* conn, DBusWatch* watch)
  : myId(id),
    myMainLoop(mainLoop),
    myConn(conn),
    myWatch(watch)
{
  toggle();
}

WatchHandler::~WatchHandler()
{
  myMainLoop.removeCallback(this);
}

void WatchHandler::toggle()
{
  if (dbus_watch_get_enabled(myWatch))
  {
    int fd = dbus_watch_get_unix_fd(myWatch);
    unsigned int flags = dbus_watch_get_flags(myWatch);

    int events = 0;
    if (flags & DBUS_WATCH_READABLE)
      events |= POLLIN;
    if (flags & DBUS_WATCH_WRITABLE)
      events |= POLLOUT;

    myMainLoop.addRawFile(fd, this, events, myId);
  }
  else
  {
    myMainLoop.removeCallback(this);
  }
}

void WatchHandler::rawFileEvent(int /*id*/, int /*fd*/, int revents)
{
  // Convert returned poll events to flags used by dbus watch
  unsigned int flags = 0;
  if (revents & POLLIN) flags |= DBUS_WATCH_READABLE;
  if (revents & POLLOUT) flags |= DBUS_WATCH_WRITABLE;
  if (revents & POLLERR) flags |= DBUS_WATCH_ERROR;
  if (revents & POLLHUP) flags |= DBUS_WATCH_HANGUP;

  dbus_watch_handle(myWatch, flags);

  while (dbus_connection_dispatch(myConn) == DBUS_DISPATCH_DATA_REMAINS)
  { /* empty */ }
}

dbus_bool_t DbusInterface::Private::addWatchCb(DBusWatch* watch, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);

  assert(d->myWatches.count(watch) == 0);

  // Reset counter if no other watches exist
  if (d->myWatches.empty())
    d->myNextWatchId = 1;

  d->myWatches.insert(std::make_pair(watch,
      new WatchHandler(d->myNextWatchId++, d->myMainLoop, d->myConn, watch)));

  return true;
}

void DbusInterface::Private::removeWatchCb(DBusWatch* watch, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);
  d->myWatches.erase(watch);
}

void DbusInterface::Private::toggleWatchCb(DBusWatch* watch, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);

  WatchMap::iterator iter(d->myWatches.find(watch));
  assert(iter != d->myWatches.end());

  iter->second->toggle();
}


TimeoutHandler::TimeoutHandler(int id, Licq::MainLoop& mainLoop, DBusConnection* conn, DBusTimeout* timeout)
  : myId(id),
    myMainLoop(mainLoop),
    myConn(conn),
    myTimeout(timeout)
{
  toggle();
}

TimeoutHandler::~TimeoutHandler()
{
  myMainLoop.removeCallback(this);
}

void TimeoutHandler::toggle()
{
  if (dbus_timeout_get_enabled(myTimeout))
  {
    myMainLoop.addTimeout(dbus_timeout_get_interval(myTimeout), this, myId, false);
  }
  else
  {
    myMainLoop.removeCallback(this);
  }
}

void TimeoutHandler::timeoutEvent(int /*id*/)
{
  dbus_timeout_handle(myTimeout);

  while (dbus_connection_dispatch(myConn) == DBUS_DISPATCH_DATA_REMAINS)
  { /* empty */ }
}

dbus_bool_t DbusInterface::Private::addTimeoutCb(DBusTimeout* timeout, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);

  assert(d->myTimeouts.count(timeout) == 0);

  // Reset counter if no other timeouts exist
  if (d->myTimeouts.empty())
    d->myNextTimeoutId = 1;

  d->myTimeouts.insert(std::make_pair(timeout,
      new TimeoutHandler(d->myNextTimeoutId++, d->myMainLoop, d->myConn, timeout)));

  return true;
}

void DbusInterface::Private::removeTimeoutCb(DBusTimeout* timeout, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);
  d->myTimeouts.erase(timeout);
}

void DbusInterface::Private::toggleTimeoutCb(DBusTimeout* timeout, void* data)
{
  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);

  TimeoutMap::iterator iter(d->myTimeouts.find(timeout));
  assert(iter != d->myTimeouts.end());

  iter->second->toggle();
}

DBusHandlerResult DbusInterface::Private::handleMessage(DBusConnection* /*connection*/, DBusMessage* message, void* data)
{
  // Don't forward D-Bus daemon messages to application
  if (dbus_message_has_path(message, "/org/freedesktop/DBus"))
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  DbusInterface::Private* d = static_cast<DbusInterface::Private*>(data);

  if (d->myCallback == NULL)
  {
    Licq::gLog.info("Unhandled D-Bus message");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  const char* path = dbus_message_get_path(message);
  const char* iface = dbus_message_get_interface(message);
  const char* member = dbus_message_get_member(message);

  const char* signature = NULL;
  DBusMessageIter args;
  if (dbus_message_iter_init(message, &args))
    signature = dbus_message_get_signature(message);

  int type = dbus_message_get_type(message);
  int error;
  switch (type)
  {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
      if (iface == NULL)
        error = DbusInterface::ErrorUnknownInterface;
      else if (member == NULL)
        error = DbusInterface::ErrorUnknownMethod;
      else if (path == NULL)
        error = DbusInterface::ErrorUnknownObject;
      else if (strcmp(iface, "org.freedesktop.DBus.Introspectable") == 0 && strcmp(member, "Introspect") == 0)
      {
        std::string data(d->myCallback->dbusIntrospect(path));
        if (data.empty())
          error = DbusInterface::ErrorUnknownObject;
        else
        {
          data.insert(0,
              "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
                  "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
              "<node>"
                "<interface name=\"org.freedesktop.DBus.Introspectable\">"
                  "<method name=\"Introspect\">"
                    "<arg name=\"xml_data\" type=\"s\" direction=\"out\"/>"
                  "</method>"
                "</interface>"
                "<interface name=\"org.freedesktop.DBus.Peer\">"
                  "<method name=\"Ping\"/>"
                  "<method name=\"GetMachineId\">"
                    "<arg name=\"machine_uuid\" type=\"s\" direction=\"out\"/>"
                  "</method>"
                "</interface>" );
          data.append("</node>");
          d->myInterface->sendReply(message, "s", data.c_str());
          error = DbusInterface::MethodReplied;
        }
      }
      else
        error = d->myCallback->dbusMethod(path, iface, member, message, signature == NULL ? NULL : &args, signature);
      break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
      d->myCallback->dbusSignal(path, iface, member, message, signature == NULL ? NULL : &args, signature);
      error = DbusInterface::MethodReplied;
      break;
    default:
      Licq::gLog.info("Unhandled D-Bus message type %i", type);
      // Only method calls should be replied to
      error = DbusInterface::MethodReplied;
  }
  if (error != DbusInterface::MethodReplied)
    d->myInterface->sendError(message, error, NULL);

  return DBUS_HANDLER_RESULT_HANDLED;
}


std::string DbusInterface::encodeObjectPathPart(const std::string& s)
{
  // Object paths may only contain "[A-Z][a-z][0-9]_"
  // Use underscore as escape character similar to percent character in URIs
  static const char allowedChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::string ret;

  size_t lastpos = 0;
  size_t pos;
  while ((pos = s.find_first_not_of(allowedChars, lastpos)) != std::string::npos)
  {
    // Add all allowed characters before position
    ret.append(s, lastpos, pos-lastpos);

    // Add extra character escaped
    ret += '_';
    ret += allowedChars[(s[pos] >> 4) & 0x0f];
    ret += allowedChars[s[pos] & 0x0f];

    lastpos = pos+1;
  }
  // Add remaining chararcters
  ret.append(s, lastpos, std::string::npos);

  return ret;
}

std::string DbusInterface::decodeObjectPathPart(const std::string& s)
{
  std::string ret;
  size_t lastpos = 0;
  size_t pos;
  // Find all escape characters (e.g. underscores)
  while ((pos = s.find('_', lastpos)) <= s.size() - 3)
  {
    // Add all characters before this escape
    ret.append(s, lastpos, pos-lastpos);

    // Get the two hex digits after escape character
    char c1 = s[pos+1];
    char c2 = s[pos+2];
    if (c1 < '0' || (c1 > '9' && c1 < 'A') || c1 > 'F' ||
        c2 < '0' || (c2 > '9' && c2 < 'A') || c2 > 'F')
    {
      // Not a valid escape, keep underscore unmodified
      ret += '_';
      lastpos = pos + 1;
      continue;
    }

    // Translate to raw byte
    ret += (char)(((c1 - '0' - (c1 > '9' ? 7 : 0)) << 4) + (c2 - '0' - (c2 > '9' ? 7 : 0)));
    lastpos = pos + 3;
  }

  // Add remaining characters
  ret.append(s, lastpos, std::string::npos);

  return ret;
}

DbusInterface::DbusInterface(Licq::MainLoop& mainLoop, DbusCallback* callback, bool systemBus)
  : myPrivate(new Private(this, mainLoop, callback, systemBus))
{
  LICQ_D();
  d->myConn = NULL;
}

DbusInterface::~DbusInterface()
{
  disconnect();
}

bool DbusInterface::connect()
{
  LICQ_D();

  if (d->myConn != NULL)
  {
    if (dbus_connection_get_is_connected(d->myConn))
      return true;

    // Lost connection, drop it and make a new
    disconnect();
  }

  // Connect to DBus
  DBusError e;
  dbus_error_init(&e);
  d->myConn = dbus_bus_get_private(d->mySystemBus ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION, &e);

  if (dbus_error_is_set(&e))
  {
    Licq::gLog.error("Failed to get bus %s: %s", e.name, e.message);
    dbus_error_free(&e);
    return false;
  }

  // Don't let libdbus terminate Licq if we're disconnected
  dbus_connection_set_exit_on_disconnect(d->myConn, false);

  // Register callback functions
  dbus_connection_set_watch_functions(d->myConn, Private::addWatchCb,
      Private::removeWatchCb, Private::toggleWatchCb, d, NULL);
  dbus_connection_set_timeout_functions(d->myConn, Private::addTimeoutCb,
      Private::removeTimeoutCb, Private::toggleTimeoutCb, d, NULL);
  dbus_connection_add_filter(d->myConn, Private::handleMessage, d, NULL);

  if (d->myCallback != NULL)
    d->myCallback->dbusConnected();

  return true;
}

void DbusInterface::disconnect()
{
  LICQ_D();

  if (d->myConn != NULL)
  {
    dbus_connection_close(d->myConn);
    dbus_connection_unref(d->myConn);
    d->myConn = NULL;
  }
  d->myWatches.clear();
  d->myTimeouts.clear();

  if (d->myCallback != NULL)
    d->myCallback->dbusDisconnected();
}

void DbusInterface::flush()
{
  LICQ_D();

  if (d->myConn == NULL)
    return;

  dbus_connection_flush(d->myConn);
}

bool DbusInterface::requestName(const std::string& name)
{
  LICQ_D();

  if (d->myConn == NULL)
    return false;

  int ret = dbus_bus_request_name(d->myConn, name.c_str(), 0, NULL);
  return (ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER || ret == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER);
}

bool DbusInterface::Private::addMsgArgs(DBusMessageIter* args, const char* fmt, va_list ap)
{
  while (true)
  {
    switch (*fmt)
    {
      case '\0':
        return true;

      case 'i':
      {
        int val = va_arg(ap, int);
        if (!dbus_message_iter_append_basic(args, DBUS_TYPE_INT32, &val))
          return false;
        break;
      }
      case 'u':
      {
        unsigned int val = va_arg(ap, unsigned int);
        if (!dbus_message_iter_append_basic(args, DBUS_TYPE_UINT32, &val))
          return false;
        break;
      }
      case 's':
      {
        const char* val = va_arg(ap, const char*);
        if (!dbus_message_iter_append_basic(args, DBUS_TYPE_STRING, &val))
          return false;
        break;
      }
      case 'a':
      {
        // Array, next character determines element type
        switch (*(++fmt))
        {
          case 's':
          case 'o':
          {
            DBusMessageIter subargs;
            if (!dbus_message_iter_open_container(args, DBUS_TYPE_ARRAY,
                (*fmt == 'o' ? DBUS_TYPE_OBJECT_PATH_AS_STRING : DBUS_TYPE_STRING_AS_STRING),
                &subargs))
              return false;

            const std::list<std::string>* vals = va_arg(ap, const std::list<std::string>*);
            std::list<std::string>::const_iterator i;
            for (i = vals->begin(); i != vals->end(); ++i)
            {
              const char* val = (*i).c_str();
              if (!dbus_message_iter_append_basic(&subargs,
                  (*fmt == 'o' ? DBUS_TYPE_OBJECT_PATH : DBUS_TYPE_STRING), &val))
                return false;
            }

            if (!dbus_message_iter_close_container(args, &subargs))
              return false;
            break;
          }
          default:
            // Unknown array type
            return false;
        }
        break;
      }
      default:
        // Unknown format
        return false;
    }
    fmt++;
  }
}

void DbusInterface::sendSignal(const std::string& path, const std::string& iface,
    const std::string& name, const char* fmt, ...)
{
  LICQ_D();

  if (d->myConn == NULL)
    return;

  DBusMessage* msg = dbus_message_new_signal(path.c_str(), iface.c_str(), name.c_str());
  if (msg == NULL)
  {
    Licq::gLog.error("Failed to allocate signal");
    return;
  }

  if (fmt != NULL)
  {
    // Append arguments to signal
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    va_list ap;
    va_start(ap, fmt);
    if (!Private::addMsgArgs(&args, fmt, ap))
    {
      Licq::gLog.error("Failed to add signal parameters");
      va_end(ap);
      dbus_message_unref(msg);
      return;
    }
    va_end(ap);
  }

  // Queue signal to be sent (will be handled by main loop)
  dbus_connection_send(d->myConn, msg, NULL);

  // Free message (lib will hold a reference until send is complete)
  dbus_message_unref(msg);
}

void DbusInterface::sendReply(DBusMessage* msgref, const char* fmt, ...)
{
  LICQ_D();

  if (d->myConn == NULL)
    return;

  DBusMessage* msg = dbus_message_new_method_return(msgref);
  if (msg == NULL)
  {
    Licq::gLog.error("Failed to allocate reply message");
    return;
  }

  if (fmt != NULL)
  {
    // Append arguments to signal
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    va_list ap;
    va_start(ap, fmt);
    if (!Private::addMsgArgs(&args, fmt, ap))
    {
      Licq::gLog.error("Failed to add reply parameters");
      va_end(ap);
      dbus_message_unref(msg);
      return;
    }
    va_end(ap);
  }

  // Queue signal to be sent (will be handled by main loop)
  dbus_connection_send(d->myConn, msg, NULL);

  // Free message (lib will hold a reference until send is complete)
  dbus_message_unref(msg);
}

void DbusInterface::sendError(DBusMessage* msgref, int type, const char* message)
{
  LICQ_D();

  if (d->myConn == NULL)
    return;

  const char* errname;
  switch (type)
  {
    default:
    case ErrorFailed:           errname = DBUS_ERROR_FAILED; break;
    case ErrorInvalidArgs:      errname = DBUS_ERROR_INVALID_ARGS; break;
    case ErrorInvalidSignature: errname = DBUS_ERROR_INVALID_SIGNATURE; break;
    case ErrorNotSupported:     errname = DBUS_ERROR_NOT_SUPPORTED; break;
    case ErrorUnknownInterface: errname = DBUS_ERROR_UNKNOWN_INTERFACE; break;
    case ErrorUnknownMethod:    errname = DBUS_ERROR_UNKNOWN_METHOD; break;
    case ErrorUnknownObject:    errname = DBUS_ERROR_UNKNOWN_OBJECT; break;
  }
  DBusMessage* reply = dbus_message_new_error(msgref, errname, message);
  if (reply == NULL)
  {
    Licq::gLog.error("Failed to allocate error message");
    return;
  }

  // Queue signal to be sent (will be handled by main loop)
  dbus_connection_send(d->myConn, reply, NULL);

  // Free message (lib will hold a reference until send is complete)
  dbus_message_unref(reply);
}

void DbusInterface::getNextMessageParamValue(DBusMessageIter* msgref, void* ret)
{
  dbus_message_iter_get_basic(msgref, ret);
}
