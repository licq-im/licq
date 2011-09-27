/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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
#include "chatdlg.h"

#include "config.h"

#include <cctype>
#include <cstdlib>
#include <unistd.h>

#include <QActionGroup>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSocketNotifier>
#include <QTextCodec>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>

#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <QFileDialog>
#endif

#include <licq/icq/chat.h>
#include <licq/logging/log.h>

#include "config/chat.h"
#include "config/general.h"
#include "config/iconmanager.h"

#include "core/messagebox.h"

#include "helpers/support.h"
#include "helpers/usercodec.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ChatDlg */

ChatDlgList ChatDlg::chatDlgs;

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
ChatDlg::ChatDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myAudio(true)
{
  Support::setWidgetProps(this, "ChatDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  myId = userId.accountId().c_str();
  myPpid = userId.protocolId();

  sn = NULL;

  m_nMode = CHAT_PANE;

  setWindowTitle(tr("Licq - Chat"));

  // Pane mode setup
  boxPane = new QGroupBox();
  paneLayout = new QGridLayout(boxPane);
  remoteLayout = new QGridLayout();
  paneLayout->addLayout(remoteLayout, 0, 0);
  lblRemote = new QLabel(tr("Remote - Not connected"), boxPane);
  remoteLayout->addWidget(lblRemote, 0, 0);
  remoteLayout->setRowStretch(1, 1);

  paneLayout->setRowMinimumHeight(1, 15);

  QGridLayout* llay = new QGridLayout();
  paneLayout->addLayout(llay, 2, 0);
  lblLocal = new QLabel(boxPane);
  mlePaneLocal = new ChatWindow(boxPane);
  mlePaneLocal->setMinimumHeight(100);
  mlePaneLocal->setMinimumWidth(150);
  mlePaneLocal->setEnabled(false);
  llay->addWidget(lblLocal, 0, 0);
  llay->addWidget(mlePaneLocal, 1, 0);
  llay->setRowStretch(1, 1);

  // IRC mode setup
  boxIRC = new QGroupBox();
  QGridLayout* lay = new QGridLayout(boxIRC);
  mleIRCRemote = new ChatWindow(boxIRC);
  mleIRCRemote->setReadOnly(true);
  mleIRCRemote->setMinimumHeight(100);
  mleIRCRemote->setMinimumWidth(150);
  lay->addWidget(mleIRCRemote, 0, 0);
  lstUsers = new QListWidget(boxIRC);
  lay->addWidget(lstUsers, 0, 1, 2, 1);
  mleIRCLocal = new ChatWindow(boxIRC);
  mleIRCLocal->setEnabled(false);
  mleIRCLocal->setFixedHeight(mleIRCLocal->fontMetrics().lineSpacing() * 4);
  lay->addWidget(mleIRCLocal, 1, 0);
  lay->setRowStretch(0, 1);
  lay->setColumnStretch(0, 1);

  // Generic setup
  mnuMain = new QMenu(tr("Chat"));
  QAction* chatAudio = mnuMain->addAction(tr("&Audio"), this, SLOT(slot_audio(bool)), Qt::ALT + Qt::Key_A);
  chatAudio->setCheckable(true);
  chatAudio->setChecked(myAudio);
  mnuMain->addAction(tr("&Save Chat"), this, SLOT(slot_save()), Qt::ALT + Qt::Key_S);
  mnuMain->addSeparator();
  mnuMain->addAction(tr("&Close Chat"), this, SLOT(close()), Qt::ALT + Qt::Key_Q);
  mnuMode = new QMenu(tr("Mode"));
  QActionGroup* modeGroup = new QActionGroup(this);
  QAction* paneModeAction = modeGroup->addAction(tr("&Pane Mode"));
  connect(paneModeAction, SIGNAL(triggered()), SLOT(SwitchToPaneMode()));
  paneModeAction->setCheckable(true);
  paneModeAction->setChecked(true);
  QAction* ircModeAction = modeGroup->addAction(tr("&IRC Mode"));
  connect(ircModeAction, SIGNAL(triggered()), SLOT(SwitchToIRCMode()));
  ircModeAction->setCheckable(true);
  mnuMode->addActions(modeGroup->actions());

  QMenuBar* menuBar = new QMenuBar(this);
  menuBar->addMenu(mnuMain);
  menuBar->addMenu(mnuMode);

  // Toolbar
  QToolBar* barChat = new QToolBar("label", this);
  barChat->setIconSize(QSize(16, 16));

   // ### FIXME: implement laughing
   // tbtLaugh = new QToolButton(LeftArrow, barChat);

  tbtIgnore = barChat->addAction(IconManager::instance()->getIcon(IconManager::IgnoreIcon), tr("Ignore user settings"));
  tbtIgnore->setToolTip(tr("Ignores user color settings"));
  connect(tbtIgnore, SIGNAL(triggered()), SLOT(updateRemoteStyle()));
  tbtIgnore->setCheckable(true);

  tbtBeep = barChat->addAction(IconManager::instance()->getIcon(IconManager::BeepIcon), tr("Beep"));
  tbtBeep->setToolTip(tr("Sends a Beep to all recipients"));
  connect(tbtBeep, SIGNAL(triggered()), SLOT(chatSendBeep()));

  barChat->addSeparator();

  tbtFg = barChat->addAction(IconManager::instance()->getIcon(IconManager::TextColorIcon), tr("Foreground color"));
  tbtFg->setToolTip(tr("Changes the foreground color"));
  mnuFg = new QMenu();
  connect(mnuFg, SIGNAL(triggered(QAction*)), SLOT(changeFrontColor(QAction*)));
  tbtFg->setMenu(mnuFg);
  dynamic_cast<QToolButton*>(barChat->widgetForAction(tbtFg))->setPopupMode(QToolButton::InstantPopup);

  tbtBg = barChat->addAction(IconManager::instance()->getIcon(IconManager::BackColorIcon), tr("Background color"));
  tbtBg->setToolTip(tr("Changes the background color"));
  mnuBg = new QMenu();
  connect(mnuBg, SIGNAL(triggered(QAction*)), SLOT(changeBackColor(QAction*)));
  tbtBg->setMenu(mnuBg);
  dynamic_cast<QToolButton*>(barChat->widgetForAction(tbtBg))->setPopupMode(QToolButton::InstantPopup);

  for(unsigned int i = 0; i < NUM_COLORS; i++)
  {
    QPixmap pix(48, 14);
    QPainter p(&pix);
    QColor c (col_array[i*3+0], col_array[i*3+1], col_array[i*3+2]);

    pix.fill(c);
    p.drawRect(0, 0, 48, 14);

    mnuBg->addAction(pix, QString())->setData(i);
    QPixmap pixf(48, 14);
    pixf.fill(palette().color(QPalette::Background));
    QPainter pf(&pixf);
    pf.setPen(c);
    pf.drawText(5, 12, QString("Abc"));
    mnuFg->addAction(pixf, QString())->setData(i);
  }
  barChat->addSeparator();

  tbtBold = barChat->addAction(IconManager::instance()->getIcon(IconManager::BoldIcon), tr("Bold"));
  tbtBold->setToolTip(tr("Toggles Bold font"));
  connect(tbtBold, SIGNAL(triggered()), SLOT(fontStyleChanged()));
  tbtBold->setCheckable(true);

  tbtItalic = barChat->addAction(IconManager::instance()->getIcon(IconManager::ItalicIcon), tr("Italic"));
  tbtItalic->setToolTip(tr("Toggles Italic font"));
  connect(tbtItalic, SIGNAL(triggered()), SLOT(fontStyleChanged()));
  tbtItalic->setCheckable(true);

  tbtUnderline = barChat->addAction(IconManager::instance()->getIcon(IconManager::UnderlineIcon), tr("Underline"));
  tbtUnderline->setToolTip(tr("Toggles Underline font"));
  connect(tbtUnderline, SIGNAL(triggered()), SLOT(fontStyleChanged()));
  tbtUnderline->setCheckable(true);

  tbtStrikeOut = barChat->addAction(IconManager::instance()->getIcon(IconManager::StrikethroughIcon), tr("StrikeOut"));
  tbtStrikeOut->setToolTip(tr("Toggles StrikeOut font"));
  connect(tbtStrikeOut, SIGNAL(triggered()), SLOT(fontStyleChanged()));
  tbtStrikeOut->setCheckable(true);

  barChat->addSeparator();

  cmbFontSize = new QComboBox();
  barChat->addWidget(cmbFontSize);
  cmbFontSize->setInsertPolicy(QComboBox::NoInsert);
  //windows font size limit seems to be 1638 (tested 98, 2000)
  cmbFontSize->setValidator(new QIntValidator(1, 1638, cmbFontSize));
  connect(cmbFontSize, SIGNAL(activated(const QString&)), SLOT(fontSizeChanged(const QString&)));
  cmbFontSize->addItem(QString::number(font().pointSize()));

  QList<int> sizes = QFontDatabase::standardSizes();
  for(int i = 0; i < sizes.count(); i++)
    if(sizes[i] != font().pointSize())
      cmbFontSize->addItem(QString::number(sizes[i]));

  QFontDatabase fb;
  cmbFontName = new QComboBox();
#if 0
  cmbFontName->setSizeLimit(15);
  QStringList sl = fb.families();
  while(sl.at(55) != sl.end())  sl.remove(sl.at(55));
#endif
//  cmbFontName->setFixedSize(cmbFontName->sizeHint());
  cmbFontName->addItems(fb.families());
  barChat->addWidget(cmbFontName);
  connect(cmbFontName, SIGNAL(activated(const QString&)), SLOT(fontNameChanged(const QString&)));

  barChat->addSeparator();

  codec = QTextCodec::codecForLocale();

  QString codec_name = QString::fromLatin1( codec->name() ).toLower(); // TODO: determine best codec
  QMenu* popupEncoding = new QMenu;
  QActionGroup* encodingsGroup = new QActionGroup(this);
  connect(encodingsGroup, SIGNAL(triggered(QAction*)), SLOT(setEncoding(QAction*)));

  // populate the popup menu
  for (UserCodec::encoding_t* it = &UserCodec::m_encodings[0]; it->encoding != NULL; ++it)
  {
    QTextCodec* check_codec = QTextCodec::codecForName(it->encoding);

    bool currentCodec = check_codec != NULL && QString::fromLatin1(check_codec->name()).toLower() == codec_name;

    if (!currentCodec && !Config::Chat::instance()->showAllEncodings() && !it->isMinimal)
      continue;

    QAction* a = new QAction(UserCodec::nameForEncoding(it->encoding), encodingsGroup);
    a->setCheckable(true);
    a->setData(it->mib);

    if (currentCodec)
      a->setChecked(true);

    if (currentCodec && !Config::Chat::instance()->showAllEncodings() && !it->isMinimal)
    {
      // if the current encoding does not appear in the minimal list
      popupEncoding->insertSeparator(popupEncoding->actions()[0]);
      popupEncoding->insertAction(popupEncoding->actions()[0], a);
    }
    else
    {
      popupEncoding->addAction(a);
    }
  }


  tbtEncoding = barChat->addAction(IconManager::instance()->getIcon(IconManager::EncodingIcon), tr("Set Encoding"));
  tbtEncoding->setMenu(popupEncoding);
  dynamic_cast<QToolButton*>(barChat->widgetForAction(tbtEncoding))->setPopupMode(QToolButton::InstantPopup);

//  QWidget* dummy = new QWidget(barChat);
//  barChat->setStretchableWidget(dummy);

  QGridLayout* g = new QGridLayout(this);
  int ml, mt, mr, mb;
  g->getContentsMargins(&ml, &mt, &mr, &mb);
  g->setContentsMargins(ml, menuBar->height(), mr, mb);
  g->addWidget(barChat, 0, 0);
  g->addWidget(boxPane, 1, 0);
  g->addWidget(boxIRC, 1, 0);

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
  chatman = new CChatManager(
      myId.toULong(), fi.family().toLocal8Bit().constData(), encoding, style,
      fi.pointSize(), fi.bold(), fi.italic(), fi.underline(), fi.strikeOut());

  sn = new QSocketNotifier(chatman->Pipe(), QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), SLOT(slot_chat()));

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
  chatname = QString::fromLocal8Bit(chatman->name().c_str());
  lstUsers->addItem(chatname);
  lblLocal->setText(tr("Local - %1").arg(chatname));

  setMinimumSize(400, 300);
  resize(550, 475);
  show();
}

