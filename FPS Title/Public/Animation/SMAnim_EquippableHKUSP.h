// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SMAnim_EquippableHKUSP.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMAnim_EquippableHKUSP : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bSlideback;
	
};
