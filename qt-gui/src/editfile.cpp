#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfileinfo.h>

#include "editfile.h"
#include "mledit.h"
#include "ewidgets.h"
#include "constants.h"

//---------------------------------------------------------------------------

EditFileDlg::EditFileDlg(QString fname, QWidget *parent, const char *name)
  : QWidget(parent, name), sFile(fname)
{
  setCaption(tr("Licq File Editor - %1").arg(fname));

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleFile = new MLEditWrap(false, this, false);
  mleFile->setMinimumHeight(mleFile->frameWidth() * 2
                            + 20 * mleFile->fontMetrics().lineSpacing());
  mleFile->setMinimumWidth(mleFile->fontMetrics().width("_") * 80);
  top_lay->addWidget(mleFile);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  btnSave = new QPushButton(tr("&Save"), this);
  connect(btnSave, SIGNAL(clicked()), this, SLOT(slot_save()));
  btnClose = new QPushButton(tr("&Close"), this);
  btnClose->setDefault(true);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(hide()));
  int bw = 75;
  bw = QMAX(bw, btnSave->sizeHint().width());
  bw = QMAX(bw, btnClose->sizeHint().width());
  btnSave->setFixedWidth(bw);
  btnClose->setFixedWidth(bw);
  lay->addStretch(1);
  lay->addWidget(btnSave);
  lay->addWidget(btnClose);

  show();

  // Load up the file
  QFile f(sFile);
  if (!f.open(IO_ReadOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(sFile));
    mleFile->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    mleFile->setText(t.read());
    f.close();
    QFileInfo fi(f);
    if (!fi.isWritable())
      setCaption(caption() + QString(tr("[ Read-Only ]")));
  }
}

// --------------------------------------------------------------------------

void EditFileDlg::hide()
{
  QWidget::hide();
  delete this;
}


void EditFileDlg::slot_save()
{
  QFile f(sFile);
  if (!f.open(IO_WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(sFile));
    mleFile->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    t << mleFile->text();
    f.close();
  }

}

#include "editfile.moc"