// -----------------------------------------------------------------------------

ChatDlg::~ChatDlg()
{
  delete chatman;

  if (sn != NULL) delete sn;
  sn = NULL;

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

void ChatDlg::fontNameChanged(const QString& txt)
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

  chatman->changeFontFamily(fi.family().toLocal8Bit().constData(), encoding, style);
}

// -----------------------------------------------------------------------------

void ChatDlg::fontStyleChanged()
{
  QFont f(mlePaneLocal->font());

  f.setBold(tbtBold->isChecked());
  f.setItalic(tbtItalic->isChecked());
  f.setUnderline(tbtUnderline->isChecked());
  f.setStrikeOut(tbtStrikeOut->isChecked());

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

void ChatDlg::changeFrontColor(QAction* action)
{
  int i = action->data().toInt();
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

void ChatDlg::changeBackColor(QAction* action)
{
  int i = action->data().toInt();
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
  if(tbtIgnore->isChecked()) {
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
      f.setFamily(iter->u->fontFamily().c_str());
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
void ChatDlg::chatSend(QKeyEvent* e)
{
  switch (e->key())
  {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
      if (m_nMode == CHAT_IRC) {
         QString text = mleIRCLocal->toPlainText();
         if (text.right(1) == "\n") text.truncate(text.length()-1);
         QByteArray encoded = codec->fromUnicode(text);
         // send the data over the wire
         char* c;
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
         mleIRCRemote->append(chatname + "> " + mlePaneLocal->lastLine());
      }

      chatman->SendNewline();
      break;
    }
    case Qt::Key_Backspace:
    {
      if (m_nMode == CHAT_IRC) {
         mlePaneLocal->backspace(); // keep the pane updated
      }

      if (m_nMode == CHAT_PANE) {
         chatman->SendBackspace();
      }

      break;
    }
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      break;

    default:
    {
      QByteArray encoded = codec->fromUnicode(e->text());

      // if in pane mode, send right away
      if (m_nMode == CHAT_PANE) {
         // for multibyte encodings
         char* c;
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

  CChatEvent* e = NULL;
  while ( (e = chatman->PopChatEvent()) != NULL)
  {
    CChatUser* u = e->Client();

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
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->name().c_str());

        if (n.isEmpty())
          n = u->userId().toString().c_str();
        chatClose(u);
        InformUser(this, tr("%1 closed connection.").arg(n));
        break;
      }

      case CHAT_CONNECTION:
      {
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->name().c_str());

        // Add the user to the listbox
        lstUsers->addItem(n);
        // If this is the first user, set up the remote mle
        if (!mlePaneLocal->isEnabled())
        {
          delete lblRemote;
          connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent*)), SLOT(chatSend(QKeyEvent*)));
          connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent*)), SLOT(chatSend(QKeyEvent*)));
          mlePaneLocal->setEnabled(true);
          mleIRCLocal->setEnabled(true);
          if (m_nMode == CHAT_PANE)
            mlePaneLocal->setFocus();
          else
            mleIRCLocal->setFocus();
        }

        ChatWindow* lePaneRemote = new ChatWindow(boxPane);
        lePaneRemote->setReadOnly(true);
        QLabel* lbl = new QLabel(n, boxPane);
        UserWindowPair uwp = { u, lePaneRemote, lbl};
        chatUserWindows.push_back(uwp);

        updateRemoteStyle();

        UpdateRemotePane();

        break;
      }

      case CHAT_NEWLINE:
      {
        QString n = UserCodec::codecForCChatUser(u)->toUnicode(u->name().c_str());

        // add to IRC box
        mleIRCRemote->append(n + QString::fromLatin1("> ") + UserCodec::codecForCChatUser(u)->toUnicode(e->data().c_str()));
        mleIRCRemote->GotoEnd();
        GetWindow(u)->appendNoNewLine("\n");
        GetWindow(u)->GotoEnd();
        break;
      }

      case CHAT_BEEP:  // beep
      {
        if (myAudio)
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

        GetWindow(u)->backspace();

        GetWindow(u)->setReadOnly(true);
        GetWindow(u)->update();

        break;
      }

      case CHAT_COLORxFG: // change foreground color
      {
        if (! tbtIgnore->isChecked())
          GetWindow(u)->setForeground(QColor (u->ColorFg()[0],
             u->ColorFg()[1], u->ColorFg()[2]));
        break;
      }

      case CHAT_COLORxBG:  // change background color
      {
        if (! tbtIgnore->isChecked())
          GetWindow(u)->setBackground(QColor (u->ColorBg()[0],
             u->ColorBg()[1], u->ColorBg()[2]));

        break;
      }

      case CHAT_FONTxFAMILY: // change font type
      {
        if (! tbtIgnore->isChecked())
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

          f.setFamily(u->fontFamily().c_str());

          GetWindow(u)->setFont(f);
        }
        break;
      }

      case CHAT_FONTxFACE: // change font style
      {
        if (! tbtIgnore->isChecked())
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
        if (! tbtIgnore->isChecked())
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
        GetWindow(u)->appendNoNewLine(UserCodec::codecForCChatUser(u)->toUnicode(e->data().c_str()));
        break;
      }

      default:
      {
        Licq::gLog.warning("Internal Error: invalid command from chat manager (%d)",
           e->Command());
        break;
      }
    }

    delete e;
  }
}


