// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define COLLISION_SMCHARACTERBASE ECC_GameTraceChannel1
#define TRACECHANNEL_BULLET ECC_GameTraceChannel2

DECLARE_LOG_CATEGORY_EXTERN(LogNIX, Log, All);

#define SM_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogNIX, Verbosity, Format, ##__VA_ARGS__); \
}

DECLARE_STATS_GROUP(TEXT("NIX_Game"), STATGROUP_NIX, STATCAT_Advanced);