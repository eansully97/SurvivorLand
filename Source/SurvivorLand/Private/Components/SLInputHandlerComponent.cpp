
#include "Components/SLInputHandlerComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Data/SLInputDataConfig.h"

USLInputHandlerComponent::USLInputHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USLInputHandlerComponent::InitializeInput(APlayerController* PC, UEnhancedInputComponent* EnhancedInputComp, const UDataAsset_InputConfig* InputConfig)
{
	if (bInitialized)
	{
		return;
	}

	if (!PC || !EnhancedInputComp || !InputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputHandlerComponent::InitializeInput missing PC/InputComp/InputConfig."));
		return;
	}

	ApplyMappingContext(PC, InputConfig);

	// Bind everything in the config
	for (const FSurvivorLandTaggedInputAction& Entry : InputConfig->TaggedInputActions)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (Entry.ValueType == EInputActionValueType::Axis2D)
		{
			// Triggered fires continuously for axes
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Triggered, this, &USLInputHandlerComponent::HandleAxis2DTriggered, Entry.InputTag);
		}
		else
		{
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Started, this, &USLInputHandlerComponent::HandleActionStarted, Entry.InputTag);
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Completed, this, &USLInputHandlerComponent::HandleActionCompleted, Entry.InputTag);
		}
	}

	bInitialized = true;
}

void USLInputHandlerComponent::BindAdditionalActions(UEnhancedInputComponent* EnhancedInputComp, const TArray<FSurvivorLandTaggedInputAction>& ActionsToBind)
{
	if (!EnhancedInputComp)
	{
		return;
	}

	for (const FSurvivorLandTaggedInputAction& Entry : ActionsToBind)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (Entry.ValueType == EInputActionValueType::Axis2D)
		{
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Triggered, this, &USLInputHandlerComponent::HandleAxis2DTriggered, Entry.InputTag);
		}
		else
		{
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Started, this, &USLInputHandlerComponent::HandleActionStarted, Entry.InputTag);
			EnhancedInputComp->BindAction(Entry.InputAction, ETriggerEvent::Completed, this, &USLInputHandlerComponent::HandleActionCompleted, Entry.InputTag);
		}
	}
}

void USLInputHandlerComponent::ApplyMappingContext(APlayerController* PC, const UDataAsset_InputConfig* InputConfig)
{
	if (!InputConfig || !InputConfig->DefaultMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputHandlerComponent: No DefaultMappingContext on InputConfig."));
		return;
	}

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(InputConfig->DefaultMappingContext, /*Priority*/ 0);
		}
	}
}

void USLInputHandlerComponent::HandleActionStarted(FGameplayTag InputTag)
{
	OnActionStarted.Broadcast(InputTag);
}

void USLInputHandlerComponent::HandleActionCompleted(FGameplayTag InputTag)
{
	OnActionCompleted.Broadcast(InputTag);
}

void USLInputHandlerComponent::HandleAxis2DTriggered(const FInputActionValue& Value, FGameplayTag InputTag)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	OnAxis2D.Broadcast(InputTag, Axis);
}