// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_Hotbar.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Items/ROItemDatabase.h"
#include "RagnarokUE/Items/ROItemBase.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "RagnarokUE/Skills/ROAbilitySystemComponent.h"

UROWidget_Hotbar::UROWidget_Hotbar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_Hotbar::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize all rows (4 rows x 9 slots)
	AllSlots.SetNum(NUM_ROWS * SLOTS_PER_ROW);
	CurrentRow = 0;

	RefreshDisplay();
}

void UROWidget_Hotbar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Tick cooldowns for the current row
	const int32 RowOffset = CurrentRow * SLOTS_PER_ROW;
	bool bNeedsRefresh = false;

	for (int32 i = 0; i < SLOTS_PER_ROW; ++i)
	{
		const int32 SlotIdx = RowOffset + i;
		if (AllSlots.IsValidIndex(SlotIdx) && AllSlots[SlotIdx].CooldownRemaining > 0.0f)
		{
			AllSlots[SlotIdx].CooldownRemaining = FMath::Max(0.0f, AllSlots[SlotIdx].CooldownRemaining - InDeltaTime);
			bNeedsRefresh = true;
		}
	}

	if (bNeedsRefresh)
	{
		RefreshDisplay();
	}
}

void UROWidget_Hotbar::AssignSkillToSlot(int32 SlotIndex, int32 SkillID)
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + SlotIndex;
	if (!AllSlots.IsValidIndex(ActualIndex))
	{
		return;
	}

	FROHotbarSlot& Slot = AllSlots[ActualIndex];
	Slot.bIsSkill = true;
	Slot.ID = SkillID;
	Slot.StackCount = 0;

	// Look up skill data from the owning player's ability system component
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		if (UAbilitySystemComponent* ASC = OwningPawn->FindComponentByClass<UAbilitySystemComponent>())
		{
			TArray<FGameplayAbilitySpec>& Specs = ASC->GetActivatableAbilities();
			for (const FGameplayAbilitySpec& Spec : Specs)
			{
				if (const UROGameplayAbility* ROAbility = Cast<UROGameplayAbility>(Spec.Ability))
				{
					if (ROAbility->SkillID == SkillID)
					{
						Slot.Level = Spec.Level;
						Slot.SPCost = FMath::RoundToInt32(ROAbility->GetSPCost());
						Slot.Icon = ROAbility->SkillIcon.LoadSynchronous();
						break;
					}
				}
			}
		}
	}

	RefreshDisplay();
}

void UROWidget_Hotbar::AssignItemToSlot(int32 SlotIndex, int32 ItemID)
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + SlotIndex;
	if (!AllSlots.IsValidIndex(ActualIndex))
	{
		return;
	}

	FROHotbarSlot& Slot = AllSlots[ActualIndex];
	Slot.bIsSkill = false;
	Slot.ID = ItemID;
	Slot.Level = 0;
	Slot.SPCost = 0;

	// Look up item data from the item database to populate icon
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UROItemDatabase* ItemDB = GI->GetSubsystem<UROItemDatabase>())
		{
			if (const UROItemBase* ItemData = ItemDB->GetItemData(ItemID))
			{
				Slot.Icon = ItemData->Icon.LoadSynchronous();
			}
		}
	}

	RefreshDisplay();
}

void UROWidget_Hotbar::UseSlot(int32 Index)
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + Index;
	if (!AllSlots.IsValidIndex(ActualIndex))
	{
		return;
	}

	const FROHotbarSlot& Slot = AllSlots[ActualIndex];
	if (Slot.IsEmpty())
	{
		return;
	}

	// Don't allow use if on cooldown
	if (Slot.CooldownRemaining > 0.0f)
	{
		return;
	}

	// Broadcast the slot use event for PlayerController to handle
	OnHotbarSlotUsed.Broadcast(Index, Slot);
}

void UROWidget_Hotbar::SwitchRow(int32 RowIndex)
{
	if (RowIndex < 0 || RowIndex >= NUM_ROWS)
	{
		return;
	}

	CurrentRow = RowIndex;

	if (RowIndicatorText)
	{
		RowIndicatorText->SetText(FText::AsNumber(CurrentRow + 1));
	}

	RefreshDisplay();
}

void UROWidget_Hotbar::ClearSlot(int32 SlotIndex)
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + SlotIndex;
	if (AllSlots.IsValidIndex(ActualIndex))
	{
		AllSlots[ActualIndex] = FROHotbarSlot();
		RefreshDisplay();
	}
}

FROHotbarSlot UROWidget_Hotbar::GetSlotData(int32 SlotIndex) const
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + SlotIndex;
	if (AllSlots.IsValidIndex(ActualIndex))
	{
		return AllSlots[ActualIndex];
	}
	return FROHotbarSlot();
}

void UROWidget_Hotbar::StartCooldown(int32 SlotIndex, float Duration)
{
	const int32 ActualIndex = CurrentRow * SLOTS_PER_ROW + SlotIndex;
	if (AllSlots.IsValidIndex(ActualIndex))
	{
		AllSlots[ActualIndex].CooldownRemaining = Duration;
		AllSlots[ActualIndex].CooldownTotal = Duration;
	}
}

void UROWidget_Hotbar::RefreshDisplay()
{
	// Visual refresh is handled in Blueprint via the bound widgets.
	// This function serves as a hook for Blueprint to update slot visuals.
	// In C++, we update the row indicator text.
	if (RowIndicatorText)
	{
		RowIndicatorText->SetText(FText::Format(
			FText::FromString(TEXT("F{0}")),
			FText::AsNumber(CurrentRow + 1)
		));
	}
}
