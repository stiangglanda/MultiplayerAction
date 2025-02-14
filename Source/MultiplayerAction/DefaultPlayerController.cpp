// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    HUD = CreateWidget(this, CrossHairHUD);
    if (HUD != nullptr)
    {
        HUD->AddToViewport();
    }
}

