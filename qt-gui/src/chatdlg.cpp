// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

// written by Graham Roff <graham@licq.org>
// Contributions by Dirk A. Mueller <dirk@licq.org>

// -----------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qapplication.h>
#include <qfontdatabase.h>
#include <ctype.h>
#include <qlayout.h>
#include <stdlib.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qsocketnotifier.h>
#include <qtoolbar.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qgroupbox.h>
#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <qmenubar.h>
#include <qtextcodec.h>
#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qpainter.h>

#include "chatdlg.h"
#include "mainwin.h"
#include "ewidgets.h"
#include "mledit.h"
#include "licq_chat.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_user.h"
#include "licq_icqd.h"

#include "usercodec.h"

ChatDlgList ChatDlg::chatDlgs;

// ---------------------------------------------------------------------------
// Toolbar icons

#include "xpm/chatBold.xpm"
#include "xpm/chatItalic.xpm"
#include "xpm/chatUnder.xpm"
#include "xpm/chatStrike.xpm"
#include "xpm/chatBeep.xpm"
#include "xpm/chatIgnore.xpm"
#include "xpm/chatChangeFg.xpm"
#include "xpm/chatChangeBg.xpm"

// ---------------------------------------------------------------------------

static const int col_array[] =
{
  0x00, 0x00, 0x00,
  0x80, 0x00, 0x00,
  0x00, 0x80, 0x00,
  0x80, 0x80, 0x00,
  0x00, 0x00, 0x80,
  0x80, 0x00, 0x80,
  0x00, 0x80, 0x80,
  0x80, 0x80, 0x80,
  0xC0, 0xC0, 0xC0,
  0xFF, 0x00, 0x00,
  0x00, 0xFF, 0x00,
  0xFF, 0xFF, 0x00,
  0x00, 0x00, 0xFF,
  0xFF, 0x00, 0xFF,
  0x00, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF
};

#define NUM_COLORS sizeof(col_array)/sizeof(int)/3

// ---------------------------------------------------------------------------
enum ChatMenu_Identifiers {
  mnuChatAudio,
  mnuChatSave
};