void ChatDlg::SwitchToIRCMode()
{
  m_nMode = CHAT_IRC;
  boxPane->hide();
  mleIRCLocal->setText(mlePaneLocal->lastLine());
  mleIRCLocal->GotoEnd();
  boxIRC->show();
  mleIRCLocal->setFocus();
}


void ChatDlg::SwitchToPaneMode()
{
  m_nMode = CHAT_PANE;
  boxIRC->hide();
  mlePaneLocal->GotoEnd();
  boxPane->show();
  mlePaneLocal->setFocus();
}


void ChatDlg::chatClose(CChatUser* u)
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
    for (int i = 0; i < lstUsers->count(); i++)
    {
      if (lstUsers->item(i)->text() == u->name().c_str())
      {
        lstUsers->removeItemWidget(lstUsers->item(i));
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
    disconnect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(chatSend(QKeyEvent*)));
    disconnect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent*)));

    lblRemote = new QLabel(tr("Remote - Not connected"), boxPane);
    remoteLayout->addWidget(lblRemote, 0, 0);
    lblRemote->show();
  }
}


void ChatDlg::closeEvent(QCloseEvent* e)
{
  if (QueryYesNo(this, tr("Do you want to save the chat session?")))
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

ChatWindow* ChatDlg::GetWindow(CChatUser* u)
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
  remoteLayout = new QGridLayout();
  paneLayout->addLayout(remoteLayout, 0, 0);

  setWindowTitle(tr("Licq - Chat %1").arg(ChatClients()));

  ChatUserWindowsList::iterator iter;
  int i;
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
  return chatman->clientsString().c_str();
}

