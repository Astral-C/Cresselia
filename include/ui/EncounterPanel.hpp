#include "Map/Encounters.hpp"
#include "GameConfig.hpp"
#include <cstdint>

namespace EncounterPanel {

bool InitEncounterPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config);
void SetEncounterData(uint16_t encounterID);
uint16_t NewEncounterData();
void DrawPanel();
    
}