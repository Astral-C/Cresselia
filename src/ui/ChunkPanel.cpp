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
#include "GameResources.hpp"
#include "imgui_internal.h"
#include <glad/glad.h>

namespace ChunkPanel {

// Rendering surface
uint32_t mFbo, mRbo, mViewTex, mPickTex;

float mPrevWinWidth { -1.0f };
float mPrevWinHeight { -1.0f };

bool InitChunkPanel(std::unique_ptr<Palkia::Nitro::Rom>& rom, const GameConfig& config){
    glGenFramebuffers(1, &mFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

	glGenRenderbuffers(1, &mRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

	glGenTextures(1, &mViewTex);
	glGenTextures(1, &mPickTex);

	glBindTexture(GL_TEXTURE_2D, mViewTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, mPickTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1280, 720, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return true;
}

void DrawPanel(USceneCamera& camera){
    ImVec2 winSize = ImGui::GetContentRegionAvail();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    
    if(winSize.x != mPrevWinWidth || winSize.y != mPrevWinHeight){
		glDeleteTextures(1, &mViewTex);
		glDeleteTextures(1, &mPickTex);
		glDeleteRenderbuffers(1, &mRbo);

		glGenRenderbuffers(1, &mRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (uint32_t)winSize.x, (uint32_t)winSize.y);

		glGenTextures(1, &mViewTex);
		glGenTextures(1, &mPickTex);

		glBindTexture(GL_TEXTURE_2D, mViewTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, mPickTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

		GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
    
    }
    
    glViewport(0, 0, (uint32_t)winSize.x, (uint32_t)winSize.y);
    
    
    glClearColor(0.19f, 0.19f, 0.19f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    int32_t unused = 0;
    glClearTexImage(mPickTex, 0, GL_RED_INTEGER, GL_INT, &unused);
    
    mPrevWinWidth = winSize.x;
    mPrevWinHeight = winSize.y;
    
    glm::mat4 projection, view;
    projection = camera.GetProjectionMatrix();
    view = camera.GetViewMatrix();
    
    // Render Models
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    
    cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 frameSize = {ImGui::GetStyle().WindowBorderSize, ImGui::GetStyle().WindowBorderSize};
    ImGui::GetWindowDrawList()->AddRectFilled(cursorPos - frameSize, cursorPos + winSize + frameSize, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Border]));
    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(cursorPos, cursorPos + winSize, 0xFF303030, 0xFF303030, 0xFF202020, 0xFF202020);
    ImGui::Image(static_cast<uintptr_t>(mViewTex), { winSize.x, winSize.y }, {0.0f, 1.0f}, {1.0f, 0.0f});
    
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
    }
    
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, winSize.x, winSize.y);
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Cleanup(){
    glDeleteFramebuffers(1, &mFbo);
	glDeleteRenderbuffers(1, &mRbo);
	glDeleteTextures(1, &mViewTex);
	glDeleteTextures(1, &mPickTex);
}

}