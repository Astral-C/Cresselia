#include <memory>
#include <imgui.h>
#include "GameConfig.hpp"
#include "Map/Matrix.hpp"

#include "ui/MatrixPanel.hpp"
#include "GameResources.hpp"
#include "imgui_internal.h"
#include <glad/glad.h>

namespace MatrixPanel {

Matrix CurrentMatrix;

bool InitMatrixPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){

	return true;
}

void DrawPanel(){

}

}