#include <memory>
#include <imgui.h>
#include "GameConfig.hpp"
#include "Map/Encounters.hpp"
#include "NDS/System/Archive.hpp"
#include "NDS/System/FileSystem.hpp"
#include "NDS/System/Rom.hpp"
#include "bstream/bstream.h"

#include "ui/EncounterPanel.hpp"
#include "GameResources.hpp"
#include "imgui_internal.h"

namespace EncounterPanel {

std::shared_ptr<Palkia::Nitro::File> EncounterDataFile { nullptr };
std::shared_ptr<Palkia::Nitro::File> EncounterArchiveFile { nullptr };
std::shared_ptr<Palkia::Nitro::Archive> EncounterDataArchive { nullptr };
Encounter CurrentEncounterData;

bool InitEncounterPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    EncounterArchiveFile = rom->GetFile(config.mEncounterDataPath);

    if(EncounterArchiveFile == nullptr){
        return false;
    }
    
    bStream::CMemoryStream stream = bStream::CMemoryStream(EncounterArchiveFile->GetData(), EncounterArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    EncounterDataArchive = std::make_shared<Palkia::Nitro::Archive>(stream);
    return true;
}

void SaveEncounterPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    if(EncounterArchiveFile == nullptr) return;
    SaveEncounterFile(EncounterDataFile, CurrentEncounterData);
    
    bStream::CMemoryStream stream = bStream::CMemoryStream(EncounterArchiveFile->GetData(), EncounterArchiveFile->GetSize(), bStream::Endianess::Little, bStream::OpenMode::In);
    EncounterDataArchive->SaveArchive(stream);

    rom->GetFile(config.mEncounterDataPath)->SetData(stream.getBuffer(), stream.getSize());
}

void SetEncounterData(uint16_t encounterID){
    if(encounterID == 0xFFFF){
        EncounterDataFile = nullptr;
        CurrentEncounterData = {0};
    } else {
        EncounterDataFile = EncounterDataArchive->GetFileByIndex(encounterID);
        CurrentEncounterData = LoadEncounterFile(EncounterDataFile, CurrentGameConfig());
    }
}

uint16_t NewEncounterData(){
    if(EncounterSetCount != 0xFFFF){
        uint16_t newEnc = EncounterSetCount++;
        CurrentEncounterData = {0};

        uint8_t data[0x298];
        EncounterDataFile = std::make_shared<Palkia::Nitro::File>();
        EncounterDataFile->SetData(data, 0x298);

        SaveEncounterFile(EncounterDataFile, CurrentEncounterData);

        EncounterDataArchive->AddFile(EncounterDataFile);
        return newEnc;
    } else {
        return 0xFFFF;
    }
}

