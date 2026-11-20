#include "Animation/AnimMontage.h"
#include "Misc/AutomationTest.h"

namespace
{
	// Blueprint notify names carry spaces ("Activate Collision"); the function names do not.
	FString NormalizeNotifyName(const FName Name)
	{
		return Name.ToString().Replace(TEXT(" "), TEXT(""));
	}

	bool MontageHasNotifyOnSection(const UAnimMontage* Montage, const FName Section, const FName NotifyName)
	{
		if (!Montage)
		{
			return false;
		}

		const int32 SectionIndex = Montage->GetSectionIndex(Section);
		if (SectionIndex == INDEX_NONE)
		{
			return false;
		}

		float Start = 0.f;
		float End = 0.f;
		Montage->GetSectionStartAndEndTime(SectionIndex, Start, End);

		const FString Wanted = NormalizeNotifyName(NotifyName);
		for (const FAnimNotifyEvent& Event : Montage->Notifies)
		{
			const float Time = Event.GetTriggerTime();
			if (Time >= Start && Time < End && NormalizeNotifyName(Event.NotifyName) == Wanted)
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_TrollCombatMontage_HasRequiredNotifies, "FirstCplusProject.Tests.AssetConfig.TrollCombatMontage",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_TrollCombatMontage_HasRequiredNotifies::RunTest(const FString& Parameters)
{
	const UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/GameProject/Enemies/Troll/TrollCombatMontage.TrollCombatMontage"));
	if (!TestNotNull(TEXT("TrollCombatMontage loads"), Montage))
	{
		return false;
	}

	TestTrue(TEXT("Attack section has ActivateCollision"), MontageHasNotifyOnSection(Montage, TEXT("Attack"), TEXT("ActivateCollision")));
	TestTrue(TEXT("Attack section has DeactivateCollision"), MontageHasNotifyOnSection(Montage, TEXT("Attack"), TEXT("DeactivateCollision")));
	TestTrue(TEXT("Attack section has AttackEnd"), MontageHasNotifyOnSection(Montage, TEXT("Attack"), TEXT("AttackEnd")));
	TestTrue(TEXT("Death section has DeathEnd"), MontageHasNotifyOnSection(Montage, TEXT("Death"), TEXT("DeathEnd")));

	TestFalse(TEXT("Helper rejects a missing section"), MontageHasNotifyOnSection(Montage, TEXT("NoSuchSection"), TEXT("AttackEnd")));
	TestFalse(TEXT("Helper rejects a notify on the wrong section"), MontageHasNotifyOnSection(Montage, TEXT("Death"), TEXT("ActivateCollision")));
	TestFalse(TEXT("Helper rejects a null montage"), MontageHasNotifyOnSection(nullptr, TEXT("Attack"), TEXT("AttackEnd")));

	return true;
}
