#pragma once

#include <vector>
#include <cstdint>
#include <string>

class SmbUtil
{
public:
  SmbUtil();

  void saveState();
  void loadState();
  void bisectGood();
  void bisectBad();

private:
  struct Region {
    Region(const char *name, uint32_t addr, uint32_t size);

    std::string briefStr();

    const char *name;
    uint32_t addr;
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

  BisectState m_bisectState;
  bool m_leftGood, m_rightGood;
  int m_nextRegionChoice;

  // The left half and right half of the region we are currently bisecting
  Region m_leftRegion, m_rightRegion;

  void saveRegion(Region& region);
  void loadRegion(const Region& region);

  void transitionToNextRegion();
  void printBisectState();

  static std::vector<Region> subtractIgnoredRegions(Region region, const std::vector<Region>&
      ignoredRegions);
  static void printRegions(const std::string& name, const std::vector<Region>& regions);
};
