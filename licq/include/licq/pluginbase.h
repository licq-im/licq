/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_PLUGINBASE_H
#define LICQ_PLUGINBASE_H

/*------------------------------------------------------------------------------
 * Plugin header file
 *
 * Note that these functions must be implemented in each plugin.
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 * Name
 *
 * Returns the name of the plugin.  Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Name();


/*------------------------------------------------------------------------------
 * Version
 *
 * Returns the version of the plugin.  Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Version();


/*------------------------------------------------------------------------------
 * Status
 *
 * Returns the status of the plugin.  Typically this will be enabled or
 * disabled.  Rarely relevant for a main interface plugin.
 * Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Status();


/*------------------------------------------------------------------------------
 * Description
 *
 * Returns a brief (one line) description of the plugin
 * Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Description();


/*------------------------------------------------------------------------------
 * Usage
 *
 * Returns a usage string.
 *----------------------------------------------------------------------------*/
const char *LP_Usage();


/*------------------------------------------------------------------------------
 * Init
 *
 * This function is called to initialize the plugin.  It is passed the
 * relevant command line parameters to be parsed using getopt().  It should
 * return whether or not it started successfully.
 *----------------------------------------------------------------------------*/
bool LP_Init(int, char **);


/*------------------------------------------------------------------------------
 * Main
 *
 * This function is called to actually run the plugin.  It is run in it's own
 * thread so may block. It *must* register with the daemon before sending any
 * requests. It returns an integer return code.
 *----------------------------------------------------------------------------*/
int LP_Main();


/*------------------------------------------------------------------------------
 * ConfigFile
 *
 * This function returns the name of the configuration file referenced
 * from the BASE_DIR.  It can be left undefined if no config file is
 * used.
 *----------------------------------------------------------------------------*/
const char *LP_ConfigFile();


/*==============================================================================
 * INTERNAL USE ONLY
 *============================================================================*/

unsigned short LP_Id;

char *LP_BuildDate()
{
  static char szDate[] = __DATE__;
  return szDate;
}

char *LP_BuildTime()
{
  static char szTime[] = __TIME__;
  return szTime;
}

#ifdef __cplusplus
}
#endif


#endif
