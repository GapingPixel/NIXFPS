// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SMAbilitySystemComponent.generated.h"

class USMGameplayAbility;
/** Data about montages that were played locally (all montages in case of server. predictive montages in case of client). Never replicated directly. */
USTRUCT()
struct NIX_API FGameplayAbilityLocalAnimMontageForMesh
{
	GENERATED_BODY();

public:
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;

	FGameplayAbilityLocalAnimMontageForMesh() : Mesh(nullptr), LocalMontageInfo()
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh)
		: Mesh(InMesh), LocalMontageInfo()
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh, FGameplayAbilityLocalAnimMontage& InLocalMontageInfo)
		: Mesh(InMesh), LocalMontageInfo(InLocalMontageInfo)
	{
	}
};

/* Struct to keep information about an ability and it's InputID grouped together, along with
 * the pressed and release handles for unbinding later on.
 */
struct FAbilityInput
{
	FAbilityInput(): InputID(0), PressedHandle(0), ReleasedHandle(0), CancelledHandle(0)
	{
	};
	
	FGameplayAbilitySpecHandle Handle;
	
	int32 InputID;

	int32 PressedHandle;
	int32 ReleasedHandle;
	int32 CancelledHandle;

	bool operator==(const FAbilityInput& other) const
	{
		return (other.Handle == Handle);
	}
};

/**
* Data about montages that is replicated to simulated clients.
*/
USTRUCT()
struct NIX_API FGameplayAbilityRepAnimMontageForMesh
{
	GENERATED_BODY();

public:
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityRepAnimMontage RepMontageInfo;

	FGameplayAbilityRepAnimMontageForMesh() : Mesh(nullptr), RepMontageInfo()
	{
	}

	FGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh)
		: Mesh(InMesh), RepMontageInfo()
	{
	}
};

/**
 * 
 */
UCLASS()
class USMAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	float PlayMontageForMesh(USkeletalMeshComponent* Mesh, USMGameplayAbility* AnimatingAbility,
	                         FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate = 1.0f,
	                         FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f, bool bReplicateMontage = true);

	// Plays a montage without updating replication/prediction structures. Used by simulated proxies when replication tells them to play a montage.
	virtual float PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None);

	// Stops whatever montage is currently playing. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
	virtual void CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	// Returns the current animating ability
	UGameplayAbility* GetAnimatingAbilityFromAnyMesh();

	// Returns the montage that is playing for the mesh
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	FGameplayAbilityLocalAnimMontageForMesh& GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* Mesh);

	// Finds the existing FGameplayAbilityRepAnimMontageForMesh for the mesh or creates one if it doesn't exist
	FGameplayAbilityRepAnimMontageForMesh& GetGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh);

	// Copy LocalAnimMontageInfo into RepAnimMontageInfo
	void AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh);
	void AnimMontage_UpdateReplicatedDataForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);

	/** Called when a prediction key that played a montage is rejected */
	void OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage);

	bool IsReadyForReplicatedMontageForMesh(USkeletalMeshComponent* InMesh);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	
	UPROPERTY()
	TArray<FGameplayAbilityLocalAnimMontageForMesh> LocalAnimMontageInfoForMeshes;

	// Data structure for replicating montage info to simulated clients
	// Will be max one element per skeletal mesh on the AvatarActor
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh)
	TArray<FGameplayAbilityRepAnimMontageForMesh> RepAnimMontageInfoForMeshes;

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontageForMesh();


	
protected:
	UEnhancedInputComponent* GetInputComponent() const;

	int32 GetNextInputID()
	{
		static int32 InputID;
		InputID += 1;
		return InputID;
	};
	
private:
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> OwnerPlayerInputComponent;
	
	int32 InvalidInputID = INDEX_NONE;

	// Array to keep track of what InputID is bound to what ability.
	TArray<FAbilityInput> AbilityInputArray;
	
};
