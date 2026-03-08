// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWeatherSystem.generated.h"

/** Per-map weather configuration. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMapWeatherConfig
{
	GENERATED_BODY()

	/** Map ID this config applies to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FName MapID;

	/** Possible weather states for this map with probability weights. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	TMap<EROWeatherType, float> WeatherWeights;

	/** Minimum time in seconds between weather changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float MinWeatherDuration = 120.0f;

	/** Maximum time in seconds between weather changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float MaxWeatherDuration = 600.0f;

	/** Whether this map forces a specific weather at all times. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	bool bForcedWeather = false;

	/** The forced weather type (only used if bForcedWeather is true). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	EROWeatherType ForcedWeatherType = EROWeatherType::Clear;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EROWeatherType, OldWeather, EROWeatherType, NewWeather);

/**
 * UROWeatherSystem
 * Manages per-map weather with random transitions based on configurable weights.
 * Affects post-processing, particle effects, and ambient audio.
 */
UCLASS()
class RAGNAROKUE_API UROWeatherSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Weather Control ----

	/** Force-set the current weather immediately. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EROWeatherType Weather);

	/** Get the current weather state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather")
	EROWeatherType GetCurrentWeather() const;

	/** Update weather logic. Call from a game mode tick or timer. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void TickWeather(float DeltaTime);

	// ---- Configuration ----

	/** Register weather config for a specific map. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void RegisterMapWeatherConfig(const FROMapWeatherConfig& Config);

	/** Set the active map for weather purposes (called on map load). */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetActiveMap(FName MapID);

	/** Get the weather config for a map. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	FROMapWeatherConfig GetMapWeatherConfig(FName MapID) const;

	// ---- Delegates ----

	/** Broadcast when the weather changes. */
	UPROPERTY(BlueprintAssignable, Category = "Weather")
	FOnWeatherChanged OnWeatherChanged;

protected:
	/** Current active weather. */
	UPROPERTY()
	EROWeatherType CurrentWeather = EROWeatherType::Clear;

	/** Current active map ID. */
	FName ActiveMapID;

	/** Per-map weather configurations. */
	TMap<FName, FROMapWeatherConfig> MapWeatherConfigs;

	/** Time remaining until next weather transition. */
	float TimeUntilNextChange = 0.0f;

	/** Whether the weather timer is active. */
	bool bWeatherTimerActive = false;

	/** Select a random weather based on the current map's weight configuration. */
	EROWeatherType SelectRandomWeather() const;

	/** Calculate the next weather change time. */
	float CalculateNextChangeTime() const;

	/** Initialize default weather configs for known maps. */
	void InitializeDefaultConfigs();
};
