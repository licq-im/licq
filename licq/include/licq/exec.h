/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2014 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_EXEC_H
#define LICQ_EXEC_H

#include <string>
#include "macro.h"

namespace Licq
{

/**
 * Utility class to execute an external command
 */
class Exec
{
public:
  /**
   * Execute an external command
   *
   * @param cmd The command to execute
   */
  Exec(const char* command);

  /**
   * Destructor, kills external command if not already dead
   */
  ~Exec();

  /**
   * Terminate the process
   *
   * @param timeout Time to wait for process to end by itself
   * @return Exit code
   */
  int kill(int timeout);

  /**
   * Send data to process and return all output
   *
   * @param input Data to process
   * @param eoi True to close stdin ofter sending input
   * @return output All data written to stdout
   */
  std::string process(const std::string& input, bool eoi = true);

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
