#include <New/Entity/AttachmentClass.h>

#include <Misc/DrawLaser.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x55B6B3, LogicClass_AI_After, 0x5)
{
	for (auto const& pAttachment : AttachmentClass::Array)
	{
		if (pAttachment != nullptr)
			pAttachment->AI();
	}

	//在这里应该就不会RE了吧
	DrawLaser::UpdateAll();

	return 0;
}
