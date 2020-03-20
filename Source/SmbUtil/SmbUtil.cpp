#include "SmbUtil.h"

#include <algorithm>

#include "../DolphinProcess/DolphinAccessor.h"
#include "../Common/CommonUtils.h"

SmbUtil::Region::Region(const char *name, uint32_t addr, uint32_t size):
    name{name}, addr{addr}, buf(size) {}

SmbUtil::SmbUtil(): m_regions
{
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
} {}

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
