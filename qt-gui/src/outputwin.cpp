#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include "outputwin.h"
#include "icq-defines.h"
#include "licq-locale.h"



//-----OutputWin::constructor----------------------------------------------------------------------
CQtLogWindow::CQtLogWindow(QWidget *parent = 0, const char *name = 0)
  : QWidget(parent, name)
{
   resize(430, 220);

   outputBox = new MLEditWrap (true, this);
   outputBox->setGeometry(5, 5, 400, 200);
   outputBox->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);
   outputBox->setReadOnly(true);

   btnHide = new QPushButton(_("Hide Window"), this);
   connect(btnHide, SIGNAL(clicked()), this, SLOT(hide()));

   btnClear = new QPushButton(_("Clear"), this);
   connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));

   sn = new QSocketNotifier(Pipe(), QSocketNotifier::Read, this);
   connect (sn, SIGNAL(activated(int)), this, SLOT(slot_log(int)));
}

void CQtLogWindow::slot_log(int s)
{
  char buf[4];
  read(s, buf, 1);
  outputBox->append(NextLogMsg());
  outputBox->goToEnd();
  if (NextLogType() == L_ERROR)
  {
    (void) new CLicqMessageBox(NextLogMsg(), QMessageBox::Critical);
  }
  ClearLog();
}


//-----OutputWin::resizeEvent----------------------------------------------------------------------
void CQtLogWindow::resizeEvent(QResizeEvent *)
{
   outputBox->resize(width() - 10, height() - 45);
   btnHide->setGeometry(5, height() - 35, 100, 30);
   btnClear->setGeometry(120, height() - 35, 100, 30);
}

#include "moc/moc_outputwin.h"
