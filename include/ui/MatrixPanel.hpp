#include <memory>
#include <imgui.h>
#include "GameConfig.hpp"
#include "Map/Matrix.hpp"

namespace MatrixPanel {

extern Matrix CurrentMatrix;

bool InitMatrixPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config);
void SetMatrixData(uint16_t matrixID);

void DrawPanel();

}