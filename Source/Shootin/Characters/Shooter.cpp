// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values
AShooter::AShooter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // カメラの作成
    {
        // カメラの作成
        camera_ = CreateDefaultSubobject<UCameraComponent>(cameraName_);
        if (camera_ != nullptr) {

            // カメラアームの作成
            cameraArm_ = CreateDefaultSubobject<USpringArmComponent>(cameraArmName_);
            if (cameraArm_ != nullptr) {

                // アタッチ
                cameraArm_->SetupAttachment(RootComponent);
                camera_->SetupAttachment(cameraArm_, USpringArmComponent::SocketName);
            }
        }
    }

    // 武器の作成
    {
        // スケルタルメッシュの作成
        weapon_ = CreateDefaultSubobject<USkeletalMeshComponent>(weaponName_);
    }
}

// Called when the game starts or when spawned
void AShooter::BeginPlay()
{
    Super::BeginPlay();

    // カメラの設定
    {
        // カメラの設定
        camera_->bUsePawnControlRotation = false;

        // カメラアームの設定
        cameraArm_->bUsePawnControlRotation = true;
        cameraArm_->bInheritPitch = true;
        cameraArm_->bInheritYaw = true;
        cameraArm_->bInheritRoll = true;
    }

    // 移動の設定
    {
        UCharacterMovementComponent *movement = GetCharacterMovement();
        if (movement != nullptr) {

            // 移動する方向にキャラクタを向かせる
            movement->bOrientRotationToMovement = true;

            // カメラの方向にキャラクタを向かせない
            bUseControllerRotationPitch = false;
            bUseControllerRotationYaw = false;
            bUseControllerRotationRoll = false;
        }
    }

    // アクションの設定
    {
        UCharacterMovementComponent *movement = GetCharacterMovement();
        if (movement != nullptr) {

            // 移動の設定
            movement->NavAgentProps.bCanWalk = true;
            movement->bCanWalkOffLedges = true;
            movement->MaxWalkSpeed = movement->MaxWalkSpeed * actionRate_;

            // ジャンプの設定
            movement->NavAgentProps.bCanJump = true;

            // しゃがみの設定
            movement->NavAgentProps.bCanCrouch = true;
            movement->bCanWalkOffLedgesWhenCrouching = true;
            movement->MaxWalkSpeedCrouched = movement->MaxWalkSpeedCrouched * actionRate_;

            // 飛行の設定
            movement->NavAgentProps.bCanFly = false;
            movement->MaxFlySpeed = movement->MaxFlySpeed * actionRate_;

            // 水泳の設定
            movement->NavAgentProps.bCanSwim = false;
            movement->MaxSwimSpeed = movement->MaxSwimSpeed * actionRate_;
        }
    }

    // 武器の設定
    {
        if (weapon_ != nullptr) {

            // アタッチ
            USkeletalMeshComponent *parent = GetMesh();
            if (parent != nullptr) {
                if (parent->DoesSocketExist(weaponHandleSocketName_)) {
                    weapon_->AttachTo(parent, weaponHandleSocketName_);
                    weapon_->SetRelativeRotation(weaponRotator_);
                }
            }

            // 武器の読み込み
            if (weaponMesh_ != nullptr) {
                weapon_->SetSkeletalMesh(weaponMesh_);
            } else {
                FStreamableManager streamableManager;
                weaponMeshLoadHandle_ = streamableManager.RequestAsyncLoad(FStringAssetReference(weaponMeshDefaultPath_), [this]() {
                    FStreamableHandle *handle = weaponMeshLoadHandle_.Get();
                    if (handle != nullptr) {
                        weaponMesh_ = Cast<USkeletalMesh>(handle->GetLoadedAsset());
                        if (weaponMesh_ != nullptr) {
                            weapon_->SetSkeletalMesh(weaponMesh_);
                        }
                    }
                });
            }
        }
    }
}

// Called every frame
void AShooter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 操作の割り当て
    {
        // 移動
        PlayerInputComponent->BindAxis("MoveForward", this, &AShooter::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &AShooter::MoveRight);

        // カメラの制御
        PlayerInputComponent->BindAxis("TurnRate", this, &AShooter::TurnAtRate);
        PlayerInputComponent->BindAxis("Turn", this, &AShooter::Turn);
        PlayerInputComponent->BindAxis("LookUpRate", this, &AShooter::LookUpAtRate);
        PlayerInputComponent->BindAxis("LookUp", this, &AShooter::LookUp);

        // ジャンプ
        PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooter::Jump);

        // しゃがみ
        PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooter::ToggleCrouch);
        PlayerInputComponent->BindAction("CrouchDown", IE_Pressed, this, &AShooter::CrouchDown);
        PlayerInputComponent->BindAction("StandUp", IE_Pressed, this, &AShooter::StandUp);

        // 発砲
        PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooter::Fire);
        PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooter::StopFiring);
    }

    // カメラの設定
    {
        // カメラの距離の設定
        cameraArm_->TargetArmLength = cameraDistance_;

        // カメラの角度の設定
        if (Controller != nullptr && Controller->IsPlayerController()) {
            APlayerController *playerController = Cast<APlayerController>(Controller);
            playerController->RotationInput.Pitch = cameraPitch_;
        }
    }
}

