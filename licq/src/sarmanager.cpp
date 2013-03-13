/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010, 2013 Licq developers <licq-dev@googlegroups.com>
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

#include "sarmanager.h"

#include <cstdio> // sprintf

#include <licq/inifile.h>

using namespace LicqDaemon;
using Licq::SarList;
using Licq::SavedAutoResponse;

// Declare global SarManager (internal for daemon)
LicqDaemon::SarManager LicqDaemon::gSarManager;

// Declare global Licq::SarManager to refer to the internal SarManager
Licq::SarManager& Licq::gSarManager(LicqDaemon::gSarManager);

const char* const SarManager::SectionNames[SarManager::NumLists] =
    { "Away", "NA", "Occupied", "DND", "FFC" };

const char* const SarManager::DefaultNames[SarManager::NumLists][SarManager::NumDefaults] = {
        { "Away", "Lunch", "Be right back", "Dog Walk", "Smoke", "Coffee", "Air" },
        { "Out for the day", "Not here", "Closed", "Fishing", "Sleeping", "Kids", "Gone" },
        { "Busy", "Meeting", "Concentration", "On the Phone", "Can't chat", "Working", "Conversing" },
        { "Do Not Disturb", "Meeting", "Concentration", "On the Phone", "Working", "Fiddling"},
        { "Chat", "Come in", "Fun", "Don't Wait", "Hear", NULL, NULL}
    };

const char* const SarManager::DefaultTexts[SarManager::NumLists][SarManager::NumDefaults] = {
    {
        "I am currently away from the keyboard.\\nPlease leave your message and I will get back to you as soon as I return!",
        "I'm out to lunch.\\nI will return shortly.",
        "Don't go anywhere!\\nI'll be back in a jiffy.",
        "I'm out with the dog.\\nBe back when he's finished",
        "Went out for a smoke.",
        "On my coffee break.",
        "Went to get some fresh air.",
    }, {
        "I am out'a here.\\nSee you tomorrow!",
        "Give it up! I 'm not in!",
        "I'm closed for the weekend/holidays.",
        "Gone fishin'!",
        "I'm sleeping. Don't wake me.",
        "Went home.\\nHad to feed the kids.",
        "Gone for good.",
    }, {
        "Please do not disturb me now.\\nDisturb me later.\\nOnly urgent messages, please!",
        "I'm currently in a meeting. I can't be disturbed.\\nOnly urgent messages, please!",
        "Don't disturb my concentration!\\nOnly urgent messages, please!",
        "hone with a very important client. Don't disturb me!\\nOnly urgent messages please!",
        "I can't chat with you now. I'm busy.\\nOnly urgent messages, please!",
        "Can't you see I'm working?\\nOnly urgent messages, please!",
        "I am conversing with my colleagues.\\nOnly urgent messages, please!",
    }, {
        "Please do not disturb me now. Disturb me later.",
        "I'm currently in a meeting.\\nI can't be disturbed.",
        "Don't disturb my concentration!",
        "I'm on the phone with a very important client.\\nDon't disturb me!",
        "I can't chat with you now. I'm busy.",
        "Can't you see I'm working?",
        "Fiddling around some things and I'm quite busy with that.\\nSo please disturb me later!",
    }, {
        "We'd love to hear what you have to say. Join our chat.",
        "Come in and join my chat room!",
        "Don't miss the fun! Join our chat!",
        "What are you waiting for? Come in!",
        "We'd love to hear\\nwhat you have to say.\\nJoin our chat!",
        NULL,
        NULL,
    } };


SarManager::SarManager()
{
  // Empty
}

SarManager::~SarManager()
{
  // Empty
}

void SarManager::initialize()
{
  Licq::IniFile sarFile("sar.conf");

  if (sarFile.loadFile())
  {
    for (int i = 0; i < NumLists; ++i)
    {
      sarFile.setSection(SectionNames[i]);
      int count;
      sarFile.get("NumSAR", count, 0);
      for (int j = 1; j <= count; ++j)
      {
        SavedAutoResponse sar;
        char key[20];
        sprintf(key, "SAR%i.Name", j);
        sarFile.get(key, sar.name, "");
        sprintf(key, "SAR%i.Text", j);
        sarFile.get(key, sar.text, "");
        if (sar.name.empty() && sar.text.empty())
          continue;

        mySarLists[i].push_back(sar);
      }
    }

    return;
  }

  // No existing config file, load up defaults
  for (int i = 0; i < NumLists; ++i)
  {
    if (mySarLists[i].size() > 0)
      continue;

    for (int j = 0; j < NumDefaults; ++j)
    {
      if (DefaultNames[i][j] == NULL)
        continue;
      SavedAutoResponse sar;
      sar.name = DefaultNames[i][j];
      sar.text = DefaultTexts[i][j];
      mySarLists[i].push_back(sar);
    }
  }

  writeConfig();
}

Licq::SarList& SarManager::getList(List list)
{
  myMutex.lock();
  return mySarLists[list];
}

void SarManager::releaseList(bool save)
{
  if (save)
    writeConfig();

  myMutex.unlock();
}

void SarManager::writeConfig()
{
  Licq::IniFile sarFile("sar.conf");
  sarFile.loadFile();

  for (int i = 0; i < NumLists; ++i)
  {
    sarFile.setSection(SectionNames[i]);
    sarFile.set("NumSAR", mySarLists[i].size());
    int count = 0;
    for (SarList::const_iterator j = mySarLists[i].begin(); j != mySarLists[i].end(); ++j)
    {
      ++count;
      char key[20];
      sprintf(key, "SAR%i.Name", count);
      sarFile.set(key, j->name);
      sprintf(key, "SAR%i.Text", count);
      sarFile.set(key, j->text);
    }
  }
  sarFile.writeFile();
}