// ---------------------------------------------------------------------------
ChatDlg::ChatDlg(const UserId& userId, CICQDaemon *daemon,
                 CMainWindow *m, QWidget *parent)
  : QMainWindow(parent, "ChatDialog", WDestructiveClose)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user != NULL)
  {
    m_szId = strdup(user->accountId().c_str());
    m_nPPID = user->ppid();
  }
  gUserManager.DropUser(user);

  m_bAudio = true;
  licqDaemon = daemon;
  sn = NULL;
  mainwin = m;

  m_nMode = CHAT_PANE;

  setCaption(tr("Licq - Chat"));
  statusBar();
  // central widget
  QWidget* widCentral = new QWidget(this);
  setCentralWidget(widCentral);

  setOpaqueMoving(true);
  setToolBarsMovable(true);
  setDockEnabled(DockBottom, true);
  setUsesBigPixmaps(false);

  // Pane mode setup
  boxPane = new QGroupBox(widCentral);
  paneLayout = new QGridLayout(boxPane, 3, 1, 4);
  remoteLayout = new QGridLayout(2, 1, 4);
  paneLayout->addLayout(remoteLayout, 0, 0);
  lblRemote = new QLabel(tr("Remote - Not connected"), boxPane);
  remoteLayout->addWidget(lblRemote, 0, 0);
  remoteLayout->setRowStretch(1, 1);

  paneLayout->addRowSpacing(1, 15);

  QGridLayout *llay = new QGridLayout(2, 1, 4);
  paneLayout->addLayout(llay, 2, 0);
  lblLocal = new QLabel(boxPane);
  mlePaneLocal = new CChatWindow(boxPane);
  mlePaneLocal->setMinimumHeight(100);
  mlePaneLocal->setMinimumWidth(150);
  mlePaneLocal->setEnabled(false);
  llay->addWidget(lblLocal, 0, 0);
  llay->addWidget(mlePaneLocal, 1, 0);
  llay->setRowStretch(1, 1);

  // IRC mode setup
  boxIRC = new QGroupBox(widCentral);
  QGridLayout *lay = new QGridLayout(boxIRC, 2, 2, 4);
  mleIRCRemote = new CChatWindow(boxIRC);
  mleIRCRemote->setReadOnly(true);
  mleIRCRemote->setMinimumHeight(100);
  mleIRCRemote->setMinimumWidth(150);
  lay->addWidget(mleIRCRemote, 0, 0);
  lstUsers = new QListBox(boxIRC);
  lay->addMultiCellWidget(lstUsers, 0, 1, 1, 1);
  mleIRCLocal = new CChatWindow(boxIRC);
  mleIRCLocal->setEnabled(false);
  mleIRCLocal->setFixedHeight(mleIRCLocal->fontMetrics().lineSpacing() * 4);
  lay->addWidget(mleIRCLocal, 1, 0);
  lay->setRowStretch(0, 1);
  lay->setColStretch(0, 1);

  // Generic setup
  mnuMain = new QPopupMenu(this);
  mnuMain->insertItem(tr("&Audio"), this, SLOT(slot_audio()), ALT + Key_A, mnuChatAudio);
  mnuMain->insertItem(tr("&Save Chat"), this, SLOT(slot_save()), ALT + Key_S, mnuChatSave);
  mnuMain->insertSeparator();
  mnuMain->insertItem(tr("&Close Chat"), this, SLOT(close()), ALT + Key_Q);
  mnuMode = new QPopupMenu(this);
  mnuMode->insertItem(tr("&Pane Mode"), this, SLOT(SwitchToPaneMode()));
  mnuMode->insertItem(tr("&IRC Mode"), this, SLOT(SwitchToIRCMode()));
  menuBar()->insertItem(tr("Chat"), mnuMain);
  menuBar()->insertItem(tr("Mode"), mnuMode);

  mnuMain->setItemChecked(mnuChatAudio, m_bAudio);

  // Toolbar
  QToolBar* barChat = new QToolBar("label", this);
  addToolBar(barChat, "Chat Toolbar");
  barChat->setHorizontalStretchable(false);
  barChat->setVerticalStretchable(true);
  barChat->setFixedHeight(barChat->height()+2);

   // ### FIXME: implement laughing
   // tbtLaugh = new QToolButton(LeftArrow, barChat);

  QPixmap* pixIgnore = new QPixmap(chatIgnore_xpm);
  qPixmaps.push_back(pixIgnore);
  tbtIgnore = new QToolButton(*pixIgnore, tr("Ignore user settings"),
    tr("Ignores user color settings"), this, SLOT(updateRemoteStyle()), barChat);
  tbtIgnore->setToggleButton(true);

  QPixmap* pixBeep = new QPixmap(chatBeep_xpm);
  qPixmaps.push_back(pixBeep);
  tbtBeep = new QToolButton(*pixBeep, tr("Beep"),
     tr("Sends a Beep to all recipients"),this, SLOT(chatSendBeep()), barChat);

  barChat->addSeparator();

  QPixmap* pixFg = new QPixmap(chatChangeFg_xpm);
  qPixmaps.push_back(pixFg);
  tbtFg = new QToolButton(*pixFg, tr("Foreground color"),
     tr("Changes the foreground color"), this, SLOT(changeFrontColor()), barChat);
  mnuFg = new QPopupMenu(this);

  QPixmap* pixBg = new QPixmap(chatChangeBg_xpm);
  qPixmaps.push_back(pixBg);
  tbtBg = new QToolButton(*pixBg, tr("Background color"),
     tr("Changes the background color"), this, SLOT(changeBackColor()), barChat);

  mnuBg = new QPopupMenu(this);

  for(unsigned int i = 0; i < NUM_COLORS; i++)
  {
    QPixmap *pix = new QPixmap(48, 14);
    qPixmaps.push_back(pix);
    QPainter p(pix);
    QColor c (col_array[i*3+0], col_array[i*3+1], col_array[i*3+2]);

    pix->fill(c);
    p.drawRect(0, 0, 48, 14);

    mnuBg->insertItem(*pix, i);
    QPixmap* pixf = new QPixmap(48, 14);
    qPixmaps.push_back(pixf);
    pixf->fill(colorGroup().background());
    QPainter pf(pixf);
    pf.setPen(c);
    pf.drawText(5, 12, QString("Abc"));
    mnuFg->insertItem(*pixf, i);
  }
  barChat->addSeparator();

  QPixmap* pixBold = new QPixmap(chatBold_xpm);
  qPixmaps.push_back(pixBold);
  tbtBold = new QToolButton(*pixBold, tr("Bold"),
    tr("Toggles Bold font") , this, SLOT(fontStyleChanged()), barChat);
  tbtBold->setToggleButton(true);

  QPixmap* pixItalic = new QPixmap(chatItalic_xpm);
  qPixmaps.push_back(pixItalic);
  tbtItalic = new QToolButton(*pixItalic, tr("Italic"),
    tr("Toggles Italic font"), this, SLOT(fontStyleChanged()), barChat);
  tbtItalic->setToggleButton(true);

  QPixmap *pixUnder = new QPixmap(chatUnder_xpm);
  qPixmaps.push_back(pixUnder);
  tbtUnderline = new QToolButton(*pixUnder, tr("Underline"),
     tr("Toggles Underline font"), this, SLOT(fontStyleChanged()), barChat);
  tbtUnderline->setToggleButton(true);

  QPixmap *pixStrike = new QPixmap(chatStrike_xpm);
  qPixmaps.push_back(pixStrike);
  tbtStrikeOut = new QToolButton(*pixStrike, tr("StrikeOut"),
     tr("Toggles StrikeOut font"), this, SLOT(fontStyleChanged()), barChat);
  tbtStrikeOut->setToggleButton(true);

  tbtBold->setAutoRaise(false);
  tbtItalic->setAutoRaise(false);
  tbtUnderline->setAutoRaise(false);
  tbtStrikeOut->setAutoRaise(false);

  barChat->addSeparator();

  cmbFontSize = new QComboBox(true, barChat);
  cmbFontSize->setInsertionPolicy(QComboBox::NoInsertion);
  //windows font size limit seems to be 1638 (tested 98, 2000)
  cmbFontSize->setValidator(new QIntValidator(1, 1638, cmbFontSize));
  connect(cmbFontSize, SIGNAL(activated(const QString&)), SLOT(fontSizeChanged(const QString&)));
  cmbFontSize->insertItem(QString::number(font().pointSize()));

  QValueList<int> sizes = QFontDatabase::standardSizes();
  for(unsigned i = 0; i < sizes.count(); i++)
    if(sizes[i] != font().pointSize())
      cmbFontSize->insertItem(QString::number(sizes[i]));

  QFontDatabase fb;
  cmbFontName = new QComboBox(false, barChat);
