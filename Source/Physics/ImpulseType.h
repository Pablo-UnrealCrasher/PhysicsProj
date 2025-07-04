// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpulseType.generated.h"

/**
 * 
 */
UENUM()
enum class EImpulseType : uint8
{
	LINEAR UMETA(DisplayName = "LINEAR"),
	RADIAL UMETA(DisplayName = "RADIAL"),
	NONE UMETA(DisplayName = "NONE"),
};
