// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipmentComponent.h"

#include "GameCode/Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "Net/UnrealNetwork.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentEquippedSlot)
	DOREPLIFETIME(UCharacterEquipmentComponent, AmunitionArray)
	DOREPLIFETIME(UCharacterEquipmentComponent, ItemsArray)
}

EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquippedItemType() const
{
	if (IsValid(CurrentEquippedItem))
	{
		return CurrentEquippedItem->GetItemType();
	}
	return EEquipableItemType::None;
}

ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentEquippedRangeWeapon;
}

void UCharacterEquipmentComponent::ReloadCurrentWeapon()
{
	check(CurrentEquippedRangeWeapon);
	int32 AvailableAmunition = GetAvailableAmunitionForCurrentWeapon();
	if (AvailableAmunition <= 0)
	{
		return;
	}
	CurrentEquippedRangeWeapon->StartReload();
}

void UCharacterEquipmentComponent::EquipItemInSlot(EEquipmentSlots Slot)
{
	if (bIsEquipping)
	{
		return;
	}

	UnEquipCurrentItem();
	CurrentEquippedItem = ItemsArray[(uint32)Slot];
	CurrentEquippedRangeWeapon = Cast<ARangeWeaponItem>(CurrentEquippedItem);
	CurrentEquippedThrowable = Cast<AThrowableItem>(CurrentEquippedItem);
	CurrentEquippedMeleeWeapon = Cast<AMeleeWeaponItem>(CurrentEquippedItem);

	if (CurrentEquippedItem)
	{
		UAnimMontage* CharacterEquipAnimMontage = CurrentEquippedItem->GetCharacterEquipAnimMontage();
		if (CharacterEquipAnimMontage)
		{
			bIsEquipping = true;
			float EquipDuration = CachedBaseCharacter->PlayAnimMontage(CharacterEquipAnimMontage);
			GetWorld()->GetTimerManager().SetTimer(EquipTimer, this,
			                                       &UCharacterEquipmentComponent::EquipAnimationFinished,
			                                       EquipDuration, false);
		}
		else
		{
			AttachCurrentItemToEquippedSocket();
		}
		CurrentEquippedSlot = Slot;
		CurrentEquippedItem->Equip();
	}
	if (CurrentEquippedRangeWeapon)
	{
		OnCurrentWeaponAmmoChangedHandle = CurrentEquippedRangeWeapon->OnAmmoChanged.AddUFunction(
			this, FName("OnCurrentWeaponUpdatedAmmo"));
		OnCurrentWeaponReloadeddHandle = CurrentEquippedRangeWeapon->OnReloadComplete.AddUFunction(
			this, FName("OnWeaponReloadComplete"));
		OnCurrentWeaponUpdatedAmmo(CurrentEquippedRangeWeapon->GetAmmo());
	}
	if (CurrentEquippedThrowable)
	{
		CurrentEquippedThrowable->OnThrowableAmmoChanged.AddDynamic(
			this, &UCharacterEquipmentComponent::OnCurrentThrowableUpdatedAmmo);
		OnCurrentThrowableUpdatedAmmo(CurrentEquippedThrowable->GetAmmoAmount());
	}
	OnEquippedItemChangedEvent.Broadcast(CurrentEquippedItem);

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemInSlot(CurrentEquippedSlot);
	}
}

void UCharacterEquipmentComponent::UnEquipCurrentItem()
{
	if (CurrentEquippedItem)
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(),
		                                       FAttachmentTransformRules::KeepRelativeTransform,
		                                       CurrentEquippedItem->GetUnEquippedSocketName());
		CurrentEquippedItem->UnEquip();
	}
	if (CurrentEquippedRangeWeapon)
	{
		CurrentEquippedRangeWeapon->StopFire();
		CurrentEquippedRangeWeapon->EndReload(false);
		CurrentEquippedRangeWeapon->OnAmmoChanged.Remove(OnCurrentWeaponAmmoChangedHandle);
		CurrentEquippedRangeWeapon->OnReloadComplete.Remove(OnCurrentWeaponReloadeddHandle);
	}
	PreviousEquippedSlot = CurrentEquippedSlot;
	CurrentEquippedSlot = EEquipmentSlots::None;
}

void UCharacterEquipmentComponent::AttachCurrentItemToEquippedSocket()
{
	if (CurrentEquippedItem)
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(),
		                                       FAttachmentTransformRules::KeepRelativeTransform,
		                                       CurrentEquippedItem->GetEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::EquipNextItem()
{
	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex);

	while (CurrentSlotIndex != NextSlotIndex
		&& IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)NextSlotIndex)
		&& ItemsArray[NextSlotIndex])
	{
		NextSlotIndex = NextItemsArraySlotIndex(NextSlotIndex);
	}
	if (CurrentSlotIndex != NextSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots)NextSlotIndex);
	}
}

void UCharacterEquipmentComponent::EquipPreviousItem()
{
	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 PreviousSlotIndex = PreviousItemsArraySlotIndex(CurrentSlotIndex);

	while (CurrentSlotIndex != PreviousSlotIndex
		&& IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)PreviousSlotIndex)
		&& ItemsArray[PreviousSlotIndex])
	{
		PreviousSlotIndex = PreviousItemsArraySlotIndex(PreviousSlotIndex);
	}
	if (CurrentSlotIndex != PreviousSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots)PreviousSlotIndex);
	}
}