QString ChatDlg::ChatName()
{
  return chatman->name().c_str();
}


bool ChatDlg::slot_save()
{
  QString t = QDateTime::currentDateTime().toString();
  for ( int l = 0; l < t.length(); ++l ) {
    if ( t[l] == ' ' ) t[l] = '-';
    if ( t[l] == ':' ) t[l] = '-';
  }
  QString n = tr("/%1.chat").arg(t);

#ifdef USE_KDE
  QString fn = KFileDialog::getSaveFileName(QDir::homePath() + n,
     QString::null, this);
#else
  QString fn = QFileDialog::getSaveFileName(this, QString(), QDir::homePath() + n);
#endif

  if (!fn.isEmpty())
  {
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly))
    {
      WarnUser(this, tr("Failed to open file:\n%1").arg(fn));
      return false;
    }
    else
    {
      QTextStream t(&f);
      t << mleIRCRemote->toPlainText();
      f.close();
    }
    return true;
  }
  else
  {
    return false;
  }
}


void ChatDlg::slot_audio(bool audio)
{
  myAudio = audio;
}


unsigned short ChatDlg::LocalPort()
{
  return chatman->LocalPort();
}

void ChatDlg::setEncoding(QAction* action)
{
  int encodingMib = action->data().toUInt();

  /* initialize a codec according to the encoding menu item id */
  QString encoding( UserCodec::encodingForMib(encodingMib) );

  if (!encoding.isNull()) {
    QTextCodec* _codec = QTextCodec::codecForName(encoding.toLatin1());
    if (_codec == NULL)
    {
      WarnUser(this, QString(tr("Unable to load encoding <b>%1</b>. Message contents may appear garbled.")).arg(encoding));
      return;
    }
    codec = _codec;

    // transmit to remote
    sendFontInfo();

    emit encodingChanged();
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ChatWindow::ChatWindow (QWidget* parent)
  : QTextEdit(parent)
{
  setLineWrapMode(WidgetWidth);
  setWordWrapMode(QTextOption::WordWrap);
  setFont(Config::General::instance()->editFont());
}


// -----------------------------------------------------------------------------

void ChatWindow::appendNoNewLine(const QString& s)
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::End);
  tc.insertText(s);
}


