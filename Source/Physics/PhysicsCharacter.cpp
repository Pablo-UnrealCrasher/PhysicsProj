// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCharacter.h"
#include "PhysicsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Components/StaticMeshComponent.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APhysicsCharacter::APhysicsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void APhysicsCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_Stamina = m_MaxStamina;
	m_Health = m_MaxHealth;
	m_BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void APhysicsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Stamina update
	if (bIsSprinting)
	{
		m_Stamina -= m_StaminaDepletionRate * DeltaSeconds;
		
		if (m_Stamina <= 0)
		{
			SetIsSprinting(false);
			m_Stamina = 0;
		}
	}
	else
	{
		m_Stamina += m_StaminaRecoveryRate * DeltaSeconds;
		m_Stamina = FMath::Min(m_Stamina, m_MaxStamina);
	}

	// Highlighting objects when we are not holding an object.
	if (m_PhysicsHandle->GetGrabbedComponent() == nullptr)
	{
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(
			Hit,
			FirstPersonCameraComponent->GetComponentLocation(),
			FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector() * m_MaxGrabDistance,
			ECC_Visibility))
		{
			UActorComponent* Component = Hit.GetComponent();
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
			{
				SetHighlightedMesh(Cast<UMeshComponent>(MeshComponent));
			}
		}
		else
		{
			SetHighlightedMesh(nullptr);
		}
	}

	// Grabbed object update
	m_PhysicsHandle->SetTargetLocation(GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector() * m_GrabDistance);
	m_PhysicsHandle->SetTargetRotation(GetFirstPersonCameraComponent()->GetComponentRotation());
}

void APhysicsCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APhysicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Look);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Started, this, &APhysicsCharacter::GrabObject);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Completed, this, &APhysicsCharacter::ReleaseObject);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APhysicsCharacter::SetIsSprinting(bool NewIsSprinting)
{
	// We don't allow to trigger sprinting when we don't have stamina.
	if (bIsSprinting == NewIsSprinting || (bIsSprinting == false && (m_Stamina == 0 || !bReleasedSprint)))
	{
		return;
	}

	bIsSprinting = NewIsSprinting;
	
	GetCharacterMovement()->MaxWalkSpeed = m_BaseSpeed * (bIsSprinting ? m_SprintSpeedMultiplier : 1.0f);
}

void APhysicsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APhysicsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APhysicsCharacter::Sprint(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SetIsSprinting(true);
		bReleasedSprint = false;
	}
	else
	{
		bReleasedSprint = true;
		SetIsSprinting(false);
	}
}

void APhysicsCharacter::GrabObject(const FInputActionValue& Value)
{
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		FirstPersonCameraComponent->GetComponentLocation(),
		FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector() * m_MaxGrabDistance,
		ECC_Visibility))
	{
		m_PhysicsHandle->GrabComponentAtLocationWithRotation(Hit.GetComponent(), Hit.BoneName, Hit.Location, Hit.Component->GetComponentRotation());
		m_PhysicsHandle->InterpolationSpeed = m_BaseInterpolationSpeed / (Hit.GetComponent()->GetMass() / 2.0f);
		m_GrabDistance = Hit.Distance;
		
		UActorComponent* ActorComponent = Hit.GetActor()->GetComponentByClass(UMeshComponent::StaticClass());
		SetHighlightedMesh(Cast<UMeshComponent>(ActorComponent));
	}
}

void APhysicsCharacter::ReleaseObject(const FInputActionValue& Value)
{
	// @TODO: Release grabbed object using UPhysicsHandleComponent
	if (m_PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		m_PhysicsHandle->ReleaseComponent();
		SetHighlightedMesh(nullptr);
	}
}

void APhysicsCharacter::SetHighlightedMesh(UMeshComponent* StaticMesh)
{
	if(m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(nullptr);
	}
	m_HighlightedMesh = StaticMesh;
	if (m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(m_HighlightMaterial);
	}
}

float APhysicsCharacter::GetStamina() const
{
	return m_Stamina;
}

float APhysicsCharacter::GetHealth() const
{
	return m_Health;
}