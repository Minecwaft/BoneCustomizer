#include "framework.h"
#include "WheelHook.h"
#include "dllmain.h"

static BYTE* s1game_sig = (BYTE*)"\x00\x00\x00\x00\x56\x8B\xF1\x8B\x48\x14\x8B\x01\xFF\x50\x18\x83\xEC\x0C\x8B\xCC";
static char s1game_mask[] = "????xxxxxxxxxxxxxxxx";

static BYTE* global_object_sig = (BYTE*)"\x00\x00\x00\x00\x6A\x01\x8B\x34\xB0\x8B\xCE\xE8\x00\x00\x00\x00";
static char global_object_mask[] = "????xxxxxxxx????";

static BYTE* global_names_sig = (BYTE*)"\x00\x00\x00\x00\xFF\x75\x08\xC7\x45\x00\x00\x00\x00\x00\x8B\x0C\x91\xE8\x00\x00\x00\x00";
static char global_names_mask[] = "????xxxxx?????xxxx????";

static bool signal = true;
S1Game* s1_game = nullptr;
uint32_t global_objects;
uint32_t global_names;

DWORD WINAPI init(LPVOID base)
{
	WheelHook hook;
	MODULEINFO tera_info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &tera_info, sizeof(MODULEINFO));
	s1_game = **(S1Game * **)find_pattern((DWORD)tera_info.lpBaseOfDll, (DWORD)tera_info.SizeOfImage, s1game_sig, s1game_mask);
	global_objects = *(uint32_t*)find_pattern((DWORD)tera_info.lpBaseOfDll, (DWORD)tera_info.SizeOfImage, global_object_sig, global_object_mask);
	global_names = *(uint32_t*)find_pattern((DWORD)tera_info.lpBaseOfDll, (DWORD)tera_info.SizeOfImage, global_names_sig, global_names_mask);

	hook.d3d9_init();
	while (signal)
	{
		Sleep(10);
	}
	hook.d3d9_free();
	FreeLibraryAndExitThread(GetModuleHandleW(L"BoneCustomizer.dll"), 0);
	return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		CreateThread(0, 0, init, 0, 0, 0);
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void draw_ui()
{
	if (!s1_game->player) return;
	static TArray<FNameEntry*>* object_names = (TArray<FNameEntry*>*)global_names;
	US1ParentSkeletalMeshComponent* component = s1_game->player->mesh_controller->actor->parent_mesh;
	
	ImGui::Begin("Bones", &signal);
	for (int i = 0; i < component->customizable_skeletal_mesh_component.CustomizeSpaceBases.count; ++i)
	{
		if (ImGui::TreeNode((void*)(intptr_t)i, "%s", object_names->data[component->m_WorkingBoneArray.data[i].FNameId]->Name))
		{
			ImGui::SliderFloat("X", &component->customizable_skeletal_mesh_component.CustomizeSpaceBases.data[i].XPlane.X, 0.0f, 5.0f, "%.2f");
			ImGui::SliderFloat("Y", &component->customizable_skeletal_mesh_component.CustomizeSpaceBases.data[i].YPlane.Y, 0.0f, 5.0f, "%.2f");
			ImGui::SliderFloat("Z", &component->customizable_skeletal_mesh_component.CustomizeSpaceBases.data[i].ZPlane.Z, 0.0f, 5.0f, "%.2f");
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

bool data_compare(const BYTE* pData, const BYTE* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD find_pattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, const char* szMask) {
	for (DWORD i = 0; i < dwLen; i++)
		if (data_compare((BYTE*)(dwAddress + i), bMask, szMask))
			return (DWORD)(dwAddress + i);
	return 0;
}