#if 0
  cmbFontName->setSizeLimit(15);
  QStringList sl = fb.families();
  while(sl.at(55) != sl.end())  sl.remove(sl.at(55));
#endif
//  cmbFontName->setFixedSize(cmbFontName->sizeHint());
  cmbFontName->insertStringList(fb.families());
  barChat->setStretchableWidget(cmbFontName);
  connect(cmbFontName, SIGNAL(activated(const QString&)), SLOT(fontNameChanged(const QString&)));

  barChat->addSeparator();

  codec = QTextCodec::codecForLocale();

  QString codec_name = QString::fromLatin1( codec->name() ).lower(); // TODO: determine best codec
  QPopupMenu *popupEncoding = new QPopupMenu;
  popupEncoding->setCheckable(true);

  // populate the popup menu
  UserCodec::encoding_t *it = &UserCodec::m_encodings[0];
  while(it->encoding != NULL) {

    if (QString::fromLatin1(it->encoding).lower() == codec_name) {
      if (mainwin->m_bShowAllEncodings || it->isMinimal) {
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib);
      } else {
        // if the current encoding does not appear in the minimal list
        popupEncoding->insertSeparator(0);
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib, 0);
      }
      popupEncoding->setItemChecked(it->mib, true);
    } else {
      if (mainwin->m_bShowAllEncodings || it->isMinimal) {
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib);
      }
    }

    ++it;
  }

  tbtEncoding = new QToolButton(barChat);
  tbtEncoding->setTextLabel(tr("Set Encoding"));
  tbtEncoding->setPopup(popupEncoding);
  tbtEncoding->setPopupDelay(0);
  tbtEncoding->setPixmap(mainwin->pmEncoding);

//  QWidget* dummy = new QWidget(barChat);
//  barChat->setStretchableWidget(dummy);

  QGridLayout *g = new QGridLayout(widCentral, 2, 1, 6, 4);
  g->addWidget(boxPane, 0, 0);
  g->addWidget(boxIRC, 0, 0);

  SwitchToPaneMode();

  // Add ourselves to the list
  chatDlgs.push_back(this);

  // Create the chat manager using our font
  QFontInfo fi(mlePaneLocal->font());
  QFontDatabase fd; //QFontInfo.fixedPitch returns incorrect info???
  unsigned char style = STYLE_DONTCARE;

  if (fd.isFixedPitch(fi.family(), fd.styleString(mlePaneLocal->font())))
    style |= STYLE_FIXEDxPITCH;
  else
    style |= STYLE_VARIABLExPITCH;

  unsigned char encoding = UserCodec::charsetForName(codec->name());
  //TODO in daemon
  chatman = new CChatManager(daemon, strtoul(m_szId, (char **)NULL, 10),
     fi.family().local8Bit(),
     encoding, style, fi.pointSize(), fi.bold(), fi.italic(), fi.underline(),
     fi.strikeOut());

  sn = new QSocketNotifier(chatman->Pipe(), QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), this, SLOT(slot_chat()));

  // But use the chat manager default colors
  mlePaneLocal->setForeground(QColor(chatman->ColorFg()[0],
     chatman->ColorFg()[1], chatman->ColorFg()[2]));
  mlePaneLocal->setBackground(QColor(chatman->ColorBg()[0],
     chatman->ColorBg()[1], chatman->ColorBg()[2]));
  mleIRCLocal->setForeground(QColor(chatman->ColorFg()[0],
     chatman->ColorFg()[1], chatman->ColorFg()[2]));
  mleIRCLocal->setBackground(QColor(chatman->ColorBg()[0],
     chatman->ColorBg()[1], chatman->ColorBg()[2]));
  mleIRCRemote->setForeground(QColor(chatman->ColorFg()[0],
     chatman->ColorFg()[1], chatman->ColorFg()[2]));
  mleIRCRemote->setBackground(QColor(chatman->ColorBg()[0],
     chatman->ColorBg()[1], chatman->ColorBg()[2]));
  chatname = QString::fromLocal8Bit(chatman->Name());
  lstUsers->insertItem(chatname);
  lblLocal->setText(tr("Local - %1").arg(chatname));

  widCentral->setMinimumSize(400, 300);
  resize(500, 475);
  show();
}

