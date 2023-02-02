#include "MemScanWidget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QRadioButton>
#include <QRegExp>
#include <QShortcut>
#include <QVBoxLayout>

#include "../GUICommon.h"

MemScanWidget::MemScanWidget()
{
  initialiseWidgets();
  makeLayouts();
}

MemScanWidget::~MemScanWidget()
{
  delete m_memScanner;
  delete m_resultsListModel;
}

void MemScanWidget::initialiseWidgets()
{
  m_memScanner = new MemScanner();

  m_resultsListModel = new ResultsListModel(this, m_memScanner);

  m_lblResultCount = new QLabel("");
  m_tblResulstList = new QTableView();
  m_tblResulstList->setModel(m_resultsListModel);
  m_tblResulstList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_tblResulstList->horizontalHeader()->setStretchLastSection(true);
  m_tblResulstList->horizontalHeader()->resizeSection(ResultsListModel::RESULT_COL_ADDRESS, 125);
  m_tblResulstList->horizontalHeader()->resizeSection(ResultsListModel::RESULT_COL_SCANNED, 150);

  m_tblResulstList->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblResulstList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_tblResulstList->setMinimumWidth(385);
  connect(m_tblResulstList, &QAbstractItemView::doubleClicked, this,
          &MemScanWidget::onResultListDoubleClicked);

  m_btnAddAll = new QPushButton(tr("Add all"));
  connect(m_btnAddAll, &QPushButton::clicked, this, &MemScanWidget::onAddAll);
  m_btnAddAll->setEnabled(false);

  m_btnAddSelection = new QPushButton(tr("Add selection"));
  connect(m_btnAddSelection, &QPushButton::clicked, this, &MemScanWidget::onAddSelection);
  m_btnAddSelection->setEnabled(false);

  m_btnRemoveSelection = new QPushButton(tr("Remove selection"));
  connect(m_btnRemoveSelection, &QPushButton::clicked, this, &MemScanWidget::onRemoveSelection);
  m_btnRemoveSelection->setEnabled(false);

  m_btnFirstScan = new QPushButton(tr("First scan"));
  m_btnNextScan = new QPushButton(tr("Next scan"));
  m_btnNextScan->hide();
  m_btnResetScan = new QPushButton(tr("Reset scan"));
  m_btnResetScan->hide();

  connect(m_btnFirstScan, &QPushButton::clicked, this, &MemScanWidget::onFirstScan);
  connect(m_btnNextScan, &QPushButton::clicked, this, &MemScanWidget::onNextScan);
  connect(m_btnResetScan, &QPushButton::clicked, this, &MemScanWidget::onResetScan);

  QShortcut* scanShortcut = new QShortcut(QKeySequence(Qt::Key::Key_Enter), this);
  connect(scanShortcut, &QShortcut::activated, this, [=] {
    if (m_memScanner->hasScanStarted())
      onNextScan();
    else
      onFirstScan();
  });

  m_txbSearchTerm1 = new QLineEdit();
  m_txbSearchTerm2 = new QLineEdit();

  m_searchTerm2Widget = new QWidget();

  m_searchTerm2Widget->hide();

  m_cmbScanType = new QComboBox();
  m_cmbScanType->addItems(GUICommon::g_memTypeNames);
  m_cmbScanType->setCurrentIndex(0);
  connect(m_cmbScanType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MemScanWidget::onScanMemTypeChanged);
  m_variableLengthType = false;

  m_cmbScanFilter = new QComboBox();
  updateScanFilterChoices();
  connect(m_cmbScanFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MemScanWidget::onScanFilterChanged);

  m_rdbBaseDecimal = new QRadioButton(tr("Decimal"));
  m_rdbBaseHexadecimal = new QRadioButton(tr("Hexadecimal"));
  m_rdbBaseOctal = new QRadioButton(tr("Octal"));
  m_rdbBaseBinary = new QRadioButton(tr("Binary"));

  m_btnGroupScanBase = new QButtonGroup(this);
  m_btnGroupScanBase->addButton(m_rdbBaseDecimal, 0);
  m_btnGroupScanBase->addButton(m_rdbBaseHexadecimal, 1);
  m_btnGroupScanBase->addButton(m_rdbBaseOctal, 2);
  m_btnGroupScanBase->addButton(m_rdbBaseBinary, 3);
  m_rdbBaseDecimal->setChecked(true);

  m_groupScanBase = new QGroupBox(tr("Base to use"));

  m_chkSignedScan = new QCheckBox(tr("Signed value scan"));
  m_chkSignedScan->setChecked(false);

  m_chkEnforceMemAlignement = new QCheckBox(tr("Enforce alignement"));
  m_chkEnforceMemAlignement->setChecked(true);

  m_currentValuesUpdateTimer = new QTimer(this);
  connect(m_currentValuesUpdateTimer, &QTimer::timeout, this,
          &MemScanWidget::onCurrentValuesUpdateTimer);
}

