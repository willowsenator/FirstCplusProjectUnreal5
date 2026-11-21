#include "CombatTestHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "FirstCplusProject/GameplayActors/MainPlayerController.h"
#include "Misc/AutomationTest.h"

namespace
{
	constexpr TCHAR EnemyHealthBarPath[] = TEXT("/Game/GameProject/HUD/EnemyHealthBar.EnemyHealthBar_C");

	void TickWorld(UWorld* World, const int32 FrameCount)
	{
		for (int32 Frame = 0; Frame < FrameCount; ++Frame)
		{
			World->Tick(LEVELTICK_All, 0.016f);
		}
	}

	/** Builds a fixture whose controller holds a real health bar widget. The test world has no local player, so the enemy can never be projected onto the screen. */
	bool PrepareControllerWithBar(FAutomationTestBase& Test, FCombatWorldFixture& Fixture, UUserWidget*& OutBar)
	{
		UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, EnemyHealthBarPath);
		if (!Test.TestNotNull(TEXT("EnemyHealthBar_C loads"), WidgetClass))
		{
			return false;
		}

		Fixture = FCombatWorldFixture::MakeCombatWorld(true);
		if (!Test.TestNotNull(TEXT("Controller spawned"), Fixture.PlayerController))
		{
			return false;
		}

		OutBar = CreateWidget<UUserWidget>(Fixture.World, WidgetClass);
		if (!Test.TestNotNull(TEXT("Health bar widget created"), OutBar))
		{
			return false;
		}

		Fixture.PlayerController->EnemyHealthBar = OutBar;
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_HiddenHealthBarSkipsRepositioning,
                                 "FirstCplusProject.Tests.MainPlayerController.Tick.HiddenHealthBarSkipsRepositioning",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_HiddenHealthBarSkipsRepositioning::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture;
	UUserWidget* Bar = nullptr;
	if (!PrepareControllerWithBar(*this, Fixture, Bar))
	{
		Fixture.Destroy();
		return false;
	}

	// A visibility that neither branch of Tick would ever write, so any touch of the widget shows.
	Fixture.PlayerController->bEnemyHealthBarVisible = false;
	Bar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	TickWorld(Fixture.World, 5);

	TestEqual(TEXT("A hidden health bar is left untouched by Tick"), Bar->GetVisibility(), ESlateVisibility::SelfHitTestInvisible);
	Fixture.Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_VisibleHealthBarHidesWhenEnemyUnprojectable,
                                 "FirstCplusProject.Tests.MainPlayerController.Tick.VisibleHealthBarHidesWhenEnemyUnprojectable",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_VisibleHealthBarHidesWhenEnemyUnprojectable::RunTest(const FString& Parameters)
{
	FCombatWorldFixture Fixture;
	UUserWidget* Bar = nullptr;
	if (!PrepareControllerWithBar(*this, Fixture, Bar))
	{
		Fixture.Destroy();
		return false;
	}

	Fixture.PlayerController->bEnemyHealthBarVisible = true;
	Bar->SetVisibility(ESlateVisibility::Visible);

	TickWorld(Fixture.World, 5);

	TestEqual(TEXT("A shown bar is hidden while the enemy cannot be projected"), Bar->GetVisibility(), ESlateVisibility::Hidden);
	Fixture.Destroy();
	return true;
}
