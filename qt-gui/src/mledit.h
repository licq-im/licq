/*
 * 
 * 
  $Id$
 
  KEdit, a simple text editor for the KDE project

  Copyright (C) 1996 Bernd Johannes Wuebben   
                     wuebben@math.cornell.edu
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#ifndef __KEDIT_H__
#define __KEDIT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qstrlist.h> 
#include <qmlined.h>
#include <qkeycode.h>

#include <ctype.h>
#include <stdlib.h>

class MLEditWrap : public QMultiLineEdit
{
   Q_OBJECT
public:
   MLEditWrap (bool wordWrap, QWidget *parent=NULL, const char *name=NULL);   
   bool format(QStrList& );
   void getpar(int line,QStrList& par);
   virtual void append(const char *s);
   void appendChar(char);
   void backspace(void);
   void newLine(void);
   void setWrapSize(int);
   void goToEnd(void);

protected:
   void keyPressEvent (QKeyEvent *);
   virtual void resizeEvent (QResizeEvent *);
   void computePosition();

signals:
   void keyPressed(QKeyEvent *);
   void signal_CtrlEnterPressed();

private:
   void mynewLine();
   QString prefixString(QString);
   QStrList	par;
   bool autoIndentMode;
   int cursor_offset;
   int line_pos, col_pos;
   bool fill_column_is_set;
   bool word_wrap_is_set;
   int fill_column_value;
};

#endif
