#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <NDS/Assets/NSBMD.hpp>
#include <NDS/Assets/NSBTX.hpp>
#include "IDManager.hpp"
#include "Map/Chunk.hpp"
#include "Util.hpp"
#include "glad/glad.h"

namespace MapGraphicsHandler {
    std::map<uint16_t, Palkia::Formats::NSBMD*> mLoadedChunkModels;
    std::map<uint16_t, Palkia::Formats::NSBMD*> mLoadedModels;

    uint32_t mChunkProgram = 0xFFFFFFFF;

    const char* map_vtx_shader_source = "#version 460\n\
        #extension GL_ARB_separate_shader_objects : enable\n\
        layout(location = 0) in vec3 inPosition;\n\
        layout(location = 1) in vec3 inNormal;\n\
        layout(location = 2) in vec3 inColor;\n\
        layout(location = 3) in vec2 inTexCoord;\n\
        layout(location = 4) in int inMtxIdx; \n\
        \
        uniform mat4 transform;\n\
        uniform mat4 stackMtx;\n\
        uniform mat4 scaleMtx;\n\
        \
        layout(location = 0) out vec2 fragTexCoord;\n\
        layout(location = 1) out vec3 fragVtxColor;\n\
        layout(location = 2) out vec3 fragVtxPos;\n\
        \
        void main()\n\
        {\
            gl_Position = transform * stackMtx * scaleMtx * vec4(inPosition, 1.0);\n\
            fragVtxColor = inColor;\n\
            fragTexCoord = inTexCoord;\n\
            fragVtxPos = inPosition;\n\
        }\
    ";
    
    // chunk stuff is a stupid hack.
    // this whole thing is a stupid hack
    // but its a coooool stupid hack
    const char* map_frg_shader_source = "#version 460\n\
        #extension GL_ARB_separate_shader_objects : enable\n\
        \
        uniform sampler2D texSampler;\n\
        uniform mat3x2 texMatrix;\n\
        uniform uint permissions[1024];\n\
        uniform uint chunkid;\n\
        layout(location = 0) in vec2 fragTexCoord;\n\
        layout(location = 1) in vec3 fragVtxColor;\n\
        layout(location = 2) in vec3 fragVtxPos;\n\
        \
        layout(location = 0) out vec4 outColor;\n\
        layout(location = 1) out uint outPick;\n\
        \
        void main()\n\
        {\n\
            vec4 texel = texture(texSampler, (texMatrix * vec3(fragTexCoord, 0)).xy );\n\
            outColor = texel * vec4(fragVtxColor.xyz, 1.0) * vec4(0.5, 0.5, 0.5, 1.0);\n\
            uint permx = (uint((fragVtxPos.x * 64) + 256) % 512) / 16;\n\
            uint permy = (uint((fragVtxPos.z * 64) + 256) % 512) / 16;\n\
            uint tileIdx = (permy * 32) + permx;\n\
            if(permissions[tileIdx] == 0x80){\n\
                outColor = outColor + vec4(0.5,0.0,0.0,0.0);\n\
            } else {\n\
                outColor = outColor + vec4(0.0,0.5,0.0,0.0);\n\
            }\n\
            if(outColor.a < 1.0 / 255.0) discard;\n\
            if(tileIdx >= 0 || tileIdx  < 1024 || (outColor.a > (1.0 / 255.0))){\n\
                outPick = (((permy * 32) + permx) << 16) | chunkid;\n\
            } else {\n\
                outPick = 0xFFFFFFFF;\n\
            }\n\
        }\
    ";

    void ClearModelCache(){
        for(auto [id, model] : mLoadedChunkModels){
            delete model;
        }

        for(auto [id, model] : mLoadedModels){
            delete model;
        }

        mLoadedChunkModels.clear();
        mLoadedModels.clear();
    }


