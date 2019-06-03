#pragma once
#include <stdint.h>
#include "psapi.h"

void draw_ui();

bool data_compare(const BYTE* pData, const BYTE* bMask, const char* szMask);
DWORD find_pattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, const char* szMask);

template< class T > struct TArray
{
public:
	T* data;
	int count;
	int max;

public:
	TArray()
	{
		data = NULL;
		count = max = 0;
	};

public:
	int num()
	{
		return this->count;
	};

	T& operator() (int i)
	{
		return this->data[i];
	};

	const T& operator() (int i) const
	{
		return this->data[i];
	};

	void add(T InputData)
	{
		data = (T*)realloc(data, sizeof(T) * (count + 1));
		data[count++] = InputData;
		max = count;
	};

	void clear()
	{
		free(data);
		count = max = 0;
	};
};

struct FNameEntry
{
	UCHAR	Unknown[0x10];	// unknowed data
	char	Name[1];		// name
};

struct FString : public TArray< wchar_t >
{
	FString() {};

	FString(wchar_t* other)
	{
		this->max = this->count = *other ? (wcslen(other) + 1) : 0;

		if (this->count)
			this->data = other;
	};

	~FString() {};

	FString operator = (wchar_t* other)
	{
		if (this->data != other)
		{
			this->max = this->count = *other ? (wcslen(other) + 1) : 0;

			if (this->count)
				this->data = other;
		}

		return *this;
	};
};

struct PSMCBone
{

};

struct WorkingBone
{
	uint32_t FNameId;
	uint32_t unk;
};

struct S1CustomizableSkeletalMeshComponent
{
	TArray< struct FMatrix >	CustomizeSpaceBases;
	TArray< struct FMatrix >	OriginalSpaceBases;
	unsigned long	m_bUseCustomize : 1;
};

struct US1ParentSkeletalMeshComponent
{
	char pad[0x478];
	struct TArray<uint8_t> visible_parts;
	char pad_0[0x2C];
	S1CustomizableSkeletalMeshComponent  customizable_skeletal_mesh_component;
	struct TArray<PSMCBone> m_PSMCBoneArray;
	struct TArray<WorkingBone>m_WorkingBoneArray;
	uint32_t m_bHaveParts;
	uint8_t m_CustomizeOffsetCriteria;
};

struct S1SkeletalMeshActor
{
	int32_t secret[0x70];
	US1ParentSkeletalMeshComponent* parent_mesh;
};

struct S1SkeletalMeshController
{
	uint32_t sercret[4];
	S1SkeletalMeshActor* actor;
};

struct S1Player
{
	uint32_t secret[0xA5];
	S1SkeletalMeshController* mesh_controller;
};

struct S1Game
{
	uint32_t secret[5];
	S1Player* player;
};

struct FVector
{
	float X;
	float Y;
	float Z;
};

struct FPlane : FVector
{
	float W;
};

struct FMatrix
{
	struct FPlane                                      XPlane;
	struct FPlane                                      YPlane;
	struct FPlane                                      ZPlane;
	struct FPlane                                      WPlane;
};