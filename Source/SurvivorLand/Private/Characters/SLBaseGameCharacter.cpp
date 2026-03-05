// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLBaseGameCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SLInputHandlerComponent.h"
#include "SurvivorLandGameplayTags.h" // your native tags namespace
#include "Camera/CameraComponent.h"
#include "Components/SLCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASLBaseGameCharacter::ASLBaseGameCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->SocketOffset = FVector(0.f,55.f,65.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InputHandlerComponent = CreateDefaultSubobject<USLInputHandlerComponent>(TEXT("InputHandlerComponent"));
	CombatComponent = CreateDefaultSubobject<USLCombatComponent>(TEXT("CombatComponent"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

}

FName ASLBaseGameCharacter::GetWeaponAttachSocket(ESLWeaponGrip Grip) const
{
	const UEnum* EnumPtr = StaticEnum<ESLWeaponGrip>();
	if (!EnumPtr) return TEXT("RightHandSocket_Pistol");

	const FString GripName = EnumPtr->GetNameStringByValue((int64)Grip);
	const FName Socket(*FString::Printf(TEXT("RightHandSocket_%s"), *GripName));

	return GetMesh() && GetMesh()->DoesSocketExist(Socket)
		? Socket
		: TEXT("RightHandSocket_Pistol");
}

void ASLBaseGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!EnhancedComp || !PC || !InputHandlerComponent || !InputConfig)
	{
		return;
	}

	// Subscribe to events
	InputHandlerComponent->OnAxis2D.AddDynamic(this, &ASLBaseGameCharacter::HandleAxis2D);
	InputHandlerComponent->OnActionStarted.AddDynamic(this, &ASLBaseGameCharacter::HandleActionStarted);
	InputHandlerComponent->OnActionCompleted.AddDynamic(this, &ASLBaseGameCharacter::HandleActionCompleted);

	// Initialize bindings
	InputHandlerComponent->InitializeInput(PC, EnhancedComp, InputConfig);
}

void ASLBaseGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAimTarget(DeltaTime);
	UpdateTurnInPlace(DeltaTime);
}

void ASLBaseGameCharacter::UpdateAimTarget(float DeltaSeconds)
{
	if (!IsLocallyControlled()) return;

	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector Start = CamLoc;
	const FVector End = Start + CamRot.Vector() * 100000.f;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AimTrace), false, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	const FVector Target = bHit ? Hit.ImpactPoint : End;

	AimTargetWorld = Target;
	AimTargetWorldSmoothed = FMath::VInterpTo(AimTargetWorldSmoothed, Target, DeltaSeconds, 15.f);
}

void ASLBaseGameCharacter::SetStrafeAimingMode(bool bEnable)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	if (bEnable)
	{
		// cache once when entering
		bCachedOrientToMovement = Move->bOrientRotationToMovement;
		bCachedUseControllerYaw = bUseControllerRotationYaw;
		CachedRotationRateYaw   = Move->RotationRate.Yaw;

		Move->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

		// tune feel
		Move->RotationRate.Yaw = 900.f; // 720–1080 feels good for ADS
	}
	else
	{
		Move->bOrientRotationToMovement = bCachedOrientToMovement;
		bUseControllerRotationYaw = bCachedUseControllerYaw;
		Move->RotationRate.Yaw = CachedRotationRateYaw;
	}
}

void ASLBaseGameCharacter::UpdateTurnInPlace(float DeltaSeconds)
{
    if (!IsLocallyControlled() || !Controller) return;
    if (!IsAiming()) return;

    // only turn in place when basically standing still
    const float Speed2D = GetVelocity().Size2D();
	if (Speed2D > 10.f) return;

    const float YawOffset = GetAimYawOffset();
    const float AbsYaw = FMath::Abs(YawOffset);

    // thresholds you can tune
    const float StartTurnThreshold = 75.f;   // start turning past this
    const float StopTurnThreshold  = 35.f;   // stop once back under this

	if (!bTurningInPlace && AbsYaw > StartTurnThreshold) bTurningInPlace = true;
	if (bTurningInPlace && AbsYaw < StopTurnThreshold)   bTurningInPlace = false;
	if (!bTurningInPlace) return;

    // rotate actor toward control yaw smoothly
    const float TurnSpeedDegPerSec = 200.f; // tune: 180–360
    const float Step = TurnSpeedDegPerSec * DeltaSeconds;

    const float DesiredYaw = Controller->GetControlRotation().Yaw;
    const float NewYaw = FMath::FixedTurn(GetActorRotation().Yaw, DesiredYaw, Step);

    SetActorRotation(FRotator(0.f, NewYaw, 0.f));
}

void ASLBaseGameCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ASLBaseGameCharacter::BindToInput(USLInputHandlerComponent* InputHandler)
{
	if (!InputHandler || bInputBound) return;

	InputHandler->OnActionStarted.AddDynamic(this, &ThisClass::HandleActionStarted);
	InputHandler->OnActionCompleted.AddDynamic(this, &ThisClass::HandleActionCompleted);
	InputHandler->OnAxis2D.AddDynamic(this, &ThisClass::HandleAxis2D);

	bInputBound = true;
}

float ASLBaseGameCharacter::GetAimYawOffset() const
{
	if (!Controller) return 0.f;

	const float ControlYaw = Controller->GetControlRotation().Yaw;
	const float ActorYaw   = GetActorRotation().Yaw;

	// normalized to [-180, 180]
	return FRotator::NormalizeAxis(ControlYaw - ActorYaw);
}

void ASLBaseGameCharacter::HandleAxis2D(FGameplayTag InputTag, FVector2D Value)
{
	// Move
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Move)
	{
		if (Controller)
		{
			const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
			const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
			const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

			AddMovementInput(Forward, Value.Y);
			AddMovementInput(Right,   Value.X);
		}
		return;
	}
	// Look
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Look)
	{
		AddControllerYawInput(Value.X * LookSensitivity);
		AddControllerPitchInput(Value.Y * LookSensitivity);
	}
}

bool ASLBaseGameCharacter::IsWeaponEquipped() const
{
	if (!CombatComponent) return false;
	return (CombatComponent->GetEquippedWeapon() != nullptr);
}

ASLWeaponBase* ASLBaseGameCharacter::GetEquippedWeapon() const
{
	if (!CombatComponent) return nullptr;

	const int32 Index = CombatComponent->EquippedIndex;
	if (!CombatComponent->Inventory.IsValidIndex(Index)) return nullptr;

	return CombatComponent->Inventory[Index].Get();
}

USLCombatComponent* ASLBaseGameCharacter::GetCombatComponent() const
{
	return CombatComponent;
}

bool ASLBaseGameCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->IsAiming();
}

void ASLBaseGameCharacter::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump) Jump();
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim) CombatComponent->SetAiming(true);
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire) CombatComponent->FirePressed();
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Drop) CombatComponent->DropEquippedWeapon();
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact) CombatComponent->TryInteract();
}

void ASLBaseGameCharacter::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump) StopJumping();
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim) CombatComponent->SetAiming(false);

}