// -----------------------------------------------------------------------------

void ChatWindow::GotoEnd()
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::End);
  setTextCursor(tc);
}


// -----------------------------------------------------------------------------

void ChatWindow::insert(const QString& s)
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::End);
  tc.insertText(s);
}


// -----------------------------------------------------------------------------

void ChatWindow::keyPressEvent(QKeyEvent* e)
{
  if ( (e->text().length() == 0 ||
        e->modifiers() & Qt::ControlModifier ||
        e->modifiers() & Qt::AltModifier) &&
       (e->key() != Qt::Key_Tab &&
        e->key() != Qt::Key_Backtab &&
        e->key() != Qt::Key_Backspace &&
        e->key() != Qt::Key_Return &&
        e->key() != Qt::Key_Enter))
    return;

  GotoEnd();

  // the order of the two is important -- on Enter, first QMultiLineEdit adds
  // a line break, and later we clear the input line, and not vice versa
  QTextEdit::keyPressEvent(e);
  emit keyPressed(e);
}


// -----------------------------------------------------------------------------

void ChatWindow::mousePressEvent(QMouseEvent*)
{
  // a user might not change the cursor position
  // and marking / cutting away text is not allowed

  // so ignore the event.
}

void ChatWindow::mouseMoveEvent(QMouseEvent*)
{
  // ignore it
}