// -----------------------------------------------------------------------------

ChatDlg::~ChatDlg()
{
  delete chatman;

  if (sn != NULL) delete sn;
  sn = NULL;

  QPixmapList::iterator i;
  for (i = qPixmaps.begin(); i != qPixmaps.end(); i++) delete *i;
  qPixmaps.clear();

  ChatDlgList::iterator iter;
  for (iter = chatDlgs.begin(); iter != chatDlgs.end(); iter++)
  {
    if (this == *iter)
    {
      chatDlgs.erase(iter);
      break;
    }
  }
}


// -----------------------------------------------------------------------------

void ChatDlg::fontSizeChanged(const QString& txt)
{
  QFont f(mlePaneLocal->font());

  int nNewSize = txt.toInt();
  if (nNewSize > 24)
    nNewSize = 24;

  f.setPointSize(nNewSize);

  mlePaneLocal->setFont(f);
  mleIRCLocal->setFont(f);
  mleIRCRemote->setFont(f);

  // if ignoring style change the remote panes too
  updateRemoteStyle();

  // transmit to remote
  QFontInfo fi(f);
  chatman->ChangeFontSize(fi.pointSize());
}


// -----------------------------------------------------------------------------

void ChatDlg::fontNameChanged(const QString &txt)
{
  QFont f(mlePaneLocal->font());

  f.setFamily(txt);

  mlePaneLocal->setFont(f);
  mleIRCLocal->setFont(f);
  mleIRCRemote->setFont(f);

  // if ignoring style change the remote panes too
  updateRemoteStyle();

  // transmit to remote
  sendFontInfo();
}

// -----------------------------------------------------------------------------

void ChatDlg::sendFontInfo()
{
  //FIXME can we get more precise style???
  QFontInfo fi(mlePaneLocal->font());
  QFontDatabase fd; //QFontInfo.fixedPitch returns incorrect info???
  unsigned char style = STYLE_DONTCARE;

  if (fd.isFixedPitch(fi.family(), fd.styleString(mlePaneLocal->font())))
    style |= STYLE_FIXEDxPITCH;
  else
    style |= STYLE_VARIABLExPITCH;

  unsigned char encoding = UserCodec::charsetForName(codec->name());

  chatman->ChangeFontFamily(fi.family().local8Bit(), encoding, style);
}

// -----------------------------------------------------------------------------

void ChatDlg::fontStyleChanged()
{
  QFont f(mlePaneLocal->font());

  f.setBold(tbtBold->state() == QButton::On);
  f.setItalic(tbtItalic->state() == QButton::On);
  f.setUnderline(tbtUnderline->state() == QButton::On);
  f.setStrikeOut(tbtStrikeOut->state() == QButton::On);

  mlePaneLocal->setFont(f);
  mleIRCLocal->setFont(f);
  mleIRCRemote->setFont(f);

  // if ignoring style change the remote panes too
  updateRemoteStyle();

  // transmit to remote
  QFontInfo fi(f);
  chatman->ChangeFontFace(fi.bold(), fi.italic(), fi.underline(), fi.strikeOut());
}


// -----------------------------------------------------------------------------

void ChatDlg::chatSendBeep()
{
  chatman->SendBeep();
  QApplication::beep();
}

// -----------------------------------------------------------------------------

void ChatDlg::changeFrontColor()
{
  int i = mnuFg->exec(tbtFg->mapToGlobal(QPoint(0,tbtFg->height()+2)));
  if (i < 0) return;

  QColor color (col_array[i*3+0], col_array[i*3+1], col_array[i*3+2]);

  mlePaneLocal->setForeground(color);
  mleIRCLocal->setForeground(color);
  mleIRCRemote->setForeground(color);

  // if ignoring style change the remote panes too
  updateRemoteStyle();

  // sent to remote
  chatman->ChangeColorFg(color.red(), color.green(), color.blue());
}


// -----------------------------------------------------------------------------

void ChatDlg::changeBackColor()
{
  int i = mnuBg->exec(tbtBg->mapToGlobal(QPoint(0,tbtBg->height()+2)));
  if (i < 0) return;

  QColor color (col_array[i*3+0], col_array[i*3+1], col_array[i*3+2]);

  mlePaneLocal->setBackground(color);
  mleIRCLocal->setBackground(color);
  mleIRCRemote->setBackground(color);

  // if ignoring style change the remote panes too
  updateRemoteStyle();

  // sent to remote
  chatman->ChangeColorBg(color.red(), color.green(), color.blue());
}

