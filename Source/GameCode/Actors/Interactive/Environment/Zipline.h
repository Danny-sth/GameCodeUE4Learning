// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameCode/Actors/Interactive/InteractiveActor.h"
#include "Zipline.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();
	virtual void OnConstruction(const FTransform& Transform) override;
	bool IsOnZipline() const { return bIsOnZipline; }
	FVector GetTargetPillarLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* FirstPillarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SecondPillarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CableMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MakeEditWidget), Category = "Zipline parameters")
	FVector FirstPillarLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MakeEditWidget), Category = "Zipline parameters")
	FVector SecondPillarLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float FirstPillarHeight = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float SecondPillarHeight = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float InteractionCapsuleRadius = 34.0f;

	UCapsuleComponent* GetZiplineInteractionCapsule();

private:
	void SetMeshHeight(UStaticMeshComponent* MeshComponent, float Height);
	void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                     const FHitResult& SweepResult);
	void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	FVector GetTargetLocationWithOffset(FVector PillarLocation) const;

	bool bIsOnZipline = false;
};
