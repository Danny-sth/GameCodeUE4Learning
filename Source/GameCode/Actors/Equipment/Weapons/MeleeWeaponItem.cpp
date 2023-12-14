// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponItem.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	EquippedSocketName = SocketCharacterWeapon;
}

void AMeleeWeaponItem::StartAttack(EMeleeAttackType AttackType)
{
	if (!GetCachedCharacterOwner())
	{
		return;
	}
	HittedActors.Empty();
	CurrentAttack = Attacks.Find(AttackType);
	if (CurrentAttack && CurrentAttack->AttackMontage)
	{
		UAnimInstance* AnimInstance = GetCachedCharacterOwner()->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float Duration = AnimInstance->Montage_Play(CurrentAttack->AttackMontage, 1.f,
			                                            EMontagePlayReturnType::Duration);
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this,
			                                &AMeleeWeaponItem::OnAttackTimerFinished, Duration, false);
		}
		else
		{
			OnAttackTimerFinished();
		}
	}
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	GetComponents<UMeleeHitRegistrator>(Registrators);
	for (auto Registrator : Registrators)
	{
		Registrator->OnMeleeHit.AddDynamic(this, &AMeleeWeaponItem::ProcessHit);
	}
}

void AMeleeWeaponItem::SetHitRegistrationActive(bool bIsActive_in)
{
	HittedActors.Empty();
	for (const auto Registrator : Registrators)
	{
		Registrator->SetHitRegistratorActive(bIsActive_in);
	}
}

void AMeleeWeaponItem::ProcessHit(const FHitResult& HitResult, const FVector& HitDirection)
{
	AActor* HittedActor = HitResult.GetActor();
	if (!CurrentAttack || !HittedActor || HittedActors.Contains(HittedActor))
	{
		return;
	}
	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = HitDirection;
	DamageEvent.DamageTypeClass = CurrentAttack->DamageTypeClass;
	AController* OwnerController = GetCachedCharacterOwner() ? GetCachedCharacterOwner()->GetController() : nullptr;
	HittedActor->TakeDamage(CurrentAttack->Damage, DamageEvent, OwnerController, GetOwner());

	HittedActors.Add(HittedActor);
}

void AMeleeWeaponItem::OnAttackTimerFinished()
{
	CurrentAttack = nullptr;
	SetHitRegistrationActive(false);
}
