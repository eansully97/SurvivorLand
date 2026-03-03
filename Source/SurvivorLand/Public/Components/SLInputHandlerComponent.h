#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Data/SLInputDataConfig.h"
#include "SLInputHandlerComponent.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UDataAsset_InputConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaggedActionEvent, FGameplayTag, InputTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaggedAxis2DEvent, FGameplayTag, InputTag, FVector2D, Value);

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLInputHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLInputHandlerComponent();

	/** Call from pawn/character when possessed or in SetupPlayerInputComponent. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void InitializeInput(class APlayerController* PC, UEnhancedInputComponent* EnhancedInputComp, const UDataAsset_InputConfig* InputConfig);

	UPROPERTY(BlueprintAssignable, Category="Input")
	FOnTaggedActionEvent OnActionStarted;

	UPROPERTY(BlueprintAssignable, Category="Input")
	FOnTaggedActionEvent OnActionCompleted;

	UPROPERTY(BlueprintAssignable, Category="Input")
	FOnTaggedAxis2DEvent OnAxis2D;

	UFUNCTION(BlueprintCallable, Category="SL|Input")
	void BindAdditionalActions(UEnhancedInputComponent* EnhancedInputComp, const TArray<FSurvivorLandTaggedInputAction>& ActionsToBind);

private:
	// Internal handlers bound to Enhanced Input
	void HandleActionStarted(FGameplayTag InputTag);
	void HandleActionCompleted(FGameplayTag InputTag);
	void HandleAxis2DTriggered(const FInputActionValue& Value, FGameplayTag InputTag);

	void ApplyMappingContext(APlayerController* PC, const UDataAsset_InputConfig* InputConfig);

	bool bInitialized = false;
};