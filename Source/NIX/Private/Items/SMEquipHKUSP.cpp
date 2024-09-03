// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMEquipHKUSP.h"
#include "Animation/SMAnim_EquippableHKUSP.h"


void ASMEquipHKUSP::BeginPlay()
{
	Super::BeginPlay();
	this->OnCurrentAmmoChanged.AddDynamic(this, &ASMEquipHKUSP::CurrentAmmoChanged);
	AnimInstanceFP = Cast<USMAnim_EquippableHKUSP>(EquippableMesh1P->GetAnimInstance());
	
}


void ASMEquipHKUSP::CurrentAmmoChanged(float OldAmmo, float NewAmmo)
{
	AnimInstanceFP->bSlideback = NewAmmo == 0; 
}
