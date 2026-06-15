#include <memory>
#include <imgui.h>
#include "GameConfig.hpp"
#include "Map/Matrix.hpp"

#include "ui/MatrixPanel.hpp"
#include "GameResources.hpp"
#include "NDS/System/FileSystem.hpp"
#include "imgui_internal.h"
#include <glad/glad.h>
#include <set>
#include <vector>

namespace MatrixPanel {

Matrix CurrentMatrix;

std::shared_ptr<Palkia::Nitro::Archive> MapMatrixArchive { nullptr };
std::shared_ptr<Palkia::Nitro::File> MapMatrixArchiveFile { nullptr };

bool InitMatrixPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){

    MapMatrixArchiveFile = rom->GetFile(config.mMapMatrixPath);
    if(MapMatrixArchiveFile == nullptr) {
        return false;
    }
    
    bStream::CMemoryStream matrixArcData(MapMatrixArchiveFile->GetData(), MapMatrixArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    MapMatrixArchive = std::make_shared<Palkia::Nitro::Archive>(matrixArcData);
    
	return true;
}

void SetMatrixData(uint16_t matrixID){
    CurrentMatrix = {};
    
    std::shared_ptr<Palkia::Nitro::File> matrixFile = MapMatrixArchive->GetFileByIndex(matrixID);
    if(matrixFile == nullptr){
        return;
    }
    
    CurrentMatrix.Load(matrixFile);
}

void DrawPanel(){
    
}

}