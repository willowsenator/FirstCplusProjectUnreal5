#include "CombatTestHelpers.h"
#include "Engine/World.h"
#include "FirstCplusProject/GameplayActors/Enemy.h"
#include "FirstCplusProject/GameplayActors/MainCharacter.h"
#include "FirstCplusProject/GameplayActors/MainPlayerController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_HealthBarTracksTeleportedEnemy,
                                 "FirstCplusProject.Tests.MainPlayerController.CombatTarget.HealthBarTracksTeleportedEnemy",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_HealthBarTracksTeleportedEnemy::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) ||
	    !TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.MainCharacter->SetCombatTarget(Fixture.Enemy);

	// No tick runs between the move and the read, so a copy refreshed by another actor's tick would still hold the old place.
	// The new place stays inside the enemy's agro sphere: leaving it would release the combat target.
	const FVector NewLocation(300.f, 300.f, 0.f);
	Fixture.Enemy->SetActorLocation(NewLocation);

	FVector TargetLocation = FVector::ZeroVector;
	TestTrue(TEXT("The controller finds the player's combat target"), Fixture.PlayerController->TryGetCombatTargetLocation(TargetLocation));
	TestEqual(TEXT("The controller reads the enemy's current location"), TargetLocation, Fixture.Enemy->GetActorLocation());

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_CombatTargetLocationNeedsALiveTarget,
                                 "FirstCplusProject.Tests.MainPlayerController.CombatTarget.NeedsALiveTarget",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_CombatTargetLocationNeedsALiveTarget::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) ||
	    !TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController))
	{
		Fixture.Destroy();
		return false;
	}

	FVector TargetLocation = FVector::ZeroVector;

	Fixture.MainCharacter->SetCombatTarget(nullptr);
	TestFalse(TEXT("No target reports none"), Fixture.PlayerController->TryGetCombatTargetLocation(TargetLocation));

	Fixture.MainCharacter->SetCombatTarget(Fixture.Enemy);
	Fixture.Enemy->Destroy();
	TestFalse(TEXT("A destroyed target reports none"), Fixture.PlayerController->TryGetCombatTargetLocation(TargetLocation));

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_CombatTargetLocationNeedsAPawn,
                                 "FirstCplusProject.Tests.MainPlayerController.CombatTarget.NeedsAPawn",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_CombatTargetLocationNeedsAPawn::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) ||
	    !TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.MainCharacter->SetCombatTarget(Fixture.Enemy);
	Fixture.PlayerController->UnPossess();

	FVector TargetLocation = FVector::ZeroVector;
	TestFalse(TEXT("A controller without a pawn reports no target"), Fixture.PlayerController->TryGetCombatTargetLocation(TargetLocation));

	Fixture.Destroy();
	return true;
}
