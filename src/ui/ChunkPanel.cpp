#include <memory>
#include <imgui.h>
#include "GameConfig.hpp"
#include "Map/Chunk.hpp"
#include "Map/Matrix.hpp"
#include "NDS/System/Archive.hpp"
#include "NDS/System/FileSystem.hpp"
#include "NDS/System/Rom.hpp"
#include "bstream/bstream.h"

#include "ui/ChunkPanel.hpp"
#include "glm/ext/vector_float2.hpp"
#include "ui/MatrixPanel.hpp"
#include "ui/HeaderPanel.hpp"
#include "GameResources.hpp"
#include "imgui_internal.h"
#include <glad/glad.h>

#include "Map/Area.hpp"

namespace ChunkPanel {

// Rendering surface
uint32_t FBO, RBO, ViewTex, PickTex;

float PrevWinWidth { -1.0f };
float PrevWinHeight { -1.0f };

glm::vec2 QueuedCameraPosition { 0.0f, 0.0f };

bool inPermsMode { false };

// LandData arc from ROM
std::shared_ptr<Palkia::Nitro::Archive> LandDataArchive { nullptr };
std::shared_ptr<Palkia::Nitro::File> LandDataArchiveFile { nullptr };

std::shared_ptr<Palkia::Nitro::Archive> AreaDataArchive { nullptr };
std::shared_ptr<Palkia::Nitro::File> AreaDataArchiveFile { nullptr };

std::shared_ptr<Palkia::Nitro::Archive> BuildModelArchive { nullptr };
std::shared_ptr<Palkia::Nitro::File> BuildModelArchiveFile { nullptr };

std::shared_ptr<Palkia::Nitro::Archive> MapTexArchive { nullptr };
std::shared_ptr<Palkia::Nitro::File> MapTexArchiveFile { nullptr };

// Current Area/Headers Loaded
uint8_t CurrentAreaID { 0xFF };
Area CurrentChunkArea { };
std::map<uint16_t, MapChunk> CurrentMapChunks {};
std::map<uint16_t, std::shared_ptr<Palkia::Nitro::File>> LandDataFiles { };


bool InitChunkPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

    glGenTextures(1, &ViewTex);
    glGenTextures(1, &PickTex);

    glBindTexture(GL_TEXTURE_2D, ViewTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, PickTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1280, 720, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ViewTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, PickTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);


