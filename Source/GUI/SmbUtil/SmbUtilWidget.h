#pragma once

#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>

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
  QPlainTextEdit* m_textEdit;
  SmbUtil* m_smbUtil;

  void printLog(const QString& string);
};
