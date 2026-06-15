#include "UCresseliaContext.hpp"

#include <format>
#include <vector>
#include <iostream>
#include <algorithm>

#include <glad/glad.h>
#include <glm/glm/ext/matrix_transform.hpp>
#include "UPointSpriteManager.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include "../lib/ImGuiFileDialog/ImGuiFileDialog.h"
#include "ImGuizmo.h"

#include <bstream/bstream.h>

#include "Text.hpp"
#include "GameResources.hpp"

#include "NDS/Assets/NSBMD.hpp"
#include "NDS/Assets/NCLR.hpp"
#include "NDS/Assets/NCGR.hpp"
#include "NDS/System/Compression.hpp"

#include "GameConfig.hpp"
#include "Util.hpp"

#include "ui/ChunkPanel.hpp"
#include "ui/EncounterPanel.hpp"
#include "ui/MatrixPanel.hpp"
#include "ui/HeaderPanel.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// fonts
#include "IconsLucide.h"
#include "lucide.h"
#include "noto_sans_jp_regular.h"
#include "cresselia_png.h"

namespace {
    std::vector<CPointSprite> mBillboards {};
    std::map<uint32_t, uint32_t> mOverworldSpriteIDs {};

    uint32_t mCresseliaImg;

}

UCresseliaContext::~UCresseliaContext(){
    ClearGameResources();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	ChunkPanel::Cleanup();
}

UCresseliaContext::UCresseliaContext(){
	srand(time(0));

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImFontConfig noto_config;
	noto_config.FontDataOwnedByAtlas = false;
	io.Fonts->AddFontFromMemoryTTF((void*)res_font_NotoSansJP_Regular_otf, res_font_NotoSansJP_Regular_otf_size, 16.0f, &noto_config, io.Fonts->GetGlyphRangesJapanese());

	static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
	ImFontConfig icons_config;
	icons_config.FontDataOwnedByAtlas = false;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphOffset.y = 2.25f;
	icons_config.GlyphMinAdvanceX = 14.0f;
	ImFont* fnt = io.Fonts->AddFontFromMemoryTTF((void*)res_font_lucide_ttf, res_font_lucide_ttf_size, icons_config.GlyphMinAdvanceX, &icons_config, icons_ranges);

	mGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

	int w { 0 }, h { 0 }, channels { 0 };
	uint8_t* data = stbi_load_from_memory((const uint8_t*)cresselia_png, cresselia_png_len, &w, &h, &channels, 4);
	
	glGenTextures(1, &mCresseliaImg);
	glBindTexture(GL_TEXTURE_2D, mCresseliaImg);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
	
	mAreaRenderer.Init();
}

bool UCresseliaContext::Update(float deltaTime) {
	if(mCurrentTool == "Chunk Editor"){
		mCamera.Update(deltaTime);

		if(ImGui::IsKeyPressed(ImGuiKey_1)){
			mGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		}

		if(ImGui::IsKeyPressed(ImGuiKey_2)){
			mGizmoOperation = ImGuizmo::OPERATION::ROTATE;
		}

		if(ImGui::IsKeyPressed(ImGuiKey_3)){
			mGizmoOperation = ImGuizmo::OPERATION::SCALE;
		}

		if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
			// deselect
		}

		if(ImGui::IsKeyPressed(ImGuiKey_O)){
			mCamera.ToggleOrtho();
		}
	}

	return true;
}

void UCresseliaContext::Render(float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();

	RenderMenuBar();

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(0, mainViewport, dockFlags);
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	if(!bIsDockingSetUp){

		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);

		ImGui::DockBuilderDockWindow("viewportWindow", mMainDockSpaceID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bIsDockingSetUp = true;
	}


	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,3.0f));
	ImGui::Begin("viewportWindow", nullptr, ImGuiWindowFlags_NoResize);
		//ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, {0.0f, 0.0f});
		ImGui::BeginTabBar("mapToolbar", ImGuiTabBarFlags_Reorderable);
		for(auto tool : mEditorTools){
		        bool toolIsQueued = mQueuedTool != "" && mQueuedTool == tool;
				if(toolIsQueued){
				    mCurrentTool = mQueuedTool;
					mQueuedTool = "";
				}
				if(ImGui::BeginTabItem(tool.data(), nullptr, toolIsQueued ? ImGuiTabItemFlags_SetSelected : 0)){
                    if(tool != mCurrentTool) mCurrentTool = tool;
              		if(mRom != nullptr){
              		    if(mCurrentTool == "Chunk Editor"){
                            ChunkPanel::DrawPanel(mCamera);
                  		} else if(mCurrentTool == "Trainer Editor"){
                    
                  		} else if(mCurrentTool == "Area Editor"){
                    
                  		} else if(mCurrentTool == "Encounter Editor"){
                  		    EncounterPanel::DrawPanel();
                  		} else if(mCurrentTool == "Map Header Table"){
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY()-4.0f);
                            MapHeaderPanel::DrawPanel(mQueuedTool);
             			} 
              		}
					ImGui::EndTabItem();
				}
			}
		ImGui::EndTabBar();


	ImGui::End();
	ImGui::PopStyleVar();
}

