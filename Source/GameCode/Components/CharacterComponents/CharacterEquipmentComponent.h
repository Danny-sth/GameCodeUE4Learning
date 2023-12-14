// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Actors/Equipment/Throwables/ThrowableItem.h"
#include "GameCode/Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "CharacterEquipmentComponent.generated.h"

typedef TArray<int32, TInlineAllocator<(uint32)EAmunitionType::Max>> TAmunitionArray;
typedef TArray<AEquipableItem*, TInlineAllocator<(uint32)EEquipmentSlots::Max>> TItemsArray;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChanged, int32, int32)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentThrowableAmmoChanged, int32, NewAmmo);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemChanged, const AEquipableItem*);

class ARangeWeaponItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EEquipableItemType GetCurrentEquippedItemType() const;
	ARangeWeaponItem* GetCurrentRangeWeapon() const;
	void ReloadCurrentWeapon();
	void EquipItemInSlot(EEquipmentSlots Slot);
	void UnEquipCurrentItem();
	void AttachCurrentItemToEquippedSocket();
	void EquipNextItem();
	void EquipPreviousItem();
	void ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo = 0, bool bCheckIsFull = false);
	void LaunchCurrentThrowable();

	AThrowableItem* GetCurrentThrowable() const { return CurrentEquippedThrowable; }
	AMeleeWeaponItem* GetCurrentMeleeWeapon() const { return CurrentEquippedMeleeWeapon; }

	FOnCurrentWeaponAmmoChanged OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableAmmoChanged OnCurrentThrowableAmmoChangedEvent;
	FOnEquippedItemChanged OnEquippedItemChangedEvent;

	bool IsEquipping() const { return bIsEquipping; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EAmunitionType, int32> MaxAmunitionAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EEquipmentSlots, TSubclassOf<AEquipableItem>> ItemsLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TSet<EEquipmentSlots> IgnoreSlotsWhileSwitching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout")
	EEquipmentSlots AutoEquipmentSlot = EEquipmentSlots::None;

private:
	UFUNCTION(Server, Reliable)
	void Server_EquipItemInSlot(EEquipmentSlots Slot);

	AEquipableItem* CurrentEquippedItem;
	ARangeWeaponItem* CurrentEquippedRangeWeapon;
	AThrowableItem* CurrentEquippedThrowable;
	AMeleeWeaponItem* CurrentEquippedMeleeWeapon;

	EEquipmentSlots PreviousEquippedSlot;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquippedSlot)
	EEquipmentSlots CurrentEquippedSlot;

	UFUNCTION()
	void OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old);

	TWeakObjectPtr<AGCBaseCharacter> CachedBaseCharacter;

	UPROPERTY(Replicated)
	TArray<int32> AmunitionArray;

	UPROPERTY(ReplicatedUsing=OnRep_ItemsArray)
	TArray<AEquipableItem*> ItemsArray;

	UFUNCTION()
	void OnRep_ItemsArray();

	FDelegateHandle OnCurrentWeaponAmmoChangedHandle;
	FDelegateHandle OnCurrentWeaponReloadeddHandle;
	bool bIsEquipping = false;

	FTimerHandle EquipTimer;

	uint32 NextItemsArraySlotIndex(uint32 CurrentSlotIndex);
	uint32 PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex);
	void CreateLoadout();
	int32 GetAvailableAmunitionForCurrentWeapon();
	int32 GetAvailableAmunitionForCurrentThrowable();

	UFUNCTION()
	void OnCurrentWeaponUpdatedAmmo(int32 Ammo);

	UFUNCTION()
	void OnCurrentThrowableUpdatedAmmo(int32 NewAmmo);

	UFUNCTION()
	void OnWeaponReloadComplete();

	void EquipAnimationFinished();

	void AutoEquip();
};