void ChatWindow::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::MidButton && !isReadOnly())
    paste();
}

// -----------------------------------------------------------------------------

void ChatWindow::paste()
{
  QString t = QApplication::clipboard()->text();

  if ( !t.isEmpty() ) {

    for (int i=0; i<t.length(); i++) {
      if ( t[i] < ' ' && t[i] != '\n' && t[i] != '\t' )
        t[i] = ' ';
    }

    for(int i=0; i<t.length(); i++) {
      QKeyEvent press(QKeyEvent::KeyPress, t[i].toLatin1() == '\n' ? Qt::Key_Enter : 0, static_cast<Qt::KeyboardModifiers>(Qt::NoModifier), QString(t[i]), false, 1);

      keyPressEvent(&press);
    }
  }
}


// -----------------------------------------------------------------------------

void ChatWindow::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QPalette::Base, c);
  pal.setColor(QPalette::Inactive, QPalette::Base, c);

  setPalette(pal);
}


// -----------------------------------------------------------------------------


void ChatWindow::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QPalette::Text, c);
  pal.setColor(QPalette::Inactive, QPalette::Text, c);

  setPalette(pal);
}

void ChatWindow::backspace()
{
  QTextCursor tc = textCursor();
  tc.deletePreviousChar();
}

QString ChatWindow::lastLine() const
{
  QString text = toPlainText();

  // Ignore the last empty line
  if (text.endsWith("\n"))
    text = text.left(text.size() - 1);

  int pos = text.lastIndexOf("\n");
  if (pos == -1)
    return text;
  return text.mid(pos + 1);
}
