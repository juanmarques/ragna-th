// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_Minimap.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

UROWidget_Minimap::UROWidget_Minimap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_Minimap::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_ZoomIn) Btn_ZoomIn->OnClicked.AddDynamic(this, &UROWidget_Minimap::OnZoomInClicked);
	if (Btn_ZoomOut) Btn_ZoomOut->OnClicked.AddDynamic(this, &UROWidget_Minimap::OnZoomOutClicked);
	if (Btn_ToggleMap) Btn_ToggleMap->OnClicked.AddDynamic(this, &UROWidget_Minimap::OnToggleMapClicked);
}

void UROWidget_Minimap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update coordinate display every tick
	if (Text_Coordinates)
	{
		const int32 X = FMath::RoundToInt(CurrentPlayerPosition.X);
		const int32 Y = FMath::RoundToInt(CurrentPlayerPosition.Y);
		Text_Coordinates->SetText(FText::FromString(FString::Printf(TEXT("%d, %d"), X, Y)));
	}
}

void UROWidget_Minimap::SetMapData(UTexture2D* MapTexture, const FString& MapName, FVector2D MapWorldSize)
{
	CurrentMapName = MapName;
	MapWorldDimensions = MapWorldSize;

	if (MinimapImage && MapTexture)
	{
		MinimapImage->SetBrushFromTexture(MapTexture);
	}

	if (Text_MapName)
	{
		Text_MapName->SetText(FText::FromString(MapName));
	}
}

void UROWidget_Minimap::SetPlayerPosition(FVector WorldPosition, float YawRotation)
{
	CurrentPlayerPosition = WorldPosition;
	CurrentPlayerYaw = YawRotation;

	// Update player arrow position and rotation
	if (PlayerArrowImage)
	{
		PlayerArrowImage->SetRenderTransformAngle(YawRotation);
	}

	RefreshDisplay();
}

void UROWidget_Minimap::SetMarkers(const TArray<FROMinimapMarker>& InMarkers)
{
	Markers = InMarkers;
	RefreshDisplay();
}

void UROWidget_Minimap::ToggleFullScreenMap()
{
	bIsFullScreen = !bIsFullScreen;
	// Blueprint handles resizing the widget between minimap and full-screen mode
	RefreshDisplay();
}

void UROWidget_Minimap::ZoomIn()
{
	ZoomLevel = FMath::Min(ZoomLevel * 1.5f, MaxZoom);
	RefreshDisplay();
}

void UROWidget_Minimap::ZoomOut()
{
	ZoomLevel = FMath::Max(ZoomLevel / 1.5f, MinZoom);
	RefreshDisplay();
}

FVector2D UROWidget_Minimap::WorldToMinimapUV(FVector WorldPosition) const
{
	if (MapWorldDimensions.X <= 0.0f || MapWorldDimensions.Y <= 0.0f)
	{
		return FVector2D(0.5f, 0.5f);
	}

	// Convert world position to 0-1 UV space
	const float U = WorldPosition.X / MapWorldDimensions.X;
	const float V = WorldPosition.Y / MapWorldDimensions.Y;

	return FVector2D(FMath::Clamp(U, 0.0f, 1.0f), FMath::Clamp(V, 0.0f, 1.0f));
}

void UROWidget_Minimap::RefreshDisplay()
{
	// Marker positions are updated in Blueprint via the MarkerCanvas.
	// C++ provides world-to-UV conversion and data; Blueprint handles visual placement.
}

void UROWidget_Minimap::OnZoomInClicked() { ZoomIn(); }
void UROWidget_Minimap::OnZoomOutClicked() { ZoomOut(); }
void UROWidget_Minimap::OnToggleMapClicked() { ToggleFullScreenMap(); }
