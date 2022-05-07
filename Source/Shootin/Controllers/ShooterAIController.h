// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/ShootinAIController.h"
#include "ShooterAIController.generated.h"

/**
 *
 */
UCLASS()
class SHOOTIN_API AShooterAIController : public AShootinAIController
{
    GENERATED_BODY()

    // AI Percetion
    UFUNCTION(BlueprintCallable, Category = "AIPerception")
        void SetPerceptionSightRadius(float radius);
    UFUNCTION(BlueprintCallable, Category = "AIPerception")
        void SetPerceptionLoseSightRadius(float radius);

};