void MemScanWidget::makeLayouts()
{
  QLabel* lblAnd = new QLabel(tr("and"));

  QHBoxLayout* multiAddButtons_layout = new QHBoxLayout();
  multiAddButtons_layout->addWidget(m_btnAddSelection);
  multiAddButtons_layout->addWidget(m_btnAddAll);
  multiAddButtons_layout->addWidget(m_btnRemoveSelection);

  QVBoxLayout* results_layout = new QVBoxLayout();
  results_layout->addWidget(m_lblResultCount);
  results_layout->addWidget(m_tblResulstList);
  results_layout->addLayout(multiAddButtons_layout);

  QHBoxLayout* buttons_layout = new QHBoxLayout();
  buttons_layout->addWidget(m_btnFirstScan);
  buttons_layout->addWidget(m_btnNextScan);
  buttons_layout->addWidget(m_btnResetScan);

  QHBoxLayout* searchTerm2_layout = new QHBoxLayout();
  searchTerm2_layout->setContentsMargins(0, 0, 0, 0);
  searchTerm2_layout->addWidget(lblAnd);
  searchTerm2_layout->addWidget(m_txbSearchTerm2);
  m_searchTerm2Widget->setLayout(searchTerm2_layout);

  QHBoxLayout* searchTerms_layout = new QHBoxLayout();
  searchTerms_layout->addWidget(m_txbSearchTerm1);
  searchTerms_layout->addWidget(m_searchTerm2Widget);
  searchTerms_layout->setSizeConstraint(QLayout::SetMinimumSize);

  QHBoxLayout* layout_buttonsBase = new QHBoxLayout();
  layout_buttonsBase->addWidget(m_rdbBaseDecimal);
  layout_buttonsBase->addWidget(m_rdbBaseHexadecimal);
  layout_buttonsBase->addWidget(m_rdbBaseOctal);
  layout_buttonsBase->addWidget(m_rdbBaseBinary);
  m_groupScanBase->setLayout(layout_buttonsBase);

  QHBoxLayout* layout_extraParams = new QHBoxLayout();
  layout_extraParams->addWidget(m_chkEnforceMemAlignement);
  layout_extraParams->addWidget(m_chkSignedScan);

  QVBoxLayout* scannerParams_layout = new QVBoxLayout();
  scannerParams_layout->addLayout(buttons_layout);
  scannerParams_layout->addWidget(m_cmbScanType);
  scannerParams_layout->addWidget(m_cmbScanFilter);
  scannerParams_layout->addLayout(searchTerms_layout);
  scannerParams_layout->addWidget(m_groupScanBase);
  scannerParams_layout->addLayout(layout_extraParams);
  scannerParams_layout->addStretch();
  scannerParams_layout->setContentsMargins(0, 0, 0, 0);

  QWidget* scannerParamsWidget = new QWidget();
  scannerParamsWidget->setLayout(scannerParams_layout);
  scannerParamsWidget->setMinimumWidth(425);

  QHBoxLayout* main_layout = new QHBoxLayout();
  main_layout->addLayout(results_layout);
  main_layout->addWidget(scannerParamsWidget);
  main_layout->setContentsMargins(3, 0, 3, 0);

  setLayout(main_layout);
}

