#include "FirstCplusProject/GameplayActors/MainCharacter.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFirstCplusProjectTests_MainCharacter_InterpPropertiesAreReflected,
                                 "FirstCplusProject.Tests.MainCharacter.Reflection.InterpProperties",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCplusProjectTests_MainCharacter_InterpPropertiesAreReflected::RunTest(const FString& Parameters)
{
	const FProperty* InterpSpeed = AMainCharacter::StaticClass()->FindPropertyByName(TEXT("InterpSpeed"));
	if (TestNotNull(TEXT("InterpSpeed is reflected"), InterpSpeed))
	{
		TestTrue(TEXT("InterpSpeed is editable (EditAnywhere)"),
		         InterpSpeed->HasAllPropertyFlags(CPF_Edit) && !InterpSpeed->HasAnyPropertyFlags(CPF_EditConst));
		TestTrue(TEXT("InterpSpeed is Blueprint read/write"),
		         InterpSpeed->HasAllPropertyFlags(CPF_BlueprintVisible) && !InterpSpeed->HasAnyPropertyFlags(CPF_BlueprintReadOnly));
		TestEqual(TEXT("InterpSpeed is in the Combat category"), InterpSpeed->GetMetaData(TEXT("Category")), FString(TEXT("Combat")));
	}

	const FProperty* InterpToEnemy = AMainCharacter::StaticClass()->FindPropertyByName(TEXT("bInterpToEnemy"));
	if (TestNotNull(TEXT("bInterpToEnemy is reflected"), InterpToEnemy))
	{
		TestTrue(TEXT("bInterpToEnemy is visible but not editable (VisibleAnywhere)"),
		         InterpToEnemy->HasAllPropertyFlags(CPF_Edit | CPF_EditConst));
		TestTrue(TEXT("bInterpToEnemy is Blueprint read-only"),
		         InterpToEnemy->HasAllPropertyFlags(CPF_BlueprintVisible | CPF_BlueprintReadOnly));
		TestEqual(TEXT("bInterpToEnemy is in the Combat category"), InterpToEnemy->GetMetaData(TEXT("Category")), FString(TEXT("Combat")));
	}

	return true;
}
