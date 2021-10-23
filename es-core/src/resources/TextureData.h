//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  TextureData.h
//
//  Low-level texture data functions.
//

#ifndef ES_CORE_RESOURCES_TEXTURE_DATA_H
#define ES_CORE_RESOURCES_TEXTURE_DATA_H

#include "utils/MathUtil.h"

#include <cmath>
#include <mutex>
#include <string>
#include <vector>

class TextureResource;

class TextureData
{
public:
    TextureData(bool tile);
    ~TextureData();

    // These functions populate mDataRGBA but do not upload the texture to VRAM.

    // Needs to be canonical path. Caller should check for duplicates before calling this.
    void initFromPath(const std::string& path);
    bool initSVGFromMemory(const std::string& fileData);
    bool initImageFromMemory(const unsigned char* fileData, size_t length);
    bool initFromRGBA(const unsigned char* dataRGBA, size_t width, size_t height);

    // Read the data into memory if necessary.
    bool load();

    bool isLoaded();

    // Upload the texture to VRAM if necessary and bind.
    // Returns true if bound correctly.
    bool uploadAndBind();

    // Release the texture from VRAM.
    void releaseVRAM();

    // Release the texture from conventional RAM.
    void releaseRAM();

    // Get the amount of VRAM currenty used by this texture.
    size_t getVRAMUsage();

    size_t width();
    size_t height();
    float sourceWidth();
    float sourceHeight();
    void setSourceSize(float width, float height);
    glm::vec2 getSize() { return glm::vec2{mWidth, mHeight}; }

    // Define a factor for scaling the file when loading it (1.0f = no scaling).
    void setScaleDuringLoad(float scale) { mScaleDuringLoad = scale; }
    // Whether to use linear filtering when magnifying the texture.
    void setLinearMagnify(bool setting) { mLinearMagnify = setting; }

    std::vector<unsigned char> getRawRGBAData() { return mDataRGBA; }
    std::string getTextureFilePath() { return mPath; }
    bool tiled() { return mTile; }

private:
    std::mutex mMutex;
    bool mTile;
    std::string mPath;
    unsigned int mTextureID;
    std::vector<unsigned char> mDataRGBA;
    int mWidth;
    int mHeight;
    float mSourceWidth;
    float mSourceHeight;
    float mScaleDuringLoad;
    bool mScalable;
    bool mLinearMagnify;
    bool mReloadable;
};

#endif // ES_CORE_RESOURCES_TEXTURE_DATA_H
