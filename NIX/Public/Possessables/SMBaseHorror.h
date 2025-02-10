// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Possessables/SMBaseCharacter.h"
#include "SMBaseHorror.generated.h"

class USMHealthAttributeSet;
class USMCombatAttributeSet;
class UStateTree;
class USMStateTreeComponent;
/**
 * 
 */
UCLASS()
class NIX_API ASMBaseHorror : public ACharacter
{
	GENERATED_BODY()


public:

	ASMBaseHorror(const class FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets")
	TObjectPtr<UAnimMontage> MeleeMontage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="StateTrees")
	TObjectPtr<UStateTree> STPatrol;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="StateTrees")
	TObjectPtr<UStateTree> STReact;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="StateTrees")
	TObjectPtr<UStateTree> STMelee;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<USMStateTreeComponent> SMStateTree;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USphereComponent> ReactionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USphereComponent> RangedAttackRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USphereComponent> MeleeAttackRadius;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components)
	TObjectPtr<USMHealthComponent> HealthComp;

	bool bSeingPlayer; 
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly,Category="Stats")
	float MovSpeed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly,Category="Stats")
	float MeleeDMG;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly,Category="Stats")
	int32 SanityLVL;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPoint = -1;

	UPROPERTY(BlueprintReadWrite)
	bool bInsideRangedAttackRadius;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeAttackHandle;
	
	UPROPERTY(BlueprintReadWrite)
	int32 HitCount;

	UPROPERTY(Replicated)
	bool bIsDead;
	
	bool ShouldReact(AActor* Actor) const;
	void ActivateNewStateTree(UStateTree* NewStateTree) const;
	
	UFUNCTION(BlueprintCallable)
	void TryToMelee();
	
	UFUNCTION()
	void ReactSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ReactSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void MeleeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void MeleeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void PerformDeath(AActor* OwningActor);

	TMap<ASMPlayerState*, float> DamageContributors;

protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
#pragma region GAS
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components)
	USMPawnComponent* PawnComponent;

protected:
	// Cached custom PlayerState so we don't have to keep casting.
	/*UPROPERTY()
	class ASMPlayerState* SMPlayerStateCache = nullptr;*/
	/* GAS
	***********************************************************************************/

public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TObjectPtr<USMAbilitySystemComponent> AbilitySystemComponent;
	
	//virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
protected:

	// Default Abilities (for testing) int32 for being level of ability.
	
	// Called when the Ability System Component is all setup on this character.
	// Register with the OnAbilitySystemInitialized delegate and broadcast if condition is already met.
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);
	
	void OnAbilitySystemComponentInitialized();

	void OnAbilitySystemComponentUnInitialized() const;

	void SetupGas();
	void SetupAttributes() const;
	
	/** The GameplayEffect that will be applied to the ASC that will define the starting attributes of this character. */
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StartingAttributes;
	
	UPROPERTY()
	TObjectPtr<USMCharacterAttributeSet> CharacterAttributeSet;
	UPROPERTY()
	TObjectPtr<USMHealthAttributeSet> HealthAttributeSet;
	UPROPERTY()
	TObjectPtr<USMCombatAttributeSet> CombatAttributeSet;

	//@TODO: Add survivorzombie attribute set here?

	/*
	 *		Classes:
	 * To help with how swapping to other types of characters (turning into a zombie for example), we should
	 * have a conceptual system put in place called something like "classes" where you can change "class" and it will
	 * do a bunch of checks and run code that will sort out for example: if turning into zombie from survivor, then
	 * drop all weapons before doing so.
	 *
	 *		Health Component and Teams:
	 * A "health component" should basically be the manager for these classes/teams so that AI and team damage can
	 * be handled properly. The health component will communicate with GAS to help with these instances so that
	 * we don't have raw float values on a component that we have to manage ourselves, we may as well just get
	 * GAS to do that anyway.
	 *
	 *		Equippables deriving from Items
	 * Should equippables derive from "items"? If not, we will have to spawn in items and attach the equippable to
	 * that item so that information carries on over. If it were derived from item, we won't have to spawn in/delete
	 * item actors at runtime each time a survivor drops/picks up an equippapble.
	 */
	
	#pragma endregion GAS

};
