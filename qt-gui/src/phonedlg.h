#ifndef PHONEDLG_H
#define PHONEDLG_H

#include <qdialog.h>

#include "licq_user.h"

class QCheckBox;

class EditPhoneDlg : public QDialog
{
  Q_OBJECT
public:
  EditPhoneDlg(QWidget *parent, const struct PhoneBookEntry *pbe = 0,
               int nEntry = -1);

protected:
  QCheckBox *cbRemove0s;
  QComboBox *cmbType, *cmbDescription, *cmbCountry, *cmbProvider;
  QLineEdit *leAreaCode, *leNumber, *leExtension, *leGateway;
  int m_nEntry;

signals:
  void updated(struct PhoneBookEntry, int);

protected slots:
  void ok();
  void UpdateDlg(int);
  void ProviderChanged(int);
};

#endif
