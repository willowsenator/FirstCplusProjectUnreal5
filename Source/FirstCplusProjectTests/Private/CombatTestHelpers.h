#pragma once
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"

class UWorld;
class AEnemy;
class AMainCharacter;
class AMainPlayerController;


struct FCombatWorldFixture
{
	UWorld* World = nullptr;
	AEnemy* Enemy = nullptr;
	AMainCharacter* MainCharacter = nullptr;
	AMainPlayerController* PlayerController = nullptr;
	
	/** Tears down */
	void Destroy();
	
	/**
	 * Builds the world. A game mode (and the game instance it needs) is only created on request:
	 * APawn::TakeDamage refuses every hit in a world with no authority game mode.
	 * EnemyClass lets a test spawn a Blueprint subclass of AEnemy instead of the native class.
	 */
	static auto MakeCombatWorld(bool bWithGameMode = false, TSubclassOf<AEnemy> EnemyClass = nullptr) -> FCombatWorldFixture;
};