ResultsListModel* MemScanWidget::getResultListModel() const
{
  return m_resultsListModel;
}

std::vector<u32> MemScanWidget::getAllResults() const
{
  return m_memScanner->getResultsConsoleAddr();
}

QModelIndexList MemScanWidget::getSelectedResults() const
{
  return m_tblResulstList->selectionModel()->selectedRows();
}

MemScanner::ScanFiter MemScanWidget::getSelectedFilter() const
{
  int index =
      GUICommon::g_memScanFilter.indexOf(QRegExp("^" + m_cmbScanFilter->currentText() + "$"));
  return static_cast<MemScanner::ScanFiter>(index);
}

void MemScanWidget::updateScanFilterChoices()
{
  Common::MemType newType = static_cast<Common::MemType>(m_cmbScanType->currentIndex());
  m_cmbScanFilter->clear();
  if (newType == Common::MemType::type_byteArray || newType == Common::MemType::type_string)
  {
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::exact)));
  }
  else if (m_memScanner->hasScanStarted())
  {
    m_cmbScanFilter->addItems(GUICommon::g_memScanFilter);
    m_cmbScanFilter->removeItem(static_cast<int>(MemScanner::ScanFiter::unknownInitial));
  }
  else
  {
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::exact)));
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::between)));
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::biggerThan)));
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::smallerThan)));
    m_cmbScanFilter->addItem(
        GUICommon::g_memScanFilter.at(static_cast<int>(MemScanner::ScanFiter::unknownInitial)));
  }
  m_cmbScanFilter->setCurrentIndex(0);
}

void MemScanWidget::updateTypeAdditionalOptions()
{
  if (m_memScanner->typeSupportsAdditionalOptions(
          static_cast<Common::MemType>(m_cmbScanType->currentIndex())))
  {
    m_chkSignedScan->show();
    m_groupScanBase->show();
  }
  else
  {
    m_chkSignedScan->hide();
    m_groupScanBase->hide();
  }
}

void MemScanWidget::onScanFilterChanged()
{
  MemScanner::ScanFiter theFilter = getSelectedFilter();
  int numTerms = m_memScanner->getTermsNumForFilter(theFilter);
  switch (numTerms)
  {
  case 0:
    m_txbSearchTerm1->hide();
    m_searchTerm2Widget->hide();
    m_chkSignedScan->hide();
    m_groupScanBase->hide();
    break;
  case 1:
    m_txbSearchTerm1->show();
    m_searchTerm2Widget->hide();
    updateTypeAdditionalOptions();
    break;
  case 2:
    m_txbSearchTerm1->show();
    m_searchTerm2Widget->show();
    updateTypeAdditionalOptions();
    break;
  }
}

void MemScanWidget::onScanMemTypeChanged()
{
  Common::MemType newType = static_cast<Common::MemType>(m_cmbScanType->currentIndex());
  if (!m_variableLengthType &&
      (newType == Common::MemType::type_string || newType == Common::MemType::type_byteArray))
  {
    updateScanFilterChoices();
    m_variableLengthType = true;
  }
  else if (m_variableLengthType && newType != Common::MemType::type_string &&
           newType != Common::MemType::type_byteArray)
  {
    updateScanFilterChoices();
    m_variableLengthType = false;
  }

  updateTypeAdditionalOptions();
}

