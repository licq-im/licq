#include "mledit.h"

void ICQFunctions::CreateHistoryTab()
{
  unsigned short CR = 0;

  tabLabel[TAB_HISTORY] = tr("History");
  fcnTab[TAB_HISTORY] = new QWidget(this, tabLabel[TAB_HISTORY]);
  QWidget *p = fcnTab[TAB_HISTORY];

  QGridLayout *lay = new QGridLayout(p, 4, 4, 8, 0);

  lblHistInfo = new QLabel(p);
  lblHistInfo->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
  lblHistInfo->setAlignment(AlignCenter);
  lay->addMultiCellWidget(lblHistInfo, CR, CR, 0, 2);
  CR++;

  mleHistory = new MLEditWrap(true, p);
  lay->addMultiCellWidget(mleHistory, CR, CR, 0, 2);

  scrHistory = new QScrollBar(Vertical, p);
  connect(scrHistory, SIGNAL(sliderMoved(int)), this, SLOT(slot_histslider(int)));
  lay->addMultiCellWidget(scrHistory, CR-1, CR, 3, 3);
  CR++;

  lblHistory = new QLabel(p);
  lay->addMultiCellWidget(lblHistory, CR, CR, 0, 1);

  chkHistoryReverse = new QCheckBox("Reverse",p);
  connect(chkHistoryReverse,SIGNAL(toggled(bool)),SLOT(ReverseHistory(bool)));
  lay->addWidget(chkHistoryReverse, CR, 2);
  chkHistoryReverse->setChecked(true);
}

void ICQFunctions::slot_histslider(int val)
{
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    HistoryListIter Iter = m_lHistoryList.end();
    int i = val;
    while(i--)
        Iter--;

    QDateTime d;
    QString s;

    d.setTime_t((*Iter)->Time());
    s = QString("%7/%8: %1 %2 (%3) [%4%5%6]")
        .arg((*Iter)->Direction() == D_RECEIVER ? "to you" : "from you")
        .arg(EventDescription(*Iter))
        .arg(d.toString())
        .arg((*Iter)->IsDirect() ? 'D' : '-')
        .arg((*Iter)->IsMultiRec() ? 'M' : '-')
        .arg((*Iter)->IsUrgent() ? 'U' : '-')
        .arg(val)
        .arg(scrHistory->maxValue());
    lblHistInfo->setText(s);

    mleHistory->setText(QString::fromLocal8Bit((*Iter)->Text()));
    gUserManager.DropUser(u);
}



//-----ICQFunctions::SetupHistory--------------------------------------------
void ICQFunctions::SetupHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  //nfoHistory->setData(u->HistoryName());
  if (!u->GetHistory(m_lHistoryList))
  {
    mleHistory->setText(tr("Error loading history"));
  }
  else
  {
      int histnum = QMIN(m_lHistoryList.size(), NUM_MSG_PER_HISTORY);
      scrHistory->setRange(1, histnum);

    m_bHistoryReverse = true;
    m_iHistoryEIter = m_lHistoryList.end();
    m_iHistorySIter = m_iHistoryEIter;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistorySIter != m_lHistoryList.begin());
         i++)
    {
      m_iHistorySIter--;
    }
    m_nHistoryIndex = m_lHistoryList.size();
    ShowHistory();
  }
  gUserManager.DropUser(u);
}

void ICQFunctions::ReverseHistory(bool newVal)
{
  if (chkHistoryReverse->isChecked() != newVal)
    chkHistoryReverse->setChecked(newVal);
  else if(m_bHistoryReverse != newVal)
  {
    m_bHistoryReverse = newVal;
    ShowHistory();
  }
}

void ICQFunctions::ShowHistoryPrev()
{
  if (m_iHistorySIter != m_lHistoryList.begin())
  {
    m_iHistoryEIter = m_iHistorySIter;
    m_nHistoryIndex -= NUM_MSG_PER_HISTORY;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistorySIter != m_lHistoryList.begin());
         i++)
    {
	    m_iHistorySIter--;
    }
    ShowHistory();
  }
}

void ICQFunctions::ShowHistoryNext()
{
  if (m_iHistoryEIter != m_lHistoryList.end())
  {
    m_iHistorySIter = m_iHistoryEIter;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistoryEIter != m_lHistoryList.end());
         i++)
    {
	    m_iHistoryEIter++;
	    m_nHistoryIndex++;
    }
    ShowHistory();
  }
}

void ItalisizeLine(QString &t, QString pre, unsigned short inspos)
{
  int i = 0;
  while ( (i = t.find(pre, i)) != -1)
  {
    t.insert(i + inspos, "<i>");
    i = t.find("<br>", i + inspos);
    if (i == -1)
    {
      t.append("</i>");
      break;
    }
    else
      t.insert(i, "</i>");
  }
}


//-----ICQFunctions::ShowHistory--------------------------------------------
void ICQFunctions::ShowHistory()
{
  m_nHistoryShowing = QMIN(m_lHistoryList.size(), NUM_MSG_PER_HISTORY);
  lblHistory->setText(tr("[<font color=\"%1\">Received</font>] "
                         "[<font color=\"%2\">Sent</font>] "
                         "%3 to %4 of %5")
                      .arg(COLOR_RECEIVED).arg(COLOR_SENT)
                      .arg(m_nHistoryIndex - m_nHistoryShowing + 1)
                      .arg(m_nHistoryIndex)
                      .arg(m_lHistoryList.size()));
  scrHistory->setValue(1);
}


//-----ICQFunctions::saveHistory---------------------------------------------
void ICQFunctions::saveHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  u->SaveHistory(mleHistory->text().local8Bit());
  gUserManager.DropUser(u);
}

