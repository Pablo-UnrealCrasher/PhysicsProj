#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "ImpulseType.h"
#include "WeaponDamageType.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class PHYSICS_API UWeaponDamageType : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	float m_Damage = 100.0f;

	UPROPERTY(EditDefaultsOnly)
	float m_FalloffRadius = 200.0f;

	UPROPERTY(EditDefaultsOnly)
	EImpulseType ImpulseType = EImpulseType::LINEAR;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> m_DamageType;
};
