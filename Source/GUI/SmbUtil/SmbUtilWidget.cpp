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

  connect(m_btnSaveState, &QPushButton::clicked, this, &SmbUtilWidget::onSaveState);
  connect(m_btnLoadState, &QPushButton::clicked, this, &SmbUtilWidget::onLoadState);
}

void SmbUtilWidget::makeLayouts() {
  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(m_btnSaveState);
  layout->addWidget(m_btnLoadState);
  setLayout(layout);
}

void SmbUtilWidget::onSaveState() {
  m_smbUtil->saveState();
}

void SmbUtilWidget::onLoadState() {
  m_smbUtil->loadState();
}

SmbUtilWidget::~SmbUtilWidget() {
  delete m_smbUtil;
}