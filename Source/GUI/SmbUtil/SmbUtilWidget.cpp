#include <QHBoxLayout>
#include "SmbUtilWidget.h"

SmbUtilWidget::SmbUtilWidget()
{
  initializeWidgets();
  makeLayouts();
}

void SmbUtilWidget::initializeWidgets() {
  m_smbUtil = new SmbUtil();

  m_btnSaveState = new QPushButton(tr("Save State"));
  m_btnLoadState = new QPushButton(tr("Load State"));
  m_btnBisectGood = new QPushButton(tr("Mark Bisect Region Good"));
  m_btnBisectBad = new QPushButton(tr("Mark Bisect Region Bad"));

  m_textEdit = new QPlainTextEdit();
  m_textEdit->setFont(QFont("Consolas", 10));
  m_textEdit->setReadOnly(true);

  connect(m_btnSaveState, &QPushButton::clicked, this, &SmbUtilWidget::onSaveState);
  connect(m_btnLoadState, &QPushButton::clicked, this, &SmbUtilWidget::onLoadState);
  connect(m_btnBisectGood, &QPushButton::clicked, this, &SmbUtilWidget::onBisectGood);
  connect(m_btnBisectBad, &QPushButton::clicked, this, &SmbUtilWidget::onBisectBad);
  connect(m_smbUtil, &SmbUtil::onLog, this, &SmbUtilWidget::printLog);

  m_smbUtil->printBisectState();
}

void SmbUtilWidget::makeLayouts() {
  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(m_btnSaveState);
  layout->addWidget(m_btnLoadState);
  layout->addWidget(m_btnBisectGood);
  layout->addWidget(m_btnBisectBad);

  QVBoxLayout* vlayout = new QVBoxLayout();
  vlayout->addLayout(layout);
  vlayout->addWidget(m_textEdit);

  setLayout(vlayout);
}

void SmbUtilWidget::onSaveState() {
  m_smbUtil->saveState();
}

void SmbUtilWidget::onLoadState() {
  m_smbUtil->loadState();
}

void SmbUtilWidget::onBisectGood() {
  m_smbUtil->bisectGood();
}

void SmbUtilWidget::onBisectBad() {
  m_smbUtil->bisectBad();
}

void SmbUtilWidget::printLog(const QString& string) {
  m_textEdit->appendPlainText(string);
  m_textEdit->moveCursor(QTextCursor::End);
}

SmbUtilWidget::~SmbUtilWidget() {
  delete m_smbUtil;
}
