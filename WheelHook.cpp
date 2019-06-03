#include "WheelHook.h"
#include <d3d9.h>
#include "detours.h"
#include "dllmain.h"


extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static WNDPROC game_wndproc = NULL;
static HWND game_hwnd;
static HWND d3d9_hwnd;



BOOL CALLBACK find_game_hwnd(HWND hwnd, LPARAM game_pid) {
	DWORD hwnd_pid = NULL;

	GetWindowThreadProcessId(hwnd, &hwnd_pid);

	if (hwnd_pid != game_pid)
		return true;

	game_hwnd = hwnd;
	return false;
}

LRESULT STDMETHODCALLTYPE wndproc_hook(HWND window, UINT message_type, WPARAM w_param, LPARAM l_param)
{
	ImGui_ImplWin32_WndProcHandler(window, message_type, w_param, l_param);
	return CallWindowProc(game_wndproc, window, message_type, w_param, l_param);
};

namespace D3D9Hooks
{
	typedef long(__stdcall* PresentScene)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	static PresentScene original_present_scene = NULL;

	typedef long(__stdcall* Reset)(D3DPRESENT_PARAMETERS* pPresentationParameters);
	static Reset original_reset = NULL;

	long __stdcall hook_present_scene(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region);
	long __stdcall hook_reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
}

WheelHook::WheelHook()
{
	this->d3d9_status = UNHOOKED;
	this->d3d9_vtable = nullptr;
}

bool WheelHook::d3d9_init()
{
	IMGUI_CHECKVERSION();
	this->context = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.WantCaptureMouse = true;

	// Setup style
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 0;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = is3D;

	style.WindowRounding = 3;
	style.ChildRounding = 3;
	style.FrameRounding = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding = 3;

	EnumWindows(find_game_hwnd, GetCurrentProcessId());

	WNDCLASSEXA windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "Wheel";
	windowClass.hIconSm = NULL;

	RegisterClassExA(&windowClass);

	HWND window = CreateWindowA(windowClass.lpszClassName, "Painwheel", 
		WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

	HMODULE d3d9_handle;
	if ((d3d9_handle = GetModuleHandleA("d3d9.dll")) == NULL)
	{
		MessageBoxA(NULL, "Failed to get module d3d9.dll", "ERROR", 0);
		DestroyWindow(window);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}

	void* Direct3DCreate9;
	if ((Direct3DCreate9 = GetProcAddress(d3d9_handle, "Direct3DCreate9")) == NULL)
	{
		MessageBoxA(NULL, "Failed to get ProcAddress of Direct3DCreate9", "ERROR", 0);
		DestroyWindow(window);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}

	LPDIRECT3D9 direct3D9;
	if ((direct3D9 = ((LPDIRECT3D9(__stdcall*)(uint32_t))(Direct3DCreate9))(D3D_SDK_VERSION)) == NULL)
	{
		MessageBoxA(NULL, "Failed to Create Direct3D", "ERROR", 0);
		DestroyWindow(window);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}

	D3DDISPLAYMODE displayMode;
	if (direct3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode) < 0)
	{
		MessageBoxA(NULL, "Failed to Get Adapter Display Mode", "ERROR", 0);
		DestroyWindow(window);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}

	D3DPRESENT_PARAMETERS params;
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = displayMode.Format;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	LPDIRECT3DDEVICE9 device;

	if (direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &device) < 0)
	{
		MessageBoxA(NULL, "Failed to Create Device", "ERROR", 0);
		direct3D9->Release();
		DestroyWindow(window);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}
	this->d3d9_vtable = (u32*)calloc(SIZE_OF_D3D9_VTABLE, sizeof(u32));
	memcpy(this->d3d9_vtable, *(u32 **)device, SIZE_OF_D3D9_VTABLE * sizeof(u32));

	direct3D9->Release();
	direct3D9 = NULL;
	device->Release();
	device = NULL;
	DestroyWindow(window);
	UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
	
	D3D9Hooks::original_present_scene = (D3D9Hooks::PresentScene)this->d3d9_vtable[D3D9_PRESENT];
	D3D9Hooks::original_reset = (D3D9Hooks::Reset)this->d3d9_vtable[D3D9_RESET];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)D3D9Hooks::original_present_scene, D3D9Hooks::hook_present_scene);
	DetourAttach(&(PVOID&)D3D9Hooks::original_reset, D3D9Hooks::hook_reset);
	DetourTransactionCommit();
	return true;
}

void WheelHook::d3d9_free()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)D3D9Hooks::original_present_scene, D3D9Hooks::hook_present_scene);
	DetourDetach(&(PVOID&)D3D9Hooks::original_reset, D3D9Hooks::hook_reset);
	DetourTransactionCommit();
	SetWindowLongPtr(d3d9_hwnd, GWLP_WNDPROC, LONG_PTR(game_wndproc));
	ImGui::DestroyContext(this->context);
}

#pragma region D3D9 Hooks
long __stdcall D3D9Hooks::hook_present_scene(IDirect3DDevice9* pDevice, const RECT* src, 
	const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region)
{
	static bool init = false;
	if (!init)
	{
		D3DDEVICE_CREATION_PARAMETERS params;
		auto hr = pDevice->GetCreationParameters(&params);
		d3d9_hwnd = params.hFocusWindow;
		ImGui_ImplWin32_Init(d3d9_hwnd);
		ImGui_ImplDX9_Init(pDevice);

		game_wndproc = (WNDPROC)SetWindowLongPtr(d3d9_hwnd, GWLP_WNDPROC, LONG_PTR(wndproc_hook)); //hook wndproc for imgui input

		init = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	draw_ui();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return D3D9Hooks::original_present_scene(pDevice, src, dest, wnd_override, dirty_region);
}

long __stdcall D3D9Hooks::hook_reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
	return D3D9Hooks::original_reset(pPresentationParameters);
}
#pragma endregion