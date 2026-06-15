#pragma once
#include <cstdint>
#include <string>
#include <map>

struct GameConfig {
    std::string mGameCode;
    uint32_t mChunkHeaderPtr;
    std::string mAreaDataPath;
    std::string mMapTablePath;
    std::string mLandDataPath;
    std::string mMapTexSetPath;
    std::string mBuildModelPath;
    std::string mMapMatrixPath;
    std::string mZoneEventPath;
    std::string mEncounterDataPath;
    std::string mMsgPath;
    std::string mMoveModel;
    std::string mMoveModelList;
    std::string mIndoorBuildingPath;
    std::string mAreaWinArcPath;
    std::string mPokemonIconPath;
    
    uint32_t mLocationNamesFileID;
    uint32_t mPokeNamesFileID;
    bool compressedArm9;
    
    uint32_t mPokemonIconPaletteMapOffset;
};

extern const GameConfig Platinum;
extern const GameConfig SoulSilver;
extern const GameConfig Black;

uint32_t CurrentGameConfig();
void CurrentGameConfig(uint32_t config);

extern std::map<uint32_t, const GameConfig> Configs;
extern std::map<uint8_t, std::string> PlatWeatherLabels;
extern std::map<uint16_t, std::string> DPPlatBGM;