// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWeatherSystem.h"

void UROWeatherSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDefaultConfigs();
	CurrentWeather = EROWeather::Clear;
	bWeatherTimerActive = false;
}

void UROWeatherSystem::Deinitialize()
{
	MapWeatherConfigs.Empty();
	Super::Deinitialize();
}

void UROWeatherSystem::SetWeather(EROWeather Weather)
{
	if (CurrentWeather == Weather)
	{
		return;
	}

	const EROWeather OldWeather = CurrentWeather;
	CurrentWeather = Weather;

	UE_LOG(LogTemp, Log, TEXT("Weather changed: %d -> %d"), static_cast<uint8>(OldWeather), static_cast<uint8>(Weather));

	// TODO: Trigger visual effects:
	// - Update post-processing volume (brightness, fog, color grading)
	// - Enable/disable particle systems (rain, snow, fog)
	// - Adjust ambient sounds

	OnWeatherChanged.Broadcast(OldWeather, Weather);
}

EROWeather UROWeatherSystem::GetCurrentWeather() const
{
	return CurrentWeather;
}

void UROWeatherSystem::TickWeather(float DeltaTime)
{
	if (!bWeatherTimerActive)
	{
		return;
	}

	// Check for forced weather
	const FROMapWeatherConfig* Config = MapWeatherConfigs.Find(ActiveMapID);
	if (Config && Config->bForcedWeather)
	{
		if (CurrentWeather != Config->ForcedWeatherType)
		{
			SetWeather(Config->ForcedWeatherType);
		}
		return;
	}

	TimeUntilNextChange -= DeltaTime;

	if (TimeUntilNextChange <= 0.0f)
	{
		// Transition to a new random weather
		const EROWeather NewWeather = SelectRandomWeather();
		SetWeather(NewWeather);
		TimeUntilNextChange = CalculateNextChangeTime();
	}
}

void UROWeatherSystem::RegisterMapWeatherConfig(const FROMapWeatherConfig& Config)
{
	MapWeatherConfigs.Add(Config.MapID, Config);
}

void UROWeatherSystem::SetActiveMap(FName MapID)
{
	ActiveMapID = MapID;
	bWeatherTimerActive = true;
	TimeUntilNextChange = CalculateNextChangeTime();

	// Apply forced weather immediately if configured
	const FROMapWeatherConfig* Config = MapWeatherConfigs.Find(MapID);
	if (Config && Config->bForcedWeather)
	{
		SetWeather(Config->ForcedWeatherType);
	}

	UE_LOG(LogTemp, Log, TEXT("Weather system active for map: %s"), *MapID.ToString());
}

FROMapWeatherConfig UROWeatherSystem::GetMapWeatherConfig(FName MapID) const
{
	const FROMapWeatherConfig* Config = MapWeatherConfigs.Find(MapID);
	if (Config)
	{
		return *Config;
	}
	return FROMapWeatherConfig();
}

EROWeather UROWeatherSystem::SelectRandomWeather() const
{
	const FROMapWeatherConfig* Config = MapWeatherConfigs.Find(ActiveMapID);
	if (!Config || Config->WeatherWeights.Num() == 0)
	{
		return EROWeather::Clear;
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (const auto& Pair : Config->WeatherWeights)
	{
		TotalWeight += Pair.Value;
	}

	if (TotalWeight <= 0.0f)
	{
		return EROWeather::Clear;
	}

	// Random selection based on weights
	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (const auto& Pair : Config->WeatherWeights)
	{
		Roll -= Pair.Value;
		if (Roll <= 0.0f)
		{
			return Pair.Key;
		}
	}

	return EROWeather::Clear;
}

float UROWeatherSystem::CalculateNextChangeTime() const
{
	const FROMapWeatherConfig* Config = MapWeatherConfigs.Find(ActiveMapID);
	if (Config)
	{
		return FMath::FRandRange(Config->MinWeatherDuration, Config->MaxWeatherDuration);
	}
	// Default: 3-8 minutes
	return FMath::FRandRange(180.0f, 480.0f);
}

void UROWeatherSystem::InitializeDefaultConfigs()
{
	// Prontera: outdoor city, mild weather
	{
		FROMapWeatherConfig Config;
		Config.MapID = FName(TEXT("prontera"));
		Config.WeatherWeights.Add(EROWeather::Clear, 50.0f);
		Config.WeatherWeights.Add(EROWeather::Cloudy, 25.0f);
		Config.WeatherWeights.Add(EROWeather::Rain, 15.0f);
		Config.WeatherWeights.Add(EROWeather::Foggy, 10.0f);
		Config.MinWeatherDuration = 180.0f;
		Config.MaxWeatherDuration = 600.0f;
		MapWeatherConfigs.Add(Config.MapID, Config);
	}

	// Prontera fields: same as city
	for (int32 i = 1; i <= 8; ++i)
	{
		FROMapWeatherConfig Config;
		Config.MapID = FName(*FString::Printf(TEXT("prt_fild%02d"), i));
		Config.WeatherWeights.Add(EROWeather::Clear, 45.0f);
		Config.WeatherWeights.Add(EROWeather::Cloudy, 25.0f);
		Config.WeatherWeights.Add(EROWeather::Rain, 20.0f);
		Config.WeatherWeights.Add(EROWeather::Storm, 5.0f);
		Config.WeatherWeights.Add(EROWeather::Foggy, 5.0f);
		Config.MinWeatherDuration = 120.0f;
		Config.MaxWeatherDuration = 480.0f;
		MapWeatherConfigs.Add(Config.MapID, Config);
	}

	// Prontera Culvert: always dark/indoor (forced)
	for (int32 i = 1; i <= 4; ++i)
	{
		FROMapWeatherConfig Config;
		Config.MapID = FName(*FString::Printf(TEXT("prt_sewb%d"), i));
		Config.bForcedWeather = true;
		Config.ForcedWeatherType = EROWeather::Foggy;
		MapWeatherConfigs.Add(Config.MapID, Config);
	}

	// Izlude: coastal town
	{
		FROMapWeatherConfig Config;
		Config.MapID = FName(TEXT("izlude"));
		Config.WeatherWeights.Add(EROWeather::Clear, 55.0f);
		Config.WeatherWeights.Add(EROWeather::Cloudy, 20.0f);
		Config.WeatherWeights.Add(EROWeather::Rain, 15.0f);
		Config.WeatherWeights.Add(EROWeather::Storm, 10.0f);
		Config.MinWeatherDuration = 150.0f;
		Config.MaxWeatherDuration = 500.0f;
		MapWeatherConfigs.Add(Config.MapID, Config);
	}

	// Byalan dungeon: underwater themed, always foggy
	for (int32 i = 1; i <= 5; ++i)
	{
		FROMapWeatherConfig Config;
		Config.MapID = FName(*FString::Printf(TEXT("iz_dun%02d"), i));
		Config.bForcedWeather = true;
		Config.ForcedWeatherType = EROWeather::Foggy;
		MapWeatherConfigs.Add(Config.MapID, Config);
	}
}
