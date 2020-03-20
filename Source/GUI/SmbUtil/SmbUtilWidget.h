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

private:
  void initializeWidgets();
  void makeLayouts();

  QPushButton* m_btnSaveState;
  QPushButton* m_btnLoadState;
  SmbUtil* m_smbUtil;

};
