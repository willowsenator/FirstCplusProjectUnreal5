#include "CombatTestHelpers.h"
#include "FirstCplusProject/GameplayActors/Enemy.h"
#include "FirstCplusProject/GameplayActors/MainCharacter.h"
#include "FirstCplusProject/GameplayActors/Weapon.h"
#include "Engine/EngineTypes.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_EnemyHitAppliesDamageWithUnsetDamageType,
                                 "FirstCplusProject.Tests.Enemy.Combat.DamageAppliesEvenWithUnsetDamageType",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_EnemyHitAppliesDamageWithUnsetDamageType::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy) || !TestNotNull(TEXT("Player spawned"), Fixture.MainCharacter))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.Enemy->DamageTypeClass = nullptr;
	const float HealthBefore = Fixture.MainCharacter->Health;

	Fixture.Enemy->CombatOnOverlapBegin(nullptr, Fixture.MainCharacter, nullptr, 0, false, FHitResult());

	TestEqual(TEXT("Player loses the enemy's damage"), Fixture.MainCharacter->Health, HealthBefore - Fixture.Enemy->Damage);
	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_WeaponHitAppliesDamageWithUnsetDamageType,
                                 "FirstCplusProject.Tests.Weapon.Combat.DamageAppliesEvenWithUnsetDamageType",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_WeaponHitAppliesDamageWithUnsetDamageType::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture = FCombatWorldFixture::MakeCombatWorld(true);
	if (!TestNotNull(TEXT("Enemy spawned"), Fixture.Enemy))
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

	Weapon->DamageTypeClass = nullptr;
	const float HealthBefore = Fixture.Enemy->Health;

	Weapon->CombatOnOverlapBegin(nullptr, Fixture.Enemy, nullptr, 0, false, FHitResult());

	TestEqual(TEXT("Enemy loses the weapon's damage"), Fixture.Enemy->Health, HealthBefore - Weapon->Damage);
	Weapon->Destroy();
	Fixture.Destroy();
	return true;
}
