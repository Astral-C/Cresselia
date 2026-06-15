#include "UCresseliaApplication.hpp"
#include "UCresseliaContext.hpp"
#include "UInput.hpp"
#include "UCamera.hpp"
#include "../lib/glfw/deps/glad/gl.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

void DealWithGLErrors(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	//std::cout << "GL CALLBACK: " << message << std::endl;
}

static UCresseliaContext* ResizeContext = nullptr;

void HandleFramebufferResize(GLFWwindow* window, int w, int h){
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	if(ResizeContext != nullptr){
		ResizeContext->GetCamera()->SetWindowSize(width, height);
	}
}

UCresseliaApplication::UCresseliaApplication() {
	mWindow = nullptr;
	mContext = nullptr;
}

bool UCresseliaApplication::Setup() {
	// Initialize GLFW
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glfwWindowHintString(GLFW_X11_CLASS_NAME, "cresselia");
	glfwWindowHintString(GLFW_WAYLAND_APP_ID, "cresselia");
	
	mWindow = glfwCreateWindow(1100, 700, "Cresselia", nullptr, nullptr);
	if (mWindow == nullptr) {
		glfwTerminate();
		return false;
	}

	UInput::SetWindow(mWindow);

	glfwSetKeyCallback(mWindow, UInput::GLFWKeyCallback);
	glfwSetCursorPosCallback(mWindow, UInput::GLFWMousePositionCallback);
	glfwSetMouseButtonCallback(mWindow, UInput::GLFWMouseButtonCallback);
	glfwSetScrollCallback(mWindow, UInput::GLFWMouseScrollCallback);
	glfwSetFramebufferSizeCallback(mWindow, HandleFramebufferResize);

	glfwMakeContextCurrent(mWindow);
	gladLoadGL(glfwGetProcAddress);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glfwSwapInterval(1);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DealWithGLErrors, nullptr);

	// Initialize imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_None;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImColor(0x20, 0x20, 0x20, 0xFF);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(0x20, 0x20, 0x20, 0xFF);
	style.Colors[ImGuiCol_WindowBg] = ImColor(0x20, 0x20, 0x20, 0xFF);

	style.Colors[ImGuiCol_MenuBarBg] = ImColor(0x28, 0x28, 0x28, 0xFF);

	style.Colors[ImGuiCol_Button] = ImColor(0x83, 0x52, 0x99, 0xFF);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(0xA2, 0x7F, 0xC2, 0xFF);

	style.Colors[ImGuiCol_Tab] = ImColor(0x83, 0x52, 0x99, 0xFF);
	style.Colors[ImGuiCol_TabActive] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_TabHovered] = ImColor(0xA2, 0x7F, 0xC2, 0xFF);

	style.Colors[ImGuiCol_TabUnfocused] = ImColor(0x83, 0x52, 0x99, 0xFF);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImColor(0x83, 0x52, 0x99, 0xFF);

	style.Colors[ImGuiCol_TabDimmed] = ImColor(0x83, 0x52, 0x99, 0xFF);
	style.Colors[ImGuiCol_TabDimmedSelected] = ImColor(0x92, 0x6F, 0xB2, 0xFF);


	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(0xA2, 0x7F, 0xC2, 0xFF);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(0x83, 0x52, 0x99, 0xFF);

	style.Colors[ImGuiCol_SliderGrab] = ImColor(0x83, 0x52, 0x99, 0xFF);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(0x92, 0x6F, 0xB2, 0xFF);

	style.Colors[ImGuiCol_ChildBg] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_PopupBg] = ImColor(0x28, 0x28, 0x28, 0xFF);

	style.FrameBorderSize = 1.5f;
	style.Colors[ImGuiCol_FrameBg] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0x32, 0x32, 0x32, 0xFF);;
	style.Colors[ImGuiCol_CheckboxSelectedBg] = ImColor(0x30, 0x30, 0x30, 0xFF);
	style.Colors[ImGuiCol_CheckMark] = ImColor(0x92, 0x6F, 0xB2, 0xFF);


	style.Colors[ImGuiCol_HeaderHovered] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_Header] = ImColor(0x92, 0x6F, 0xB2, 0xFF);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImColor(0x18, 0x18, 0x18, 0xA0);

	style.FrameRounding = 2.f;
	style.FramePadding = ImVec2(5.0f,3.0f);

	
	glEnable(GL_MULTISAMPLE);	

	// Create viewer context
	mContext = new UCresseliaContext();
	ResizeContext = mContext;

	return true;
}

bool UCresseliaApplication::Teardown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	glfwDestroyWindow(mWindow);
	glfwTerminate();

	delete mContext;

	return true;
}

bool UCresseliaApplication::Execute(float deltaTime) {
	// Try to make sure we return an error if anything's fucky
	if (mContext == nullptr || mWindow == nullptr || glfwWindowShouldClose(mWindow))
		return false;

	// Update viewer context
	mContext->Update(deltaTime);
	
	// Begin actual rendering
	glfwMakeContextCurrent(mWindow);
	glfwPollEvents();

	UInput::UpdateInputState();

	// The context renders both the ImGui elements and the background elements.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Update buffer size
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);

	glDepthMask(true);
	// Clear buffers
	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render viewer context
	mContext->Render(deltaTime);
	
	// Render imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glfwSwapBuffers(mWindow);

	return true;
}