    void InitMapShader(){
        char glErrorLogBuffer[4096];
        uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
        uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs, 1, &map_vtx_shader_source, NULL);
        glShaderSource(fs, 1, &map_frg_shader_source, NULL);
        glCompileShader(vs);
        int32_t status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            int32_t infoLogLength;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
            printf("[NSBMD Loader]: Compile failure in mdl vertex shader:\n%s\n", glErrorLogBuffer);
        }
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            int32_t infoLogLength;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
            printf("[NSBMD Loader]: Compile failure in mdl fragment shader:\n%s\n", glErrorLogBuffer);
        }
        mChunkProgram = glCreateProgram();
        glAttachShader(mChunkProgram, vs);
        glAttachShader(mChunkProgram, fs);
        glLinkProgram(mChunkProgram);
        glGetProgramiv(mChunkProgram, GL_LINK_STATUS, &status);
        if(GL_FALSE == status) {
            int32_t logLen;
            glGetProgramiv(mChunkProgram, GL_INFO_LOG_LENGTH, &logLen);
            glGetProgramInfoLog(mChunkProgram, logLen, NULL, glErrorLogBuffer);
            //printf("[NSBMD Loader]: Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
        }
        glDetachShader(mChunkProgram, vs);
        glDetachShader(mChunkProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
}

void MapChunkHeader::Read(bStream::CStream& stream, uint32_t gameCode){
    switch (gameCode){
    case (uint32_t)'EUPC': {
        mAreaID = stream.readUInt8();
        mMoveModelID = stream.readUInt8() & 0b11110000;
        mMatrixID = stream.readUInt16();
        mScriptID = stream.readUInt16();
        mSpScriptID = stream.readUInt16();
        mMsgID = stream.readUInt16();
        mBgmDayID = stream.readUInt16();
        mBgmNightID = stream.readUInt16();
        mEncDataID = stream.readUInt16();
        mEventDataID = stream.readUInt16();
        mPlaceNameID = stream.readUInt8();
        mTextBoxType = stream.readUInt8();
        mWeatherID = stream.readUInt8();
        mCameraID = stream.readUInt8();
        
        uint16_t data = stream.readUInt16();
        mMapType = data & 0b1111111;
        mBattleBgType = (data >> 7) & 0b1111111;

        uint8_t byte = (data >> 12) & 0b1111111;
        //uint8_t byte = stream.readUInt8();
        mBattleBgType = byte & 0b11110000;
        mBicycleFlag = byte & 0b00001000;
        mDashFlag = byte & 0b00000100;
        mEscapeFlag = byte & 0b00000010;
        mFlyFlag = byte & 0b00000001;
        break;
    }

    case (uint32_t)'EGPI' : {
        mEncDataID = stream.readUInt8();
        mAreaID = stream.readUInt8();
        uint16_t tmp = stream.readUInt16();
        mMoveModelID = tmp & 0b1111000000000000;
        //townmap x z = next 12  bits, 6 each
        mMatrixID = stream.readUInt16();
        mScriptID = stream.readUInt16();
        mSpScriptID = stream.readUInt16();
        mMsgID = stream.readUInt16();
        mBgmDayID = stream.readUInt16();
        mBgmNightID = stream.readUInt16();
        mEventDataID = stream.readUInt16();
        mPlaceNameID = stream.readUInt8();
        mTextBoxType = stream.readUInt8();

        uint32_t flags = stream.readUInt32();
        mKantoFlag = flags & 0b1;
        mWeatherID = flags & 0b01111111;
        mMapType = (flags << 8) & 0b1111;
        mFlyFlag = flags & 0b000010000;
        mEscapeFlag = flags & 0b00001000;
        mBicycleFlag = flags & 0b00000010;
        break;
    }

    default:
        break;
    }
}