// -----------------------------------------------------------------------------

void ChatDlg::updateRemoteStyle()
{
  if(tbtIgnore->state() == QButton::On) {
    QColor fg(chatman->ColorFg()[0], chatman->ColorFg()[1],
                       chatman->ColorFg()[2]);
    QColor bg(chatman->ColorBg()[0], chatman->ColorBg()[1],
                       chatman->ColorBg()[2]);
    QFont f(mlePaneLocal->font());
    ChatUserWindowsList::iterator iter;
    for (iter = chatUserWindows.begin(); iter != chatUserWindows.end(); iter++)
    {
      iter->w->setForeground(fg);
      iter->w->setBackground(bg);
      iter->w->setFont(f);
    }
  }
  else {
    ChatUserWindowsList::iterator iter;
    for (iter = chatUserWindows.begin(); iter != chatUserWindows.end(); iter++)
    {
      QColor fg(iter->u->ColorFg()[0], iter->u->ColorFg()[1],
                         iter->u->ColorFg()[2]);
      QColor bg(iter->u->ColorBg()[0], iter->u->ColorBg()[1],
                         iter->u->ColorBg()[2]);
      QFont f(iter->w->font());
      f.setFixedPitch((iter->u->FontStyle() & 0x0F) == STYLE_FIXEDxPITCH);

      switch (iter->u->FontStyle() & 0xF0)
      {
      case STYLE_ROMAN:
        f.setStyleHint(QFont::Serif);
        break;
      case STYLE_SWISS:
        f.setStyleHint(QFont::SansSerif);
        break;
      case STYLE_DECORATIVE:
        f.setStyleHint(QFont::Decorative);
        break;
      case STYLE_DONTCARE:
      case STYLE_MODERN:
      case STYLE_SCRIPT:
      default:
        f.setStyleHint(QFont::AnyStyle);
        break;
      }
      f.setFamily(iter->u->FontFamily());
      f.setPointSize(iter->u->FontSize());
      f.setBold(iter->u->FontBold());
      f.setItalic(iter->u->FontItalic());
      f.setUnderline(iter->u->FontUnderline());
      f.setStrikeOut(iter->u->FontStrikeOut());
      iter->w->setForeground(fg);
      iter->w->setBackground(bg);
      iter->w->setFont(f);
    }
  }
}


//-----ChatDlg::StartAsServer------------------------------------------------
bool ChatDlg::StartAsServer()
{
  lblRemote->setText(tr("Remote - Waiting for joiners..."));
  if (!chatman->StartAsServer()) return false;
  return true;
}



//-----ChatDlg::StartAsClient------------------------------------------------
bool ChatDlg::StartAsClient(unsigned short nPort)
{
  lblRemote->setText(tr("Remote - Connecting..."));
  chatman->StartAsClient(nPort);
  return true;
}



//-----chatSend-----------------------------------------------------------------
void ChatDlg::chatSend(QKeyEvent *e)
{
  switch (e->key())
  {
    case Key_Enter:
    case Key_Return:
    {
      if (m_nMode == CHAT_IRC) {
         QString text = mleIRCLocal->text();
         if (text.right(1) == "\n") text.truncate(text.length()-1);
         QCString encoded = codec->fromUnicode(text);
         // send the data over the wire
         char *c;
         for (c = encoded.data(); *c; c++)
            chatman->SendCharacter(*c);

         // even if the pane didn't trigger the event,
         // we need to keep it updated
         mlePaneLocal->appendNoNewLine("\n");
         // so you'll get some idea what your buddy sees (encoding-wise)
         mleIRCRemote->append(chatname + "> " + codec->toUnicode(encoded));
         mleIRCRemote->GotoEnd();

         mleIRCLocal->clear();
      } else {
         // keep IRC updated anyway (encoding should be already properly represented)
         mleIRCRemote->append(chatname + "> " + mlePaneLocal->textLine(mlePaneLocal->numLines()-2));
      }

      chatman->SendNewline();
      break;
    }
    case Key_Backspace:
    {
      if (m_nMode == CHAT_IRC) {
         mlePaneLocal->backspace(); // keep the pane updated
      }

      if (m_nMode == CHAT_PANE) {
         chatman->SendBackspace();
      }

      break;
    }
    case Key_Tab:
    case Key_Backtab:
      break;

    default:
    {
      QCString encoded = codec->fromUnicode(e->text());

      // if in pane mode, send right away
      if (m_nMode == CHAT_PANE) {
         // for multibyte encodings
         char *c;
         for (c = encoded.data(); *c; c++)
            chatman->SendCharacter(*c);
      } else {
         // if the pane is not what triggered the key press, it still needs
         // to be updated
         mlePaneLocal->appendNoNewLine(codec->toUnicode(encoded));
      }

      break;
    }
  }
}





