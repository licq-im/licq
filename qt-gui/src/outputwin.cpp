#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qdatetime.h>
#include <qlayout.h>

#include "outputwin.h"
#include "icq-defines.h"
#include "licq-locale.h"

//---------------------------------------------------------------------------

CQtLogWindow::CQtLogWindow(QWidget *parent = 0, const char *name = 0)
  : QDialog(parent, name)
{
  setCaption(_("Licq Network Log"));

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  outputBox = new MLEditWrap(false, this);
  outputBox->setMinimumHeight(outputBox->frameWidth()*2
                              + 10*outputBox->fontMetrics().lineSpacing());
  outputBox->setMinimumWidth(380);
  outputBox->setReadOnly(true);
  top_lay->addWidget(outputBox);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  btnClear = new QPushButton(_("C&lear"), this);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));
  lay->addStretch(1);
  lay->addWidget(btnClear);

  btnHide = new QPushButton(_("&Close"), this);
  btnHide->setDefault(true);
  connect(btnHide, SIGNAL(clicked()), this, SLOT(hide()));
  lay->addWidget(btnHide);

  sn = new QSocketNotifier(Pipe(), QSocketNotifier::Read, this);
  connect (sn, SIGNAL(activated(int)), this, SLOT(slot_log(int)));
}

// --------------------------------------------------------------------------

void CQtLogWindow::showEvent(QShowEvent *)
{
  // move Cursor to the end of the QMultiLineEdit
  outputBox->goToEnd();
}

// --------------------------------------------------------------------------

void CQtLogWindow::slot_log(int s)
{
  char buf[4];
  read(s, buf, 1);

  outputBox->appendNNL(NextLogMsg());
  outputBox->goToEnd();

  // hardcoded limit, maybe should be user configurable?
  if(outputBox->numLines() > 500)
    outputBox->removeLine(0);

  if (NextLogType() == L_ERROR)
    (void) new CLicqMessageBox(NextLogMsg(), QMessageBox::Critical);

  ClearLog();
}


#include "moc/moc_outputwin.h"
