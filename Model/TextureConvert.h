#pragma once

#include <cstdint>
#include <string>
#include <assert.h>

enum TexConversionFlags
{
    kSRGB = 1,          // Texture contains sRGB colors
    kPreserveAlpha = 2, // Keep four channels
    kNormalMap = 4,     // Texture contains normals
    kBumpToNormal = 8,  // Generate a normal map from a bump map
    kDefaultBC = 16,    // Apply standard block compression (BC1-5)
    kQualityBC = 32,    // Apply quality block compression (BC6H/7)
    kFlipVertical = 64,
};

inline uint8_t TextureOptions(bool sRGB, bool hasAlpha=false, bool invertY=false)
{
    return (sRGB ? kSRGB : 0) | (hasAlpha ? kPreserveAlpha : 0) | (invertY ? kFlipVertical : 0);
}

// If the DDS version of the texture specified does not exist or is older than the source texture, reconvert it.
void CompileTextureOnDemand(const std::wstring& originalFile, uint32_t flags);

// Loads a non-DDS texture such as TGA, PNG, or JPG, then converts it to a DDS file. 
bool ConvertToDDS(
    const std::wstring& filePath,	// UTF8-encoded path to source file
    uint32_t Flags                  // flags ORed together
);