void MemScanWidget::onFirstScan()
{
  m_memScanner->setType(static_cast<Common::MemType>(m_cmbScanType->currentIndex()));
  m_memScanner->setIsSigned(m_chkSignedScan->isChecked());
  m_memScanner->setEnforceMemAlignement(m_chkEnforceMemAlignement->isChecked());
  m_memScanner->setBase(static_cast<Common::MemBase>(m_btnGroupScanBase->checkedId()));
  Common::MemOperationReturnCode scannerReturn =
      m_memScanner->firstScan(getSelectedFilter(), m_txbSearchTerm1->text().toStdString(),
                              m_txbSearchTerm2->text().toStdString());
  if (scannerReturn != Common::MemOperationReturnCode::OK)
  {
    handleScannerErrors(scannerReturn);
  }
  else
  {
    int resultsFound = static_cast<int>(m_memScanner->getResultCount());
    m_lblResultCount->setText(
        tr("%1 result(s) found", "", resultsFound).arg(QString::number(resultsFound)));
    if (resultsFound <= 1000 && resultsFound != 0)
    {
      m_btnAddAll->setEnabled(true);
      m_btnAddSelection->setEnabled(true);
      m_btnRemoveSelection->setEnabled(true);
    }
    m_btnFirstScan->hide();
    m_btnNextScan->show();
    m_btnResetScan->show();
    m_cmbScanType->setDisabled(true);
    m_chkSignedScan->setDisabled(true);
    m_chkEnforceMemAlignement->setDisabled(true);
    m_groupScanBase->setDisabled(true);
    updateScanFilterChoices();
  }
}

void MemScanWidget::onNextScan()
{
  Common::MemOperationReturnCode scannerReturn =
      m_memScanner->nextScan(getSelectedFilter(), m_txbSearchTerm1->text().toStdString(),
                             m_txbSearchTerm2->text().toStdString());
  if (scannerReturn != Common::MemOperationReturnCode::OK)
  {
    handleScannerErrors(scannerReturn);
  }
  else
  {
    int resultsFound = static_cast<int>(m_memScanner->getResultCount());
    m_lblResultCount->setText(
        tr("%1 result(s) found", "", resultsFound).arg(QString::number(resultsFound)));
    if (resultsFound <= 1000 && resultsFound != 0)
    {
      m_btnAddAll->setEnabled(true);
      m_btnAddSelection->setEnabled(true);
      m_btnRemoveSelection->setEnabled(true);
    }
  }
}

void MemScanWidget::onResetScan()
{
  m_memScanner->reset();
  m_lblResultCount->setText("");
  m_btnAddAll->setEnabled(false);
  m_btnAddSelection->setEnabled(false);
  m_btnRemoveSelection->setEnabled(false);
  m_btnFirstScan->show();
  m_btnNextScan->hide();
  m_btnResetScan->hide();
  m_cmbScanType->setEnabled(true);
  m_chkSignedScan->setEnabled(true);
  m_chkEnforceMemAlignement->setEnabled(true);
  m_groupScanBase->setEnabled(true);
  m_resultsListModel->updateAfterScannerReset();
  updateScanFilterChoices();
}

void MemScanWidget::onAddSelection()
{
  emit requestAddSelectedResultsToWatchList(m_memScanner->getType(), m_memScanner->getLength(),
                                            m_memScanner->getIsUnsigned(), m_memScanner->getBase());
}

void MemScanWidget::onRemoveSelection()
{
  if (!m_tblResulstList->selectionModel()->hasSelection())
    return;

  while (m_tblResulstList->selectionModel()->hasSelection())
    m_resultsListModel->removeRow(m_tblResulstList->selectionModel()->selectedRows().at(0).row());

  // The result count is already updated at the backend by this point
  int resultsFound = static_cast<int>(m_memScanner->getResultCount());
  m_lblResultCount->setText(
      tr("%1 result(s) found", "", resultsFound).arg(QString::number(resultsFound)));
}

void MemScanWidget::onAddAll()
{
  emit requestAddAllResultsToWatchList(m_memScanner->getType(), m_memScanner->getLength(),
                                       m_memScanner->getIsUnsigned(), m_memScanner->getBase());
}

void MemScanWidget::handleScannerErrors(const Common::MemOperationReturnCode errorCode)
{
  if (errorCode == Common::MemOperationReturnCode::invalidInput)
  {
    QMessageBox* errorBox =
        new QMessageBox(QMessageBox::Critical, tr("Invalid term(s)"),
                        tr("The search term(s) you entered for the type %1 is/are invalid")
                            .arg(m_cmbScanType->currentText()),
                        QMessageBox::Ok, this);
    errorBox->exec();
  }
  else if (errorCode == Common::MemOperationReturnCode::operationFailed)
  {
    emit mustUnhook();
  }
}

