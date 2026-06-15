#include <cstdint>
#include <memory>
#include <imgui.h>
#include <string>
#include "GameConfig.hpp"
#include "IconsForkAwesome.h"
#include "IconsLucide.h"
#include "Map/Chunk.hpp"
#include "Map/Encounters.hpp"
#include "NDS/System/Archive.hpp"
#include "NDS/System/FileSystem.hpp"
#include "NDS/System/Rom.hpp"
#include "bstream/bstream.h"

#include "ui/HeaderPanel.hpp"
#include "ui/EncounterPanel.hpp"
#include "ui/MatrixPanel.hpp"
#include "ui/ChunkPanel.hpp"
#include "GameResources.hpp"
#include "imgui_internal.h"
#include "IconsForkAwesome.h"

namespace MapHeaderPanel {

std::vector<std::string> HeaderNames = {};
std::vector<MapChunkHeader> Headers = {};
std::string SelectedPlaceName = "(none)";
int SelectedHeaderIndex = -1;

bool InitHeaderPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    auto arm9 = rom->GetFile("@arm9.bin");
    
    bStream::CMemoryStream armStream(arm9->GetData(), arm9->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    armStream.seek(config.mChunkHeaderPtr);

    auto mapTable = rom->GetFile(config.mMapTablePath);
    auto stream = bStream::CMemoryStream(mapTable->GetData(), mapTable->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    for(int x = 0; x < mapTable->GetSize() / 16; x++){
        HeaderNames.push_back(stream.readString(16));
    }

    uint8_t areaCount = 0;
    for(int i = 0; i < HeaderNames.size(); i++){
        MapChunkHeader header;
        header.Read(armStream, rom->GetHeader().gameCode);
        areaCount = std::max(header.mAreaID, areaCount);
        Headers.push_back(header);
    }
    HeaderNames.shrink_to_fit();
    Headers.shrink_to_fit();
    return true;
}

void SaveHeaders(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){

}

void DrawPanel(std::string& ContextTool){
    const GameConfig config = Configs[CurrentGameConfig()];
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {7.f, 7.f});
    ImGui::BeginChild("##headerPanel", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AlwaysUseWindowPadding);

    ImVec2 placenameBoxSize = ImGui::GetContentRegionAvail() * ImVec2(0.15f, 1.0f);
    ImVec2 headerNameBoxSize = ImGui::GetContentRegionAvail() * ImVec2(0.125f, 1.0f);
    
    { // Location List
        ImGui::BeginGroup();
        ImGui::BeginListBox("##locationList", placenameBoxSize * ImVec2(1.0f, 0.96f));
        for(std::size_t location = 0; location < LocationNames.size(); location++){
            if(ImGui::Selectable(LocationNames[location].c_str(), LocationNames[location] == SelectedPlaceName)){
                SelectedPlaceName = LocationNames[location];
                for(std::size_t headerIdx = 0; headerIdx < Headers.size(); headerIdx++){
                    if(Headers[headerIdx].mPlaceNameID == location){
                        SelectedHeaderIndex = headerIdx; 
                        if(Headers[SelectedHeaderIndex].mEncDataID != 0xFFFF) EncounterPanel::SetEncounterData(Headers[SelectedHeaderIndex].mEncDataID);
                        if(Headers[SelectedHeaderIndex].mMatrixID != 0xFFFF) MatrixPanel::SetMatrixData(Headers[SelectedHeaderIndex].mMatrixID);
                        if(Headers[SelectedHeaderIndex].mAreaID == 0xFF){
                            ChunkPanel::SetChunkData(0, SelectedHeaderIndex, Configs[CurrentGameConfig()]);
                        } else {
                            ChunkPanel::SetChunkData(Headers[SelectedHeaderIndex].mAreaID, SelectedHeaderIndex, Configs[CurrentGameConfig()]);
                        }                        
                        break;
                    }
                }
            }
        }
        ImGui::EndListBox();
        ImVec2 btnSize = (placenameBoxSize * ImVec2(0.5f, 0.035f)) - ImVec2(style.ItemSpacing.x * 0.510f, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing - ImVec2(0.5f, 0.0f));
        if(ImGui::Button("Add##addLocationName", btnSize)){
            //open modal
        }
        ImGui::SameLine();
        if(ImGui::Button("Remove##removeLocationName", btnSize) && SelectedPlaceName != ""){
            // modal warn
            for(auto& header : Headers){
                if(SelectedPlaceName == LocationNames[header.mPlaceNameID]){
                    header.mPlaceNameID = 0;
                }
            }
            std::erase_if(LocationNames, [](std::string s){ return s == SelectedPlaceName; });
            SelectedPlaceName = "";
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();
    }

    ImGui::SameLine();
    
    if(config.mGameCode == "EUPC") {
        // Header name box
        ImGui::BeginListBox("##locations", headerNameBoxSize);
        for (std::size_t i = 0; i < Headers.size(); i++) {
            if(LocationNames[Headers[i].mPlaceNameID] == SelectedPlaceName){
                if(ImGui::Selectable(HeaderNames[i].c_str(), SelectedHeaderIndex == i)){
                    SelectedHeaderIndex = i;
                    if(Headers[i].mEncDataID != 0xFFFF) EncounterPanel::SetEncounterData(Headers[i].mEncDataID);
                    if(Headers[i].mMatrixID != 0xFFFF) MatrixPanel::SetMatrixData(Headers[i].mMatrixID);
                    if(Headers[i].mAreaID == 0xFF){
                        ChunkPanel::SetChunkData(0, i, Configs[CurrentGameConfig()]);
                    } else {
                        ChunkPanel::SetChunkData(Headers[i].mAreaID, i, Configs[CurrentGameConfig()]);
                    }
                }
            }
        }
        ImGui::EndListBox();

        ImGui::SameLine();
        
        if(SelectedHeaderIndex != -1){
            ImGui::BeginChild("##chunkHeaderEdit");
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 15.0f));

            // Label Position for later
            ImVec2 labelPos = ImGui::GetCursorPos() - style.FramePadding;

            {
                // Header Name Edit
                // Draw header name label
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_MAP_PIN " Header Name");
                ImGui::PopFont();
                // Draw header name input, extra padding to make text a bit more centered
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.0f, 15.0f));
                ImGui::InputText("##mapHeaderName", HeaderNames[SelectedHeaderIndex].data(), 16);
                ImGui::PopStyleVar();
                ImGui::SameLine(); // preserve line for area placard combo
            }
            
            // Draw area placard label, a bit messy
            ImVec2 tmpPos = ImGui::GetCursorPos();
            { 
                // Area Placard
                ImGui::SetCursorPos(ImVec2(tmpPos.x, labelPos.y));
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_IMAGE " Placard");
                ImGui::PopFont();
                ImGui::SetCursorPos(tmpPos);
    
                // Draw Combo box of area placard images
                ImGui::SetNextItemWidth(190);
                if(ImGui::BeginCombo(std::format("##areaWinGraSelect{}", SelectedHeaderIndex).data(), nullptr, ImGuiComboFlags_CustomPreview)){
                    for(uint32_t j = 0; j < AreaTypeImages.size(); j++){
                        bool is_selected = (Headers[SelectedHeaderIndex].mTextBoxType-1 == j);
                        float cursorX = ImGui::GetCursorPosX();
                        if (ImGui::Selectable(std::format("##areaWin{}#{}",j,SelectedHeaderIndex).c_str(), is_selected, 0, {136, 40})){
                            Headers[SelectedHeaderIndex].mTextBoxType = j+1;
                        }
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(cursorX);
                        ImGui::Image(AreaTypeImages[j], {136, 40});
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
        
                if(ImGui::BeginComboPreview()){
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-12.5f); // hate
                    if(Headers[SelectedHeaderIndex].mTextBoxType >= AreaTypeImages.size()){
                        ImGui::Image(AreaTypeImages[0], {136, 40});
                    } else {
                        ImGui::Image(AreaTypeImages[Headers[SelectedHeaderIndex].mTextBoxType-1], {136, 40});
                    }
                    ImGui::EndComboPreview();
                }
            }

            labelPos = ImGui::GetCursorPos() - style.FramePadding;
            { 
                // Encounter Combo
                // Draw Encounter Set label
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_TREES " Encounter Set");
                float textWidth = ImGui::CalcTextSize(ICON_LC_TREES " Encounter Set").x;
                ImGui::PopFont();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()-16);

                ImGui::SetNextItemWidth(textWidth * 2.0f);
                // Draw Encounter Set combo, needs padding for the same reasons as the header name
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.0f, 15.0f));
                if(ImGui::BeginCombo("##encounterSelectCombo", Headers[SelectedHeaderIndex].mEncDataID != 0xFFFF ? std::format("Encounter Set {}", Headers[SelectedHeaderIndex].mEncDataID).data() : "(NONE)")){
                    if(ImGui::Selectable(ICON_LC_CIRCLE_PLUS " New", false)){
                        Headers[SelectedHeaderIndex].mEncDataID = EncounterPanel::NewEncounterData();
                    }
                    if(ImGui::Selectable("(NONE)", Headers[SelectedHeaderIndex].mEncDataID == 0xFFFF)){
                        Headers[SelectedHeaderIndex].mEncDataID = 0xFFFF;
                        EncounterPanel::SetEncounterData(0xFFFF);
                    }
                    for(uint16_t set = 0; set < EncounterSetCount; set++){
                        if(ImGui::Selectable(std::format("Encounter Set {}", set).data(), set == Headers[SelectedHeaderIndex].mEncDataID)){
                        Headers[SelectedHeaderIndex].mEncDataID = set;
                        EncounterPanel::SetEncounterData(set);
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleVar();
            }
            
            ImGui::SameLine();

            { 
                // Weather Combo
                tmpPos = ImGui::GetCursorPos();
                ImGui::SetCursorPos({tmpPos.x, labelPos.y});
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_CLOUD " Weather");
                ImGui::PopFont();
                ImGui::SetCursorPos(tmpPos);
                ImGui::SetNextItemWidth(200);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.0f, 15.0f));
                if(ImGui::BeginCombo("##weatherSelectCombo", PlatWeatherLabels[Headers[SelectedHeaderIndex].mWeatherID].data())){
                    for(auto [id, label] : PlatWeatherLabels){
                        if(ImGui::Selectable(label.data(), id == Headers[SelectedHeaderIndex].mWeatherID)){
                        Headers[SelectedHeaderIndex].mWeatherID = id;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleVar();
            }

            float height = ImGui::GetCursorPosY() - tmpPos.y;

            ImGui::SameLine();

            { // Map Flags
                tmpPos = ImGui::GetCursorPos();
                ImGui::SetCursorPos({tmpPos.x, labelPos.y});
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_FLAG " Map Flags");
                ImGui::PopFont();
                ImGui::SetCursorPos(tmpPos);

                ImGui::BeginGroup();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.5f, 2.5f));
                bool enabled =  Headers[SelectedHeaderIndex].mBicycleFlag == 1;
                if(ImGui::Checkbox( ICON_LC_BIKE " Can Bike", &enabled)){
                    Headers[SelectedHeaderIndex].mBicycleFlag = enabled ? 1 : 0;
                }
                ImGui::SameLine();

                enabled = Headers[SelectedHeaderIndex].mEscapeFlag == 1;
                if(ImGui::Checkbox( ICON_LC_ARROW_BIG_UP_DASH " Escape Rope", &enabled)){
                    Headers[SelectedHeaderIndex].mEscapeFlag = enabled ? 1 : 0;
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY()-0.145f);
                enabled = Headers[SelectedHeaderIndex].mDashFlag == 1;
                if(ImGui::Checkbox( ICON_LC_FOOTPRINTS " Can Run ", &enabled)){
                    Headers[SelectedHeaderIndex].mDashFlag = enabled ? 1 : 0;
                }
                ImGui::SameLine();
                
                enabled = Headers[SelectedHeaderIndex].mFlyFlag == 1;
                if(ImGui::Checkbox( ICON_LC_BIRD " Fly Enabled", &enabled)){
                    Headers[SelectedHeaderIndex].mFlyFlag = enabled ? 1 : 0;
                }
                ImGui::PopStyleVar();
                ImGui::EndGroup();
            }

            labelPos = ImGui::GetCursorPos() - style.FramePadding;

            { 
                // BGM Settings
                ImGui::PushFont(nullptr, 24.0f);
                ImGui::Text(ICON_LC_MUSIC " BGM Settings");
                ImGui::PopFont();
                
                
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.0f, 15.0f));

                ImGui::PushFont(nullptr, 24.0f);
                ImGui::AlignTextToFramePadding();
                ImGui::Text(ICON_LC_SUN);
                ImGui::PopFont();
                ImGui::SameLine();
                ImGui::SetNextItemWidth(300);
                if(ImGui::BeginCombo("##bgmDayCombo", DPPlatBGM[Headers[SelectedHeaderIndex].mBgmDayID].data())){
                    for(auto [id, label] : DPPlatBGM){
                        if(ImGui::Selectable(label.data(), id == Headers[SelectedHeaderIndex].mBgmDayID)){
                        Headers[SelectedHeaderIndex].mBgmDayID = id;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();

                ImGui::PushFont(nullptr, 24.0f);
                ImGui::AlignTextToFramePadding();
                ImGui::Text(ICON_LC_MOON_STAR);
                ImGui::PopFont();
                ImGui::SameLine();
                ImGui::SetNextItemWidth(300);
                if(ImGui::BeginCombo("##bgmNightCombo", DPPlatBGM[Headers[SelectedHeaderIndex].mBgmNightID].data())){
                    for(auto [id, label] : DPPlatBGM){
                        if(ImGui::Selectable(label.data(), id == Headers[SelectedHeaderIndex].mBgmNightID)){
                        Headers[SelectedHeaderIndex].mBgmNightID = id;
                        }
                    }
                    ImGui::EndCombo();
                }
                
                ImGui::PopStyleVar();
            }
            
            ImGui::PopStyleVar();
            ImGui::EndChild();
        }
	}
    ImGui::PopStyleVar();
	ImGui::EndChild();
}

}