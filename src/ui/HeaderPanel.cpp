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
#include "GameResources.hpp"
#include "imgui_internal.h"
#include "IconsForkAwesome.h"

namespace MapHeaderPanel {

std::vector<std::string> mHeaderNames = {};
std::vector<MapChunkHeader> mHeaders = {};
std::string mSelectedPlaceName = "(none)";
int mSelectedHeaderIndex = -1;

bool InitHeaderPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    auto arm9 = rom->GetFile("@arm9.bin");
    
    bStream::CMemoryStream armStream(arm9->GetData(), arm9->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    armStream.seek(config.mChunkHeaderPtr);

    auto mapTable = rom->GetFile(config.mMapTablePath);
    auto stream = bStream::CMemoryStream(mapTable->GetData(), mapTable->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    for(int x = 0; x < mapTable->GetSize() / 16; x++){
        mHeaderNames.push_back(stream.readString(16));
    }

    uint8_t areaCount = 0;
    for(int i = 0; i < mHeaderNames.size(); i++){
        MapChunkHeader header;
        header.Read(armStream, rom->GetHeader().gameCode);
        areaCount = std::max(header.mAreaID, areaCount);
        mHeaders.push_back(header);
    }
    mHeaderNames.shrink_to_fit();
    mHeaders.shrink_to_fit();
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
            if(ImGui::Selectable(LocationNames[location].c_str(), LocationNames[location] == mSelectedPlaceName)){
                mSelectedPlaceName = LocationNames[location];
                for(std::size_t headerIdx = 0; headerIdx < mHeaders.size(); headerIdx++){
                    if(mHeaders[headerIdx].mPlaceNameID == location){
                        mSelectedHeaderIndex = headerIdx; 
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
        if(ImGui::Button("Remove##removeLocationName", btnSize) && mSelectedPlaceName != ""){
            // modal warn
            for(auto& header : mHeaders){
                if(mSelectedPlaceName == LocationNames[header.mPlaceNameID]){
                    header.mPlaceNameID = 0;
                }
            }
            std::erase_if(LocationNames, [](std::string s){ return s == mSelectedPlaceName; });
            mSelectedPlaceName = "";
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();
    }

    ImGui::SameLine();
    
    if(config.mGameCode == "EUPC") {
        // Header name box
        ImGui::BeginListBox("##locations", headerNameBoxSize);
        for (std::size_t i = 0; i < mHeaders.size(); i++) {
            if(LocationNames[mHeaders[i].mPlaceNameID] == mSelectedPlaceName){
                if(ImGui::Selectable(mHeaderNames[i].c_str(), mSelectedHeaderIndex == i)){
                    mSelectedHeaderIndex = i;
                    if(mHeaders[i].mEncDataID != 0xFFFF) EncounterPanel::SetEncounterData(mHeaders[i].mEncDataID);
                    //if(mHeaders[i].mMatrixID != 0xFFFF) MatrixPanel::SetMatrixData(mHeaders[i].mMatrixID);
                }
            }
        }
        ImGui::EndListBox();

        ImGui::SameLine();
        
        if(mSelectedHeaderIndex != -1){
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
                ImGui::InputText("##mapHeaderName", mHeaderNames[mSelectedHeaderIndex].data(), 16);
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
                if(ImGui::BeginCombo(std::format("##areaWinGraSelect{}", mSelectedHeaderIndex).data(), nullptr, ImGuiComboFlags_CustomPreview)){
                    for(uint32_t j = 0; j < AreaTypeImages.size(); j++){
                        bool is_selected = (mHeaders[mSelectedHeaderIndex].mTextBoxType-1 == j);
                        float cursorX = ImGui::GetCursorPosX();
                        if (ImGui::Selectable(std::format("##areaWin{}#{}",j,mSelectedHeaderIndex).c_str(), is_selected, 0, {136, 40})){
                            mHeaders[mSelectedHeaderIndex].mTextBoxType = j+1;
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
                    if(mHeaders[mSelectedHeaderIndex].mTextBoxType >= AreaTypeImages.size()){
                        ImGui::Image(AreaTypeImages[0], {136, 40});
                    } else {
                        ImGui::Image(AreaTypeImages[mHeaders[mSelectedHeaderIndex].mTextBoxType-1], {136, 40});
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
                if(ImGui::BeginCombo("##encounterSelectCombo", mHeaders[mSelectedHeaderIndex].mEncDataID != 0xFFFF ? std::format("Encounter Set {}", mHeaders[mSelectedHeaderIndex].mEncDataID).data() : "(NONE)")){
                    if(ImGui::Selectable(ICON_LC_CIRCLE_PLUS " New", false)){
                        mHeaders[mSelectedHeaderIndex].mEncDataID = EncounterPanel::NewEncounterData();
                    }
                    if(ImGui::Selectable("(NONE)", mHeaders[mSelectedHeaderIndex].mEncDataID == 0xFFFF)){
                        mHeaders[mSelectedHeaderIndex].mEncDataID = 0xFFFF;
                        EncounterPanel::SetEncounterData(0xFFFF);
                    }
                    for(uint16_t set = 0; set < EncounterSetCount; set++){
                        if(ImGui::Selectable(std::format("Encounter Set {}", set).data(), set == mHeaders[mSelectedHeaderIndex].mEncDataID)){
                        mHeaders[mSelectedHeaderIndex].mEncDataID = set;
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
                if(ImGui::BeginCombo("##weatherSelectCombo", PlatWeatherLabels[mHeaders[mSelectedHeaderIndex].mWeatherID].data())){
                    for(auto [id, label] : PlatWeatherLabels){
                        if(ImGui::Selectable(label.data(), id == mHeaders[mSelectedHeaderIndex].mWeatherID)){
                        mHeaders[mSelectedHeaderIndex].mWeatherID = id;
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
                bool enabled =  mHeaders[mSelectedHeaderIndex].mBicycleFlag == 1;
                if(ImGui::Checkbox( ICON_LC_BIKE " Can Bike", &enabled)){
                    mHeaders[mSelectedHeaderIndex].mBicycleFlag = enabled ? 1 : 0;
                }
                ImGui::SameLine();

                enabled = mHeaders[mSelectedHeaderIndex].mEscapeFlag == 1;
                if(ImGui::Checkbox( ICON_LC_ARROW_BIG_UP_DASH " Escape Rope", &enabled)){
                    mHeaders[mSelectedHeaderIndex].mEscapeFlag = enabled ? 1 : 0;
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY()-0.145f);
                enabled = mHeaders[mSelectedHeaderIndex].mDashFlag == 1;
                if(ImGui::Checkbox( ICON_LC_FOOTPRINTS " Can Run ", &enabled)){
                    mHeaders[mSelectedHeaderIndex].mDashFlag = enabled ? 1 : 0;
                }
                ImGui::SameLine();
                
                enabled = mHeaders[mSelectedHeaderIndex].mFlyFlag == 1;
                if(ImGui::Checkbox( ICON_LC_BIRD " Fly Enabled", &enabled)){
                    mHeaders[mSelectedHeaderIndex].mFlyFlag = enabled ? 1 : 0;
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
                ImGui::SetNextItemWidth(300);
                if(ImGui::BeginCombo(ICON_LC_SUN " Day##bgmDayCombo", DPPlatBGM[mHeaders[mSelectedHeaderIndex].mBgmDayID].data())){
                    for(auto [id, label] : DPPlatBGM){
                        if(ImGui::Selectable(label.data(), id == mHeaders[mSelectedHeaderIndex].mBgmDayID)){
                        mHeaders[mSelectedHeaderIndex].mBgmDayID = id;
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(300);
                if(ImGui::BeginCombo(ICON_LC_MOON_STAR " Night##bgmNightCombo", DPPlatBGM[mHeaders[mSelectedHeaderIndex].mBgmNightID].data())){
                    for(auto [id, label] : DPPlatBGM){
                        if(ImGui::Selectable(label.data(), id == mHeaders[mSelectedHeaderIndex].mBgmNightID)){
                        mHeaders[mSelectedHeaderIndex].mBgmNightID = id;
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