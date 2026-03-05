// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "SLMonsterCharacter.generated.h"

class USLMonsterCombatComponent;

UCLASS()
class SURVIVORLAND_API ASLMonsterCharacter : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:
	ASLMonsterCharacter();
	
protected:

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLMonsterCombatComponent> MonsterCombatComponent;

};