void UCresseliaContext::RenderMainWindow(float deltaTime) {


}

void UCresseliaContext::RenderMenuBar() {
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
	ImGui::BeginMainMenuBar();
	mOptionsOpen = false;

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem(ICON_LC_FOLDER_OPEN " Open...")) {
			bIsFileDialogOpen = true;
		}
		if (ImGui::MenuItem(ICON_LC_SAVE " Save...")) {
			bIsSaveDialogOpen = true;
		}

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		if(ImGui::MenuItem(ICON_LC_COG " Settings")){
			mOptionsOpen = true;
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(ICON_LC_CIRCLE_QUESTION_MARK)) {
		if(ImGui::MenuItem("About")){
			mAboutOpen = true;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (bIsFileDialogOpen) {
		IGFD::FileDialogConfig config;
		ImGuiFileDialog::Instance()->OpenDialog("OpenRomDialog", "Choose Pokemon ROM", ".nds", config);
	}

	ImGui::SetNextWindowSize(viewportSize * 0.75f);
	ImGui::SetNextWindowPos((viewportSize * 0.5f) - ((viewportSize * 0.75f) * 0.5f));
	if (ImGuiFileDialog::Instance()->Display("OpenRomDialog", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();
			std::cout << FilePath << std::endl;

			// load rom here
			mRom = std::make_unique<Palkia::Nitro::Rom>(std::filesystem::path(FilePath));

			CurrentGameConfig(mRom->GetHeader().gameCode);
			
			LoadGameResources(mRom);

			EncounterPanel::InitEncounterPanel(mRom, Configs[CurrentGameConfig()]);
			MapHeaderPanel::InitHeaderPanel(mRom, Configs[CurrentGameConfig()]);
			ChunkPanel::InitChunkPanel(mRom, Configs[CurrentGameConfig()]);
			MatrixPanel::InitMatrixPanel(mRom, Configs[CurrentGameConfig()]);
			
			bIsFileDialogOpen = false;
		} else {
			bIsFileDialogOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (bIsSaveDialogOpen) {
		IGFD::FileDialogConfig config;
		ImGuiFileDialog::Instance()->OpenDialog("SaveRomDialog", "Save Pokemon ROM", ".nds", config);
	}

	ImGui::SetNextWindowSize(viewportSize * 0.75f);
	ImGui::SetNextWindowPos((viewportSize * 0.5f) - ((viewportSize * 0.75f) * 0.5f));
	if (ImGuiFileDialog::Instance()->Display("SaveRomDialog", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();
			std::cout << FilePath << std::endl;

			try {
				mRom->Save(FilePath);
			}
			catch (std::runtime_error e) {
				std::cout << "Failed to save rom " << FilePath << "! Exception: " << e.what() << "\n";
			}
			catch (std::exception e) {
				std::cout << "Failed to save rom " << FilePath << "! Exception: " << e.what() << "\n";
			}

			bIsSaveDialogOpen = false;
		} else {
			bIsSaveDialogOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::SetNextWindowSize(viewportSize * 0.75f);
	ImGui::SetNextWindowPos((viewportSize * 0.5f) - ((viewportSize * 0.75f) * 0.5f));
	if (ImGui::BeginPopupModal("ROM Load Error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)){
		ImGui::Text("Error Loading Rom!\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120,0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if(mAboutOpen){
		ImGui::OpenPopup("About Window");
	}

	ImGui::SetNextWindowSize(viewportSize * 0.3f);
	ImGui::SetNextWindowPos((viewportSize * 0.5f) - ((viewportSize * 0.3f) * 0.5f));
	if (ImGui::BeginPopupModal("About Window", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)){
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize("Cresselia").x;
		ImGuiStyle* style = &ImGui::GetStyle();

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Cresselia");

		ImGui::Separator();

		ImGui::SetCursorPosX((windowWidth - 80) * 0.5f);
		ImGui::Image(static_cast<uintptr_t>(mCresseliaImg), { 80, 80 }, {0.0f, 0.0f}, {1.0f, 1.0f});
		
		textWidth = ImGui::CalcTextSize("github.com/Astral-C/Cresselia").x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("github.com/Astral-C/Cresselia");

		textWidth = ImGui::CalcTextSize("Made by veebs").x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Made by veebs");

		ImGui::Separator();

		
		float size = 120 + style->FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * 0.5;
		if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		if (ImGui::Button("Close", ImVec2(120, 0))) {
			mAboutOpen = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if(mOptionsOpen){
		ImGui::OpenPopup("Options");
	}

}
