#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "skinbrowser.h"
#include "licq-locale.h"
#include "support.h"
#include "constants.h"
#include "log.h"

int selectPlugins(const struct dirent *d)
{
  return(strncmp(d->d_name, "skin.", 5) == 0);
}
int selectIcons(const struct dirent *d)
{
  return(strncmp(d->d_name, "icons.", 6) == 0);
}

SkinBrowserDlg::SkinBrowserDlg(CMainWindow *_mainwin, QWidget *parent = 0, const char *name = 0 )
  : QWidget(parent, name)
{
  mainwin = _mainwin;

  grpSkins = new QGroupBox(_("Skins"), this);
  lstSkins = new QListBox(grpSkins);
  btnApplySkin = new QPushButton(_("Apply"), grpSkins);
  grpIcons = new QGroupBox(_("Icons"), this);
  lstIcons = new QListBox(grpIcons);
  btnApplyIcons = new QPushButton(_("Apply"), grpIcons);
  btnDone = new QPushButton(_("Done"), this);
  setCaption(_("Licq Skin Browser"));

  // Load up the available packs
  char szDir[MAX_FILENAME_LEN];
  struct dirent **namelist;
  sprintf(szDir, "%s%s", SHARE_DIR, QTGUI_DIR);
  int n = scandir_r(szDir, &namelist, selectPlugins, alphasort);
  if (n < 0)
  {
    gLog.Error("%sError reading qt-gui directory %s:\n%s%s.\n",
               L_ERRORxSTR, szDir, L_BLANKxSTR, strerror(errno));
    lstSkins->insertItem(_("ERROR"));
    lstSkins->setEnabled(false);
  }
  else
  {
    for (unsigned short i = 0; i < n; i++)
    {
      lstSkins->insertItem(&namelist[i]->d_name[5]);
      free (namelist[i]);
    }
    free (namelist);
  }
  n = scandir_r(szDir, &namelist, selectIcons, alphasort);
  if (n < 0)
  {
    gLog.Error("%sError reading qt-gui directory %s:\n%s%s.\n",
               L_ERRORxSTR, szDir, L_BLANKxSTR, strerror(errno));
    lstSkins->insertItem(_("ERROR"));
    lstSkins->setEnabled(false);
  }
  else
  {
    for (unsigned short i = 0; i < n; i++)
    {
      lstIcons->insertItem(&namelist[i]->d_name[6]);
      free (namelist[i]);
    }
    free (namelist);
  }

  setGeometry(mainwin->x() - 100, mainwin->y() + 100, 300,
              150 + (lstSkins->count() > lstIcons->count() ? lstSkins->count() : lstIcons->count()) * 15);

  connect(btnApplySkin, SIGNAL(clicked()), this, SLOT(slot_applyskin()));
  connect(btnApplyIcons, SIGNAL(clicked()), this, SLOT(slot_applyicons()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
}


void SkinBrowserDlg::slot_applyskin()
{
  if (lstSkins->currentItem() == -1) return;
  mainwin->ApplySkin(lstSkins->text(lstSkins->currentItem()));
}


void SkinBrowserDlg::slot_applyicons()
{
  if (lstIcons->currentItem() == -1) return;
  mainwin->ApplyIcons(lstIcons->text(lstIcons->currentItem()));
}


void SkinBrowserDlg::hide()
{
   QWidget::hide();
   delete this;
}


void SkinBrowserDlg::resizeEvent (QResizeEvent *)
{
  grpSkins->setGeometry(10, 10, width() / 2 - 20, height() - 60);
  lstSkins->setGeometry(10, 20, grpSkins->width() - 20, grpSkins->height() - 70);
  btnApplySkin->setGeometry(grpSkins->width() / 2 - 50, grpSkins->height() - 40, 100, 30);
  grpIcons->setGeometry(width() / 2 + 5, 10, width() / 2 - 20, height() - 60);
  lstIcons->setGeometry(10, 20, grpIcons->width() - 20, grpIcons->height() - 70);
  btnApplyIcons->setGeometry(grpIcons->width() / 2 - 50, grpIcons->height() - 40, 100, 30);
  btnDone->setGeometry(width() / 2 - 50, height() - 40, 100, 30);
}

#include "moc/moc_skinbrowser.h"
