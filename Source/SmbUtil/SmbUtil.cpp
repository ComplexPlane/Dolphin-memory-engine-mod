#include "SmbUtil.h"

#include <algorithm>
#include <utility>

#include "../DolphinProcess/DolphinAccessor.h"
#include "../Common/CommonUtils.h"

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

//      {"", 0x805bc9a0, 0x1b0}, // Entire ball struct

//      {"", 0x805E90B4, 0x530}, // Subregion of sprite array at 0x805e90ac GC
//      {"", 0x805E9650, 0x70}, // Also part of the sprite array
//      {"", 0x805EA1B4, 0x1C}, // Aaaalso part of the sprite array
//      {"", 0x805EA208, 0x1B8}, // You guessed it, also part of the sprite array

        // Character rotation quaternion in char heap, directly affects physics
        // (this is with meemee on curve bridge)
//        {"", 0x81080248, 0x10},

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
//      {"mainloop_bss0_part2", 0x805bb600, 0x6ed2c},

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
        {"main_game_stage_heap",          0x80adae00, 3276832},
//      {"main_game_bg_heap", 0x80dfae20, 2293792},
//      {"main_game_char_heap", 0x8102ae40, 4718624},
//      {"main_game_replay_heap", 0x814aae60, 163872},

//      {"small_banana_related", 0x805D4D54, 0x4},
//      {"mainloop_bss_quarter1_spark_something", 0x8054E1A4, 56},
        {"sprites", 0x805e90ac, 80 * 208},
        {"sprite_status_list", 0x805bc694, 80},
    },

    m_bisectRegions{
//      {"main_game_bg_heap", 0x80dfae20, 2293792},
//      {"main_game_char_heap", 0x8102ae40, 4718624},
//      {"main_game_replay_heap", 0x814aae60, 163872},

//        {"banana_related", 0x805D4EA0, 0x6f0},
//      {"mainloop_bss0_first_quarter", 0x8054c8e0, 0x37696},
//        {"mainloop_bss0_part1.5", 0x80583f6a, 0x37696},
      {"mainloop_bss0_part2", 0x805bb600, 0x6ed2c},
//        {"main_game_stage_heap", 0x80adae00, 3276832},
//      {"dol_bss0",  0x80144d20, 0x53b20},
//        {"literally_everything", 0x80000000, 0x1800000},
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
      Common::dolphinAddrToOffset(region.addr),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::loadRegion(const SmbUtil::Region &region) {
  DolphinComm::DolphinAccessor::writeToRAM(
      Common::dolphinAddrToOffset(region.addr),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::saveDynamicRegion(SmbUtil::DynamicRegion &region) {
  DolphinComm::DolphinAccessor::readFromRAM(
      Common::dolphinAddrToOffset(getDynamicAddr(region)),
      region.buf.data(),
      region.buf.size(),
      false);
}

void SmbUtil::loadDynamicRegion(const SmbUtil::DynamicRegion &region) {
  DolphinComm::DolphinAccessor::writeToRAM(
      Common::dolphinAddrToOffset(getDynamicAddr(region)),
      region.buf.data(),
      region.buf.size(),
      false);
}

uint32_t SmbUtil::getDynamicAddr(const SmbUtil::DynamicRegion &region) {
  uint32_t addr = region.addrChain.at(0);
  for (int i = 1; i < region.addrChain.size(); i++) {
    uint8_t bigEndianAddr[4] = {};
    DolphinComm::DolphinAccessor::readFromRAM(
        Common::dolphinAddrToOffset(addr),
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
        printf("No region chosen to bisect, finishing.\n");
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
    printf("Both regions work?\n");
    m_bisectState = BisectState::CHOOSE_NEW_REGION;
    m_nextRegionChoice = 0;
  } else if (!m_leftGood && !m_rightGood) {
    printf("Neither region works alone.\n");

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
      printf("Trying left region: %s\n", m_leftRegion.briefStr().c_str());
      break;
    }

    case BisectState::TRYING_RIGHT: {
      printf("Trying right region: %s\n", m_rightRegion.briefStr().c_str());
      break;
    }

    case BisectState::CHOOSE_NEW_REGION: {
      int32_t bisectRegionTotalSize = 0;
      for (auto &region : m_bisectRegions) {
        bisectRegionTotalSize += region.buf.size();
      }
      printf("Available bisect regions (%d bytes total):\n", bisectRegionTotalSize);
      for (auto &region : m_bisectRegions) {
        printf("%s\n", region.briefStr().c_str());
      }
      printf("End available bisect regions.\n");
      printf("Bisect this region next? Index %d (%d avail) %s\n",
             m_nextRegionChoice,
             m_bisectRegions.size(),
             m_bisectRegions[m_nextRegionChoice].briefStr().c_str());
      break;
    }

    case BisectState::DONE: {
      printf("Done bisecting.\n");
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

void SmbUtil::printRegions(const std::string &name, const std::vector<Region> &regions) {
  printf("%s\n", name.c_str());
  for (auto &region : regions) {
    printf("%s 0x%08X 0x%08X\n", region.name, region.addr, region.addr + region.buf.size());
  }
  printf("\n");
}

