#pragma once

#include <glm/glm/glm.hpp>
#include "Chunk.hpp"
#include <memory>
#include <vector>
#include <map>

struct MatrixEntry {
    uint8_t mHeight;
    uint16_t mChunk;
    uint16_t mHeader;
};

class Matrix {
    std::string mName { "[unset]" };
    int mWidth, mHeight;
    std::vector<MatrixEntry> mEntries;
public:
    std::string GetName() { return mName; }
    std::vector<MatrixEntry>& GetEntries() { return mEntries; }

    int GetWidth() { return mWidth; }
    int GetHeight() { return mHeight; }

    void Load(std::shared_ptr<Palkia::Nitro::File>);

    Matrix();
    ~Matrix();
};