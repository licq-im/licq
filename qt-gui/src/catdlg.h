#ifndef CATDLG_H
#define CATDLG_H

#include <qdialog.h>

#include "licq_user.h"

class QPushButton;
class QComboBox;
class QLineEdit;

class EditCategoryDlg : public QDialog
{
  Q_OBJECT
public:
  EditCategoryDlg(QWidget *parent, ICQUserCategory *uCat);

protected:
  static const unsigned MAX_CAT = ICQUserCategory::MAX_CATEGORIES;
  QComboBox *cbCat[MAX_CAT];
  QLineEdit *leDescr[MAX_CAT];
  const struct SCategory *(*m_fGetEntry)(unsigned short);
  UserCat m_uc;
  unsigned short m_nCats;
signals:
  void updated(ICQUserCategory *);

protected slots:
  void ok();
  void checkEnabled(int index);
};

#endif
