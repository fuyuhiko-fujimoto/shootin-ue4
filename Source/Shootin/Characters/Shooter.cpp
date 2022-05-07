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

    // �J�����̍쐬
    {
        // �J�����̍쐬
        camera_ = CreateDefaultSubobject<UCameraComponent>(cameraName_);
        if (camera_ != nullptr) {

            // �J�����A�[���̍쐬
            cameraArm_ = CreateDefaultSubobject<USpringArmComponent>(cameraArmName_);
            if (cameraArm_ != nullptr) {

                // �A�^�b�`
                cameraArm_->SetupAttachment(RootComponent);
                camera_->SetupAttachment(cameraArm_, USpringArmComponent::SocketName);
            }
        }
    }

    // ����̍쐬
    {
        // �X�P���^�����b�V���̍쐬
        weapon_ = CreateDefaultSubobject<USkeletalMeshComponent>(weaponName_);
    }
}

// Called when the game starts or when spawned
void AShooter::BeginPlay()
{
    Super::BeginPlay();

    // �J�����̐ݒ�
    {
        // �J�����̐ݒ�
        camera_->bUsePawnControlRotation = false;

        // �J�����A�[���̐ݒ�
        cameraArm_->bUsePawnControlRotation = true;
        cameraArm_->bInheritPitch = true;
        cameraArm_->bInheritYaw = true;
        cameraArm_->bInheritRoll = true;
    }

    // �ړ��̐ݒ�
    {
        UCharacterMovementComponent *movement = GetCharacterMovement();
        if (movement != nullptr) {

            // �ړ���������ɃL�����N�^����������
            movement->bOrientRotationToMovement = true;

            // �J�����̕����ɃL�����N�^���������Ȃ�
            bUseControllerRotationPitch = false;
            bUseControllerRotationYaw = false;
            bUseControllerRotationRoll = false;
        }
    }

    // �A�N�V�����̐ݒ�
    {
        UCharacterMovementComponent *movement = GetCharacterMovement();
        if (movement != nullptr) {

            // �ړ��̐ݒ�
            movement->NavAgentProps.bCanWalk = true;
            movement->bCanWalkOffLedges = true;
            movement->MaxWalkSpeed = movement->MaxWalkSpeed * actionRate_;

            // �W�����v�̐ݒ�
            movement->NavAgentProps.bCanJump = true;

            // ���Ⴊ�݂̐ݒ�
            movement->NavAgentProps.bCanCrouch = true;
            movement->bCanWalkOffLedgesWhenCrouching = true;
            movement->MaxWalkSpeedCrouched = movement->MaxWalkSpeedCrouched * actionRate_;

            // ��s�̐ݒ�
            movement->NavAgentProps.bCanFly = false;
            movement->MaxFlySpeed = movement->MaxFlySpeed * actionRate_;

            // ���j�̐ݒ�
            movement->NavAgentProps.bCanSwim = false;
            movement->MaxSwimSpeed = movement->MaxSwimSpeed * actionRate_;
        }
    }

    // ����̐ݒ�
    {
        if (weapon_ != nullptr) {

            // �A�^�b�`
            USkeletalMeshComponent *parent = GetMesh();
            if (parent != nullptr) {
                if (parent->DoesSocketExist(weaponHandleSocketName_)) {
                    weapon_->AttachTo(parent, weaponHandleSocketName_);
                    weapon_->SetRelativeRotation(weaponRotator_);
                }
            }

            // ����̓ǂݍ���
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

    // ����̊��蓖��
    {
        // �ړ�
        PlayerInputComponent->BindAxis("MoveForward", this, &AShooter::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &AShooter::MoveRight);

        // �J�����̐���
        PlayerInputComponent->BindAxis("TurnRate", this, &AShooter::TurnAtRate);
        PlayerInputComponent->BindAxis("Turn", this, &AShooter::Turn);
        PlayerInputComponent->BindAxis("LookUpRate", this, &AShooter::LookUpAtRate);
        PlayerInputComponent->BindAxis("LookUp", this, &AShooter::LookUp);

        // �W�����v
        PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooter::Jump);

        // ���Ⴊ��
        PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooter::ToggleCrouch);
        PlayerInputComponent->BindAction("CrouchDown", IE_Pressed, this, &AShooter::CrouchDown);
        PlayerInputComponent->BindAction("StandUp", IE_Pressed, this, &AShooter::StandUp);

        // ���C
        PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooter::Fire);
        PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooter::StopFiring);
    }

    // �J�����̐ݒ�
    {
        // �J�����̋����̐ݒ�
        cameraArm_->TargetArmLength = cameraDistance_;

        // �J�����̊p�x�̐ݒ�
        if (Controller != nullptr && Controller->IsPlayerController()) {
            APlayerController *playerController = Cast<APlayerController>(Controller);
            playerController->RotationInput.Pitch = cameraPitch_;
        }
    }
}

