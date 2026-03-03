// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "SLMonsterLinkedAnimLayer.generated.h"

class USLMonsterAnimInstance;
/**
 * 
 */
UCLASS()
class SURVIVORLAND_API USLMonsterLinkedAnimLayer : public USLBasePlayerAnimInstance
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure)
	USLMonsterAnimInstance* GetMonsterAnimInstance() const;
};