void UCharacterEquipmentComponent::ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo, bool bCheckIsFull)
{
	int32 AvailableAmunition = GetAvailableAmunitionForCurrentWeapon();
	int32 CurrentAmmo = CurrentEquippedRangeWeapon->GetAmmo();
	int32 AmmoToReload = CurrentEquippedRangeWeapon->GetMaxAmmo() - CurrentAmmo;
	int ReloadedAmmo = FMath::Min(FMath::Min(AvailableAmunition, AmmoToReload), NumberOfAmmo);
	if (NumberOfAmmo > 0)
	{
		ReloadedAmmo = FMath::Min(ReloadedAmmo, NumberOfAmmo);
	}
	AmunitionArray[(uint32)CurrentEquippedRangeWeapon->GetAmmoType()] -= ReloadedAmmo;
	CurrentEquippedRangeWeapon->SetAmmo(ReloadedAmmo + CurrentAmmo);
	if (bCheckIsFull)
	{
		AvailableAmunition -= AmunitionArray[(uint32)CurrentEquippedRangeWeapon->GetAmmoType()];
		bool bIsFullyReloaded = CurrentEquippedRangeWeapon->GetAmmo() == CurrentEquippedRangeWeapon->GetMaxAmmo();
		if (AvailableAmunition == 0 || bIsFullyReloaded)
		{
			CurrentEquippedRangeWeapon->EndReload(true);
		}
	}
}

void UCharacterEquipmentComponent::LaunchCurrentThrowable()
{
	if (CurrentEquippedThrowable)
	{
		CurrentEquippedThrowable->Throw();
		EquipItemInSlot(PreviousEquippedSlot);
	}
}

// Called when the game starts
void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<AGCBaseCharacter>(),
	       TEXT(
		       "UCharacterEquipmentComponent::BeginPlay() CharacterEquipmentComponent can used only with a BaseCharacter"
	       ));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());

	const auto GrenadesAmount = MaxAmunitionAmount.Find(EAmunitionType::FragGrenades);
	CachedBaseCharacter->GetCharacterEquipmentComponent()->
	                     OnCurrentThrowableAmmoChangedEvent.Broadcast(*GrenadesAmount);

	CreateLoadout();
	AutoEquip();
}

void UCharacterEquipmentComponent::Server_EquipItemInSlot_Implementation(EEquipmentSlots Slot)
{
	EquipItemInSlot(Slot);
}

void UCharacterEquipmentComponent::OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old)
{
	EquipItemInSlot(CurrentEquippedSlot);
}

void UCharacterEquipmentComponent::OnRep_ItemsArray()
{
	for (auto Item : ItemsArray)
	{
		if (Item)
		{
			Item->UnEquip();
		}
	}
}

uint32 UCharacterEquipmentComponent::NextItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == ItemsArray.Num() - 1)
	{
		return 0;
	}
	else
	{
		return ++CurrentSlotIndex;
	}
}

uint32 UCharacterEquipmentComponent::PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == 0)
	{
		return ItemsArray.Num() - 1;
	}
	else
	{
		return --CurrentSlotIndex;
	}
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	AmunitionArray.AddZeroed((uint32)EAmunitionType::Max);
	for (const TPair<EAmunitionType, int32>& AmmoPair : MaxAmunitionAmount)
	{
		if ((uint32)AmmoPair.Key < (uint32)MaxAmunitionAmount.Num())
		{
			AmunitionArray[(uint32)AmmoPair.Key] = FMath::Max(AmmoPair.Value, 0);
		}
	}

	ItemsArray.AddZeroed((uint32)EEquipmentSlots::Max);
	for (const TPair<EEquipmentSlots, TSubclassOf<AEquipableItem>>& ItemPair : ItemsLoadout)
	{
		if (!ItemPair.Value)
		{
			continue;
		}
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(ItemPair.Value);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(),
		                        FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnEquippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[(uint32)ItemPair.Key] = Item;
	}
}

int32 UCharacterEquipmentComponent::GetAvailableAmunitionForCurrentWeapon()
{
	check(GetCurrentRangeWeapon())
	return AmunitionArray[(uint32)GetCurrentRangeWeapon()->GetAmmoType()];
}

void UCharacterEquipmentComponent::OnCurrentWeaponUpdatedAmmo(int32 Ammo)
{
	if (OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo, GetAvailableAmunitionForCurrentWeapon());
	}
}

void UCharacterEquipmentComponent::OnCurrentThrowableUpdatedAmmo(int32 NewAmmo)
{
	OnCurrentThrowableAmmoChangedEvent.Broadcast(NewAmmo);
}

void UCharacterEquipmentComponent::OnWeaponReloadComplete()
{
	ReloadAmmoInCurrentWeapon();
}

void UCharacterEquipmentComponent::EquipAnimationFinished()
{
	bIsEquipping = false;
	if (CurrentEquippedThrowable)
	{
		EquipItemInSlot(PreviousEquippedSlot);
	}
	AttachCurrentItemToEquippedSocket();
}

void UCharacterEquipmentComponent::AutoEquip()
{
	if (AutoEquipmentSlot != EEquipmentSlots::None)
	{
		EquipItemInSlot(AutoEquipmentSlot);
	}
}
