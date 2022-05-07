// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"


// ターゲットに気づく最大視野距離を設定する
void AShooterAIController::SetPerceptionSightRadius(float radius)
{
	UAIPerceptionComponent *perception = GetAIPerceptionComponent();
	if (perception != nullptr) {
		UAISenseConfig_Sight *config = Cast<UAISenseConfig_Sight>(perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()));
		if (config != nullptr) {
			config->SightRadius = radius;
			perception->ConfigureSense(*config);
		}
	}
}

// すでに目撃されたターゲットに気づく最大視野距離を設定する
void AShooterAIController::SetPerceptionLoseSightRadius(float radius)
{
	UAIPerceptionComponent *perception = GetAIPerceptionComponent();
	if (perception != nullptr) {
		UAISenseConfig_Sight *config = Cast<UAISenseConfig_Sight>(perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()));
		if (config != nullptr) {
			config->LoseSightRadius = radius;
			perception->ConfigureSense(*config);
		}
	}
}