void MemScanWidget::onCurrentValuesUpdateTimer()
{
  if (m_memScanner->getResultCount() > 0 && m_memScanner->getResultCount() <= 1000)
  {
    Common::MemOperationReturnCode updateReturn = m_resultsListModel->updateScannerCurrentCache();
    if (updateReturn != Common::MemOperationReturnCode::OK)
      handleScannerErrors(updateReturn);
  }
}

QTimer* MemScanWidget::getUpdateTimer() const
{
  return m_currentValuesUpdateTimer;
}

void MemScanWidget::onResultListDoubleClicked(const QModelIndex& index)
{
  if (index != QVariant())
  {
    emit requestAddWatchEntry(m_resultsListModel->getResultAddress(index.row()),
                              m_memScanner->getType(), m_memScanner->getLength(),
                              m_memScanner->getIsUnsigned(), m_memScanner->getBase());
  }
}

#include <algorithm>
#include <utility>

#include "../../DolphinProcess/DolphinAccessor.h"
#include "../../Common/CommonUtils.h"

// TODO
// - Terminate bisection when region size reaches 4 bytes
// - Sort regions by size after each bisect
// - Merge intervals together and print ready to paste into C++?

SmbUtil::Region::Region(const char *name, uint32_t addr, uint32_t size) :
    name{name}, addr{addr}, buf(size) {}

SmbUtil::DynamicRegion::DynamicRegion(
    const char *name,
    std::vector<uint32_t> addrChain,
    uint32_t size) :
    name{name}, addrChain{std::move(addrChain)}, buf(size) {}

std::string SmbUtil::Region::briefStr() {
  // Lol C-style
  char out[100] = {};
  snprintf(out, sizeof(out), "[0x%08X, 0x%08X) (size 0x%X)",
           addr,
           addr + static_cast<uint32_t>(buf.size()),
           buf.size());
  return out;
}

