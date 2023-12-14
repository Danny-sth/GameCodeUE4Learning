// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "Kismet/KismetMathLibrary.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Zipline root");

	FirstPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("First Pillar"));
	FirstPillarMeshComponent->SetupAttachment(RootComponent);

	SecondPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Second Pillar"));
	SecondPillarMeshComponent->SetupAttachment(RootComponent);

	CableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction volume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FirstPillarMeshComponent->SetRelativeLocation(FirstPillarLocation + FirstPillarHeight * 0.5 * FVector::UpVector);
	SecondPillarMeshComponent->SetRelativeLocation(SecondPillarLocation + SecondPillarHeight * 0.5 * FVector::UpVector);

	SetMeshHeight(FirstPillarMeshComponent, FirstPillarHeight);
	SetMeshHeight(SecondPillarMeshComponent, SecondPillarHeight);

	FVector FirstPillarTop = FirstPillarLocation + (FVector::UpVector * FirstPillarHeight);
	FVector SecondPillarTop = SecondPillarLocation + (FVector::UpVector * SecondPillarHeight);
	UStaticMesh* CableMesh = CableMeshComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float MeshLength = CableMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshLength))
		{
			float CableLength = (FirstPillarTop - SecondPillarTop).Size();
			CableMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, CableLength / MeshLength));
			UCapsuleComponent* ZiplineInteractionCapsule = GetZiplineInteractionCapsule();
			ZiplineInteractionCapsule->SetCapsuleHalfHeight(CableLength * 0.5);
			ZiplineInteractionCapsule->SetCapsuleRadius(InteractionCapsuleRadius);
		}
	}
	FVector CableLocation = (FirstPillarTop + SecondPillarTop) * 0.5;
	CableMeshComponent->SetRelativeLocation(CableLocation);
	FRotator CableRotation = UKismetMathLibrary::FindLookAtRotation(FirstPillarTop, SecondPillarTop);
	CableRotation.Pitch -= 90;
	CableMeshComponent->SetRelativeRotation(CableRotation);
	CableRotation.Pitch -= 90;
	InteractionVolume->SetRelativeRotation(CableRotation);
	InteractionVolume->SetRelativeLocation(CableLocation);
}

FVector AZipline::GetTargetPillarLocation() const
{
	const FVector FirstPillarLoc = FirstPillarMeshComponent->GetComponentLocation();
	const FVector SecondPillarLoc = SecondPillarMeshComponent->GetComponentLocation();
	return FirstPillarLoc.Z > SecondPillarLoc.Z
		       ? GetTargetLocationWithOffset(SecondPillarLoc)
		       : GetTargetLocationWithOffset(FirstPillarLoc);
}

FVector AZipline::GetTargetLocationWithOffset(const FVector PillarLocation) const
{
	const FVector Direction = (PillarLocation - GetActorLocation()).GetSafeNormal();
	const FVector Offset = Direction * 100.f;
	return PillarLocation - Offset;
}

UCapsuleComponent* AZipline::GetZiplineInteractionCapsule()
{
	return StaticCast<UCapsuleComponent*>(InteractionVolume);
}

void AZipline::SetMeshHeight(UStaticMeshComponent* MeshComponent, float Height)
{
	UStaticMesh* PillarMesh = MeshComponent->GetStaticMesh();
	if (IsValid(PillarMesh))
	{
		float MeshHeight = PillarMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			MeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, Height / MeshHeight));
		}
	}
}

void AZipline::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
	Super::OnInteractionVolumeOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,
	                                       SweepResult);
	if (OverlappedComponent == InteractionVolume)
	{
		bIsOnZipline = true;
	}
}

void AZipline::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolumeOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OverlappedComponent == InteractionVolume)
	{
		bIsOnZipline = false;
	}
}
