#include "Decoder.hpp"

#include <bit>
#include <fstream>

namespace Decoder {
    int countValueBits(unsigned int n) {
        if (n == 0) return 0;

        // Returns the number of bits needed to represent the value
        return std::bit_width(n);
    }

    int getMaxValue(unsigned int bits) {
        if (bits == 0) return 0;

        // Returns the number of bits needed to represent the value
        return (1u << bits) - 1;
    }

    uint8_t* ReadRLEFile(uint8_t indexCount, uint16_t width, uint16_t height, std::ifstream& file) {
        if (!file.is_open()) return nullptr;
        uint8_t indexBitCount = countValueBits(indexCount - 1);
        uint8_t* data = new uint8_t[height * width]{};
        uint8_t* dataPtr = &data[0];

        uint8_t maxCount = getMaxValue(8 - indexBitCount);
        unsigned int pixelIndex = 0;

        while (true) {
            uint8_t data = 0;
            if (!file.read(reinterpret_cast<char*>(&data), sizeof(uint8_t))) break;

            uint8_t count = ((data >> indexBitCount) & maxCount) + 1;
            uint8_t colorIndex = data & getMaxValue(indexBitCount);

            if (colorIndex >= indexCount || pixelIndex + count > (width * height)) {
                break;
            }

            for (uint8_t i = 0; i < count; i++) {
                *dataPtr = colorIndex;
                dataPtr++;
                pixelIndex++;
            }
        }

        file.close();
        return data;
    }

    uint8_t* ReadSpritesheetData(const char* filePath, uint16_t& width, uint16_t& height, uint8_t**& colorPalette, uint8_t& colorPaletteSize) {
        std::ifstream spritesheet(filePath, std::ios::binary);

        // Read Header
        spritesheet.read(reinterpret_cast<char*>(&width), sizeof(uint16_t));
        spritesheet.read(reinterpret_cast<char*>(&height), sizeof(uint16_t));
        spritesheet.read(reinterpret_cast<char*>(&colorPaletteSize), sizeof(uint8_t));

        // Get Color Palette Data
        colorPalette = new uint8_t*[colorPaletteSize];
        uint8_t** colorPalettePtr = colorPalette;
        for (unsigned int i = 0; i < colorPaletteSize; i++) {
            uint8_t* color = new uint8_t[3];
            spritesheet.read(reinterpret_cast<char*>(&color[0]), sizeof(uint8_t));
            spritesheet.read(reinterpret_cast<char*>(&color[1]), sizeof(uint8_t));
            spritesheet.read(reinterpret_cast<char*>(&color[2]), sizeof(uint8_t));

            *colorPalettePtr = color;
            colorPalettePtr++;
        }

        return ReadRLEFile(colorPaletteSize, width, height, spritesheet);

        // Packed Bit System (Old)
        // Count how many bits total (should be 3 if color palette count is 8)
        //uint8_t paletteBitCount = countValueBits(colorPaletteSize - 1);
        //uint8_t* spritesheetData = new uint8_t[height * width]{};
        //uint8_t* spritesheetDataPtr = &spritesheetData[0];

        //int totalPixels = width * height;
        //int totalBits = totalPixels * paletteBitCount;
        //int totalBytes = (totalBits + 7) / 8;

        //uint8_t* packedData = new uint8_t[totalBytes];
        //spritesheet.read(reinterpret_cast<char*>(packedData), totalBytes);

        //// Retrieve Data
        //int bitPosition = 0;
        //for (int i = 0; i < totalPixels; i++) {
        //    uint8_t idx = 0;
        //    for (int b = 0; b < paletteBitCount; b++) {
        //        if (packedData[bitPosition / 8] & (1 << (bitPosition % 8)))
        //            idx |= (1 << b);
        //        bitPosition++;
        //    }

        //    // Add index to spritesheetData
        //    spritesheetData[i] = idx;
        //}

        //delete[] packedData;

        //uint8_t maxCount = getMaxValue(8 - paletteBitCount);
        //unsigned int pixelIndex = 0;

        //while (true) {
        //    uint8_t data = 0;
        //    if (!spritesheet.read(reinterpret_cast<char*>(&data), sizeof(uint8_t))) break;
        //    
        //    uint8_t count = ((data >> paletteBitCount) & maxCount) + 1;
        //    uint8_t colorIndex = data & getMaxValue(paletteBitCount);

        //    if (colorIndex >= colorPaletteSize || pixelIndex + count > (width * height)) {
        //        break;
        //    }

        //    for (uint8_t i = 0; i < count; i++) {
        //        *spritesheetDataPtr = colorIndex;
        //        spritesheetDataPtr++;
        //        pixelIndex++;
        //    }
        //}

        //spritesheet.close();
        //return spritesheetData;
    }

    uint8_t* ReadRoomData(const char* filePath, uint8_t& width, uint8_t& height) {
        std::ifstream room(filePath, std::ios::binary);

        // Read Header
        room.read(reinterpret_cast<char*>(&width), sizeof(uint8_t));
        room.read(reinterpret_cast<char*>(&height), sizeof(uint8_t));

        return ReadRLEFile(16, width, height, room);
    }

    uint8_t* ReadRoomTriggerData(const char* filePath, uint8_t& triggerCount) {
        std::ifstream room(filePath, std::ios::binary);

        uint8_t width, height;
        // Read Header
        room.read(reinterpret_cast<char*>(&width), sizeof(uint8_t));
        room.read(reinterpret_cast<char*>(&height), sizeof(uint8_t));
        room.read(reinterpret_cast<char*>(&triggerCount), sizeof(uint8_t));

        return ReadRLEFile(triggerCount + 1, width, height, room);
    }
}