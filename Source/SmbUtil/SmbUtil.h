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
    TRYING_LEFT,
    TRYING_RIGHT,
    NEITHER_WORK,
    BOTH_WORK,
  };

  std::vector<Region> m_regions;

  BisectState m_bisectState;
  bool m_leftGood, m_rightGood;
  Region m_leftRegion, m_rightRegion;

  void saveRegion(Region& region);
  void loadRegion(const Region& region);

  void transitionToNextRegion();
  void printBisectState();

  static std::vector<Region> subtractIgnoredRegions(Region region, const std::vector<Region>&
      ignoredRegions);
  static void printRegions(const std::string& name, const std::vector<Region>& regions);
};