// 前進する
void AShooter::MoveForward(float axisValue)
{
    if (Controller != nullptr && !FMath::IsNearlyZero(axisValue) && IsMoving()) {
        FRotator rotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
        FVector direction = FRotationMatrix(rotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, axisValue * moveRate_);
    }
}

// 右に移動する
void AShooter::MoveRight(float axisValue)
{
    if (Controller != nullptr && !FMath::IsNearlyZero(axisValue) && IsMoving()) {
        FRotator rotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
        FVector direction = FRotationMatrix(rotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, axisValue * moveRate_);
    }
}

// 移動できるかどうかを判定する
bool AShooter::IsMoving()
{
    return !firing_ && CanBeDamaged();
}

// 視点を左右する
void AShooter::TurnAtRate(float axisValue)
{
    if (!FMath::IsNearlyZero(axisValue)) {
        AddControllerYawInput(axisValue * turnRate_ * GetWorld()->GetDeltaSeconds());
    }
}
void AShooter::Turn(float axisValue)
{
    if (!FMath::IsNearlyZero(axisValue)) {
        AddControllerYawInput(axisValue);
    }
}

// 視点を上下する
void AShooter::LookUpAtRate(float axisValue)
{
    if (!FMath::IsNearlyZero(axisValue)) {
        AddControllerPitchInput(axisValue * lookUpRate_ * GetWorld()->GetDeltaSeconds());
    }
}
void AShooter::LookUp(float axisValue)
{
    if (!FMath::IsNearlyZero(axisValue)) {
        AddControllerPitchInput(axisValue);
    }
}

// ジャンプを開始する
void AShooter::Jump()
{
    if (CanJump()) {
        Super::Jump();
    }
}

// ジャンプを終了する
void AShooter::StopJumping()
{
    Super::StopJumping();

}

// しゃがみを開始する
void AShooter::Crouch(bool bClientSimulation)
{
    Super::Crouch(bClientSimulation);

}

// しゃがみを終了する
void AShooter::UnCrouch(bool bClientSimulation)
{
    Super::UnCrouch(bClientSimulation);

}

// しゃがみを切り替える
void AShooter::ToggleCrouch()
{
    if (CanCrouch()) {
        Crouch();
    } else {
        UnCrouch();
    }

}

// しゃがむ
void AShooter::CrouchDown()
{
    if (CanCrouch()) {
        Super::Crouch();
    }
}

// 立ち上がる
void AShooter::StandUp()
{
    if (!CanCrouch()) {
        Super::UnCrouch();
    }
}

// 発砲を開始する
void AShooter::Fire()
{
    if (CanCrouch()) {
        firing_ = true;
        triggering_ = true;
    }
}

// 発砲を終了する
void AShooter::StopFiring()
{
    triggering_ = false;
}

// 一発発砲した
AActor *AShooter::FireAShot()
{
    // ヒットしたアクタの取得
    AActor *actor = nullptr;
    {
        if (weapon_ != nullptr && weapon_->DoesSocketExist(weaponMuzzleSocketName_)) {

            // トレースを行う
            FTransform transform = weapon_->GetSocketTransform(weaponMuzzleSocketName_, ERelativeTransformSpace::RTS_World);
            FVector start = transform.GetLocation();
            FVector end = start + transform.GetRotation().GetForwardVector() * fireDistance_;
            TArray<AActor *> ignore;
            ignore.Init(this, 1);
            FHitResult result;
            UKismetSystemLibrary::SphereTraceSingle(GetWorld(), start, end, fireRadius_, ETraceTypeQuery::TraceTypeQuery1, false, ignore, EDrawDebugTrace::Type::ForDuration, result, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
            actor = result.GetActor();

            // ヒットした相手にダメージを与える
            if (actor != nullptr) {
                TSubclassOf<UDamageType> damageType;
                UGameplayStatics::ApplyDamage(actor, 1.0f, Controller, this, damageType);
            }
        }
    }

    // 終了
    return actor;
}

// 一回の発砲が完了した
void AShooter::Fired()
{
    if (!triggering_) {
        firing_ = false;
    }
}

// 発砲中かどうかを判定する
bool AShooter::IsFiring()
{
    return firing_;
}

// ダメージを喰らう
float AShooter::TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
    return Damage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
float AShooter::Damage_Implementation(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
    float damage = 0.0f;
    if (CanBeDamaged()) {
        damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
        SetCanBeDamaged(false);
    }
    return damage;
}

// ダメージが完了した
void AShooter::Damaged()
{
    SetCanBeDamaged(true);
}