//-----ChatDlg::slot_chat----------------------------------------------------
void ChatDlg::slot_chat()
{
  // Read out any pending events
  char buf[32];
  read(chatman->Pipe(), buf, 32);

  CChatEvent *e = NULL;
  while ( (e = chatman->PopChatEvent()) != NULL)
  {
    CChatUser *u = e->Client();

    switch(e->Command())
    {
      case CHAT_ERRORxBIND:
      {
        WarnUser(this, tr("Unable to bind to a port.\nSee Network Window "
                          "for details."));
        chatClose(u);
        break;
      }

      case CHAT_ERRORxCONNECT:
      {
        WarnUser(this, tr("Unable to connect to the remote chat.\nSee Network "
                          "Window for details."));
        chatClose(u);
        break;
      }

      case CHAT_ERRORxRESOURCES:
      {
        WarnUser(this, tr("Unable to create new thread.\nSee Network Window "
                          "for details."));
        chatClose(u);
        break;
      }

      case CHAT_DISCONNECTION:
      {
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->Name());

        if (n.isEmpty())
          n = USERID_TOSTR(u->userId());
        chatClose(u);
        InformUser(this, tr("%1 closed connection.").arg(n));
        break;
      }

      case CHAT_CONNECTION:
      {
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->Name());

        // Add the user to the listbox
        lstUsers->insertItem(n);
        // If this is the first user, set up the remote mle
        if (!mlePaneLocal->isEnabled())
        {
          delete lblRemote;
          connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
          connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
          mlePaneLocal->setEnabled(true);
          mleIRCLocal->setEnabled(true);
          if (m_nMode == CHAT_PANE)
            mlePaneLocal->setFocus();
          else
            mleIRCLocal->setFocus();
        }

        CChatWindow *lePaneRemote = new CChatWindow(boxPane);
        lePaneRemote->setReadOnly(true);
        QLabel *lbl = new QLabel(n, boxPane);
        UserWindowPair uwp = { u, lePaneRemote, lbl};
        chatUserWindows.push_back(uwp);

        updateRemoteStyle();

        UpdateRemotePane();

        break;
      }

      case CHAT_NEWLINE:
      {
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->Name());

        // add to IRC box
        mleIRCRemote->append(n + QString::fromLatin1("> ") + UserCodec::codecForCChatUser(u)->toUnicode(e->Data()));
        mleIRCRemote->GotoEnd();
        GetWindow(u)->appendNoNewLine("\n");
        GetWindow(u)->GotoEnd();
        break;
      }

      case CHAT_BEEP:  // beep
      {
        if (m_bAudio)
          QApplication::beep();
        else
        {
          GetWindow(u)->append(tr("\n<--BEEP-->\n"));
          mleIRCRemote->append(chatname + tr("> <--BEEP-->\n"));
        }
        break;
      }

      case CHAT_BACKSPACE:   // backspace
      {

        GetWindow(u)->setReadOnly(false);
        GetWindow(u)->setCursorPosition(-1, -1, false);

        GetWindow(u)->backspace();

        GetWindow(u)->setReadOnly(true);
        GetWindow(u)->update();

        break;
      }

      case CHAT_COLORxFG: // change foreground color
      {
        if (tbtIgnore->state() == QButton::Off)
          GetWindow(u)->setForeground(QColor (u->ColorFg()[0],
             u->ColorFg()[1], u->ColorFg()[2]));
        break;
      }

      case CHAT_COLORxBG:  // change background color
      {
        if (tbtIgnore->state() == QButton::Off)
          GetWindow(u)->setBackground(QColor (u->ColorBg()[0],
             u->ColorBg()[1], u->ColorBg()[2]));

        break;
      }

      case CHAT_FONTxFAMILY: // change font type
      {
        if (tbtIgnore->state() == QButton::Off)
        {
          QFont f(GetWindow(u)->font());
          f.setFixedPitch((u->FontStyle() & 0x0F) == STYLE_FIXEDxPITCH);

          switch (u->FontStyle() & 0xF0)
          {
          case STYLE_ROMAN:
            f.setStyleHint(QFont::Serif);
            break;
          case STYLE_SWISS:
            f.setStyleHint(QFont::SansSerif);
            break;
          case STYLE_DECORATIVE:
            f.setStyleHint(QFont::Decorative);
            break;
          case STYLE_DONTCARE:
          case STYLE_MODERN:
          case STYLE_SCRIPT:
          default:
            f.setStyleHint(QFont::AnyStyle);
            break;
          }

          f.setFamily(u->FontFamily());

          GetWindow(u)->setFont(f);
        }
        break;
      }

      case CHAT_FONTxFACE: // change font style
      {
        if (tbtIgnore->state() == QButton::Off)
        {
          QFont f(GetWindow(u)->font());
          f.setBold(u->FontBold());
          f.setItalic(u->FontItalic());
          f.setUnderline(u->FontUnderline());
          f.setStrikeOut(u->FontStrikeOut());
          GetWindow(u)->setFont(f);
        }
        break;
      }

      case CHAT_FONTxSIZE: // change font size
      {
        if (tbtIgnore->state() == QButton::Off)
        {
          QFont f(GetWindow(u)->font());
          f.setPointSize(u->FontSize() > 24 ? 24 : u->FontSize());
          GetWindow(u)->setFont(f);
        }
        break;
      }

      case CHAT_FOCUSxOUT:
      case CHAT_FOCUSxIN:
      case CHAT_SLEEPxON:
      case CHAT_SLEEPxOFF:
      {
        // TODO add some visible indication of these
        break;
      }

      case CHAT_CHARACTER:
      {
        GetWindow(u)->appendNoNewLine(UserCodec::codecForCChatUser(u)->toUnicode(e->Data()));
        break;
      }

      default:
      {
        gLog.Warn("%sInternal Error: invalid command from chat manager (%d).\n",
           L_ERRORxSTR, e->Command());
        break;
      }
    }

    delete e;
  }
}