SmbUtil::SmbUtil() :
    m_alwaysRegions{
        // New sub-256-byte mainloop BSS chunks which appear to work with stage and char heap saving
        // Except not rly, playing with aiai on expert ex 9 screws up the sprites again...
        {"timer",                         0x80553974, 2},
        {"",                              0x8054E03C, 0xe0}, // Camera-related

//      {"", 0x805BC66C, 0x70}, // Sprite-related probably

        {"",                              0x805bc9a0, 0x5c}, // First part of player 1 ball struct
        {"",                              0x805BD830, 0x1C}, // Physics-related probably

        {"some_ball_physics_bitfield",    0x805BCA3C, 0x4},

        {"some_ape_region_in_chara_heap", 0x81080046, 0x1},

      {"", 0x805bc9a0, 0x1b0}, // Entire ball struct

//      {"", 0x805E90B4, 0x530}, // Subregion of sprite array at 0x805e90ac GC
//      {"", 0x805E9650, 0x70}, // Also part of the sprite array
//      {"", 0x805EA1B4, 0x1C}, // Aaaalso part of the sprite array
//      {"", 0x805EA208, 0x1B8}, // You guessed it, also part of the sprite array

        // Character rotation quaternion in char heap, directly affects physics
        // (this is with meemee on curve bridge)
        {"", 0x81080248, 0x10},

        // A good start to savestates! But something is clearly missing (besides stage heap)
//    {"magic_mainloop_bss_region1", 0x8054E03C, 224},
//    {"magic_mainloop_bss_region2", 0x805BC974, 112},
//    {"magic_mainloop_bss_region3", 0x805BD82E, 28},
//    {"mainloop_bss0_smth", 0x8054c8e0, 0xdda4},

//    {"camera", 0x8054E058, 32},
//    {"ball", 0x805bc9a0, 432},
//    {"something_with_stage_tilt", 0x805bd838, 12},
//      {"inputs", 0x80145120, 544},
//      {"inputs_rawsi_something", 0x80146600, 8},

//      {"mainloop_bss0_part1", 0x8054c8e0, 0x6ed20},
      {"mainloop_bss0_part2", 0x805bb600, 0x6ed2c},

//      {"dol_data0", 0x80005620, 0x740},
//      {"dol_data1", 0x80005d60, 0xae0},
//      {"dol_data5", 0x80081640, 0xc36e0},
//      {"dol_bss0",  0x80144d20, 0x53b20},
//      {"dol_data6", 0x80198840, 0x2e0},
//      {"dol_bss1",  0x80198b20, 0x8e0},

//      {"mainloop_data0", 0x803dd600, 0x4},
//      {"mainloop_data1", 0x803dd604, 0x4},
//      {"mainloop_data3", 0x80444160, 0x806bc},
//      {"mainloop_bss0", 0x8054c8e0, 0xdda4c},

//      {"maingame_data0", 0x80914ab0, 0x4},
//      {"maingame_data1", 0x80914ab4, 0x4},
//      {"maingame_data3", 0x80915678, 0x5641c},
//      {"maingame_bss0", 0x8097f4a0, 0x65f0},
//
//      {"main_game_main_heap", 0x808f3fc0, 1994304},
//        {"main_game_stage_heap",          0x80adae00, 3276832},
//      {"main_game_bg_heap", 0x80dfae20, 2293792},
//      {"main_game_char_heap", 0x8102ae40, 4718624},
//      {"main_game_replay_heap", 0x814aae60, 163872},

//      {"small_banana_related", 0x805D4D54, 0x4},
//      {"mainloop_bss_quarter1_spark_something", 0x8054E1A4, 56},
        {"sprites", 0x805e90ac, 80 * 208},
        {"sprite_status_list", 0x805bc694, 80},
        {"sub mode", 0x8054dc34, 4},
        {"ball mode", 0x80553970, 4},
    },

    m_bisectRegions{
//      {"main_game_bg_heap", 0x80dfae20, 2293792},
//      {"main_game_char_heap", 0x8102ae40, 4718624},
//      {"main_game_replay_heap", 0x814aae60, 163872},

//        {"banana_related", 0x805D4EA0, 0x6f0},
//      {"mainloop_bss0_first_quarter", 0x8054c8e0, 0x37696},
//        {"mainloop_bss0_part1.5", 0x80583f6a, 0x37696},
//      {"mainloop_bss0_part2", 0x805bb600, 0x6ed2c},
//        {"main_game_stage_heap", 0x80adae00, 3276832},
//      {"dol_bss0",  0x80144d20, 0x53b20},
//        {"literally_everything", 0x80000000, 0x1800000},

        {"temporary ape region", 0x81094a40, 0x2400},
        {"empty",                 0x80000000, 0},
    },

    m_dynamicRegions{
        {"chara_rot_quat", {0x805bc9a0 + 260, 648}, 16},
    },

    m_bisectState{BisectState::CHOOSE_NEW_REGION},
    m_leftGood{false},
    m_rightGood{false},
    m_nextRegionChoice{0},

    m_leftRegion{"empty", 0x80000000, 0},
    m_rightRegion{"empty", 0x80000000, 0} {
  printBisectState();
}

void SmbUtil::saveState() {
  for (auto &region : m_alwaysRegions) {
    saveRegion(region);
  }

  for (auto &region : m_bisectRegions) {
    saveRegion(region);
  }

  for (auto &region : m_dynamicRegions) {
    saveDynamicRegion(region);
  }

  switch (m_bisectState) {
    case BisectState::TRYING_LEFT:
      saveRegion(m_leftRegion);
      break;

    case BisectState::TRYING_RIGHT:
      saveRegion(m_rightRegion);
      break;

    case BisectState::CHOOSE_NEW_REGION:
    case BisectState::DONE:
      break;
  }
}

