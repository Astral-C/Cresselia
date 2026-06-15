#ifndef __POKEDATA_H__
#define __POKEDATA_H__
#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include "NDS/System/Rom.hpp"
#include "NDS/System/Archive.hpp"
#include "NDS/System/Compression.hpp"
#include "NDS/Assets/NCGR.hpp"
#include "NDS/Assets/NCLR.hpp"

extern std::vector<uint32_t> PokemonIcons; //539
extern std::vector<uint32_t> AreaTypeImages; //9

extern std::vector<std::string> PokemonNames;
extern std::vector<std::string> LocationNames;
extern std::size_t EncounterSetCount;

void LoadGameResources(std::unique_ptr<Palkia::Nitro::Rom>& romFile);
void ClearGameResources();

#endif