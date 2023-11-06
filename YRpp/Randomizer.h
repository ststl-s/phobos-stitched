#pragma once

#include <Helpers/CompileTime.h>

class Randomizer
{
public:
	// for any randomization happening inside a match (odds of a survivor, crate, etc), use the ScenarioClass::Random object instead!
	// this object should only be used for RMG and other randomness outside a match
	static constexpr reference<Randomizer, 0x886B88u> const Global{};

	using result_type = int;

	static result_type min()
	{ return std::numeric_limits<int>::min(); }

	static result_type max()
	{ return std::numeric_limits<int>::max(); }

	int Random()
	{ JMP_THIS(0x65C780); }

	int RandomRanged(int nMax)
	{ RandomRanged(0, nMax); }

	int RandomRanged(int nMin, int nMax)
	{ JMP_THIS(0x65C7E0); }

	Randomizer(DWORD dwSeed = *reinterpret_cast<DWORD*>(0xA8ED94))
	{ JMP_THIS(0x65C6D0); }

	// helper methods
	double RandomDouble()
	{ return this->RandomRanged(1, INT_MAX) / (double)((unsigned int)INT_MAX + 1); }

	result_type operator()()
	{ return Random(); }

	result_type operator()(int nMax)
	{ return RandomRanged(nMax); }

	result_type operator()(int nMin, int nMax)
	{ return RandomRanged(nMin, nMax); }

	//Properties

public:

	bool unknown_00;
	PROTECTED_PROPERTY(BYTE, align_1[3]);
	int Next1; //from Table
	int Next2; //from Table
	DWORD Table [0xFA];
};
