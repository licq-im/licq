#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlayout.h>

#include <unistd.h>


#include "outputwin.h"
#include "icq-defines.h"
#include "licq-locale.h"



//-----OutputWin::constructor----------------------------------------------------------------------
CQtLogWindow::CQtLogWindow(QWidget *parent = 0, const char *name = 0)
  : QWidget(parent, name)
{
  setCaption(_("Licq Network Log"));
  
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);
  
  outputBox = new MLEditWrap (true, this);
  outputBox->setFixedVisibleLines(10);
  outputBox->setMinimumWidth(360);
  outputBox->setReadOnly(true);
  top_lay->addWidget(outputBox);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);
  
  btnClear = new QPushButton(_("C&lear"), this);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));
  lay->addWidget(btnClear);
  lay->addStretch(1);
  
  btnHide = new QPushButton(_("&Close"), this);
  connect(btnHide, SIGNAL(clicked()), this, SLOT(hide()));
  lay->addWidget(btnHide);
  
  sn = new QSocketNotifier(Pipe(), QSocketNotifier::Read, this);
  connect (sn, SIGNAL(activated(int)), this, SLOT(slot_log(int)));
}

void CQtLogWindow::slot_log(int s)
{
  char buf[4];
  read(s, buf, 1);
  outputBox->append(QString(NextLogMsg()).stripWhiteSpace());
  outputBox->goToEnd();
  if (NextLogType() == L_ERROR)
  {
    (void) new CLicqMessageBox(NextLogMsg(), QMessageBox::Critical);
  }
  ClearLog();
}

#include "moc/moc_outputwin.h"
