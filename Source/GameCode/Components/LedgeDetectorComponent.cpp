#include "LedgeDetectorComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Subsystems/DebugSubsystem.h"
#include "GameCode/Utils/GCTraceUtils.h"

void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ACharacter>(),
	       TEXT("ULedgeDetectorComponent::BeginPlay() only a character can use UledgeDetectorComponent"));
	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());
	CachedCapsuleHalfHeight = CachedCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); 
}

bool ULedgeDetectorComponent::DetectLedge(OUT FLedgeDescription& LedgeDescription)
{
	UCapsuleComponent* CapsuleComponent = CachedCharacterOwner->GetCapsuleComponent();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubSystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bIsDebugEnabled = false;
#endif

	float DrawTime = 2.0f;

	float BottomZOffset = 2.0f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() - (CapsuleComponent->GetScaledCapsuleHalfHeight()
		- BottomZOffset) * FVector::UpVector;

	//1. Forward check 
	float ForwardCheckCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float ForwardCheckCapsuleHalfHeight = (MaximumLedgeHeight - MinimumLedgeHeight) * 0.5f;

	FHitResult ForwardCheckHitResult;
	FVector ForwardStartLocation = CharacterBottom + (MinimumLedgeHeight + ForwardCheckCapsuleHalfHeight) *
		FVector::UpVector;
	FVector ForwardEndLocation = ForwardStartLocation + CachedCharacterOwner->GetActorForwardVector() *
		ForwardCheckDistance;

	if (!GCTraceUtils::SweepCapsuleSingleByChanel(GetWorld(), ForwardCheckHitResult, ForwardStartLocation,
	                                              ForwardEndLocation, ForwardCheckCapsuleRadius,
	                                              ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_Climbing,
	                                              QueryParams, FCollisionResponseParams::DefaultResponseParam,
	                                              bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//2. Downward check
	FHitResult DownwardCheckHitResult;
	float DownwardSphereCheckRadius = CapsuleComponent->GetScaledCapsuleRadius();

	float DownwardCheckDepthOffset = 10.0f;
	FVector DownwardStartLocation = ForwardCheckHitResult.ImpactPoint - ForwardCheckHitResult.ImpactNormal *
		DownwardCheckDepthOffset;
	DownwardStartLocation.Z = CharacterBottom.Z + MaximumLedgeHeight + DownwardSphereCheckRadius;
	FVector DownwardEndLocation(DownwardStartLocation.X, DownwardStartLocation.Y, CharacterBottom.Z);

	if (!GCTraceUtils::SweepSphereSingleByChanel(GetWorld(), DownwardCheckHitResult, DownwardStartLocation,
	                                             DownwardEndLocation, DownwardSphereCheckRadius, ECC_Climbing,
	                                             QueryParams, FCollisionResponseParams::DefaultResponseParam,
	                                             bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//3. Overlap check 
	float OverlapCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float OverlapCapsuleHalfHeight = CachedCapsuleHalfHeight; 
	float OverlapCapsuleFloorOffset = 2.0f;
	FVector OverlapLocation =
		DownwardCheckHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + OverlapCapsuleFloorOffset) * FVector::UpVector;
	if (GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius,
	                                                  OverlapCapsuleHalfHeight, FQuat::Identity, CollisionProfilePawn,
	                                                  QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.Location = OverlapLocation;
	LedgeDescription.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.0f, -1.0f, 0.0f)).
		ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardCheckHitResult.ImpactNormal.GetSafeNormal2D();
	LedgeDescription.MantlingHeight = DownwardCheckHitResult.ImpactPoint.Z - CharacterBottom.Z; 
	if (DownwardCheckHitResult.Actor != nullptr) 
	{
		LedgeDescription.TargetActor = Cast<UPrimitiveComponent>(DownwardCheckHitResult.Actor); 
	}

	return true;
}
