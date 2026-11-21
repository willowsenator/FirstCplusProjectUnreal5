#include "CombatTestHelpers.h"
#include "TakeDamageListener.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "FirstCplusProject/GameplayActors/Enemy.h"
#include "FirstCplusProject/GameplayActors/MainCharacter.h"
#include "FirstCplusProject/GameplayActors/MainPlayerController.h"
#include "FirstCplusProject/GameplayActors/Weapon.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	AWeapon* SpawnWeaponOwnedBy(UWorld* World, AActor* Owner)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AWeapon* Weapon = World->SpawnActor<AWeapon>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (Weapon)
		{
			Weapon->SetOwner(Owner);
		}
		return Weapon;
	}

	AMainPlayerController* SpawnSecondController(UWorld* World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AMainPlayerController>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_WeaponInstigatorTracksRepossession,
                                 "FirstCplusProject.Tests.Weapon.Instigator.TracksRepossession",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_WeaponInstigatorTracksRepossession::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	AMainPlayerController* SecondController = Fixture.World ? SpawnSecondController(Fixture.World) : nullptr;
	AWeapon* Weapon = Fixture.World ? SpawnWeaponOwnedBy(Fixture.World, Fixture.MainCharacter) : nullptr;
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) ||
	    !TestNotNull(TEXT("First controller spawned"), Fixture.PlayerController) ||
	    !TestNotNull(TEXT("Second controller spawned"), SecondController) || !TestNotNull(TEXT("Weapon spawned"), Weapon))
	{
		Fixture.Destroy();
		return false;
	}

	// Equip captures the controller that holds the character at that moment.
	Weapon->SetWeaponInstigator(Fixture.PlayerController);

	Fixture.PlayerController->UnPossess();
	SecondController->Possess(Fixture.MainCharacter);

	TStrongObjectPtr<UTakeDamageListener> Listener(NewObject<UTakeDamageListener>());
	Fixture.Enemy->OnTakeAnyDamage.AddDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);

	Weapon->CombatOnOverlapBegin(nullptr, Fixture.Enemy, nullptr, 0, false, FHitResult());

	TestTrue(TEXT("The hit landed"), Listener->bFired);
	TestTrue(TEXT("Hit credit goes to the controller that holds the character now"), Listener->LastInstigatedBy == SecondController);

	Fixture.Enemy->OnTakeAnyDamage.RemoveDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);
	Weapon->Destroy();
	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_WeaponCreditsNobodyWhenOwnerHasNoController,
                                 "FirstCplusProject.Tests.Weapon.Instigator.CreditsNobodyWhenOwnerHasNoController",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_WeaponCreditsNobodyWhenOwnerHasNoController::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	AWeapon* Weapon = Fixture.World ? SpawnWeaponOwnedBy(Fixture.World, Fixture.MainCharacter) : nullptr;
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) ||
	    !TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController) || !TestNotNull(TEXT("Weapon spawned"), Weapon))
	{
		Fixture.Destroy();
		return false;
	}

	// An unpossessed owner must not hand credit to the controller cached when the weapon was equipped.
	Weapon->SetWeaponInstigator(Fixture.PlayerController);
	Fixture.PlayerController->UnPossess();

	TStrongObjectPtr<UTakeDamageListener> Listener(NewObject<UTakeDamageListener>());
	Fixture.Enemy->OnTakeAnyDamage.AddDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);

	Weapon->CombatOnOverlapBegin(nullptr, Fixture.Enemy, nullptr, 0, false, FHitResult());

	TestTrue(TEXT("The hit landed"), Listener->bFired);
	TestNull(TEXT("An owner with no controller credits nobody"), Listener->LastInstigatedBy);

	Fixture.Enemy->OnTakeAnyDamage.RemoveDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);
	Weapon->Destroy();
	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_WeaponWithoutOwnerFallsBackToCachedInstigator,
                                 "FirstCplusProject.Tests.Weapon.Instigator.FallsBackToCachedInstigatorWithoutOwner",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_WeaponWithoutOwnerFallsBackToCachedInstigator::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	AWeapon* Weapon = Fixture.World ? SpawnWeaponOwnedBy(Fixture.World, nullptr) : nullptr;
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController) ||
	    !TestNotNull(TEXT("Weapon spawned"), Weapon))
	{
		Fixture.Destroy();
		return false;
	}

	Weapon->SetWeaponInstigator(Fixture.PlayerController);

	TStrongObjectPtr<UTakeDamageListener> Listener(NewObject<UTakeDamageListener>());
	Fixture.Enemy->OnTakeAnyDamage.AddDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);

	Weapon->CombatOnOverlapBegin(nullptr, Fixture.Enemy, nullptr, 0, false, FHitResult());

	TestTrue(TEXT("The hit landed"), Listener->bFired);
	TestTrue(TEXT("Without an owner the cached instigator is used"), Listener->LastInstigatedBy == Fixture.PlayerController);

	Fixture.Enemy->OnTakeAnyDamage.RemoveDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);
	Weapon->Destroy();
	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_CharacterControllerCacheTracksPossession,
                                 "FirstCplusProject.Tests.MainCharacter.ControllerCache.TracksPossession",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_CharacterControllerCacheTracksPossession::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	AMainPlayerController* SecondController = Fixture.World ? SpawnSecondController(Fixture.World) : nullptr;
	if (!TestNotNull(TEXT("Character spawned"), Fixture.MainCharacter) || !TestNotNull(TEXT("Second controller spawned"), SecondController))
	{
		Fixture.Destroy();
		return false;
	}

	// The fixture possesses the character after it began play.
	TestTrue(TEXT("Cache holds the controller that possessed the character"), Fixture.MainCharacter->MainPlayerController == Fixture.PlayerController);

	Fixture.PlayerController->UnPossess();
	TestNull(TEXT("Cache is cleared when the character is unpossessed"), Fixture.MainCharacter->MainPlayerController);

	SecondController->Possess(Fixture.MainCharacter);
	TestTrue(TEXT("Cache follows the new controller"), Fixture.MainCharacter->MainPlayerController == SecondController);

	Fixture.Destroy();
	return true;
}
