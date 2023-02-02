#pragma once

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableView>
#include <QTimer>
#include <QWidget>

#include "ResultsListModel.h"

class MemScanWidget : public QWidget
{
  Q_OBJECT

public:
  MemScanWidget();
  ~MemScanWidget();

  ResultsListModel* getResultListModel() const;
  std::vector<u32> getAllResults() const;
  QModelIndexList getSelectedResults() const;

  void onScanFilterChanged();
  void onScanMemTypeChanged();
  void onCurrentValuesUpdateTimer();
  void onResultListDoubleClicked(const QModelIndex& index);
  void handleScannerErrors(const Common::MemOperationReturnCode errorCode);
  void onFirstScan();
  void onNextScan();
  void onResetScan();
  void onAddSelection();
  void onRemoveSelection();
  void onAddAll();
  QTimer* getUpdateTimer() const;

signals:
  void requestAddWatchEntry(u32 address, Common::MemType type, size_t length, bool isUnsigned,
                            Common::MemBase base);
  void requestAddSelectedResultsToWatchList(Common::MemType type, size_t length, bool isUnsigned,
                                            Common::MemBase base);
  void requestAddAllResultsToWatchList(Common::MemType type, size_t length, bool isUnsigned,
                                       Common::MemBase base);
  void mustUnhook();

private:
  void initialiseWidgets();
  void makeLayouts();

  MemScanner::ScanFiter getSelectedFilter() const;
  void updateScanFilterChoices();
  void updateTypeAdditionalOptions();

  MemScanner* m_memScanner;
  ResultsListModel* m_resultsListModel;
  QPushButton* m_btnFirstScan;
  QPushButton* m_btnNextScan;
  QPushButton* m_btnResetScan;
  QPushButton* m_btnAddSelection;
  QPushButton* m_btnAddAll;
  QPushButton* m_btnRemoveSelection;
  QLineEdit* m_txbSearchTerm1;
  QLineEdit* m_txbSearchTerm2;
  QWidget* m_searchTerm2Widget;
  QTimer* m_currentValuesUpdateTimer;
  QComboBox* m_cmbScanFilter;
  QComboBox* m_cmbScanType;
  QLabel* m_lblResultCount;
  QCheckBox* m_chkSignedScan;
  QCheckBox* m_chkEnforceMemAlignement;
  QButtonGroup* m_btnGroupScanBase;
  QRadioButton* m_rdbBaseDecimal;
  QRadioButton* m_rdbBaseHexadecimal;
  QRadioButton* m_rdbBaseOctal;
  QRadioButton* m_rdbBaseBinary;
  QGroupBox* m_groupScanBase;
  QTableView* m_tblResulstList;
  bool m_variableLengthType;
};

#include <QtCore/qobjectdefs.h>
#include <QtCore/QObject>
#include <cstdint>
#include <string>
#include <vector>

class SmbUtil : public QObject
{
  Q_OBJECT

public:
  SmbUtil();

  void saveState();
  void loadState();
  void bisectGood();
  void bisectBad();
  void printBisectState();

private:
  struct Region {
    Region(const char *name, uint32_t addr, uint32_t size);

    std::string briefStr();

    const char *name;
    uint32_t addr;
    std::vector<char> buf;
  };

  struct DynamicRegion {
    DynamicRegion(const char *name, std::vector<uint32_t> addrChain, uint32_t size);

    const char *name;
    std::vector<uint32_t> addrChain;
    std::vector<char> buf;
  };

  enum class BisectState {
    CHOOSE_NEW_REGION,
    TRYING_LEFT,
    TRYING_RIGHT,
    DONE,
  };

  // Fixed set of regions to always save/restore
  std::vector<Region> m_alwaysRegions;

  // Pool of regions which we can choose to bisect
  std::vector<Region> m_bisectRegions;

  // Dynamic regions (always saved/restored)
  std::vector<DynamicRegion> m_dynamicRegions;

  BisectState m_bisectState;
  bool m_leftGood, m_rightGood;
  int m_nextRegionChoice;

  // The left half and right half of the region we are currently bisecting
  Region m_leftRegion, m_rightRegion;

  void saveRegion(Region& region);
  void loadRegion(const Region& region);
  void saveDynamicRegion(DynamicRegion& region);
  void loadDynamicRegion(const DynamicRegion& region);
  uint32_t getDynamicAddr(const DynamicRegion &region);

  void transitionToNextRegion();

  static std::vector<Region> subtractIgnoredRegions(Region region, const std::vector<Region>&
      ignoredRegions);

  void logf(const char *fmt, ...);

signals:
  void onLog(const QString& string);
};

#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>

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

  void hookStatusChanged(bool status);

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
