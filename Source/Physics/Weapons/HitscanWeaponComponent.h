// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include "HitscanWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHitscanImpact, class AActor*, impactedActor, FVector, impactPosition, FVector, impactDirection);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSICS_API UHitscanWeaponComponent : public UPhysicsWeaponComponent
{
	GENERATED_BODY()
public:
	/** UPhysicsWeaponComponent **/
	virtual void Fire() override;
public:
	UPROPERTY(EditAnywhere, Category= Weapon)
	float m_fRange;
	
	UPROPERTY(EditAnywhere, Category= Weapon)
	float m_fSphereTraceRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category= Weapon)
	UParticleSystem* ParticleSystem;

	UPROPERTY(BlueprintAssignable)
	FHitscanImpact onHitscanImpact;
};
