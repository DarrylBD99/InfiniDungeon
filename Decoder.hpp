#pragma once

#include "Dataset.hpp"

namespace Decoder {
    // Read Spritesheet binary file
    uint8_t* ReadSpritesheetData(const char* filePath, uint16_t& width, uint16_t& height, uint8_t**& colorPalette, uint8_t& colorPaletteSize);
    uint8_t* ReadRoomData(const char* filePath, uint8_t& width, uint8_t& height);
    uint8_t* ReadRoomTriggerData(const char* filePath, uint8_t& triggerCount);
}