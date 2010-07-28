/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_FILELOGSINK_H
#define LICQDAEMON_FILELOGSINK_H

#include "streamlogsink.h"

#include <fstream>

namespace LicqDaemon
{

class FileLogSink : public StreamLogSink
{
public:
  inline FileLogSink(const std::string& filename);

  bool isOpen() const
  // const_cast needed on Solaris where ofstream::is_open isn't declared const
  { return const_cast<std::ofstream&>(myFile).is_open(); }

private:
  std::ofstream myFile;
};

inline FileLogSink::FileLogSink(const std::string& filename)
  : StreamLogSink(myFile)
{
  setUseColors(false);
  myFile.open(filename.c_str(), std::ios::out | std::ios::app);
}

} // namespace LicqDaemon

#endif
