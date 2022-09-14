#pragma once

//https://github.com/secsome

struct VoxelIndex1Key
{
	VoxelIndex1Key() { *reinterpret_cast<int*>(this) = 0; }

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned FrameIndex : 5;
private:
	unsigned bitfield_5 : 11;
public:
	unsigned UseAuxVoxel : 1; // !(!pUnit->Type->NoSpawnAlt || pUnit->SpawnManager->Draw_State())
private:
	unsigned bitfield_17 : 15;
};

struct VoxelIndex2Key
{
	VoxelIndex2Key() { *reinterpret_cast<int*>(this) = 0; }

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned Facing : 5;
	unsigned HasTurretOffset : 5;
private:
	unsigned bitfield_10 : 6;
public:
	unsigned FrameIndex : 8;
	unsigned TurretWeaponIndex : 8;
};

using VoxelIndex3Key = int;

struct VoxelIndex4Key
{
	VoxelIndex4Key() { *reinterpret_cast<int*>(this) = 0; }

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned Facing : 5;
	unsigned HasTurretOffset : 5;
private:
	unsigned bitfield_10 : 6;
public:
	unsigned FrameIndex : 8;
private:
	unsigned bitfield_24 : 8;
};

struct ReservedIndexKey
{
private:
	unsigned bitfield_1 : 10;
public:
	unsigned ReservedIndex : 6;
private:
	unsigned bitfield_16 : 16;
};

union VoxelIndexKey
{
	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

	VoxelIndex1Key Index1;
	VoxelIndex2Key Index2;
	VoxelIndex3Key Index3;
	VoxelIndex4Key Index4;
	ReservedIndexKey ReservedIndex;
};

static_assert(sizeof(VoxelIndexKey) == sizeof(int));
