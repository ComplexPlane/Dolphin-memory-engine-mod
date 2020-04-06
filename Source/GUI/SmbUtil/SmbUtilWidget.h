#pragma once

#include <QWidget>
#include <QPushButton>

#include "../../SmbUtil/SmbUtil.h"

class SmbUtilWidget : public QWidget
{
  Q_OBJECT

public:
  SmbUtilWidget();
  ~SmbUtilWidget();

  void onSaveState();
  void onLoadState();
  void onBisectGood();
  void onBisectBad();

private:
  void initializeWidgets();
  void makeLayouts();

  QPushButton* m_btnSaveState;
  QPushButton* m_btnLoadState;
  QPushButton* m_btnBisectGood;
  QPushButton* m_btnBisectBad;
  SmbUtil* m_smbUtil;

};
