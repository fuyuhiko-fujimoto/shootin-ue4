// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ShootinCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "Shooter.generated.h"


/**
 *
 */
UCLASS()
class SHOOTIN_API AShooter : public AShootinCharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AShooter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // カメラ
protected:
    const FName cameraName_ = FName(TEXT("Camera"));
    const FName cameraArmName_ = FName(TEXT("CameraArm"));
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
        UCameraComponent *camera_;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
        USpringArmComponent *cameraArm_;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
        float cameraDistance_ = 900.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
        float cameraPitch_ = -45.0f;

    // アクション
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
        float actionRate_ = 1.0f;

    // 移動
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
        float moveRate_ = 1.0f;
protected:
    void MoveForward(float axisValue);
    void MoveRight(float axisValue);
    bool IsMoving();

    // 視野
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
        float turnRate_ = 45.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
        float lookUpRate_ = 45.0f;
protected:
    void TurnAtRate(float axisValue);
    void Turn(float axisValue);
    void LookUpAtRate(float axisValue);
    void LookUp(float axisValue);

    // ジャンプ
protected:
    virtual void Jump() override;
    virtual void StopJumping() override;

    // しゃがみ
protected:
    virtual void Crouch(bool bClientSimulation = false) override;
    virtual void UnCrouch(bool bClientSimulation = false) override;
    void ToggleCrouch();
    void CrouchDown();
    void StandUp();

    // 発砲
protected:
    bool firing_ = false;
    bool triggering_ = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire")
        float fireDistance_ = 10000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire")
        float fireRadius_ = 15.0f;
protected:
    void Fire();
    void StopFiring();
    UFUNCTION(BlueprintCallable, Category = "Fire")
        AActor *FireAShot();
    UFUNCTION(BlueprintCallable, Category = "Fire")
        void Fired();
    UFUNCTION(BlueprintCallable, Category = "Fire")
        bool IsFiring();

    // ダメージ
protected:
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage")
        float Damage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser);
    UFUNCTION(BlueprintCallable, Category = "Damage")
        void Damaged();

    // 武器
protected:
    const FName weaponName_ = FName(TEXT("Weapon"));
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        FName weaponHandleSocketName_ = FName(TEXT("socket_weapon"));
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        FName weaponMuzzleSocketName_ = FName(TEXT("socket_muzzle"));
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        USkeletalMeshComponent *weapon_ = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        FRotator weaponRotator_ = FRotator(0.0f, 90.0f, 0.0f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        USkeletalMesh *weaponMesh_ = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
        FString weaponMeshDefaultPath_ = FString(TEXT("SkeletalMesh'/Game/Characters/Models/Weapons/Meshes/Ka47/SK_KA47.SK_KA47'"));
    TSharedPtr<FStreamableHandle> weaponMeshLoadHandle_;


};