void SmbUtil::loadState() {
  for (auto &region : m_alwaysRegions) {
    loadRegion(region);
  }

  for (auto &region : m_bisectRegions) {
    loadRegion(region);
  }

  for (auto &region : m_dynamicRegions) {
    loadDynamicRegion(region);
  }

  switch (m_bisectState) {
    case BisectState::TRYING_LEFT:
      loadRegion(m_leftRegion);
      break;

    case BisectState::TRYING_RIGHT:
      loadRegion(m_rightRegion);
      break;

    case BisectState::CHOOSE_NEW_REGION:
    case BisectState::DONE:
      break;
  }
}

void SmbUtil::saveRegion(SmbUtil::Region &region) {
  DolphinComm::DolphinAccessor::readFromRAM(
      Common::dolphinAddrToOffset(region.addr, false),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::loadRegion(const SmbUtil::Region &region) {
  DolphinComm::DolphinAccessor::writeToRAM(
      Common::dolphinAddrToOffset(region.addr, false),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::saveDynamicRegion(SmbUtil::DynamicRegion &region) {
  DolphinComm::DolphinAccessor::readFromRAM(
      Common::dolphinAddrToOffset(getDynamicAddr(region), false),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::loadDynamicRegion(const SmbUtil::DynamicRegion &region) {
  DolphinComm::DolphinAccessor::writeToRAM(
      Common::dolphinAddrToOffset(getDynamicAddr(region), false),
      region.buf.data(),
      region.buf.size(),
      false);
}

uint32_t SmbUtil::getDynamicAddr(const SmbUtil::DynamicRegion &region) {
  uint32_t addr = region.addrChain.at(0);
  for (int i = 1; i < region.addrChain.size(); i++) {
    uint8_t bigEndianAddr[4] = {};
    DolphinComm::DolphinAccessor::readFromRAM(
        Common::dolphinAddrToOffset(addr, false),
        reinterpret_cast<char *>(bigEndianAddr),
        sizeof(bigEndianAddr),
        false);

    uint32_t baseAddr = bigEndianAddr[0] << 24u
                        | bigEndianAddr[1] << 16u
                        | bigEndianAddr[2] << 8u
                        | bigEndianAddr[3] << 0u;

    addr = baseAddr + region.addrChain[i];
  }

  return addr;
}

void SmbUtil::bisectGood() {
  switch (m_bisectState) {
    case BisectState::TRYING_LEFT:
      m_leftGood = true;
      m_bisectState = BisectState::TRYING_RIGHT;
      break;

    case BisectState::TRYING_RIGHT:
      m_rightGood = true;
      transitionToNextRegion();
      break;

    case BisectState::CHOOSE_NEW_REGION:
      m_bisectState = BisectState::TRYING_LEFT;
      m_leftRegion = m_bisectRegions[m_nextRegionChoice];
      m_bisectRegions.erase(m_bisectRegions.begin() + m_nextRegionChoice);
      m_rightRegion = {"empty", 0x80000000, 0};
      break;

    case BisectState::DONE:
      break;
  }

  printBisectState();
}

void SmbUtil::bisectBad() {
  switch (m_bisectState) {
    case BisectState::TRYING_LEFT:
      m_leftGood = false;
      m_bisectState = BisectState::TRYING_RIGHT;
      break;

    case BisectState::TRYING_RIGHT:
      m_rightGood = false;
      transitionToNextRegion();
      break;

    case BisectState::CHOOSE_NEW_REGION:
      m_nextRegionChoice++;
      if (m_nextRegionChoice >= m_bisectRegions.size()) {
        logf("No region chosen to bisect, finishing.");
        m_bisectState = BisectState::DONE;
      }
      break;

    case BisectState::DONE:
      break;
  }

  printBisectState();
}

void SmbUtil::transitionToNextRegion() {
  if (m_leftGood && m_rightGood) {
    logf("Both regions work?");
    m_bisectState = BisectState::CHOOSE_NEW_REGION;
    m_nextRegionChoice = 0;
  } else if (!m_leftGood && !m_rightGood) {
    logf("Neither region works alone.");

    m_bisectState = BisectState::CHOOSE_NEW_REGION;
    m_nextRegionChoice = 0;
    m_bisectRegions.insert(m_bisectRegions.begin(), m_rightRegion);
    m_bisectRegions.insert(m_bisectRegions.begin(), m_leftRegion);
  } else {
    Region &goodRegion = m_leftGood ? m_leftRegion : m_rightRegion;
    uint32_t newLeftSize = goodRegion.buf.size() / 2;
    newLeftSize &= ~static_cast<uint32_t>(0b11); // Round down to nearest multiple of 4
    uint32_t newRightSize = goodRegion.buf.size() - newLeftSize;

    Region newLeftRegion(goodRegion.name, goodRegion.addr, newLeftSize);
    Region newRightRegion(goodRegion.name, goodRegion.addr + newLeftSize, newRightSize);

    m_leftRegion = newLeftRegion;
    m_rightRegion = newRightRegion;

    m_leftGood = false;
    m_rightGood = false;
    m_bisectState = BisectState::TRYING_LEFT;
  }
}

void SmbUtil::printBisectState() {
  switch (m_bisectState) {
    case BisectState::TRYING_LEFT: {
      logf("Trying left region: %s", m_leftRegion.briefStr().c_str());
      break;
    }

    case BisectState::TRYING_RIGHT: {
      logf("Trying right region: %s", m_rightRegion.briefStr().c_str());
      break;
    }

    case BisectState::CHOOSE_NEW_REGION: {
      int32_t bisectRegionTotalSize = 0;
      for (auto &region : m_bisectRegions) {
        bisectRegionTotalSize += region.buf.size();
      }
      logf("Available bisect regions (%d bytes total):", bisectRegionTotalSize);
      for (auto &region : m_bisectRegions) {
        logf("%s", region.briefStr().c_str());
      }
      logf("End available bisect regions.");
      logf("Bisect this region next? Index %d (%d avail) %s",
             m_nextRegionChoice,
             m_bisectRegions.size(),
             m_bisectRegions[m_nextRegionChoice].briefStr().c_str());
      break;
    }

    case BisectState::DONE: {
      logf("Done bisecting.");
      break;
    }
  }
}

std::vector<SmbUtil::Region> SmbUtil::subtractIgnoredRegions(SmbUtil::Region region, const
std::vector<Region> &ignoredRegions) {
  std::vector<Region> oldRegions{region};

  for (auto &ignoreRegion : ignoredRegions) {
    std::vector<Region> newRegions;

    for (auto &oldRegion : oldRegions) {
      uint32_t ignoreStart = ignoreRegion.addr;
      uint32_t ignoreEnd = ignoreRegion.addr + ignoreRegion.buf.size();
      uint32_t oldStart = oldRegion.addr;
      uint32_t oldEnd = oldRegion.addr + oldRegion.buf.size();

      uint32_t new1Start = std::min(ignoreStart, oldStart);
      uint32_t new1End = std::min(ignoreStart, oldEnd);
      uint32_t new2Start = std::max(ignoreEnd, oldStart);
      uint32_t new2End = std::max(ignoreEnd, oldEnd);

      if (new1Start != new1End) {
        newRegions.emplace_back(oldRegion.name, new1Start, new1End - new1Start);
      }
      if (new2Start != new2End) {
        newRegions.emplace_back(oldRegion.name, new2Start, new2End - new2Start);
      }
    }

    oldRegions = newRegions;
  }

  return oldRegions;
}

void SmbUtil::logf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  QString msg = QString::vasprintf(fmt, args);
  va_end(args);

  emit onLog(msg);
}

//
// SMB Util Widget
//

#include <QHBoxLayout>

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

  hookStatusChanged(false);
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

void SmbUtilWidget::hookStatusChanged(bool status)
{
  m_btnSaveState->setEnabled(status);
  m_btnLoadState->setEnabled(status);
  m_btnBisectGood->setEnabled(status);
  m_btnBisectBad->setEnabled(status);
  m_textEdit->setEnabled(status);
}
