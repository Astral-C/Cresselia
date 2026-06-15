#include <Map/Matrix.hpp>
#include <map>
#include <set>

Matrix::Matrix(){}
Matrix::~Matrix(){}

void Matrix::Load(std::shared_ptr<Palkia::Nitro::File> matrixData, uint32_t gameCode){
    bStream::CMemoryStream stream(matrixData->GetData(), matrixData->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    
    mWidth = stream.readUInt8();
    mHeight = stream.readUInt8();

    uint8_t hasHeaders = stream.readUInt8();
    uint8_t hasAltitude = stream.readUInt8();
    uint8_t nameLength = stream.readUInt8();

    mName = stream.readString(nameLength);

    mEntries.resize(mWidth * mHeight);

    if(hasHeaders){
        for (uint8_t y = 0; y < mHeight; y++){
            for (uint8_t x = 0; x < mWidth; x++){
                mEntries[(y * mWidth) + x].mHeader = stream.readUInt16();
            }
        }
    }
    
    if(hasAltitude){
        for (uint8_t y = 0; y < mHeight; y++){
            for (uint8_t x = 0; x < mWidth; x++){
                mEntries[(y * mWidth) + x].mHeight = stream.readUInt8();
            }
        }
    }

    for (uint8_t y = 0; y < mHeight; y++){
        for (uint8_t x = 0; x < mWidth; x++){
            mEntries[(y * mWidth) + x].mChunk = stream.readUInt16();
        }
    }
    

}
