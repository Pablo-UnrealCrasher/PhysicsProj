// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitscanWeaponComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "PhysicsCharacter.h"
#include "PhysicsWeaponComponent.h"
#include <Camera/CameraComponent.h>
#include <Components/SphereComponent.h>

#include "PhysicsEngine/PhysicsHandleComponent.h"

void UHitscanWeaponComponent::Fire()
{
	Super::Fire();

	UCameraComponent* Camera = Character->GetFirstPersonCameraComponent();
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);
	ActorsToIgnore.Add(GetOwner());

	FHitResult Hit;

	if (IsValid(Camera) && UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Camera->GetComponentLocation() + Camera->GetForwardVector() * 100,
		Camera->GetComponentLocation() + Camera->GetForwardVector() * m_fRange,
		m_fSphereTraceRadius,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		Hit,
		true
		))
	{
		ApplyDamage(Hit);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, Hit.Location,FRotator::ZeroRotator, true);
	}
}
