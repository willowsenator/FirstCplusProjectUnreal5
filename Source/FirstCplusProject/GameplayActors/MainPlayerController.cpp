// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"

#include "Enemy.h"
#include "MainCharacter.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (HUDOverlayAsset)
    {
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
        
        if (HUDOverlay)
        {
            HUDOverlay->AddToViewport();
            HUDOverlay->SetVisibility(ESlateVisibility::Visible);
        }
    }
    
    if (WEnemyHealthBar)
    {
        EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
        if (EnemyHealthBar)
        {
            EnemyHealthBar->AddToViewport();
            EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
            const FVector2D Alignment(0.0f, 0.0f);
            EnemyHealthBar->SetAlignmentInViewport(Alignment);
        }
    }
    
    
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
    if (EnemyHealthBar)
    {
        bEnemyHealthBarVisible = true;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
    if (EnemyHealthBar)
    {
        bEnemyHealthBarVisible = false;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool AMainPlayerController::TryGetCombatTargetLocation(FVector& OutLocation) const
{
    const AMainCharacter* PlayerCharacter = Cast<AMainCharacter>(GetPawn());
    if (!PlayerCharacter || !IsValid(PlayerCharacter->CombatTarget))
    {
        return false;
    }
    OutLocation = PlayerCharacter->CombatTarget->GetActorLocation();
    return true;
}

void AMainPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // A hidden bar needs no repositioning: RemoveEnemyHealthBar already hid the widget.
    if (!EnemyHealthBar || !bEnemyHealthBarVisible)
    {
        return;
    }

    FVector TargetLocation = FVector::ZeroVector;
    FVector2D PositionInViewport = FVector2D::ZeroVector;
    if (!TryGetCombatTargetLocation(TargetLocation) || !ProjectWorldLocationToScreen(TargetLocation, PositionInViewport))
    {
        // No target, or it is behind the camera: hide the bar rather than place it from an unset position.
        EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
    PositionInViewport.Y -= 50.f;

    const FVector2D SizeInViewport(300.f, 25.f);

    EnemyHealthBar->SetPositionInViewport(PositionInViewport, true);
    EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
}