#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qfile.h>

#include "editskin.h"
#include "ewidgets.h"
#include "constants.h"

//---------------------------------------------------------------------------

EditSkinDlg::EditSkinDlg(QString skin, QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  setCaption(tr("Licq Skin Editor - %1").arg(skin));

  sSkin = skin;

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleSkin = new QMultiLineEdit(this);
  mleSkin->setMinimumHeight(mleSkin->frameWidth() * 2
                            + 20 * mleSkin->fontMetrics().lineSpacing());
  mleSkin->setMinimumWidth(mleSkin->fontMetrics().width("_") * 80);
  top_lay->addWidget(mleSkin);

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

  // Load up the skin
  sSkinConf = QString(SHARE_DIR) + QString("qt-gui/skin.") +
              sSkin + QChar('/') + sSkin + QString(".skin");
  QFile f(sSkinConf);
  if (!f.open(IO_ReadOnly))
  {
    WarnUser(this, tr("Failed to open skin configuration file:\n%1").arg(sSkinConf));
    mleSkin->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    mleSkin->setText(t.read());
    f.close();
  }
}

// --------------------------------------------------------------------------

void EditSkinDlg::hide()
{
  QWidget::hide();
  delete this;
}


void EditSkinDlg::slot_save()
{
  QFile f(sSkinConf);
  if (!f.open(IO_WriteOnly))
  {
    WarnUser(this, tr("Failed to open skin configuration file:\n%1").arg(sSkinConf));
    mleSkin->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    t << mleSkin->text();
    f.close();
  }

}


#include "moc/moc_editskin.h"
