// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SMHealthComponent.h"
#include "SMPlayerAttributesComponent.generated.h"

class AMonsterHallucination;
class ASM_FalsePickUp;
class ASMPlayerCharacter;
class USMSanityAttributeSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSMSanity_AttributeChanged, USMPlayerAttributesComponent*, HealthComponent, float,
                                              OldValue, float, NewValue, AActor*, Instigator);
/**
 * 
 */

UENUM(BlueprintType)
enum class EHallucinationEvent : uint8
{
	None UMETA(DisplayName = "None"),
	//Auditory
	Footsteps UMETA(DisplayName = "Footsteps"),
	ImpactNoises UMETA(DisplayName = "Impact Noises"),
	ZombieNoises UMETA(DisplayName = "Zombie Noises"),
	CameraZombieNoise UMETA(DisplayName = "Camera Zombie Noise"),

	//Visual
	FakeMonster UMETA(DisplayName = "FakeMonster"),
	FakeDamageMarker UMETA(DisplayName = "FakeDamageMarker"),
	FakePickUp UMETA(DisplayName = "FakePickUp"),

	//Mechanic
	GunJam UMETA(DisplayName = "Gun Jam"),
	Insomnia UMETA(DisplayName = "Insomnia"),
	Jitter UMETA(DisplayName = "Jitter"),
	Suicide UMETA(DisplayName = "Suicide")
};

USTRUCT(BlueprintType)
struct FSanityEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name; // Name of the event

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DurationRangeMinutes; // Duration range (min, max)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRunning = false; // Whether the event is active
};

USTRUCT(BlueprintType)
struct FHallucinationType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSanityEvent> Events; // List of events for this hallucination type

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NextEventTime = 0.0f; // Time until the next event (if needed)
};

USTRUCT(BlueprintType)
struct FHallucinationTypeParameters
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BucketIndex;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<EHallucinationEvent> Events;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FIntPoint SimultaneousEventsRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CooldownTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ZeroRollRatio;
	
};

USTRUCT(BlueprintType)
struct FSanityEventBucket
{
	GENERATED_BODY()

	int8 BucketIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHallucinationTypeParameters Auditory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHallucinationTypeParameters Visual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHallucinationTypeParameters Mechanic;
	//FSanityEventBucket() = default;
	//FSanityEventBucket(int8 BucketIndex, FHallucinationTypeParameters HallucinationTypeParameters, FHallucinationTypeParameters HallucinationTypeParameters1, FHallucinationTypeParameters HallucinationTypeParameters2);
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NIX_API USMPlayerAttributesComponent : public USMHealthComponent
{
	GENERATED_BODY()
	
public:
	USMPlayerAttributesComponent();
	
	UPROPERTY(BlueprintAssignable)
	FSMSanity_AttributeChanged OnSanityChanged;
	
	UPROPERTY(BlueprintAssignable)
	FSMSanity_AttributeChanged OnMaxSanityChanged;

	void ResetSanity();
	
	UFUNCTION(BlueprintCallable, Category = "NIX|Stamina")
	float GetSanity() const;

	UFUNCTION(BlueprintCallable, Category = "NIX|Stamina")
	float GetMaxSanity() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityRegenGE;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityResetGE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityCostGE;

	virtual void InitializeWithAbilitySystemComponent(USMAbilitySystemComponent* InASC) override;
	virtual void UninitializeComponent() override;
protected:

	
	
	virtual void HandleSanityChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxSanityChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleOutOfSanity(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

	FTimerHandle LowSanityHandle;

	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> PlayerCharacterIns;
public:
	UPROPERTY()
	const USMSanityAttributeSet* SanitySet;
	
	void StartSanityRegeneration();
	
	FTimerHandle SanityRegenTimerHandle;
	
	void StopSanityRegeneration();

private:
	

	FTimerHandle AuditoryCooldownHandle;
	FTimerHandle VisualCooldownHandle;
	FTimerHandle MechanicCooldownHandle;

	void EvaluateSanity();
	void RollHallucinations(const FHallucinationTypeParameters& Parameters, const FString& Type,const int8 BucketIndex);
	bool IsCooldownActive(FTimerHandle& CooldownHandle) const;
	void StartCooldown(FTimerHandle& CooldownHandle, float CooldownTime) const;

	void TriggerHallucinations(const FString& Type, int32 NumEvents, const int32 BucketIndex);

	void ProcessSanityEvent(const EHallucinationEvent& Event);
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FSanityEventBucket> SanityBuckets;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FSanityEvent> AuditoryEvents;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FSanityEvent> VisualEvents;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FSanityEvent> MechanicEvents;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FHallucinationType AuditoryHallucination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FHallucinationType VisualHallucination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FHallucinationType MechanicHallucination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ASM_FalsePickUp> FalsePickUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMonsterHallucination> FakeEnemy;

	void SpawnActorWithinRadius(UClass* ActorClass, float Radius);
};
