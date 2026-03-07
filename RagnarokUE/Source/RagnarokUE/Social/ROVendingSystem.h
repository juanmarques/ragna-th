// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROVendingSystem.generated.h"

/** A single item listed for sale in a vending shop. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROVendingItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	FROItemInstance Item;

	/** Price per unit in Zeny. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	int32 Price = 0;

	/** Number of this item available for sale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	int32 Amount = 1;
};

/** Information about an active vending shop. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROVendingShop
{
	GENERATED_BODY()

	/** The player who owns this shop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	int32 VendorPlayerID = 0;

	/** Shop title displayed above the vendor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	FString ShopTitle;

	/** Items available for purchase. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	TArray<FROVendingItem> Items;

	/** World location of the shop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
	FVector ShopLocation = FVector::ZeroVector;

	bool IsValid() const { return VendorPlayerID > 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVendingShopEvent, int32, VendorPlayerID);

/**
 * UROVendingSystem
 * Manages Merchant-class vending shops where players can sell items to other players.
 * Requires the Vending skill from the Merchant class tree.
 */
UCLASS()
class RAGNAROKUE_API UROVendingSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Maximum items a shop can list (based on Vending skill level in RO: 3 + skill level). */
	static constexpr int32 MaxVendingItems = 12;

	// ---- Shop Management ----

	/**
	 * Open a vending shop. The player sits down and the shop becomes visible to others.
	 * Requires Merchant class with Vending skill (checked externally).
	 * @return True if the shop was opened successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	bool OpenShop(int32 PlayerID, const FString& Title, const TArray<FROVendingItem>& Items);

	/** Close the player's vending shop. */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	void CloseShop(int32 PlayerID);

	// ---- Shopping ----

	/** Browse a vendor's shop to see available items. */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	TArray<FROVendingItem> BrowseShop(int32 BrowserID, int32 VendorID) const;

	/**
	 * Buy an item from a vendor's shop.
	 * @return True if the purchase was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	bool BuyFromShop(int32 BuyerID, int32 VendorID, int32 ItemIndex, int32 Amount);

	// ---- Queries ----

	/** Get all currently active vending shops (for shop search feature). */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	TArray<FROVendingShop> GetActiveShops() const;

	/** Check if a player has an active shop. */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	bool HasActiveShop(int32 PlayerID) const;

	/** Get a specific player's shop info. */
	UFUNCTION(BlueprintCallable, Category = "Vending")
	FROVendingShop GetShopInfo(int32 VendorID) const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Vending")
	FOnVendingShopEvent OnShopOpened;

	UPROPERTY(BlueprintAssignable, Category = "Vending")
	FOnVendingShopEvent OnShopClosed;

protected:
	/** All active vending shops, keyed by VendorPlayerID. */
	UPROPERTY()
	TMap<int32, FROVendingShop> ActiveShops;
};
