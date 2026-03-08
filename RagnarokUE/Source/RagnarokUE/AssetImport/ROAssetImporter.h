// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ROAssetImporter.generated.h"

class FROGRFReader;

/**
 * Mapping of RO resource paths to UE5 content paths.
 */
USTRUCT(BlueprintType)
struct FROAssetMapping
{
	GENERATED_BODY()

	/** Original RO GRF path (e.g., "data\\sprite\\npc\\merchant.spr"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RO Import")
	FString GRFPath;

	/** Target UE5 content path (e.g., "/Game/UI/Icons/NPC/Merchant"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RO Import")
	FString ContentPath;
};

/**
 * Result from an import operation.
 */
USTRUCT(BlueprintType)
struct FROImportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RO Import")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "RO Import")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "RO Import")
	int32 ImportedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RO Import")
	int32 FailedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RO Import")
	TArray<FString> ImportedAssets;
};

/**
 * UROAssetImporter
 * GameInstance subsystem that imports Ragnarok Online assets from GRF archives
 * into UE5-compatible formats (UTexture2D for sprites/icons).
 *
 * Usage:
 * 1. Call OpenGRF() with the path to a data.grf or rdata.grf file
 * 2. Use ImportItemIcons(), ImportSkillIcons(), etc. to batch-import assets
 * 3. Or use ImportSPRToTextures() for custom sprite imports
 * 4. Call CloseGRF() when done
 *
 * At runtime (without editor), use LoadSpriteAsTexture() to load individual
 * SPR files from a GRF and create transient UTexture2D objects.
 */
UCLASS()
class RAGNAROKUE_API UROAssetImporter : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- GRF Management ---

	/**
	 * Open a GRF archive for importing assets.
	 * @param GRFFilePath Absolute path to a .grf file.
	 * @return true if the archive was opened successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	bool OpenGRF(const FString& GRFFilePath);

	/** Close the currently open GRF archive. */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	void CloseGRF();

	/** Whether a GRF archive is currently open. */
	UFUNCTION(BlueprintPure, Category = "RO|AssetImport")
	bool IsGRFOpen() const;

	/**
	 * List all files in the GRF matching a pattern.
	 * @param Pattern Wildcard pattern (e.g., "data\\sprite\\*item*.spr").
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	TArray<FString> ListGRFFiles(const FString& Pattern) const;

	// --- Sprite Import ---

	/**
	 * Load a single SPR file from the GRF and create a transient UTexture2D
	 * from a specific frame. Works at runtime without editor.
	 *
	 * @param SPRPath Internal GRF path to the .spr file.
	 * @param FrameIndex Which frame to use (0 = first frame, typically the default icon).
	 * @return The created texture, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	UTexture2D* LoadSpriteAsTexture(const FString& SPRPath, int32 FrameIndex = 0);

	/**
	 * Import all item collection icons from the GRF.
	 * Looks for sprites in data\sprite\¾ÆÀÌÅÛ\ÄÝ·º¼Ç\ (item collection path).
	 * Creates UTexture2D assets in /Game/Items/Icons/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportItemIcons();

	/**
	 * Import skill icons from the GRF.
	 * Looks for BMP/TGA textures in data\texture\À¯ÀúÀÎÅÍÆäÀÌ½º\item\ (skill icon path).
	 * Creates UTexture2D assets in /Game/UI/Icons/Skills/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportSkillIcons();

	/**
	 * Import NPC sprites from the GRF.
	 * Looks for sprites in data\sprite\npc\.
	 * Creates UTexture2D assets (first frame) in /Game/UI/Icons/NPC/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportNPCSprites();

	/**
	 * Import minimap images from the GRF.
	 * Looks for BMP files in data\texture\À¯ÀúÀÎÅÍÆäÀÌ½º\map\.
	 * Creates UTexture2D assets in /Game/UI/Minimap/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportMinimapTextures();

	/**
	 * Import monster sprites from the GRF.
	 * Extracts the first idle frame for UI display.
	 * Creates UTexture2D assets in /Game/UI/Icons/Monsters/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportMonsterSprites();

	/**
	 * Import a custom set of SPR files with explicit path mappings.
	 * @param Mappings Array of source GRF path -> target content path mappings.
	 * @param FrameIndex Which SPR frame to extract (default 0).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportCustomSprites(const TArray<FROAssetMapping>& Mappings, int32 FrameIndex = 0);

	// --- Batch Import ---

	/**
	 * Import all supported asset types from the GRF in one call.
	 * Equivalent to calling ImportItemIcons, ImportSkillIcons, ImportNPCSprites,
	 * ImportMinimapTextures, and ImportMonsterSprites.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportAllAssets();

	// --- Element Icons ---

	/**
	 * Import element attribute icons from the GRF.
	 * Creates UTexture2D assets in /Game/UI/Icons/Elements/.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	FROImportResult ImportElementIcons();

	// --- Texture Cache ---

	/**
	 * Get a previously loaded transient texture by its GRF path.
	 * @param SPRPath The GRF path used to load the texture.
	 * @return The cached texture, or nullptr if not cached.
	 */
	UFUNCTION(BlueprintPure, Category = "RO|AssetImport")
	UTexture2D* GetCachedTexture(const FString& SPRPath) const;

	/** Clear the transient texture cache. */
	UFUNCTION(BlueprintCallable, Category = "RO|AssetImport")
	void ClearTextureCache();

private:
	/** Create a UTexture2D from raw RGBA pixel data. */
	UTexture2D* CreateTextureFromRGBA(const FString& TextureName, int32 Width, int32 Height, const uint8* PixelData, bool bTransient = true);

	/** Extract a BMP/TGA file from GRF and create a texture from it. */
	UTexture2D* LoadImageAsTexture(const FString& GRFPath, bool bTransient = true);

	/** Import sprites matching a pattern and save to a content path. */
	FROImportResult ImportSpritesByPattern(const FString& GRFPattern, const FString& ContentBasePath, int32 FrameIndex = 0);

	/** Import image files (BMP/TGA) matching a pattern. */
	FROImportResult ImportImagesByPattern(const FString& GRFPattern, const FString& ContentBasePath);

	/** The currently open GRF reader. */
	TSharedPtr<FROGRFReader> GRFReader;

	/** Cache of transient textures loaded at runtime (GRF path -> texture). */
	UPROPERTY()
	TMap<FString, UTexture2D*> TextureCache;
};
