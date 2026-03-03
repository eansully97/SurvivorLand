// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "SLSurvivorLinkedAnimLayer.generated.h"

class USLSurvivorAnimInstance;
/**
 * 
 */
UCLASS()
class SURVIVORLAND_API USLSurvivorLinkedAnimLayer : public USLBasePlayerAnimInstance
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure)
	USLSurvivorAnimInstance* GetSurvivorAnimInstance() const;
};