void MapChunk::LoadGraphics(std::shared_ptr<Palkia::Nitro::File> mapTex, std::shared_ptr<Palkia::Nitro::Archive> buildModels){
    bStream::CMemoryStream mapModelStream(mModelData.data(), mModelData.size(), bStream::Endianess::Little, bStream::OpenMode::In);
    Palkia::Formats::NSBMD* mapModel = new Palkia::Formats::NSBMD();
    mapModel->Load(mapModelStream);

    bStream::CMemoryStream mapTexStrm(mapTex->GetData(), mapTex->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    Palkia::Formats::NSBTX mapTextureSet;
    mapTextureSet.Load(mapTexStrm);

    mapModel->AttachNSBTX(&mapTextureSet);

    MapGraphicsHandler::mLoadedChunkModels[mID] = mapModel;

    //std::cout << "Building Count " << mBuildings.size() << std::endl;
    for(auto building : mBuildings){
        //std::cout << "Building ID " << building.mModelID << std::endl;
        if(!MapGraphicsHandler::mLoadedModels.contains(building.mModelID) && building.mModelID < buildModels->GetFileCount()){
            auto buildingModel = buildModels->GetFileByIndex(building.mModelID);
            bStream::CMemoryStream buildingModelStream(buildingModel->GetData(), buildingModel->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
            MapGraphicsHandler::mLoadedModels[building.mModelID] = new Palkia::Formats::NSBMD();
            MapGraphicsHandler::mLoadedModels[building.mModelID]->Load(buildingModelStream);
        }
    }



}

void MapChunk::Draw(uint8_t cx, uint8_t cy, uint8_t cz, glm::mat4 v){
    glm::mat4 chunkTransform = v * glm::translate(glm::mat4(1.0f), glm::vec3(cx * 512, cz, cy * 512));

    uint32_t perms[1024] = {0};

    for(int t = 0; t < 1024; t++){
        perms[t] = mMovementPermissions[t].second;
    }

    if(MapGraphicsHandler::mChunkProgram == 0xFFFFFFFF){
        MapGraphicsHandler::InitMapShader();
    }
    
    glUseProgram(MapGraphicsHandler::mChunkProgram);
    glUniformMatrix4fv(glGetUniformLocation(MapGraphicsHandler::mChunkProgram, "transform"), 1, 0, &chunkTransform[0][0]);
    glUniform1uiv(glGetUniformLocation(MapGraphicsHandler::mChunkProgram, "permissions"), 1024, perms);
    glUniform1ui(glGetUniformLocation(MapGraphicsHandler::mChunkProgram, "chunkid"), static_cast<uint32_t>(mID));
    if(MapGraphicsHandler::mLoadedChunkModels[mID] != nullptr){
        MapGraphicsHandler::mLoadedChunkModels[mID]->Render(chunkTransform, 0, true);
    }
    glUseProgram(0);

    for(auto building : mBuildings){
        glm::mat4 modelTransform = v * glm::translate(glm::mat4(1.0f), glm::vec3((cx * 512) + building.x, cz + building.y, (cy * 512) + building.z));
        if(MapGraphicsHandler::mLoadedModels[building.mModelID] != nullptr) MapGraphicsHandler::mLoadedModels[building.mModelID]->Render(modelTransform, building.mPickID);
    }

}

Building* MapChunk::Select(uint32_t id){
    //std::cout << "Selecting building in chunk ID " << mID << std::endl;
    for(int i = 0; i < mBuildings.size(); i++){
        if(id != 0 && id == mBuildings[i].mPickID){
            return &mBuildings[i];
        }
    }
    return nullptr;
}

MapChunk::MapChunk(bStream::CStream& stream, std::string gameCode){
    uint32_t permissionsSize = stream.readUInt32();
    uint32_t buildingsSize = stream.readUInt32();
    uint32_t modelSize = stream.readUInt32();
    uint32_t bdhcSize = stream.readUInt32();

    mBuildings = std::vector<Building>();

    uint32_t bgmPlateSize = 0x00000000;

    if(gameCode == "EGPI"){
        uint16_t bgSig = stream.readUInt16();
        uint16_t bgDataLen = stream.readUInt16();
        bgmPlateSize = bgDataLen + 0x04;
        stream.skip(bgDataLen);
    }

    stream.seek(bgmPlateSize + 0x10);
    for(int i = 0; i < 1024; i++){
        mMovementPermissions[i].first = stream.readUInt8();
        mMovementPermissions[i].second = stream.readUInt8();
    }

    stream.seek(bgmPlateSize + permissionsSize + 0x10);
    while(stream.tell() < bgmPlateSize + permissionsSize + buildingsSize + 0x10){
        Building b;
        b.mModelID = stream.readUInt32();
        b.x = Palkia::fixed(stream.readInt32());
        b.y = Palkia::fixed(stream.readInt32());
        b.z = Palkia::fixed(stream.readInt32());
        b.rx = Palkia::fixed(stream.readInt32());
        b.ry = Palkia::fixed(stream.readInt32());
        b.rz = Palkia::fixed(stream.readInt32());
        b.l = Palkia::fixed(stream.readUInt32());
        b.w = Palkia::fixed(stream.readUInt32());
        b.h = Palkia::fixed(stream.readUInt32());
        b.unk1 = stream.readUInt32();
        b.unk2 = stream.readUInt32();

        b.mPickID = GetID();
        mBuildings.push_back(b);
    }


    stream.seek(bgmPlateSize + permissionsSize + buildingsSize + 0x10);
    // read model buffer into memory so we can load this as and when we need to!
    mModelData = {};
    mModelData.resize(modelSize);
    stream.readBytesTo(mModelData.data(), modelSize);

    // TODO: figure out bdhc stuff!
    mBDHCData = {};
    mBDHCData.resize(bdhcSize);
    stream.seek(bgmPlateSize + permissionsSize + buildingsSize + modelSize + 0x10);
    stream.readBytesTo(mBDHCData.data(), bdhcSize);
}

void MapChunk::Save(std::shared_ptr<Palkia::Nitro::Archive> archive){
    uint32_t dataSize = (mMovementPermissions.size() * 2) + (0x30 * mBuildings.size()) + mModelData.size() + mBDHCData.size();
    bStream::CMemoryStream stream(dataSize, bStream::Endianess::Little, bStream::OpenMode::Out);

    //std::cout << "Saving Chunk ID " << mID << std::endl;

    stream.writeUInt32(mMovementPermissions.size() * 2);
    stream.writeUInt32(0x30 * mBuildings.size());
    stream.writeUInt32(mModelData.size());
    stream.writeUInt32(mBDHCData.size());

    for(int i = 0; i < 1024; i++){
        stream.writeUInt8(mMovementPermissions[i].first);
        stream.writeUInt8(mMovementPermissions[i].second);
    }

    for(auto building : mBuildings){
        stream.writeUInt32(building.mModelID);

        stream.writeUInt32((uint32_t)(building.x * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.y * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.z * (1 << 12)));

        stream.writeUInt32((uint32_t)(building.rx * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.ry * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.rz * (1 << 12)));

        stream.writeUInt32((uint32_t)(building.l * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.w * (1 << 12)));
        stream.writeUInt32((uint32_t)(building.h * (1 << 12)));

        stream.writeUInt32(building.unk1);
        stream.writeUInt32(building.unk2);
    }

    stream.writeBytes(mModelData.data(), mModelData.size());
    stream.writeBytes(mBDHCData.data(), mBDHCData.size());

    archive->GetFileByIndex(mID)->SetData(stream.getBuffer(), stream.getSize());
}

void MapChunk::ImportChunkNSBMD(std::string path){
    mModelData.clear();

    bStream::CFileStream model(path, bStream::Endianess::Little, bStream::OpenMode::In);

    mModelData.resize(model.getSize());
    model.readBytesTo(mModelData.data(), mModelData.size());
}

void MapChunk::ExportChunkNSBMD(std::string path){
    bStream::CFileStream model(path, bStream::Endianess::Little, bStream::OpenMode::Out);
    model.writeBytes(mModelData.data(), mModelData.size());
}

MapChunk::MapChunk(){}
MapChunk::~MapChunk(){}