void ChatDlg::SwitchToIRCMode()
{
  m_nMode = CHAT_IRC;
  mnuMode->setItemChecked(mnuMode->idAt(0), false);
  mnuMode->setItemChecked(mnuMode->idAt(1), true);
  boxPane->hide();
  mleIRCLocal->setText(mlePaneLocal->textLine(mlePaneLocal->numLines()-2));
  mleIRCLocal->GotoEnd();
  boxIRC->show();
  mleIRCLocal->setFocus();
}


void ChatDlg::SwitchToPaneMode()
{
  m_nMode = CHAT_PANE;
  mnuMode->setItemChecked(mnuMode->idAt(0), true);
  mnuMode->setItemChecked(mnuMode->idAt(1), false);
  boxIRC->hide();
  mlePaneLocal->GotoEnd();
  boxPane->show();
  mlePaneLocal->setFocus();
}


void ChatDlg::chatClose(CChatUser *u)
{
  if (u == NULL)
  {
    chatUserWindows.clear();
    lstUsers->clear();
    disconnect(sn, SIGNAL(activated(int)), this, SLOT(slot_chat()));
    chatman->CloseChat();
  }
  else
  {
    // Remove the user from the list box
    for (unsigned short i = 0; i < lstUsers->count(); i++)
    {
      if (lstUsers->text(i) == u->Name())
      {
        lstUsers->removeItem(i);
        break;
      }
    }
    ChatUserWindowsList::iterator iter;
    for (iter = chatUserWindows.begin(); iter != chatUserWindows.end(); iter++)
    {
      if (iter->u == u)
      {
        delete iter->w;
        delete iter->l;
        chatUserWindows.erase(iter);
        break;
      }
    }
    UpdateRemotePane();
  }

  // Modify the dialogs
  if (chatman->ConnectedUsers() == 0)
  {
    mleIRCLocal->setEnabled(false);
    mlePaneLocal->setEnabled(false);
    disconnect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
    disconnect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));

    lblRemote = new QLabel(tr("Remote - Not connected"), boxPane);
    remoteLayout->addWidget(lblRemote, 0, 0);
    lblRemote->show();
  }
}


void ChatDlg::closeEvent(QCloseEvent* e)
{
  if(QueryUser(this, tr("Do you want to save the chat session?"),
               tr("Yes"), tr("No")))
  {
    if (!slot_save())
    {
      e->ignore();
      return;
    }
  }

  e->accept();
  chatClose(NULL);
}

CChatWindow *ChatDlg::GetWindow(CChatUser *u)
{
  ChatUserWindowsList::iterator iter;
  for (iter = chatUserWindows.begin(); iter != chatUserWindows.end(); iter++)
    if (iter->u == u)
      return iter->w;
  return NULL;
}

void ChatDlg::UpdateRemotePane()
{
  delete remoteLayout;
  remoteLayout = new QGridLayout(2, chatUserWindows.size()+1, 4);
  paneLayout->addLayout(remoteLayout, 0, 0);

  setCaption(tr("Licq - Chat %1").arg(ChatClients()));

  ChatUserWindowsList::iterator iter;
  unsigned int i;
  for (i = 0, iter = chatUserWindows.begin(); iter != chatUserWindows.end();
       i++, iter++)
  {
    remoteLayout->addWidget(iter->l, 0, i);
    remoteLayout->addWidget(iter->w, 1, i);
    iter->l->show();
    iter->w->show();
  }
  remoteLayout->setRowStretch(1, 1);
}


QString ChatDlg::ChatClients()
{
  char *sz = chatman->ClientsStr();
  QString n = sz;
  delete [] sz;
  return n;
}

QString ChatDlg::ChatName()
{
  QString n = chatman->Name();
  return n;
}


