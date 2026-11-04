#include "Decoder.hpp"

#include <bit>
#include <fstream>
#include <iostream>

namespace Decoder {
    int countValueBits(unsigned int n) {
        if (n == 0) return 0;

        // Returns the number of bits needed to represent the value
        return std::bit_width(n);
    }

    uint8_t* ReadSpritesheetData(const std::string& filePath, uint16_t& width, uint16_t& height, uint8_t**& colorPalette) {
        std::ifstream spritesheet(filePath, std::ios::binary);
        uint8_t colorPaletteSize = 0;

        // Read Header
        spritesheet.read(reinterpret_cast<char*>(&width), sizeof(uint16_t));
        spritesheet.read(reinterpret_cast<char*>(&height), sizeof(uint16_t));
        spritesheet.read(reinterpret_cast<char*>(&colorPaletteSize), sizeof(uint8_t));

        // Get Color Palette Data
        colorPalette = new uint8_t * [colorPaletteSize];
        for (unsigned int i = 0; i < colorPaletteSize; i++) {
            colorPalette[i] = new uint8_t[3];
            spritesheet.read(reinterpret_cast<char*>(&colorPalette[i][0]), sizeof(uint8_t));
            spritesheet.read(reinterpret_cast<char*>(&colorPalette[i][1]), sizeof(uint8_t));
            spritesheet.read(reinterpret_cast<char*>(&colorPalette[i][2]), sizeof(uint8_t));
        }

        // Count how many bits total (should be 3 if color palette count is 8)
        uint8_t paletteBitCount = countValueBits(colorPaletteSize - 1);
        uint8_t* spritesheetData = new uint8_t[height * width];

        int totalPixels = width * height;
        int totalBits = totalPixels * paletteBitCount;
        int totalBytes = (totalBits + 7) / 8;

        uint8_t* packedData = new uint8_t[totalBytes];
        spritesheet.read(reinterpret_cast<char*>(packedData), totalBytes);

        // Retrieve Data
        int bitPosition = 0;
        for (int i = 0; i < totalPixels; i++) {
            uint8_t idx = 0;
            for (int b = 0; b < paletteBitCount; b++) {
                if (packedData[bitPosition / 8] & (1 << (bitPosition % 8)))
                    idx |= (1 << b);
                bitPosition++;
            }

            // Add index to spritesheetData
            spritesheetData[i] = idx;
        }

        delete[] packedData;

        spritesheet.close();
        return spritesheetData;
    }

    uint8_t* ReadRoomData(const std::string& filePath, uint8_t& width, uint8_t& height) {
        std::ifstream file(filePath);

        unsigned int w, h;

        file >> w >> h;

        width = (uint8_t)w;
        height = (uint8_t)h;

        uint8_t* tileIDs = new uint8_t[width * height];
        for (unsigned int i = 0; i < (unsigned int)(width * height); i++) {
            unsigned int tileID;
            file >> tileID;
            tileIDs[i] = (uint8_t)tileID;
        }

        return tileIDs;
    }
}