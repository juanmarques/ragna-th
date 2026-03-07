// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ROWidget_Minimap.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UCanvasPanel;

/**
 * FROMinimapMarker
 * Represents a marker on the minimap (NPC, portal, party member, etc.).
 */
USTRUCT(BlueprintType)
struct FROMinimapMarker
{
	GENERATED_BODY()

	/** World position of the marker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	FVector WorldPosition = FVector::ZeroVector;

	/** Marker type for icon selection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	FName MarkerType; // "NPC", "Portal", "Party", "Player"

	/** Display name (for tooltip). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	FString DisplayName;

	/** Marker color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	FLinearColor Color = FLinearColor::White;
};

/**
 * UROWidget_Minimap
 * Minimap display in top-right corner with player position, markers,
 * map name, coordinates, and zoom controls. Toggles between mini and full-screen.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_Minimap : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_Minimap(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- Data Update ---

	/** Set the current map texture and name. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void SetMapData(UTexture2D* MapTexture, const FString& MapName, FVector2D MapWorldSize);

	/** Set the player's current world position and rotation. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void SetPlayerPosition(FVector WorldPosition, float YawRotation);

	/** Update all markers on the minimap. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void SetMarkers(const TArray<FROMinimapMarker>& InMarkers);

	/** Toggle between minimap and full-screen map. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void ToggleFullScreenMap();

	/** Whether the full-screen map is showing. */
	UFUNCTION(BlueprintPure, Category = "RO|Minimap")
	bool IsFullScreenMap() const { return bIsFullScreen; }

	/** Zoom in. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void ZoomIn();

	/** Zoom out. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void ZoomOut();

	/** Get the current zoom level. */
	UFUNCTION(BlueprintPure, Category = "RO|Minimap")
	float GetZoomLevel() const { return ZoomLevel; }

protected:
	/** Refresh marker positions on the minimap. */
	UFUNCTION(BlueprintCallable, Category = "RO|Minimap")
	void RefreshDisplay();

	/** Convert world position to minimap UV coordinates. */
	FVector2D WorldToMinimapUV(FVector WorldPosition) const;

	// --- Button handlers ---
	UFUNCTION()
	void OnZoomInClicked();
	UFUNCTION()
	void OnZoomOutClicked();
	UFUNCTION()
	void OnToggleMapClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* MinimapImage;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* MarkerCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* PlayerArrowImage;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MapName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Coordinates;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ZoomIn;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ZoomOut;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ToggleMap;

private:
	/** Current player world position. */
	FVector CurrentPlayerPosition = FVector::ZeroVector;

	/** Current player yaw rotation. */
	float CurrentPlayerYaw = 0.0f;

	/** Map world dimensions for UV calculation. */
	FVector2D MapWorldDimensions = FVector2D(10000.0f, 10000.0f);

	/** All active markers. */
	TArray<FROMinimapMarker> Markers;

	/** Current zoom level (1.0 = default). */
	float ZoomLevel = 1.0f;

	/** Minimum and maximum zoom. */
	float MinZoom = 0.5f;
	float MaxZoom = 4.0f;

	/** Whether full-screen map is showing. */
	bool bIsFullScreen = false;

	/** Map name for display. */
	FString CurrentMapName;
};
