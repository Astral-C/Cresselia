#include <cstdint>
#include <memory>
#include "Text.hpp"
#include "GameResources.hpp"
#include "GameConfig.hpp"
#include "glad/glad.h"

std::vector<uint32_t> PokemonIcons = {0};
std::vector<uint32_t> AreaTypeImages = {0};
std::vector<std::string> PokemonNames = {"NAMES NOT LOADED"};
std::vector<std::string> LocationNames = {"NAMES NOT LOADED"};
std::size_t EncounterSetCount = 0;

void LoadGameResources(std::unique_ptr<Palkia::Nitro::Rom>& mRom){
    const GameConfig config = Configs[mRom->GetHeader().gameCode];

    auto textArcFile = mRom->GetFile(Configs[mRom->GetHeader().gameCode].mMsgPath);
	bStream::CMemoryStream textArcStream(textArcFile->GetData(), textArcFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);

	auto textArchive = Palkia::Nitro::Archive(textArcStream);
	
	auto locationNamesFile = textArchive.GetFileByIndex(config.mLocationNamesFileID);
	auto pokemonNamesFile = textArchive.GetFileByIndex(config.mPokeNamesFileID);

	bStream::CMemoryStream locationNamesStream(locationNamesFile->GetData(), locationNamesFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    bStream::CMemoryStream pokemonNamesStream(pokemonNamesFile->GetData(), pokemonNamesFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    
	LocationNames = Text::DecodeStringList(locationNamesStream);
    PokemonNames = Text::DecodeStringList(pokemonNamesStream);

    LocationNames.shrink_to_fit();
	PokemonNames.shrink_to_fit();

	{
	    auto encounterArc = mRom->GetFile(Configs[mRom->GetHeader().gameCode].mEncounterDataPath);
		bStream::CMemoryStream encounterArcStream(encounterArc->GetData(), encounterArc->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
		auto encounterArchive = Palkia::Nitro::Archive(encounterArcStream);
		EncounterSetCount = encounterArchive.GetFileCount();
	}
    auto pokeIconArcFile = mRom->GetFile(config.mPokemonIconPath);
	bStream::CMemoryStream pokeIconStrm(pokeIconArcFile->GetData(), pokeIconArcFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    auto pokeIconArc = Palkia::Nitro::Archive(pokeIconStrm);

    PokemonIcons.resize(pokeIconArc.GetFileCount() - 7);
    
    glGenTextures((uint32_t)PokemonIcons.size(), &PokemonIcons[0]);

    auto arm9 = mRom->GetFile("@arm9.bin");

    if(config.compressedArm9){
        Palkia::Nitro::Compression::BLZDecompress(arm9);
    }                
    
    bStream::CMemoryStream arm(arm9->GetData(), arm9->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    arm.seek(config.mPokemonIconPaletteMapOffset); // this is just for platinum. gonna need to be from game config

    std::vector<uint8_t> paletteIndices = {};
    paletteIndices.resize(PokemonIcons.size());
    arm.readBytesTo(paletteIndices.data(), paletteIndices.size());
    
    Palkia::Formats::NCLR pokemonIconPalette;

    {
        auto pokemonIconPaletteFile = pokeIconArc.GetFileByIndex(0);
        bStream::CMemoryStream paletteStream(pokemonIconPaletteFile->GetData(), pokemonIconPaletteFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        pokemonIconPalette.Load(paletteStream);
    }
    
    for(int i = 0; i < PokemonIcons.size(); i++){
        auto ncgrFile = pokeIconArc.GetFileByIndex(i+7);

        Palkia::Formats::NCGR ncgr;
        bStream::CMemoryStream ncgrStrm(ncgrFile->GetData(), ncgrFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);

        ncgr.Load(ncgrStrm);

        auto data = ncgr.Convert(32, 32, pokemonIconPalette, paletteIndices[i]);

        glBindTexture(GL_TEXTURE_2D, PokemonIcons[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glBindTexture(GL_TEXTURE_2D, 0);
	}

	auto areaWinArc = mRom->GetFile(config.mAreaWinArcPath);
	bStream::CMemoryStream areaWinStrm(areaWinArc->GetData(), areaWinArc->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
	
	auto areaWin = Palkia::Nitro::Archive(areaWinStrm);

	AreaTypeImages.resize(areaWin.GetFileCount() / 2);

	glGenTextures(AreaTypeImages.size(), &AreaTypeImages[0]);

	for(int i = 0; i < AreaTypeImages.size(); i++){
        auto ncgrFile = areaWin.GetFileByIndex(i * 2);
        auto nclrFile = areaWin.GetFileByIndex((i * 2) + 1);

        Palkia::Formats::NCGR ncgr;
        Palkia::Formats::NCLR nclr;

        bStream::CMemoryStream ncgrStrm(ncgrFile->GetData(), ncgrFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        bStream::CMemoryStream nclrStrm(nclrFile->GetData(), nclrFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);

        ncgr.Load(ncgrStrm);
        nclr.Load(nclrStrm);

        auto data = ncgr.Convert(136, 40, nclr);

        glBindTexture(GL_TEXTURE_2D, AreaTypeImages[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 136, 40, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glBindTexture(GL_TEXTURE_2D, 0);

	}
}

void ClearGameResources(){
    glDeleteTextures(AreaTypeImages.size(), &AreaTypeImages[0]);
    glDeleteTextures(PokemonIcons.size(), &PokemonIcons[0]);
}