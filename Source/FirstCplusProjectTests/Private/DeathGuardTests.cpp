#include "CombatTestHelpers.h"
#include "TakeDamageListener.h"
#include "FirstCplusProject/GameplayActors/Enemy.h"
#include "FirstCplusProject/GameplayActors/MainCharacter.h"
#include "FirstCplusProject/GameplayActors/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	constexpr float LethalDamage = 100000.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_EnemyStaysDeadAfterStatusChange, "FirstCplusProject.Tests.Enemy.Die.StaysDeadAfterStatusChange",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_EnemyStaysDeadAfterStatusChange::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.Enemy->DecreaseHealth(LethalDamage);
	TestFalse(TEXT("Enemy is dead after lethal damage"), Fixture.Enemy->Alive());

	// Overlap-end and attack-end handlers move a living enemy back to idle; a corpse must stay dead.
	Fixture.Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
	TestFalse(TEXT("Dead enemy stays dead after an idle transition"), Fixture.Enemy->Alive());

	Fixture.Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
	TestFalse(TEXT("Dead enemy stays dead after a move transition"), Fixture.Enemy->Alive());

	TestEqual(TEXT("Dead enemy cannot move"), Fixture.Enemy->GetCharacterMovement()->MovementMode.GetValue(), MOVE_None);

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DeadEnemyTakesNoDamage, "FirstCplusProject.Tests.Enemy.TakeDamage.IgnoredWhenDead",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DeadEnemyTakesNoDamage::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.Enemy->DecreaseHealth(LethalDamage);

	const float Applied = Fixture.Enemy->TakeDamage(5.f, FDamageEvent(), nullptr, nullptr);

	TestEqual(TEXT("A dead enemy reports no damage applied"), Applied, 0.f);
	TestEqual(TEXT("Health stays at zero"), Fixture.Enemy->Health, 0.f);
	TestFalse(TEXT("Enemy is still dead"), Fixture.Enemy->Alive());

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DieSurvivesNullComponents, "FirstCplusProject.Tests.Enemy.Die.SurvivesNullComponents",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DieSurvivesNullComponents::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.Enemy->CombatCollision = nullptr;
	Fixture.Enemy->AgroSphere = nullptr;
	Fixture.Enemy->CombatSphere = nullptr;

	Fixture.Enemy->DecreaseHealth(LethalDamage);

	TestFalse(TEXT("Enemy dies even with its collision components missing"), Fixture.Enemy->Alive());
	TestEqual(TEXT("Dead enemy cannot move"), Fixture.Enemy->GetCharacterMovement()->MovementMode.GetValue(), MOVE_None);

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DieDisablesCollision, "FirstCplusProject.Tests.Enemy.Die.DisablesCollision",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DieDisablesCollision::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy)
		|| !TestNotNull(TEXT("CombatCollision exists"), Fixture.Enemy->CombatCollision)
		|| !TestNotNull(TEXT("AgroSphere exists"), Fixture.Enemy->AgroSphere)
		|| !TestNotNull(TEXT("CombatSphere exists"), Fixture.Enemy->CombatSphere))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.Enemy->DecreaseHealth(LethalDamage);

	TestEqual(TEXT("Dead enemy's weapon hit volume is off"), Fixture.Enemy->CombatCollision->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestEqual(TEXT("Dead enemy no longer detects the player"), Fixture.Enemy->AgroSphere->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestEqual(TEXT("Dead enemy no longer starts attacks"), Fixture.Enemy->CombatSphere->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestEqual(TEXT("Dead enemy's capsule is off"), Fixture.Enemy->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DeadPlayerCannotAttack, "FirstCplusProject.Tests.MainCharacter.Die.CannotAttackWhileHoldingLMB",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DeadPlayerCannotAttack::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("MainCharacter spawned"), Fixture.MainCharacter))
	{
		Fixture.Destroy();
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWeapon* Weapon = Fixture.World->SpawnActor<AWeapon>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!TestNotNull(TEXT("Weapon spawned"), Weapon))
	{
		Fixture.Destroy();
		return false;
	}
	Fixture.MainCharacter->SetEquippedWeapon(Weapon);

	// The player dies mid-swing while still holding the attack button.
	Fixture.MainCharacter->bLMB = true;
	Fixture.MainCharacter->bAttacking = true;
	Fixture.MainCharacter->Die();

	TestFalse(TEXT("Dying releases the attack button state"), Fixture.MainCharacter->bLMB);
	TestFalse(TEXT("Dying ends the swing in progress"), Fixture.MainCharacter->bAttacking);

	// The attack timer or the montage end then reports the swing finished.
	Fixture.MainCharacter->AttackEnd();
	TestFalse(TEXT("A finished swing does not chain a new attack after death"), Fixture.MainCharacter->bAttacking);

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DeadPlayerAttackIsRefused, "FirstCplusProject.Tests.MainCharacter.Attack.RefusedWhenDead",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DeadPlayerAttackIsRefused::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld();
	if (!TestNotNull(TEXT("MainCharacter spawned"), Fixture.MainCharacter))
	{
		Fixture.Destroy();
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWeapon* Weapon = Fixture.World->SpawnActor<AWeapon>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!TestNotNull(TEXT("Weapon spawned"), Weapon))
	{
		Fixture.Destroy();
		return false;
	}
	Fixture.MainCharacter->SetEquippedWeapon(Weapon);
	Fixture.MainCharacter->Die();

	Fixture.MainCharacter->Attack();

	TestFalse(TEXT("A dead player with a weapon does not start an attack"), Fixture.MainCharacter->bAttacking);
	TestFalse(TEXT("A dead player does not turn toward a target"), Fixture.MainCharacter->bInterpToEnemy);

	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_DeadPlayerTakesNoDamage, "FirstCplusProject.Tests.MainCharacter.TakeDamage.IgnoredWhenDead",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_DeadPlayerTakesNoDamage::RunTest(const FString& Parameters)
{
	// A game mode is required, otherwise APawn::TakeDamage returns 0 and the test would pass vacuously.
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("MainCharacter spawned"), Fixture.MainCharacter))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.MainCharacter->DecreaseHealth(LethalDamage);
	TestTrue(TEXT("Player is dead after lethal damage"), Fixture.MainCharacter->bIsDead);

	TStrongObjectPtr<UTakeDamageListener> Listener(NewObject<UTakeDamageListener>());
	Fixture.MainCharacter->OnTakeAnyDamage.AddDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);

	const float Applied = Fixture.MainCharacter->TakeDamage(5.f, FDamageEvent(), nullptr, nullptr);

	TestEqual(TEXT("A dead player reports no damage applied"), Applied, 0.f);
	TestFalse(TEXT("A dead player broadcasts no damage event"), Listener->bFired);
	TestEqual(TEXT("Health stays at zero"), Fixture.MainCharacter->Health, 0.f);

	Fixture.MainCharacter->OnTakeAnyDamage.RemoveDynamic(Listener.Get(), &UTakeDamageListener::OnDamage);
	Fixture.Destroy();
	return true;
}
