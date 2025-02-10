// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_GrabObject.h"

#include "Actors/BarricadeClasses.h"
#include "Actors/SMInteractableActor.h"
#include "Actors/SMLiftableObject.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "Possessables/SMPlayerCharacter.h"

void USMGA_GrabObject::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	Player = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (Player->CanInteractWithBarricadeObject())
	{
		Player->CurrentBarricadeObject->Interact();
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return;
	}
	
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		if (!Player->CanGrabObject())
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		}
		return;
	}
	if (Player->GrabbedObject != nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return;
	}
	GrabbedObject = Player->CanGrabObject();
	if (GrabbedObject == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
	} else
	{
		Player->InventoryComponent->UnEquipCurrent();
		Player->InventoryComponent->UnEquipCurrentClient();
		Player->HealthComp->StopStaminaRegeneration();
		GetWorld()->GetTimerManager().SetTimer(StaminaHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{//Stamina Cost
			USMHealthComponent* HealthC = Player->HealthComp;
			const float CurrentStamina = HealthC->GetStamina();
			const auto AbilitySystem = Player->GetAbilitySystemComponent();
			const FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
			const FGameplayEffectSpecHandle StaminaCostSpec = AbilitySystem->MakeOutgoingSpec(GEStaminaCost, 1, EffectContext);
			const float ActualStaminaCost = FMath::Min(StaminaCostPerSecond, CurrentStamina);
			StaminaCostSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost")), ActualStaminaCost);
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*StaminaCostSpec.Data.Get());
			HealthC->OnStaminaChanged.Broadcast(HealthC, CurrentStamina, HealthC->GetStamina(), nullptr);
			const float NewStamina = HealthC->GetStamina();
			
			if (NewStamina < FMath::Abs(StaminaCostPerSecond))
			{
				EndGrab();
			}
		}), 0.33f, true);

		if (GrabbedObject->IsA<ASMLiftableObject>())
		{
				Cast<ASMLiftableObject>(GrabbedObject)->AddPlayerInteraction(Player);
				GetWorld()->GetTimerManager().SetTimer(LiftableHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					FVector CameraLocation = Player->FirstPersonCamera->GetComponentLocation();
					FRotator CameraRotation = Player->FirstPersonCamera->GetComponentRotation();
					Player->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
					// Get the direction the player is looking
					const FVector ForwardVector = CameraRotation.Vector();
					// Calculate the vector from the player camera to the object
					const FVector ObjectLocation = GrabbedObject->GetActorLocation();
					FVector ToObject = ObjectLocation - CameraLocation;
					ToObject.Normalize();
					// If the dot product of the forward vector and the vector to the object is above a threshold,
					// it means the player is looking at the object
					const float DotProduct = FVector::DotProduct(ForwardVector, ToObject);
					if  (DotProduct < 0.7f)
					{
						EndGrab();
					} 
				}), 0.4, true);
		} else
		{
			Cast<ASMInteractableActor>(GrabbedObject)->UpdateCollisionForPlayerHandling();
		}
	}
}

void USMGA_GrabObject::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("Grab Pressed"));
	EndGrab();
	
}

void USMGA_GrabObject::EndGrab_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaHandle);
	if (GrabbedObject)
	{
		Player->DropObject();
		Player->HealthComp->StartStaminaRegeneration();
		if (ASMLiftableObject* Liftable = Cast<ASMLiftableObject>(GrabbedObject))
		{
			Liftable->RemovePlayerInteraction(Player);
			GetWorld()->GetTimerManager().ClearTimer(LiftableHandle);
		} else
		{
			Cast<ASMInteractableActor>(GrabbedObject)->UpdateCollisionForWorld();
		}
	}
	K2_EndAbility();
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Blue, TEXT("END GRAB"));
}
