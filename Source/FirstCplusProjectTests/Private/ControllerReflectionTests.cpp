#include "FirstCplusProject/GameplayActors/MainPlayerController.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_MainPlayerController_HealthBarFlagIsReflected,
                                 "FirstCplusProject.Tests.MainPlayerController.Reflection.HealthBarVisible",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_MainPlayerController_HealthBarFlagIsReflected::RunTest(const FString& Parameters)
{
	const FProperty* Visible = AMainPlayerController::StaticClass()->FindPropertyByName(TEXT("bEnemyHealthBarVisible"));
	if (TestNotNull(TEXT("bEnemyHealthBarVisible is reflected"), Visible))
	{
		TestTrue(TEXT("bEnemyHealthBarVisible is visible but not editable (VisibleAnywhere)"),
		         Visible->HasAllPropertyFlags(CPF_Edit | CPF_EditConst));
		TestTrue(TEXT("bEnemyHealthBarVisible is Blueprint read-only"),
		         Visible->HasAllPropertyFlags(CPF_BlueprintVisible | CPF_BlueprintReadOnly));
		TestEqual(TEXT("bEnemyHealthBarVisible is in the Widgets category"), Visible->GetMetaData(TEXT("Category")), FString(TEXT("Widgets")));
	}

	return true;
}
