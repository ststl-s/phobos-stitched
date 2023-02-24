#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,
	AdjustLighting = 507,
	CreateBannerGlobal = 8000, // any banner w/ global variable
	CreateBannerLocal = 8001, // any banner w/ local variable
	DeleteBanner = 8002,
	LoadExternVarToLocalVar = 9927,
	LoadExternVarToGlobalVar = 9928,
	SaveLocalVarToExternVar = 9929,
	SaveGlobalVarToExternVar = 9930,
	MessageForSpecifiedHouse = 9931,
	RandomTriggerPut = 9932,
	RandomTriggerEnable = 9933,
	RandomTriggerRemove = 9934,
	ScoreCampaignText = 9935,
	ScoreCampaignTheme = 9936,
	SetNextMission = 9937,
	AttachTriggerForNearestTechno = 9938,
	AttachTriggerForNearestNTechnos = 9939,
	DrawLaserBetweenWeaypoints = 9940,

	PlayOtherTheme = 10005,
	PlayOtherAnim = 10006,
	PlayOtherSound = 10007,
	GetMoney = 10008,
	SetBasic = 10009,
	SetBriefing = 10010,

	CanSaveGame = 15000,
	SelectOption = 15001,

	ExternalVartoVar = 20000,
	VartoExternalVar = 20001,
	EditINI = 20002,

	GetSuperTimer = 20003,
	SetSuperTimer = 20004,

	MoneytoVar = 20005,
	VartoMoney = 20006
};

enum class TargetCate : BYTE
{
	None = 0,
	Building = 1,
	Infantry = 2,
	Vehicle = 3,
	Aircraft = 4,
	Foot = 5,
	Techno = 6
};

class TActionExt
{
public:
	using base_type = TActionClass;

	class ExtData final : public Extension<TActionClass>
	{
	public:

		std::string Value1;
		std::string Value2;
		std::string Parm3;
		std::string Parm4;
		std::string Parm5;
		std::string Parm6;

		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void RecreateLightSources();

	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#pragma push_macro("ACTION_FUNC")
#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	ACTION_FUNC(AdjustLighting);
	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);
	ACTION_FUNC(CreateBannerGlobal);
	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(DeleteBanner);
	ACTION_FUNC(LoadExternVarToLocalVar);
	ACTION_FUNC(LoadExternVarToGlobalVar);
	ACTION_FUNC(SaveLocalVarToExternVar);
	ACTION_FUNC(SaveGlobalVarToExternVar);
	ACTION_FUNC(MessageForSpecifiedHouse);
	ACTION_FUNC(RandomTriggerPut);
	ACTION_FUNC(RandomTriggerEnable);
	ACTION_FUNC(RandomTriggerRemove);
	ACTION_FUNC(ScoreCampaignText);
	ACTION_FUNC(ScoreCampaignTheme);
	ACTION_FUNC(SetNextMission);
	ACTION_FUNC(AttachTriggerForNearestTechno);
	ACTION_FUNC(AttachTriggerForNearestNTechnos);
	ACTION_FUNC(DrawLaserBetweenWaypoints);

	ACTION_FUNC(PlayOtherTheme);
	ACTION_FUNC(PlayOtherAnim);
	ACTION_FUNC(PlayOtherSound);
	ACTION_FUNC(GetMoney);
	ACTION_FUNC(SetBasic);
	ACTION_FUNC(SetBriefing);

	ACTION_FUNC(CanSaveGame);
	ACTION_FUNC(SelectOption);

	ACTION_FUNC(ExternalVartoVar);
	ACTION_FUNC(VartoExternalVar);
	ACTION_FUNC(EditINI);

	ACTION_FUNC(GetSuperTimer);
	ACTION_FUNC(SetSuperTimer);

	ACTION_FUNC(MoneytoVar);
	ACTION_FUNC(VartoMoney);

	static int ReadINI(char* pFile, char* pSection, char* pKey);
	static void WriteINI(char* pFile, char* pSection, char* pKey, char* pValue);

	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
