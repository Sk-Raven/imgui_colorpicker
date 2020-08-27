// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include <stdio.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

bool ImRectangleContainsPoint(const  ImVec2 &tl, const  ImVec2 &br, const  ImVec2 &p);
static void RenderRectForVerticalBar(ImDrawList* draw_list, ImVec2 pos, float bars_width, float alpha);
static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha);
static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

unsigned int loadTexture(char const * path);

int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImFont* font = io.Fonts->AddFontFromFileTTF("../res/fonts/Roboto-Medium.ttf", 16.0f);
	unsigned int pickerTexture = loadTexture("../res/ColorPick.png");
	unsigned int transparentTexture = loadTexture("../res//transparent.png");

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		//Init
		static float col[4] = { 0.0,0.0,0.0,0.0 };
		static int inputR = 0;
		static int inputG = 0;
		static int inputB = 0;
		static float inputH = 0;
		static float inputS = 0;
		static int inputV = 0;
		static int inputA = 0;

		static int combo_item_current = 0;
		static float &R = col[0], &G = col[1], &B = col[2];
		static float H = 0, S = 0, V = 0;
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		//ImGui::ColorPicker4("1", col, ImGuiColorEditFlags_DisplayHSV);

		ImGuiContext& g = *GImGui;
		ImGuiWindow* windows = ImGui::GetCurrentWindow();
		if (windows->SkipItems)
			return false;
		ImDrawList* draw_list = windows->DrawList;
		ImGuiStyle& style = g.Style;
		ImGuiIO& io = g.IO;
		const float width = ImGui::CalcItemWidth();
		g.NextItemData.ClearFlags();
		ImGui::PushID("colorpicker");
		ImGui::BeginGroup();

		//PickerImage
		ImGui::Image((GLuint *)pickerTexture, ImVec2(19, 19), ImVec2(0, 1), ImVec2(1, 0));

		ImVec2 picker_pos = windows->DC.CursorPos;
		float square_sz = ImGui::GetFrameHeight();
		float preview_size = 50;
		float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
		float sv_picker_size = ImMax(bars_width * 1, width - (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
		float preview_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x + 20;
		float bar_pos_x = picker_pos.x + 30;
		const float bar_pos_y = picker_pos.y + sv_picker_size + 20;
		float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

		//wheel && cube
		float wheel_thickness = sv_picker_size * 0.08f;
		float wheel_r_outer = sv_picker_size * 0.50f;
		float wheel_r_inner = wheel_r_outer - wheel_thickness;
		ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width)*0.5f, picker_pos.y + sv_picker_size * 0.5f); //
		ImVec2 cube_pos(wheel_center.x - 0.25 * sv_picker_size, wheel_center.y - 0.25 * sv_picker_size);
		float alpha = ImSaturate(col[3]);


		bool value_changed_h = false, value_changed_sv = false;

		ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
		// Hue wheel + SV rectangle logic
		ImGui::InvisibleButton("hsv", ImVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
		if (ImGui::IsItemActive())
		{
			ImVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
			ImVec2 current_off = g.IO.MousePos - wheel_center;
			float initial_dist2 = ImLengthSqr(initial_off);
			if (initial_dist2 >= (wheel_r_inner - 1)*(wheel_r_inner - 1) && initial_dist2 <= (wheel_r_outer + 1)*(wheel_r_outer + 1))
			{
				// Interactive with Hue wheel
				H = ImAtan2(current_off.y, current_off.x) / IM_PI * 0.5f;
				if (H < 0.0f)
					H += 1.0f;
				value_changed_h = true;
			}
			if (ImRectangleContainsPoint(cube_pos, cube_pos + ImVec2(0.5*sv_picker_size, 0.5*sv_picker_size), g.IO.MouseClickedPos[0]))
			{
				S = ImSaturate((io.MousePos.x - cube_pos.x) / (sv_picker_size * 0.5));
				V = 1.0f - ImSaturate((io.MousePos.y - cube_pos.y) / (sv_picker_size * 0.5));
				value_changed_sv = true;
			}
		}

		if (value_changed_sv || value_changed_h)
		{
			ImGui::ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
		}



		bool value_changed = false;
		//Input
		inputR = (int)(R * 255 + 0.5);
		inputG = (int)(G * 255 + 0.5);
		inputB = (int)(B * 255 + 0.5);
		inputA = (int)(alpha * 255 + 0.5);
		inputH = H;
		inputS = S;
		inputV = (int)(V * 255 + 0.5);
		ImGui::NewLine();
		if (combo_item_current == 0)
			ImGui::Text("  R");
		else if (combo_item_current == 1)
			ImGui::Text("  H");
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size, bar_pos_y));
		if (combo_item_current == 0)
		{
			if (ImGui::InputInt("##inputR", &inputR))
			{
				value_changed = true;
				inputR = ImClamp(inputR, 0, 255);
				col[0] = inputR / 255.0;
			}
		}
		else if (combo_item_current == 1)
		{
			if (ImGui::InputFloat("##inputH", &inputH))
			{
				value_changed = true;
				inputH = ((inputH < 0) ? 0 : (inputH > 1.0) ? 0.9999 : inputH);
				H = inputH;
			}
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x - 30, bar_pos_y + 32));
		if (combo_item_current == 0)
			ImGui::Text("  G");
		else if (combo_item_current == 1)
			ImGui::Text("  S");
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size, bar_pos_y + 32));
		if (combo_item_current == 0)
		{
			if (ImGui::InputInt("##inputG", &inputG))
			{
				value_changed = true;
				inputG = ImClamp(inputG, 0, 255);
				col[1] = inputG / 255.0;
			}
		}
		else if (combo_item_current == 1)
		{
			if (ImGui::InputFloat("##inputS", &inputS))
			{
				value_changed = true;
				inputS = ((inputS < 0) ? 0 : (inputS > 1.0) ? 1.0 : inputS);
				S = inputS;
			}
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x - 30, bar_pos_y + 64));
		if (combo_item_current == 0)
			ImGui::Text("  B");
		else if (combo_item_current == 1)
			ImGui::Text("  V");
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size, bar_pos_y + 64));
		if (combo_item_current == 0)
		{
			if (ImGui::InputInt("##inputB", &inputB))
			{
				value_changed = true;
				inputB = ImClamp(inputB, 0, 255);
				col[2] = inputB / 255.0; 
			}
		}
		else if (combo_item_current == 1)
		{
			if (ImGui::InputInt("##inputV", &inputV))
			{
				value_changed = true;
				inputV = ImClamp(inputV, 0, 255);
				V = inputV / 255.0;
			}
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x - 30, bar_pos_y + 96));
		ImGui::Text("  A");
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size, bar_pos_y + 96));
		if (ImGui::InputInt("##inputA", &inputA))
		{
			value_changed = true;
			inputA = ImClamp(inputA, 0, 255);
			col[3] = inputA / 255.0;
		}

		if (value_changed)
		{
			if (combo_item_current == 0)
			{
				ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], H, S, V);
			}
			else if (combo_item_current == 1)
			{
				ImGui::ColorConvertRGBtoHSV(H, S, V, col[0], col[1], col[2]);
			}
		}



		value_changed = false;
		// RGBA&HSV bar logic
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x, bar_pos_y));
		ImGui::InvisibleButton("R", ImVec2(sv_picker_size, bars_width));
		if (ImGui::IsItemActive())
		{
			value_changed = true;
			if (combo_item_current == 0)
				col[0] = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
			else if (combo_item_current == 1)
				H = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x, bar_pos_y + 32));
		ImGui::InvisibleButton("G", ImVec2(sv_picker_size, bars_width));
		if (ImGui::IsItemActive())
		{
			value_changed = true;
			if (combo_item_current == 0)
				col[1] = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
			else if (combo_item_current == 1)
				S = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x, bar_pos_y + 64));
		ImGui::InvisibleButton("B", ImVec2(sv_picker_size, bars_width));
		if (ImGui::IsItemActive())
		{
			value_changed = true;
			if (combo_item_current == 0)
				col[2] = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
			else if (combo_item_current == 1)
				V = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
		}
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x, bar_pos_y + 96));
		ImGui::InvisibleButton("A", ImVec2(sv_picker_size, bars_width));
		if (ImGui::IsItemActive())
		{
			col[3] = ImSaturate((io.MousePos.x - bar_pos_x + 30) / (sv_picker_size - 1));
			value_changed = true;
		}

		if (value_changed)
		{
			if (combo_item_current == 0)
				ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], H, S, V);
			else if (combo_item_current == 1)
				ImGui::ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
		}
		ImGui::PopItemFlag(); // ImGuiItemFlags_NoNav

		//Hexadecimal Input
		value_changed = false;
		ImGui::PushItemWidth(180);
		ImGui::Text("  Hexadecimal");
		ImGui::SameLine(180);
		value_changed = ImGui::ColorEdit4("##Hexadecimal", col, ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview);
		ImGui::PopItemWidth();
		if (value_changed)
		{
			ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], H, S, V);
		}

		//Combo Box
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size - 20, bar_pos_y - 32));
		const char* items[] = { "RGB 0-255", "HSV" };
		ImGui::Combo("combo", &combo_item_current, items, IM_ARRAYSIZE(items));

		const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
		const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
		const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
		const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
		const ImU32 col_hues[6 + 1] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(0,255,255,style_alpha8), IM_COL32(0,0,255,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(255,0,0,style_alpha8) };
		ImVec4 hue_color_f(1, 1, 1, style.Alpha); ImGui::ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
		ImU32 hue_color32 = ImGui::ColorConvertFloat4ToU32(hue_color_f);
		ImU32 user_col32_striped_of_alpha = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!
		ImU32 user_col32_red_start = ImGui::ColorConvertFloat4ToU32(ImVec4(0, G, B, style.Alpha));
		ImU32 user_col32_red_end = ImGui::ColorConvertFloat4ToU32(ImVec4(255, G, B, style.Alpha));
		ImU32 user_col32_green_start = ImGui::ColorConvertFloat4ToU32(ImVec4(R, 0, B, style.Alpha));
		ImU32 user_col32_green_end = ImGui::ColorConvertFloat4ToU32(ImVec4(R, 255, B, style.Alpha));
		ImU32 user_col32_blue_start = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, 0, style.Alpha));
		ImU32 user_col32_blue_end = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, 255, style.Alpha));
		ImU32 user_col32_alpha_start = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, B, 0.0));
		ImU32 user_col32_alpha_end = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, B, 1.0));
		float tempR, tempG, tempB;
		ImGui::ColorConvertHSVtoRGB(H, 0.0, V, tempR, tempG, tempB);
		ImU32 user_col32_S_start = ImGui::ColorConvertFloat4ToU32(ImVec4(tempR, tempG, tempB, style.Alpha));
		ImGui::ColorConvertHSVtoRGB(H, S, 0.0, tempR, tempG, tempB);
		ImU32 user_col32_V_start = ImGui::ColorConvertFloat4ToU32(ImVec4(tempR, tempG, tempB, style.Alpha));
		ImGui::ColorConvertHSVtoRGB(H, 1.0, V, tempR, tempG, tempB);
		ImU32 user_col32_S_end = ImGui::ColorConvertFloat4ToU32(ImVec4(tempR, tempG, tempB, style.Alpha));
		ImGui::ColorConvertHSVtoRGB(H, S, 1.0, tempR, tempG, tempB);
		ImU32 user_col32_V_end = ImGui::ColorConvertFloat4ToU32(ImVec4(tempR, tempG, tempB, style.Alpha));
		ImVec2 sv_cursor_pos;

		// Render Hue Wheel
		const float aeps = 0.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
		const int segment_per_arc = ImMax(4, (int)wheel_r_outer / 12);
		for (int n = 0; n < 6; n++)
		{
			const float a0 = (n) / 6.0f * 2.0f * IM_PI - aeps;
			const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + aeps;
			const int vert_start_idx = draw_list->VtxBuffer.Size;
			draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer)*0.5f, a0, a1, segment_per_arc);
			draw_list->PathStroke(col_white, false, wheel_thickness);
			const int vert_end_idx = draw_list->VtxBuffer.Size;

			// Paint colors over existing vertices
			ImVec2 gradient_p0(wheel_center.x + ImCos(a0) * wheel_r_inner, wheel_center.y + ImSin(a0) * wheel_r_inner);
			ImVec2 gradient_p1(wheel_center.x + ImCos(a1) * wheel_r_inner, wheel_center.y + ImSin(a1) * wheel_r_inner);
			ImGui::ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
		}

		// Render Cursor + preview on Hue Wheel
		float cos_hue_angle = ImCos(H * 2.0f * IM_PI);
		float sin_hue_angle = ImSin(H * 2.0f * IM_PI);
		ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer)*0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner + wheel_r_outer)*0.5f);
		float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
		int hue_cursor_segments = ImClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
		draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey, hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, col_white, hue_cursor_segments);

		draw_list->AddRectFilledMultiColor(cube_pos, cube_pos + ImVec2(0.5*sv_picker_size, 0.5*sv_picker_size), col_white, hue_color32, hue_color32, col_white);
		draw_list->AddRectFilledMultiColor(cube_pos, cube_pos + ImVec2(0.5*sv_picker_size, 0.5*sv_picker_size), 0, 0, col_black, col_black);
		ImGui::RenderFrameBorder(picker_pos, picker_pos + ImVec2(0.5*sv_picker_size, 0.5*sv_picker_size), 0.0f);
		sv_picker_size *= 0.5;
		sv_cursor_pos.x = ImClamp(IM_ROUND(cube_pos.x + ImSaturate(S)     * sv_picker_size), cube_pos.x, cube_pos.x + sv_picker_size); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = ImClamp(IM_ROUND(cube_pos.y + ImSaturate(1 - V) * sv_picker_size), cube_pos.y, cube_pos.y + sv_picker_size);




		//Render widgets	
		float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
		sv_picker_size *= 1.5;
		draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, user_col32_striped_of_alpha, 12);
		draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey, 12);
		draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white, 12);

		ImRect bar1_bb(bar_pos_x, bar_pos_y, bar_pos_x + sv_picker_size, bar_pos_y + bars_width);
		ImRect bar2_bb(bar_pos_x, bar_pos_y + 30, bar_pos_x + sv_picker_size, bar_pos_y + bars_width + 30);
		ImRect bar3_bb(bar_pos_x, bar_pos_y + 60, bar_pos_x + sv_picker_size, bar_pos_y + bars_width + 60);
		ImRect bar4_bb(bar_pos_x, bar_pos_y + 90, bar_pos_x + sv_picker_size, bar_pos_y + bars_width + 90);
		ImRect bar5_bb(preview_pos_x + preview_size, picker_pos.y, preview_pos_x + preview_size + preview_size, picker_pos.y + preview_size);

		if (combo_item_current == 0)
		{
			draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, user_col32_red_start, user_col32_red_end, user_col32_red_end, user_col32_red_start);
			draw_list->AddRectFilledMultiColor(bar2_bb.Min, bar2_bb.Max, user_col32_green_start, user_col32_green_end, user_col32_green_end, user_col32_green_start);
			draw_list->AddRectFilledMultiColor(bar3_bb.Min, bar3_bb.Max, user_col32_blue_start, user_col32_blue_end, user_col32_blue_end, user_col32_blue_start);
		}
		else if (combo_item_current == 1)
		{
			for (int i = 0; i < 6; ++i)
				draw_list->AddRectFilledMultiColor(ImVec2(bar1_bb.Min.x + i * (sv_picker_size / 6), bar1_bb.Min.y), ImVec2(bar1_bb.Min.x + (i+1)* (sv_picker_size / 6),bar1_bb.Max.y), col_hues[i], col_hues[i+1], col_hues[i + 1], col_hues[i]);

			draw_list->AddRectFilledMultiColor(bar2_bb.Min, bar2_bb.Max, user_col32_S_start, user_col32_S_end, user_col32_S_end, user_col32_S_start);
			draw_list->AddRectFilledMultiColor(bar3_bb.Min, bar3_bb.Max, user_col32_V_start, user_col32_V_end, user_col32_V_end, user_col32_V_start);
		}
		

		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x, bar_pos_y + 90));
		ImGui::Image((GLuint *)transparentTexture, ImVec2(sv_picker_size / 3, bars_width), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size / 3, bar_pos_y + 90));
		ImGui::Image((GLuint *)transparentTexture, ImVec2(sv_picker_size / 3, bars_width), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SetCursorScreenPos(ImVec2(bar_pos_x + sv_picker_size * 2 / 3, bar_pos_y + 90));
		ImGui::Image((GLuint *)transparentTexture, ImVec2(sv_picker_size / 3, bars_width), ImVec2(0, 1), ImVec2(1, 0));
		draw_list->AddRectFilledMultiColor(bar4_bb.Min, bar4_bb.Max, user_col32_alpha_start, user_col32_alpha_end, user_col32_alpha_end, user_col32_alpha_start);
		float bar_line_A = IM_ROUND(bar_pos_x + (alpha)* sv_picker_size - 3);
		ImGui::SetCursorScreenPos(ImVec2(preview_pos_x, picker_pos.y));
		ImGui::Image((GLuint *)transparentTexture, ImVec2(preview_size * 2, preview_size), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::RenderRectFilledRangeH(draw_list, bar5_bb, IM_COL32(255 * R, 255 * G, 255 * B, 255 * alpha), 0, 1, 0.0);

		float bar_line_1 = 0;
		float bar_line_2 = 0;
		float bar_line_3 = 0;
		if (combo_item_current == 0)
		{
			bar_line_1 = IM_ROUND(bar_pos_x + (R)* sv_picker_size - 3);
			bar_line_2 = IM_ROUND(bar_pos_x + (G)* sv_picker_size - 3);
			bar_line_3 = IM_ROUND(bar_pos_x + (B)* sv_picker_size - 3);
		}
		else if (combo_item_current == 1)
		{
			bar_line_1 = IM_ROUND(bar_pos_x + (H)* sv_picker_size - 3);
			bar_line_2 = IM_ROUND(bar_pos_x + (S)* sv_picker_size - 3);
			bar_line_3 = IM_ROUND(bar_pos_x + (V)* sv_picker_size - 3);
		}


		RenderRectForVerticalBar(draw_list, ImVec2(bar_line_1, bar_pos_y), 22, style.Alpha);
		RenderRectForVerticalBar(draw_list, ImVec2(bar_line_2, bar_pos_y + 30), 22, style.Alpha);
		RenderRectForVerticalBar(draw_list, ImVec2(bar_line_3, bar_pos_y + 60), 22, style.Alpha);
		RenderRectForVerticalBar(draw_list, ImVec2(bar_line_A, bar_pos_y + 90), 22, style.Alpha);
		ImGui::EndGroup();
		ImGui::PopID();


		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glDeleteTextures(1, &pickerTexture);
	glDeleteTextures(1, &transparentTexture);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

bool ImRectangleContainsPoint(const  ImVec2 &tl, const  ImVec2 &br, const  ImVec2 &p)
{
	return p.x > tl.x && p.x < br.x && p.y > tl.y && p.y < br.y;
}



static void RenderRectForVerticalBar(ImDrawList* draw_list, ImVec2 pos, float bars_width, float alpha)
{
	ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
	ImRect rect(pos.x, pos.y, pos.x + 5, pos.y + bars_width);
	ImGui::RenderRectFilledRangeH(draw_list, rect, IM_COL32(255, 255, 255, alpha8), 0, 1, 0.0);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	unsigned char *data;
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	if (path[strlen(path) - 2] == 'p')
		data = stbi_load(path, &width, &height, &nrComponents, 0);
	else
		data = stbi_load(path, &width, &height, &nrComponents, STBI_rgb_alpha);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{

		stbi_image_free(data);
	}

	return textureID;
}