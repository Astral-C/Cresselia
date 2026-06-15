#include "Map/Chunk.hpp"
#include "GameConfig.hpp"
#include "UCamera.hpp"

namespace ChunkPanel {

bool InitChunkPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config);
void SetChunkData(uint8_t areaID, uint16_t header, const GameConfig& config);
void DrawPanel(USceneCamera& camera);
void Cleanup();

}