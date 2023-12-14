#include "EquipableItem.h"

#include "Net/UnrealNetwork.h"

AEquipableItem::AEquipableItem()
{
	SetReplicates(true);
}

void AEquipableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if (NewOwner)
	{
		checkf(GetOwner()->IsA<AGCBaseCharacter>(),
		       TEXT("AEquipableItem::SetOwner() only Character can be an owner of equipable item"));
		CachedCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedCharacterOwner = nullptr;
	}
}

void AEquipableItem::Equip()
{
	OnEquipmentStateChanged.Broadcast(true);
}

void AEquipableItem::UnEquip()
{
	OnEquipmentStateChanged.Broadcast(false);
}

AGCBaseCharacter* AEquipableItem::GetCachedCharacterOwner() const
{
	return CachedCharacterOwner.IsValid() ? CachedCharacterOwner.Get() : nullptr;
}
