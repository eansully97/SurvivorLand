// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLBaseGameCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SLInputHandlerComponent.h"
#include "SurvivorLandGameplayTags.h" // your native tags namespace
#include "Camera/CameraComponent.h"
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


void ASLBaseGameCharacter::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump) Jump();
}

void ASLBaseGameCharacter::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump) StopJumping();
}