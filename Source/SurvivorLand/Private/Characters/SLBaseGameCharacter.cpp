// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLBaseGameCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SLInputHandlerComponent.h"
#include "SurvivorLandGameplayTags.h" // your native tags namespace


ASLBaseGameCharacter::ASLBaseGameCharacter()
{
	InputHandlerComponent = CreateDefaultSubobject<USLInputHandlerComponent>(TEXT("InputHandlerComponent"));
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
		AddControllerYawInput(Value.X);
		AddControllerPitchInput(Value.Y);
		return;
	}
}

void ASLBaseGameCharacter::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump)
	{
		Jump();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		UE_LOG(LogTemp, Log, TEXT("Survivor Fire pressed"));
		return;
	}
}

void ASLBaseGameCharacter::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Jump)
	{
		StopJumping();
		return;
	}
}