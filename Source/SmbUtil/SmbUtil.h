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

private:
  struct Region {
    Region(const char *name, uint32_t addr, uint32_t size);

    const char *name;
    uint32_t addr;
    std::vector<char> buf;
  };

  std::vector<Region> m_regions;

  static std::vector<Region> subtractIgnoredRegions(Region region, const std::vector<Region>&
      ignoredRegions);
  static void printRegions(const std::string& name, const std::vector<Region>& regions);
};