void DrawPanel(){
    const GameConfig config = Configs[CurrentGameConfig()];
    ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {5, 5});

	// its sucgh a mess :<
	auto windowWidth = ImGui::GetWindowSize().x;
	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize("Save Encounters").x + (style.FramePadding.x * 2) + 2));
	if(ImGui::Button("Save Encounters")){
	    
	}
	if(config.mGameCode == "EGPI"){
		ImGui::BeginTable("##defaultEncounterRate", 7, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Morning");
		ImGui::TableSetupColumn("Day");
		ImGui::TableSetupColumn("Night");
		ImGui::TableSetupColumn("Surfing");
		ImGui::TableSetupColumn("Old Rod");
		ImGui::TableSetupColumn("Good Rod");
		ImGui::TableSetupColumn("Super Rod");
		ImGui::TableHeadersRow();

		std::array<int, 12> slotTitles = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};
		std::array<int, 5> fiveSlotTitles = {20, 10, 5, 4, 1};

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##walkingEncounterRate", (int*)&CurrentEncounterData.mWalkingEncounterRate);
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##surfingEncounterRate", (int*)&CurrentEncounterData.mSurfEncounterRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##oldRodEncounterRate", (int*)&CurrentEncounterData.mOldRodRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##goodRodEncounterRate", (int*)&CurrentEncounterData.mGoodRodRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##superRodEncounterRate", (int*)&CurrentEncounterData.mSuperRodRate);

		for(int x = 0; x < 12; x++){
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(std::format("{}%%", slotTitles[x]).data());
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Morning", x).data(), PokemonNames[CurrentEncounterData.mMorningPokemon[x]].data())){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mMorningPokemon[x] == i);
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
							CurrentEncounterData.mMorningPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::TableNextColumn();
			ImGui::Text(std::format("{}%%", slotTitles[x]).data());
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Day", x).data(), PokemonNames[CurrentEncounterData.mDayPokemon[x]].data())){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mDayPokemon[x] == i);
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
							CurrentEncounterData.mDayPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Level");
			ImGui::SameLine();
			ImGui::InputInt(std::format("##pokeSlot{}WalkLevel", x).data(), (int*)&CurrentEncounterData.mWalkingLevel[x]);
			ImGui::NewLine();

			ImGui::TableNextColumn();
			ImGui::Text(std::format("{}%%", slotTitles[x]).data());
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Night", x).data(), PokemonNames[CurrentEncounterData.mNightPokemon[x]].data())){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mNightPokemon[x] == i);
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
							CurrentEncounterData.mNightPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if(x < 5){
				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}Surf", x).data(), PokemonNames[CurrentEncounterData.mSurf[x]].data())){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mSurf[x] == i);
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
								CurrentEncounterData.mSurf[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SurfMaxLevel", x).data(), (int*)&CurrentEncounterData.mSurfMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SurfMinLevel", x).data(), (int*)&CurrentEncounterData.mSurfMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}OldRod", x).data(), PokemonNames[CurrentEncounterData.mOldRod[x]].data())){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mOldRod[x] == i);
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
								CurrentEncounterData.mOldRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}OldRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mOldMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}OldRodMinLevel", x).data(), (int*)&CurrentEncounterData.mOldMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}GoodRod", x).data(), PokemonNames[CurrentEncounterData.mGoodRod[x]].data())){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mGoodRod[x] == i);
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
								CurrentEncounterData.mGoodRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}GoodRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mGoodRodMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}GoodRodMinLevel", x).data(), (int*)&CurrentEncounterData.mGoodRodMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}SuperRod", x).data(), PokemonNames[CurrentEncounterData.mSuperRod[x]].data())){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mSuperRod[x] == i);
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
								CurrentEncounterData.mSuperRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SuperRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mSuperRodMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SuperRodMinLevel", x).data(), (int*)&CurrentEncounterData.mSuperRodMinLevels[x]);

			} else {
				for(int i = 0; i < 4; i++) ImGui::TableNextColumn();
			}
			//PokemonNames[CurrentEncounterData.mWalkingEncounters[x]].data()
		}

		ImGui::EndTable();

		auto textWidth = ImGui::CalcTextSize("Special Encounters").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Special Encounters");

		ImGui::BeginTable("##specialCurrentEncounterData", 2, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Swarm");
		ImGui::TableSetupColumn("Rock Smash");
		ImGui::TableHeadersRow();
		for(int x = 0; x < 5; x++){
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if(x < 4){
				ImGui::Text("20%%");
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}Swarm", x).data(), PokemonNames[CurrentEncounterData.mSwarmPokemon[x]].data())){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mSwarmPokemon[x] == i);
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
								CurrentEncounterData.mSwarmPokemon[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			ImGui::TableNextColumn();

			ImGui::Text("Min Level");
			ImGui::SameLine();
			ImGui::InputInt(std::format("##pokeSlot{}RockSmashMinLevel", x).data(), (int*)&CurrentEncounterData.mRockSmashMinLevels[x]);

			ImGui::Text("Max Level");
			ImGui::SameLine();
			ImGui::InputInt(std::format("##pokeSlot{}RockSmashMaxLevel", x).data(), (int*)&CurrentEncounterData.mRockSmashMaxLevels[x]);

			if(ImGui::BeginCombo(std::format("##pokeSlot{}RockSmash", x).data(), PokemonNames[CurrentEncounterData.mRockSmashPokemon[x]].data())){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mRockSmashPokemon[x] == i);
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected)){
							CurrentEncounterData.mRockSmashPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		ImGui::EndTable();
	} else if(config.mGameCode == "EUPC") {
		ImGui::BeginTable("##CurrentEncounterData", 5, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Walking");
		ImGui::TableSetupColumn("Surfing");
		ImGui::TableSetupColumn("Old Rod");
		ImGui::TableSetupColumn("Good Rod");
		ImGui::TableSetupColumn("Super Rod");
		ImGui::TableHeadersRow();

		std::array<int, 12> slotTitles = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};
		std::array<int, 5> fiveSlotTitles = {20, 10, 5, 4, 1};

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##walkingEncounterRate", (int*)&CurrentEncounterData.mWalkingEncounterRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##surfingEncounterRate", (int*)&CurrentEncounterData.mSurfEncounterRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##oldRodEncounterRate", (int*)&CurrentEncounterData.mOldRodRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##goodRodEncounterRate", (int*)&CurrentEncounterData.mGoodRodRate);
		ImGui::TableNextColumn();

		ImGui::Text("Rate");
		ImGui::SameLine();
		ImGui::InputInt("##superRodEncounterRate", (int*)&CurrentEncounterData.mSuperRodRate);


		for(int x = 0; x < 12; x++){
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(std::format("{}%%", slotTitles[x]).data());
			ImGui::SameLine();

			if(ImGui::BeginCombo(std::format("##pokeSlot{}Walk", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mWalking[x] == i);
						if(i < PokemonIcons.size()){
						    ImGui::Image(PokemonIcons[i], {32,32});
						} else {
						    ImGui::Image(PokemonIcons[0], {32,32});
						}
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mWalking[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mWalking[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mWalking[x]].data());
                ImGui::EndComboPreview();
			}


			ImGui::Text("Level");
			ImGui::SameLine();
			ImGui::InputInt(std::format("##pokeSlot{}WalkLevel", x).data(), (int*)&CurrentEncounterData.mWalkingLevel[x]);
			ImGui::NewLine();

			if(x < 5){
				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}Surf", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mSurf[x] == i);
							ImGui::Image(PokemonIcons[i], {32,32});
							ImGui::SameLine();
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
								CurrentEncounterData.mSurf[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if(ImGui::BeginComboPreview()){
				    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                    ImGui::Image(PokemonIcons[CurrentEncounterData.mSurf[x]], {32, 32});
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                    ImGui::Text(PokemonNames[CurrentEncounterData.mSurf[x]].data());
                    ImGui::EndComboPreview();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SurfMaxLevel", x).data(), (int*)&CurrentEncounterData.mSurfMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SurfMinLevel", x).data(), (int*)&CurrentEncounterData.mSurfMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}OldRod", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mOldRod[x] == i);
							ImGui::Image(PokemonIcons[i], {32,32});
							ImGui::SameLine();
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
								CurrentEncounterData.mOldRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if(ImGui::BeginComboPreview()){
				    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                    ImGui::Image(PokemonIcons[CurrentEncounterData.mOldRod[x]], {32, 32});
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                    ImGui::Text(PokemonNames[CurrentEncounterData.mOldRod[x]].data());
                    ImGui::EndComboPreview();
				}


				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}OldRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mOldMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}OldRodMinLevel", x).data(), (int*)&CurrentEncounterData.mOldMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}GoodRod", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mGoodRod[x] == i);
							ImGui::Image(PokemonIcons[i], {32,32});
							ImGui::SameLine();
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
								CurrentEncounterData.mGoodRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if(ImGui::BeginComboPreview()){
				    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                    ImGui::Image(PokemonIcons[CurrentEncounterData.mGoodRod[x]], {32, 32});
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                    ImGui::Text(PokemonNames[CurrentEncounterData.mGoodRod[x]].data());
                    ImGui::EndComboPreview();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}GoodRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mGoodRodMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}GoodRodMinLevel", x).data(), (int*)&CurrentEncounterData.mGoodRodMinLevels[x]);

				ImGui::TableNextColumn();
				ImGui::Text(std::format("{}%%", fiveSlotTitles[x]).data());
				ImGui::SameLine();
				if(ImGui::BeginCombo(std::format("##pokeSlot{}SuperRod", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
					for(uint32_t i = 0; i < PokemonNames.size(); i++){
							bool is_selected = (CurrentEncounterData.mSuperRod[x] == i);
							ImGui::Image(PokemonIcons[i], {32,32});
							ImGui::SameLine();
							if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
								CurrentEncounterData.mSuperRod[x] = i;
							}
							if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if(ImGui::BeginComboPreview()){
				    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                    ImGui::Image(PokemonIcons[CurrentEncounterData.mSuperRod[x]], {32, 32});
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                    ImGui::Text(PokemonNames[CurrentEncounterData.mSuperRod[x]].data());
                    ImGui::EndComboPreview();
				}

				ImGui::Text("Max Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SuperRodMaxLevel", x).data(), (int*)&CurrentEncounterData.mSuperRodMaxLevels[x]);

				ImGui::Text("Min Level");
				ImGui::SameLine();
				ImGui::InputInt(std::format("##pokeSlot{}SuperRodMinLevel", x).data(), (int*)&CurrentEncounterData.mSuperRodMinLevels[x]);

			} else {
				for(int i = 0; i < 4; i++) ImGui::TableNextColumn();
			}
			//PokemonNames[CurrentEncounterData.mWalkingEncounters[x]].data()
		}

		ImGui::EndTable();

		ImGui::BeginTable("##timeCurrentEncounterData", 3, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Morning");
		ImGui::TableSetupColumn("Night");
		ImGui::TableSetupColumn("Swarm");
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("10%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Morning", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mMorningPokemon[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mMorningPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mMorningPokemon[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mMorningPokemon[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("10%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Night", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mNightPokemon[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mNightPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mNightPokemon[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mNightPokemon[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("20%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Swarm", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mSwarmPokemon[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mSwarmPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mSwarmPokemon[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mSwarmPokemon[x]].data());
                ImGui::EndComboPreview();
			}
		}
		ImGui::EndTable();

		uint32_t textWidth = ImGui::CalcTextSize("Poke Radar").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Poke Radar");

//			ImGui::TableSetupColumn("Radar");
		ImGui::BeginTable("##radarEncounters", 4, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("10%");
		ImGui::TableSetupColumn("10%");
		ImGui::TableSetupColumn("1%");
		ImGui::TableSetupColumn("1%");
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();

		for(int x = 0; x < 4; x++){
			ImGui::TableNextColumn();
			if(ImGui::BeginCombo(std::format("##pokeSlotRadar{}", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mRadarPokemon[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mRadarPokemon[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mRadarPokemon[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mRadarPokemon[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::EndTable();

		textWidth = ImGui::CalcTextSize("Dual Cart Encounters").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Dual Cart Encounters");


		ImGui::BeginTable("##CurrentEncounterDataDualCart", 5, ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Ruby");
		ImGui::TableSetupColumn("Sapphire");
		ImGui::TableSetupColumn("Emerald");
		ImGui::TableSetupColumn("Fire Red");
		ImGui::TableSetupColumn("Leaf Green");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("4%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Ruby", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mRuby[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mRuby[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mRuby[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mRuby[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("4%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Sapphire", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mSapphire[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mSapphire[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mSapphire[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mSapphire[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("4%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}Emerald", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mEmerald[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mEmerald[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mEmerald[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mEmerald[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("4%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}FireRed", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mFireRed[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mFireRed[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mFireRed[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mFireRed[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::TableNextColumn();
		for(int x = 0; x < 2; x++){
			ImGui::Text("4%%");
			ImGui::SameLine();
			if(ImGui::BeginCombo(std::format("##pokeSlot{}LeafGreen", x).data(), nullptr, ImGuiComboFlags_CustomPreview)){
				for(uint32_t i = 0; i < PokemonNames.size(); i++){
						bool is_selected = (CurrentEncounterData.mLeafGreen[x] == i);
						ImGui::Image(PokemonIcons[i], {32,32});
						ImGui::SameLine();
						if (ImGui::Selectable(PokemonNames[i].data(), is_selected, ImGuiSelectableFlags_SpanAvailWidth, {-1, 32})){
							CurrentEncounterData.mLeafGreen[x] = i;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::BeginComboPreview()){
			    ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);
                ImGui::Image(PokemonIcons[CurrentEncounterData.mLeafGreen[x]], {32, 32});
                ImGui::SameLine();
                ImGui::SetCursorPosY(   ImGui::GetCursorPosY()+10);
                ImGui::Text(PokemonNames[CurrentEncounterData.mLeafGreen[x]].data());
                ImGui::EndComboPreview();
			}
		}

		ImGui::EndTable();
	}
	ImGui::PopStyleVar();    
}

}