// �O�i����
void AShooter::MoveForward(float axisValue)
{
    if (Controller != nullptr && !FMath::IsNearlyZero(axisValue) && IsMoving()) {
        FRotator rotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
        FVector direction = FRotationMatrix(rotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, axisValue * moveRate_);
    }
}

// �E�Ɉړ�����
void AShooter::MoveRight(float axisValue)
{
    if (Controller != nullptr && !FMath::IsNearlyZero(axisValue) && IsMoving()) {
        FRotator rotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
        FVector direction = FRotationMatrix(rotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, axisValue * moveRate_);
    }
}

// �ړ��ł��邩�ǂ����𔻒肷��
bool AShooter::IsMoving()
{
    return !firing_ && CanBeDamaged();
}

// ���_�����E����
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

// ���_���㉺����
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

// �W�����v���J�n����
void AShooter::Jump()
{
    if (CanJump()) {
        Super::Jump();
    }
}

// �W�����v���I������
void AShooter::StopJumping()
{
    Super::StopJumping();

}

// ���Ⴊ�݂��J�n����
void AShooter::Crouch(bool bClientSimulation)
{
    Super::Crouch(bClientSimulation);

}

// ���Ⴊ�݂��I������
void AShooter::UnCrouch(bool bClientSimulation)
{
    Super::UnCrouch(bClientSimulation);

}

// ���Ⴊ�݂�؂�ւ���
void AShooter::ToggleCrouch()
{
    if (CanCrouch()) {
        Crouch();
    } else {
        UnCrouch();
    }

}

// ���Ⴊ��
void AShooter::CrouchDown()
{
    if (CanCrouch()) {
        Super::Crouch();
    }
}

// �����オ��
void AShooter::StandUp()
{
    if (!CanCrouch()) {
        Super::UnCrouch();
    }
}

// ���C���J�n����
void AShooter::Fire()
{
    if (CanCrouch()) {
        firing_ = true;
        triggering_ = true;
    }
}

// ���C���I������
void AShooter::StopFiring()
{
    triggering_ = false;
}

// �ꔭ���C����
AActor *AShooter::FireAShot()
{
    // �q�b�g�����A�N�^�̎擾
    AActor *actor = nullptr;
    {
        if (weapon_ != nullptr && weapon_->DoesSocketExist(weaponMuzzleSocketName_)) {

            // �g���[�X���s��
            FTransform transform = weapon_->GetSocketTransform(weaponMuzzleSocketName_, ERelativeTransformSpace::RTS_World);
            FVector start = transform.GetLocation();
            FVector end = start + transform.GetRotation().GetForwardVector() * fireDistance_;
            TArray<AActor *> ignore;
            ignore.Init(this, 1);
            FHitResult result;
            UKismetSystemLibrary::SphereTraceSingle(GetWorld(), start, end, fireRadius_, ETraceTypeQuery::TraceTypeQuery1, false, ignore, EDrawDebugTrace::Type::ForDuration, result, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
            actor = result.GetActor();

            // �q�b�g��������Ƀ_���[�W��^����
            if (actor != nullptr) {
                TSubclassOf<UDamageType> damageType;
                UGameplayStatics::ApplyDamage(actor, 1.0f, Controller, this, damageType);
            }
        }
    }

    // �I��
    return actor;
}

// ���̔��C����������
void AShooter::Fired()
{
    if (!triggering_) {
        firing_ = false;
    }
}

// ���C�����ǂ����𔻒肷��
bool AShooter::IsFiring()
{
    return firing_;
}

// �_���[�W����炤
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

// �_���[�W����������
void AShooter::Damaged()
{
    SetCanBeDamaged(true);
}

