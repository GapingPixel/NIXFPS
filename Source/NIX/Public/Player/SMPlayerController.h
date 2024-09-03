// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMWPlayerHUD;
class SDialogWidget;
class UInputMappingContext;
struct FInputActionValue;
class UInputAction;
/**
 * 
 */
UCLASS()
class NIX_API ASMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	

	/*UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	TSubclassOf<UUserWidget> PlayerHUDReference; */
	/**Applies recoil to the camera.
	@param RecoilAmount the amount to recoil by. X is the yaw, Y is the pitch
	@param RecoilSpeed the speed to bump the camera up per second from the recoil
	@param RecoilResetSpeed the speed the camera will return to center at per second after the recoil is finished
	@param Shake an optional camera shake to play with the recoil*/
	void ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed);
	void Turn(float Value);
	void LookUp(float Value);
	
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> PlayerHUDReference;
	
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	//TSharedPtr<SDialogWidget> DialogWidget;
	
	//The amount of recoil to apply. We store this in a variable as we smoothly apply the recoil over several frames
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilBumpAmount;

	//The amount of recoil the gun has had, that we need to reset (After shooting we slowly want the recoil to return to normal.)
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilResetAmount;

	//The speed at which the recoil bumps up per second
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float CurrentRecoilSpeed;

	//The speed at which the recoil resets per second
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float CurrentRecoilResetSpeed;

	//The last time that we applied recoil
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float LastRecoilTime;
};
