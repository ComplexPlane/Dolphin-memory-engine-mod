#pragma once

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
