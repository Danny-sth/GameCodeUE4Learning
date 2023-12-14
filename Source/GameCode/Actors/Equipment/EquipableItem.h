#pragma once

#include "CoreMinimal.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentStateChanged, bool, bIsEquipped);

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:

	AEquipableItem();
	
	virtual void SetOwner(AActor* NewOwner) override;

	EEquipableItemType GetItemType() const { return ItemType; }
	FName GetUnEquippedSocketName() const { return UnEquippedSocketName; }
	FName GetEquippedSocketName() const { return EquippedSocketName; }
	UAnimMontage* GetCharacterEquipAnimMontage() const { return CharacterEquipAnimMontage; }

	virtual void Equip();
	virtual void UnEquip();

	virtual EReticleType GetReticleType() const { return ReticleType; }

protected:
	UPROPERTY(BlueprintAssignable)
	FOnEquipmentStateChanged OnEquipmentStateChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipable item")
	EEquipableItemType ItemType = EEquipableItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipable item")
	FName UnEquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipable item")
	FName EquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipable item")
	UAnimMontage* CharacterEquipAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reticle")
	EReticleType ReticleType = EReticleType::None;

	AGCBaseCharacter* GetCachedCharacterOwner() const;

private:
	TWeakObjectPtr<AGCBaseCharacter> CachedCharacterOwner;
};
