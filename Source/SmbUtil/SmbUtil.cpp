#include "SmbUtil.h"

#include <algorithm>

#include "../DolphinProcess/DolphinAccessor.h"
#include "../Common/CommonUtils.h"


SmbUtil::Region::Region(const char *name, uint32_t addr, uint32_t size):
    name{name}, addr{addr}, buf(size) {}

SmbUtil::SmbUtil()
{
  std::vector<Region> positiveRegions{
//    {"ball", 0x805bc9a0, 432},
      {"dol_data0", 0x80005620, 0x740},
      {"dol_data1", 0x80005d60, 0xae0},
      {"dol_data5", 0x80081640, 0xc36e0},
      {"dol_bss0",  0x80144d20, 0x53b20},
      {"dol_data6", 0x80198840, 0x2e0},
      {"dol_bss1",  0x80198b20, 0x8e0},

      {"mainloop_data0", 0x803dd600, 0x4},
      {"mainloop_data1", 0x803dd604, 0x4},
      {"mainloop_data3", 0x80444160, 0x806bc},
      {"mainloop_bss0", 0x8054c8e0, 0xdda4c},

      {"maingame_data0", 0x80914ab0, 0x4},
      {"maingame_data1", 0x80914ab4, 0x4},
      {"maingame_data3", 0x80915678, 0x5641c},
      {"maingame_bss0", 0x8097f4a0, 0x65f0},

      {"heaps", 0x808f3fc0, 0xbdeec0},
  };

  std::vector<Region> ignoreRegions{
      // Don't save and restore GXFifoObj
      { "fifoobj1", 0x80147498, 128},
      { "fifoobj2", 0x8018fe80, 128},
  };

  for (auto& posRegion : positiveRegions)
  {
    std::vector<Region> splitRegions = subtractIgnoredRegions(posRegion, ignoreRegions);
    m_regions.insert(m_regions.end(), splitRegions.begin(), splitRegions.end());
  }

  printRegions("Positive regions", positiveRegions);
  printRegions("Negative regions", ignoreRegions);
  printRegions("Final regions", m_regions);
}

void SmbUtil::saveState()
{
  for (auto& region : m_regions)
  {
    DolphinComm::DolphinAccessor::readFromRAM(
        Common::dolphinAddrToOffset(region.addr),
        region.buf.data(),
        region.buf.size(),
        false
    );
  }
}

void SmbUtil::loadState()
{
  for (auto& region : m_regions)
  {
    DolphinComm::DolphinAccessor::writeToRAM(
        Common::dolphinAddrToOffset(region.addr),
        region.buf.data(),
        region.buf.size(),
        false
    );
  }
}

std::vector<SmbUtil::Region> SmbUtil::subtractIgnoredRegions(SmbUtil::Region region, const
std::vector<Region>& ignoredRegions)
{
  std::vector<Region> oldRegions{region};

  for (auto& ignoreRegion : ignoredRegions)
  {
    std::vector<Region> newRegions;

    for (auto& oldRegion : oldRegions)
    {
      uint32_t ignoreStart = ignoreRegion.addr;
      uint32_t ignoreEnd = ignoreRegion.addr + ignoreRegion.buf.size();
      uint32_t oldStart = oldRegion.addr;
      uint32_t oldEnd = oldRegion.addr + oldRegion.buf.size();

      uint32_t new1Start = std::min(ignoreStart, oldStart);
      uint32_t new1End = std::min(ignoreStart, oldEnd);
      uint32_t new2Start = std::max(ignoreEnd, oldStart);
      uint32_t new2End = std::max(ignoreEnd, oldEnd);

      if (new1Start != new1End)
      {
        newRegions.emplace_back(oldRegion.name, new1Start, new1End - new1Start);
      }
      if (new2Start != new2End)
      {
        newRegions.emplace_back(oldRegion.name, new2Start, new2End - new2Start);
      }
    }

    oldRegions = newRegions;
  }

  return oldRegions;
}

void SmbUtil::printRegions(const std::string& name, const std::vector<Region>& regions)
{
  printf("%s\n", name.c_str());
  for (auto& region : regions)
  {
    printf("%s 0x%08X 0x%08X\n", region.name, region.addr, region.addr + region.buf.size());
  }
  printf("\n");
}