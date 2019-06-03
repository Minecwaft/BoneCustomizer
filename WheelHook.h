#pragma once
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#define SIZE_OF_D3D9_VTABLE 119

class WheelHook
{
public:
	enum HookStatus
	{
		UNHOOKED = 0,
		HOOKED = 1
	};

	enum D3D9_VTABLE
	{
		D3D9_RESET = 16,
		D3D9_PRESENT = 17
	};

	bool d3d9_status;
	u32* d3d9_vtable;
	ImGuiContext* context;
	WheelHook();
	bool d3d9_init();
	void d3d9_free();
};