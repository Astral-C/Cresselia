#include "Map/Area.hpp"

void Area::Save(bStream::CStream& stream, bool isHGSS){
    stream.writeUInt16(mBuildingSet);
    stream.writeUInt16(mMapTileset);

    if(isHGSS){
        stream.writeUInt16(mDynamicTextureType);
        stream.writeUInt8(mAreaType);
        stream.writeUInt8(mLightType);
    } else {
        stream.writeUInt16(mUnknown);
        stream.writeUInt16(mLightType);
    }
}

Area::Area(bStream::CStream& stream, bool isHGSS){
    mBuildingSet = stream.readUInt16();
    mMapTileset = stream.readUInt16();

    if(isHGSS){
        mDynamicTextureType = stream.readUInt16();
        mAreaType = stream.readUInt8();
        mLightType = stream.readUInt8();
    } else {
        mUnknown = stream.readUInt16();
        mLightType = stream.readUInt16();
    }
}

Area::Area(){}
Area::~Area(){}
