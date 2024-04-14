#pragma once

// Used with color schemes and what not.

class HashString
{
public:
	char Name[256];
};

class HashObject
{
public:
	HashString Key;
	DWORD Value;
};

class HashTable
{
public:
	DynamicVectorClass<HashObject>* Buckets;
	DWORD BucketHashFunction;
	int BucketCount;
	int BucketGrowthStep;
};

class HashIterator
{
public:
	int BucketIndex;
	int InBucketIndex;
	bool OutOfBuckets;
	bool Unused1;
	bool Unused2;
	bool Unused3;
};
