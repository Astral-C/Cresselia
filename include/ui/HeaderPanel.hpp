#include "Map/Chunk.hpp"
#include "GameConfig.hpp"
namespace MapHeaderPanel {

extern std::vector<MapChunkHeader> Headers;

bool InitHeaderPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config);
void DrawPanel(std::string& ContextTool);

}