    // Load some archives that we need
    {
        AreaDataArchiveFile = rom->GetFile(config.mAreaDataPath);
        if(AreaDataArchiveFile == nullptr) {
            return false;
        }
        
        bStream::CMemoryStream areaData(AreaDataArchiveFile->GetData(), AreaDataArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        AreaDataArchive = std::make_shared<Palkia::Nitro::Archive>(areaData);


        LandDataArchiveFile = rom->GetFile(config.mLandDataPath);
        if(LandDataArchiveFile == nullptr) {
            return false;
        }
        
        bStream::CMemoryStream landData(LandDataArchiveFile->GetData(), LandDataArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        LandDataArchive = std::make_shared<Palkia::Nitro::Archive>(landData);        

        BuildModelArchiveFile = rom->GetFile(config.mBuildModelPath);
        if(BuildModelArchiveFile == nullptr) {
            return false;
        }
        
        bStream::CMemoryStream buildModelData(BuildModelArchiveFile->GetData(), BuildModelArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        BuildModelArchive = std::make_shared<Palkia::Nitro::Archive>(buildModelData);                


        MapTexArchiveFile = rom->GetFile(config.mMapTexSetPath);
        if(MapTexArchiveFile == nullptr) {
            return false;
        }
        
        bStream::CMemoryStream mapTexData(MapTexArchiveFile->GetData(), MapTexArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        MapTexArchive = std::make_shared<Palkia::Nitro::Archive>(mapTexData);                
    }
    
    return true;
}

void DrawPanel(USceneCamera& camera){
    ImVec2 winSize = ImGui::GetContentRegionAvail();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if(QueuedCameraPosition.x != 0.0f && QueuedCameraPosition.y != 0.0f){
        camera.SetPosition(QueuedCameraPosition);
        QueuedCameraPosition = { 0.0f, 0.0f };
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    if(winSize.x != PrevWinWidth || winSize.y != PrevWinHeight){
        glDeleteTextures(1, &ViewTex);
        glDeleteTextures(1, &PickTex);
        glDeleteRenderbuffers(1, &RBO);

        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (uint32_t)winSize.x, (uint32_t)winSize.y);

        glGenTextures(1, &ViewTex);
        glGenTextures(1, &PickTex);

        glBindTexture(GL_TEXTURE_2D, ViewTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, PickTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ViewTex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, PickTex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    
    }
    
    glViewport(0, 0, (uint32_t)winSize.x, (uint32_t)winSize.y);
    
    glClearColor(0.19f, 0.19f, 0.19f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    int32_t unused = 0;
    glClearTexImage(PickTex, 0, GL_RED_INTEGER, GL_INT, &unused);
    
    PrevWinWidth = winSize.x;
    PrevWinHeight = winSize.y;
    
    glm::mat4 projection, view;
    projection = camera.GetProjectionMatrix();
    view = camera.GetViewMatrix();
    
    // Render Models
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    std::vector<MatrixEntry>& entries = MatrixPanel::CurrentMatrix.GetEntries();

    for(uint8_t y = 0; y < MatrixPanel::CurrentMatrix.GetHeight(); y++){
        for(uint8_t x = 0; x < MatrixPanel::CurrentMatrix.GetWidth(); x++){
            MatrixEntry entry = entries[(y * MatrixPanel::CurrentMatrix.GetWidth() )+ x];
            if(entry.mChunk == 0xFFFF || entry.mHeader == 0xFFFF 
                || MapHeaderPanel::Headers[entry.mHeader].mAreaID != CurrentAreaID
                || MapHeaderPanel::Headers[entry.mHeader].mPlaceNameID != MapHeaderPanel::Headers[MapHeaderPanel::SelectedHeaderIndex].mPlaceNameID) continue;
            CurrentMapChunks[entry.mChunk].Draw(x, y, entry.mHeight*8, projection * view);
        }        
    }
    
    
    cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 frameSize = {ImGui::GetStyle().WindowBorderSize, ImGui::GetStyle().WindowBorderSize};
    ImGui::GetWindowDrawList()->AddRectFilled(cursorPos - frameSize, cursorPos + winSize + frameSize, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Border]));
    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(cursorPos, cursorPos + winSize, 0xFF303030, 0xFF303030, 0xFF202020, 0xFF202020);
    ImGui::Image(static_cast<uintptr_t>(ViewTex), { winSize.x, winSize.y }, {0.0f, 1.0f}, {1.0f, 0.0f});
    
    if(ImGui::IsItemClicked(0) && !ImGuizmo::IsOver()){
        ImVec2 mousePos = ImGui::GetMousePos();

        ImVec2 pickPos = {
            mousePos.x - cursorPos.x,
            winSize.y - (mousePos.y - cursorPos.y)
        };

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        uint32_t id = 0xFFFFFFFF;
        glReadPixels(static_cast<GLint>(pickPos.x), static_cast<GLint>(pickPos.y), 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, (void*)&id);
        
        if(id != 0xFFFFFFFF){
            uint chunkid = id & 0xFFFF;
            uint tileid = (id >> 16) & 0xFFFF;

            if(tileid >= 0 && tileid < 1024){
                //auto perms = CurrentMapChunks[chunkid].mMovementPermissions();
                if(CurrentMapChunks[chunkid].mMovementPermissions[tileid].second == 0x00){
                    CurrentMapChunks[chunkid].mMovementPermissions[tileid].second = 0x80;
                } else {
                    CurrentMapChunks[chunkid].mMovementPermissions[tileid].second = 0x00;
                }
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, winSize.x, winSize.y);
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SetChunkData(uint8_t areaID, uint16_t header, const GameConfig& config){
    CurrentChunkArea = {};
    CurrentMapChunks.clear();

    CurrentAreaID = areaID;
    
    {
        std::shared_ptr<Palkia::Nitro::File> areaFile = AreaDataArchive->GetFileByIndex(areaID);
        bStream::CMemoryStream areaFileStream(areaFile->GetData(), areaFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
        CurrentChunkArea = Area(areaFileStream, false); // needs to take game config info

        
        std::vector<MatrixEntry>& entries = MatrixPanel::CurrentMatrix.GetEntries();
    
        for(uint8_t y = 0; y < MatrixPanel::CurrentMatrix.GetHeight(); y++){
            for(uint8_t x = 0; x < MatrixPanel::CurrentMatrix.GetWidth(); x++){
                MatrixEntry entry = entries[(y * MatrixPanel::CurrentMatrix.GetWidth()) + x];
                if(entry.mChunk == 0xFFFF || entry.mHeader == 0xFFFF 
                    || MapHeaderPanel::Headers[entry.mHeader].mAreaID != areaID
                    || MapHeaderPanel::Headers[entry.mHeader].mPlaceNameID != MapHeaderPanel::Headers[MapHeaderPanel::SelectedHeaderIndex].mPlaceNameID) continue;

                QueuedCameraPosition = {(x * 512), (y * 512) + 256};
                
                std::shared_ptr<Palkia::Nitro::File> chunkFile = LandDataArchive->GetFileByIndex(entry.mChunk);
                bStream::CMemoryStream chunkStream(chunkFile->GetData(), chunkFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
                CurrentMapChunks[entry.mChunk] = MapChunk(chunkStream, config.mGameCode);
                CurrentMapChunks[entry.mChunk].mID = entry.mChunk;
                CurrentMapChunks[entry.mChunk].LoadGraphics(MapTexArchive->GetFileByIndex(CurrentChunkArea.mMapTileset), BuildModelArchive);
            }        
        }
    }
}

void Cleanup(){
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteTextures(1, &ViewTex);
    glDeleteTextures(1, &PickTex);
}

}