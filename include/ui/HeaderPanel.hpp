#include "Map/Chunk.hpp"
#include "GameConfig.hpp"
namespace MapHeaderPanel {

extern std::vector<MapChunkHeader> Headers;
extern int SelectedHeaderIndex;
bool InitHeaderPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config);
void DrawPanel(std::string& ContextTool);

}