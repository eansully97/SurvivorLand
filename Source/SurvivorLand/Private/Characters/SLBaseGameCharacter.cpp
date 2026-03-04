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

void ASLBaseGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind Input to Combat Component
	if (CombatComponent && InputHandlerComponent)
	{
		CombatComponent->BindToInput(InputHandlerComponent);
	}
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
		return;
	}
}

bool ASLBaseGameCharacter::IsWeaponEquipped() const
{
	if (!CombatComponent) return false;
	return (CombatComponent->GetEquippedWeapon() != nullptr);
}

ASLWeaponBase* ASLBaseGameCharacter::GetEquippedWeapon() const
{
	if (CombatComponent && !CombatComponent->Inventory.IsEmpty() && CombatComponent->Inventory[CombatComponent->EquippedIndex])
	{
		return CombatComponent->Inventory[CombatComponent->EquippedIndex];
	}
	return nullptr;
}

bool ASLBaseGameCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->IsAiming();
}

void ASLBaseGameCharacter::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump)
	{
		Jump();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		CombatComponent->SetAiming(true);
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		// Gun->Fire
	}
}

void ASLBaseGameCharacter::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump)
	{
		StopJumping();
		return;
	}
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		CombatComponent->SetAiming(false);
		return;
	}
}