bool ChatDlg::slot_save()
{
  QString t = QDateTime::currentDateTime().toString();
  for ( unsigned l = 0; l < t.length(); ++l ) {
    if ( t[l] == ' ' ) t[l] = '-';
    if ( t[l] == ':' ) t[l] = '-';
  }
  QString n = tr("/%1.chat").arg(t);

#ifdef USE_KDE
  QString fn = KFileDialog::getSaveFileName(QDir::homeDirPath() + n,
     QString::null, this);
#else
  QString fn = QFileDialog::getSaveFileName(QDir::homeDirPath() + n,
     QString::null, this);
#endif

  if (!fn.isEmpty())
  {
    QFile f(fn);
    if (!f.open(IO_WriteOnly))
    {
      WarnUser(this, tr("Failed to open file:\n%1").arg(fn));
      return false;
    }
    else
    {
      QTextStream t(&f);
      t << mleIRCRemote->text();
      f.close();
    }
    return true;
  }
  else
  {
    return false;
  }
}


void ChatDlg::slot_audio()
{
  m_bAudio = !m_bAudio;
  mnuMain->setItemChecked(mnuChatAudio, m_bAudio);
}


unsigned short ChatDlg::LocalPort()
{
  return chatman->LocalPort();
}

void ChatDlg::slot_setEncoding(int encodingMib)
{
  /* initialize a codec according to the encoding menu item id */
  QString encoding( UserCodec::encodingForMib(encodingMib) );

  if (!encoding.isNull()) {
    QTextCodec * _codec = QTextCodec::codecForName(encoding.latin1());
    if (_codec == NULL)
    {
      WarnUser(this, QString(tr("Unable to load encoding <b>%1</b>. Message contents may appear garbled.")).arg(encoding));
      return;
    }
    codec = _codec;

    QPopupMenu * popupEncoding = tbtEncoding->popup(); 

    /* uncheck all encodings */
    for (unsigned int i=0; i<popupEncoding->count(); ++i) {
      popupEncoding->setItemChecked(popupEncoding->idAt(i), false);
    }

    /* make the chosen encoding checked */
    popupEncoding->setItemChecked(encodingMib, true);

    // transmit to remote
    sendFontInfo();

    emit encodingChanged();
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CChatWindow::CChatWindow (QWidget *parent)
  : QMultiLineEdit(parent, 0)
{
  setWordWrap(WidgetWidth);
  setWrapPolicy(AtWhiteSpace);
  if(MLEditWrap::editFont) QWidget::setFont(*MLEditWrap::editFont);
}


// -----------------------------------------------------------------------------

void CChatWindow::appendNoNewLine(QString s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}


// -----------------------------------------------------------------------------

void CChatWindow::GotoEnd()
{
  setCursorPosition(numLines() - 1, lineLength(numLines() - 1));
}


// -----------------------------------------------------------------------------

void CChatWindow::insert(const QString &s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}


// -----------------------------------------------------------------------------

void CChatWindow::keyPressEvent (QKeyEvent *e)
{
  if ( (e->text().length() == 0 ||
        e->state() & ControlButton ||
        e->state() & AltButton) &&
       (e->key() != Key_Tab &&
        e->key() != Key_Backtab &&
        e->key() != Key_Backspace &&
        e->key() != Key_Return &&
        e->key() != Key_Enter))
    return;

  GotoEnd();

  // the order of the two is important -- on Enter, first QMultiLineEdit adds
  // a line break, and later we clear the input line, and not vice versa
  QMultiLineEdit::keyPressEvent(e);
  emit keyPressed(e);
}


// -----------------------------------------------------------------------------

void CChatWindow::mousePressEvent( QMouseEvent * )
{
  // a user might not change the cursor position
  // and marking / cutting away text is not allowed

  // so ignore the event.
}

void CChatWindow::mouseMoveEvent( QMouseEvent*)
{
  // ignore it
}

void CChatWindow::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() == MidButton && !isReadOnly() )
    paste();
}

// -----------------------------------------------------------------------------

void CChatWindow::paste()
{
  QString t = QApplication::clipboard()->text();

  if ( !t.isEmpty() ) {

    for (int i=0; (uint)i<t.length(); i++) {
      if ( t[i] < ' ' && t[i] != '\n' && t[i] != '\t' )
        t[i] = ' ';
    }

    for(int i=0; (unsigned) i<t.length(); i++) {
      QKeyEvent press(QEvent::KeyPress, t[i].latin1() == '\n' ? Key_Enter : 0, t[i].latin1(), 0, QString(t[i]));

      keyPressEvent(&press);
    }
  }
}


// -----------------------------------------------------------------------------

void CChatWindow::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);

  setPalette(pal);
}


// -----------------------------------------------------------------------------


void CChatWindow::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Text, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

  setPalette(pal);
}

// -----------------------------------------------------------------------------


#include "chatdlg.moc"
