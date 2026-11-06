#pragma once

#include <string>
#include "Dataset.hpp"

namespace Decoder {
    // Read Spritesheet binary file
    uint8_t* ReadSpritesheetData(const std::string& filePath, uint16_t& width, uint16_t& height, uint8_t**& colorPalette);
    uint8_t* ReadRoomData(const std::string& filePath, uint8_t& width, uint8_t& height);
}