#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qlayout.h>
#if USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qfile.h>
#include <qtextstream.h>

#include "outputwin.h"
#include "licq_icq.h"

//---------------------------------------------------------------------------

CQtLogWindow::CQtLogWindow(QWidget *parent)
  : QDialog(parent, "NetworkLog")
{
  setCaption(tr("Licq Network Log"));

  QBoxLayout* top_lay = new QVBoxLayout(this, 8);

  outputBox = new CLogWidget(this);
  outputBox->setMinimumHeight(outputBox->frameWidth()*2
                              + 16*outputBox->fontMetrics().lineSpacing());
  outputBox->setMinimumWidth(outputBox->minimumHeight()*2);
  top_lay->addWidget(outputBox);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 8);

  btnSave = new QPushButton(tr("&Save"), this);
  connect(btnSave, SIGNAL(clicked()), SLOT(slot_save()));
  btnClear = new QPushButton(tr("C&lear"), this);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));
#if QT_VERSION < 210
  btnClear->setEnabled(false);
#endif
  btnHide = new QPushButton(tr("&Close"), this);
  btnHide->setDefault(true);
  connect(btnHide, SIGNAL(clicked()), this, SLOT(hide()));
  int bw = 75;
  bw = QMAX(bw, btnClear->sizeHint().width());
  bw = QMAX(bw, btnHide->sizeHint().width());
  bw = QMAX(bw, btnSave->sizeHint().width());
  btnClear->setFixedWidth(bw);
  btnHide->setFixedWidth(bw);
  btnSave->setFixedWidth(bw);
  lay->addStretch(1);
  lay->addWidget(btnSave);
  lay->addSpacing(30);
  lay->addWidget(btnClear);
  lay->addWidget(btnHide);

  sn = new QSocketNotifier(Pipe(), QSocketNotifier::Read, this);
  connect (sn, SIGNAL(activated(int)), this, SLOT(slot_log(int)));
}

// --------------------------------------------------------------------------

void CQtLogWindow::showEvent(QShowEvent *)
{
  // move Cursor to the end of the QMultiLineEdit
  outputBox->GotoEnd();
}

// --------------------------------------------------------------------------

void CQtLogWindow::slot_log(int s)
{
  char buf[4];
  read(s, buf, 1);

  QString str = QString::fromLocal8Bit(NextLogMsg());

  outputBox->appendNoNewLine(str);
  outputBox->GotoEnd();

  // hardcoded limit, maybe should be user configurable?
  while (outputBox->numLines() > 500)
    outputBox->removeLine(0);

  if (NextLogType() == L_ERROR)
    CriticalUser(NULL, str);
  else if (NextLogType() == L_MESSAGE)
    InformUser(NULL, str);

  ClearLog();
}

void CQtLogWindow::slot_save()
{
  QString fn;
#ifdef USE_KDE
  KURL u = KFileDialog::getSaveURL(
      QString(QDir::homeDirPath() + "/licq.log"),QString::null, this);
  fn = u.path();
#else
  fn = QFileDialog::getSaveFileName(
         QString(QDir::homeDirPath() + "/licq.log"),
         QString::null, this);
#endif
  if (fn.isNull()) return;

  QFile f(fn);
  if (!f.open(IO_WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(fn));
  }
  else
  {
    QTextStream t(&f);
    t << outputBox->text();
    f.close();
  }

}

#include "outputwin.moc"
