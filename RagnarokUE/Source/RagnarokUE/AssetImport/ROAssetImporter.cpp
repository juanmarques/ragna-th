// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAssetImporter.h"
#include "ROGRFReader.h"
#include "ROSPRReader.h"
#include "ROACTReader.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"
#endif

void UROAssetImporter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Initialized"));
}

void UROAssetImporter::Deinitialize()
{
	CloseGRF();
	ClearTextureCache();
	Super::Deinitialize();
}

// --- GRF Management ---

bool UROAssetImporter::OpenGRF(const FString& GRFFilePath)
{
	CloseGRF();

	GRFReader = MakeShared<FROGRFReader>();
	if (!GRFReader->Open(GRFFilePath))
	{
		GRFReader.Reset();
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Opened GRF with %d files"), GRFReader->GetFileCount());
	return true;
}

void UROAssetImporter::CloseGRF()
{
	if (GRFReader.IsValid())
	{
		GRFReader->Close();
		GRFReader.Reset();
	}
}

bool UROAssetImporter::IsGRFOpen() const
{
	return GRFReader.IsValid() && GRFReader->IsOpen();
}

TArray<FString> UROAssetImporter::ListGRFFiles(const FString& Pattern) const
{
	if (!IsGRFOpen())
	{
		return TArray<FString>();
	}
	return GRFReader->FindFiles(Pattern);
}

// --- Sprite Import ---

UTexture2D* UROAssetImporter::LoadSpriteAsTexture(const FString& SPRPath, int32 FrameIndex)
{
	if (!IsGRFOpen())
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: No GRF open"));
		return nullptr;
	}

	// Check cache first
	if (UTexture2D* Cached = GetCachedTexture(SPRPath))
	{
		return Cached;
	}

	// Extract SPR data from GRF
	TArray<uint8> SPRData;
	if (!GRFReader->ExtractFile(SPRPath, SPRData))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to extract: %s"), *SPRPath);
		return nullptr;
	}

	// Parse the SPR file
	FROSPRReader SPRReader;
	if (!SPRReader.Parse(SPRData))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to parse SPR: %s"), *SPRPath);
		return nullptr;
	}

	const FROSPRFrame* Frame = SPRReader.GetFrame(FrameIndex);
	if (!Frame || !Frame->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Invalid frame %d in: %s"), FrameIndex, *SPRPath);
		return nullptr;
	}

	// Create texture name from file path
	FString TextureName = FPaths::GetBaseFilename(SPRPath);
	UTexture2D* Texture = CreateTextureFromRGBA(TextureName, Frame->Width, Frame->Height, Frame->PixelData.GetData());

	if (Texture)
	{
		TextureCache.Add(SPRPath, Texture);
	}

	return Texture;
}

FROImportResult UROAssetImporter::ImportItemIcons()
{
	// RO item collection sprites are in data\sprite\¾ÆÀÌÅÛ\ÄÝ·º¼Ç\ (Korean encoded)
	// Also check the ASCII-safe alternative path
	FROImportResult Result = ImportSpritesByPattern(
		TEXT("data\\sprite\\*item*\\*.spr"),
		TEXT("/Game/Items/Icons/")
	);

	// Also try the direct item icon BMP/TGA path
	FROImportResult ImageResult = ImportImagesByPattern(
		TEXT("data\\texture\\*item*.bmp"),
		TEXT("/Game/Items/Icons/")
	);

	Result.ImportedCount += ImageResult.ImportedCount;
	Result.FailedCount += ImageResult.FailedCount;
	Result.ImportedAssets.Append(ImageResult.ImportedAssets);
	Result.bSuccess = Result.bSuccess || ImageResult.bSuccess;

	if (Result.ImportedCount > 0)
	{
		Result.Message = FString::Printf(TEXT("Imported %d item icons"), Result.ImportedCount);
	}
	else
	{
		Result.Message = TEXT("No item icons found in GRF. Check that the GRF contains data\\sprite\\ item paths.");
	}

	return Result;
}

FROImportResult UROAssetImporter::ImportSkillIcons()
{
	// Skill icons in RO are stored as BMP files in the UI texture path
	FROImportResult Result = ImportImagesByPattern(
		TEXT("data\\texture\\*skill*.bmp"),
		TEXT("/Game/UI/Icons/Skills/")
	);

	// Also check TGA format
	FROImportResult TGAResult = ImportImagesByPattern(
		TEXT("data\\texture\\*skill*.tga"),
		TEXT("/Game/UI/Icons/Skills/")
	);

	Result.ImportedCount += TGAResult.ImportedCount;
	Result.FailedCount += TGAResult.FailedCount;
	Result.ImportedAssets.Append(TGAResult.ImportedAssets);

	if (Result.ImportedCount > 0)
	{
		Result.Message = FString::Printf(TEXT("Imported %d skill icons"), Result.ImportedCount);
		Result.bSuccess = true;
	}
	else
	{
		Result.Message = TEXT("No skill icons found in GRF. Check that the GRF contains skill icon textures.");
	}

	return Result;
}

FROImportResult UROAssetImporter::ImportNPCSprites()
{
	return ImportSpritesByPattern(
		TEXT("data\\sprite\\npc\\*.spr"),
		TEXT("/Game/UI/Icons/NPC/")
	);
}

FROImportResult UROAssetImporter::ImportMinimapTextures()
{
	FROImportResult Result = ImportImagesByPattern(
		TEXT("data\\texture\\*map*.bmp"),
		TEXT("/Game/UI/Minimap/")
	);

	FROImportResult TGAResult = ImportImagesByPattern(
		TEXT("data\\texture\\*map*.tga"),
		TEXT("/Game/UI/Minimap/")
	);

	Result.ImportedCount += TGAResult.ImportedCount;
	Result.FailedCount += TGAResult.FailedCount;
	Result.ImportedAssets.Append(TGAResult.ImportedAssets);
	Result.bSuccess = Result.bSuccess || TGAResult.bSuccess;

	if (Result.ImportedCount > 0)
	{
		Result.Message = FString::Printf(TEXT("Imported %d minimap textures"), Result.ImportedCount);
	}
	else
	{
		Result.Message = TEXT("No minimap textures found in GRF.");
	}

	return Result;
}

FROImportResult UROAssetImporter::ImportMonsterSprites()
{
	return ImportSpritesByPattern(
		TEXT("data\\sprite\\¸ó½ºÅÍ\\*.spr"),
		TEXT("/Game/UI/Icons/Monsters/")
	);
}

FROImportResult UROAssetImporter::ImportCustomSprites(const TArray<FROAssetMapping>& Mappings, int32 FrameIndex)
{
	FROImportResult Result;

	if (!IsGRFOpen())
	{
		Result.Message = TEXT("No GRF archive is open.");
		return Result;
	}

	for (const FROAssetMapping& Mapping : Mappings)
	{
		UTexture2D* Texture = LoadSpriteAsTexture(Mapping.GRFPath, FrameIndex);
		if (Texture)
		{
			Result.ImportedCount++;
			Result.ImportedAssets.Add(Mapping.ContentPath);
		}
		else
		{
			Result.FailedCount++;
		}
	}

	Result.bSuccess = Result.ImportedCount > 0;
	Result.Message = FString::Printf(TEXT("Imported %d/%d custom sprites"),
		Result.ImportedCount, Mappings.Num());

	return Result;
}

FROImportResult UROAssetImporter::ImportAllAssets()
{
	FROImportResult TotalResult;

	if (!IsGRFOpen())
	{
		TotalResult.Message = TEXT("No GRF archive is open.");
		return TotalResult;
	}

	auto MergeResult = [&TotalResult](const FROImportResult& Sub)
	{
		TotalResult.ImportedCount += Sub.ImportedCount;
		TotalResult.FailedCount += Sub.FailedCount;
		TotalResult.ImportedAssets.Append(Sub.ImportedAssets);
	};

	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Starting full asset import..."));

	MergeResult(ImportItemIcons());
	MergeResult(ImportSkillIcons());
	MergeResult(ImportNPCSprites());
	MergeResult(ImportMinimapTextures());
	MergeResult(ImportMonsterSprites());
	MergeResult(ImportElementIcons());

	TotalResult.bSuccess = TotalResult.ImportedCount > 0;
	TotalResult.Message = FString::Printf(
		TEXT("Full import complete: %d assets imported, %d failed"),
		TotalResult.ImportedCount, TotalResult.FailedCount);

	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: %s"), *TotalResult.Message);
	return TotalResult;
}

FROImportResult UROAssetImporter::ImportElementIcons()
{
	FROImportResult Result = ImportImagesByPattern(
		TEXT("data\\texture\\*attr*.bmp"),
		TEXT("/Game/UI/Icons/Elements/")
	);

	if (Result.ImportedCount == 0)
	{
		// Try sprite-based element icons
		Result = ImportSpritesByPattern(
			TEXT("data\\sprite\\*attr*.spr"),
			TEXT("/Game/UI/Icons/Elements/")
		);
	}

	if (Result.ImportedCount > 0)
	{
		Result.Message = FString::Printf(TEXT("Imported %d element icons"), Result.ImportedCount);
	}
	else
	{
		Result.Message = TEXT("No element icons found in GRF.");
	}

	return Result;
}

// --- PNG File Loading ---

FROImportResult UROAssetImporter::LoadFromPNGFiles()
{
	FROImportResult Result;

	// Find the Content directory
	FString ContentDir = FPaths::ProjectContentDir();
	FString ManifestPath = ContentDir / TEXT("Data") / TEXT("asset_manifest.json");

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ManifestPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: No asset manifest found at: %s"), *ManifestPath);
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Run 'python3 Tools/download_ro_assets.py' to download assets first."));
		Result.Message = TEXT("No asset_manifest.json found. Run Tools/download_ro_assets.py first.");
		return Result;
	}

	TSharedPtr<FJsonObject> JsonRoot;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
	{
		Result.Message = TEXT("Failed to parse asset_manifest.json");
		return Result;
	}

	auto LoadCategory = [&](const FString& Category, TMap<int32, UTexture2D*>& Cache)
	{
		const TSharedPtr<FJsonObject>* CategoryObj = nullptr;
		if (!JsonRoot->TryGetObjectField(Category, CategoryObj))
		{
			return;
		}

		for (const auto& Pair : (*CategoryObj)->Values)
		{
			int32 ID = FCString::Atoi(*Pair.Key);
			FString RelPath = Pair.Value->AsString();
			FString FullPath = ContentDir / RelPath;

			if (!FPaths::FileExists(FullPath))
			{
				Result.FailedCount++;
				continue;
			}

			UTexture2D* Texture = LoadPNGAsTexture(FullPath);
			if (Texture)
			{
				Cache.Add(ID, Texture);
				Result.ImportedCount++;
				Result.ImportedAssets.Add(RelPath);
			}
			else
			{
				Result.FailedCount++;
			}
		}
	};

	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Loading PNG assets from manifest..."));

	LoadCategory(TEXT("items"), ItemIconCache);
	LoadCategory(TEXT("skills"), SkillIconCache);
	LoadCategory(TEXT("monsters"), MonsterIconCache);
	LoadCategory(TEXT("elements"), ElementIconCache);

	Result.bSuccess = Result.ImportedCount > 0;
	Result.Message = FString::Printf(TEXT("Loaded %d PNG assets (%d failed)"),
		Result.ImportedCount, Result.FailedCount);
	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: %s"), *Result.Message);

	return Result;
}

UTexture2D* UROAssetImporter::LoadPNGAsTexture(const FString& FilePath)
{
	// Check cache
	if (UTexture2D* Cached = GetCachedTexture(FilePath))
	{
		return Cached;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to read file: %s"), *FilePath);
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	// Detect format from extension
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	EImageFormat Format = EImageFormat::PNG;
	if (Extension == TEXT("bmp"))
	{
		Format = EImageFormat::BMP;
	}
	else if (Extension == TEXT("tga"))
	{
		Format = EImageFormat::TGA;
	}
	else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
	{
		Format = EImageFormat::JPEG;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to decode: %s"), *FilePath);
		return nullptr;
	}

	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
	{
		return nullptr;
	}

	FString TextureName = FPaths::GetBaseFilename(FilePath);
	UTexture2D* Texture = CreateTextureFromRGBA(TextureName, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), RawData.GetData());

	if (Texture)
	{
		TextureCache.Add(FilePath, Texture);
	}

	return Texture;
}

UTexture2D* UROAssetImporter::GetItemIcon(int32 ItemID) const
{
	const TObjectPtr<UTexture2D>* Found = ItemIconCache.Find(ItemID);
	return Found ? *Found : nullptr;
}

UTexture2D* UROAssetImporter::GetSkillIcon(int32 SkillID) const
{
	const TObjectPtr<UTexture2D>* Found = SkillIconCache.Find(SkillID);
	return Found ? *Found : nullptr;
}

UTexture2D* UROAssetImporter::GetMonsterIcon(int32 MonsterID) const
{
	const TObjectPtr<UTexture2D>* Found = MonsterIconCache.Find(MonsterID);
	return Found ? *Found : nullptr;
}

UTexture2D* UROAssetImporter::GetElementIcon(int32 ElementIndex) const
{
	const TObjectPtr<UTexture2D>* Found = ElementIconCache.Find(ElementIndex);
	return Found ? *Found : nullptr;
}

// --- Texture Cache ---

UTexture2D* UROAssetImporter::GetCachedTexture(const FString& Key) const
{
	const TObjectPtr<UTexture2D>* Found = TextureCache.Find(Key);
	return Found ? *Found : nullptr;
}

void UROAssetImporter::ClearTextureCache()
{
	TextureCache.Empty();
	ItemIconCache.Empty();
	SkillIconCache.Empty();
	MonsterIconCache.Empty();
	ElementIconCache.Empty();
}

// --- Private Helpers ---

UTexture2D* UROAssetImporter::CreateTextureFromRGBA(const FString& TextureName, int32 Width, int32 Height, const uint8* PixelData, bool bTransient)
{
	if (Width <= 0 || Height <= 0 || !PixelData)
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8, *TextureName);
	if (!Texture)
	{
		return nullptr;
	}

	// Lock and copy pixel data
	void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, PixelData, Width * Height * 4);
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Configure texture settings for pixel art
	Texture->Filter = TF_Nearest;
	Texture->CompressionSettings = TC_EditorIcon;
	Texture->SRGB = true;
	Texture->LODGroup = TEXTUREGROUP_UI;
	Texture->NeverStream = true;

	Texture->UpdateResource();

	return Texture;
}

UTexture2D* UROAssetImporter::LoadImageAsTexture(const FString& GRFPath, bool bTransient)
{
	if (!IsGRFOpen())
	{
		return nullptr;
	}

	TArray<uint8> FileData;
	if (!GRFReader->ExtractFile(GRFPath, FileData))
	{
		return nullptr;
	}

	// Determine format and decode
	FString Extension = FPaths::GetExtension(GRFPath).ToLower();
	FString TextureName = FPaths::GetBaseFilename(GRFPath);

	// Use UE's built-in image wrapper to decode BMP/TGA/PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	EImageFormat Format = EImageFormat::Invalid;
	if (Extension == TEXT("bmp"))
	{
		Format = EImageFormat::BMP;
	}
	else if (Extension == TEXT("tga"))
	{
		Format = EImageFormat::TGA;
	}
	else if (Extension == TEXT("png"))
	{
		Format = EImageFormat::PNG;
	}
	else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
	{
		Format = EImageFormat::JPEG;
	}

	if (Format == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Unsupported image format: %s"), *Extension);
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to decode image: %s"), *GRFPath);
		return nullptr;
	}

	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAssetImporter: Failed to get raw pixels from: %s"), *GRFPath);
		return nullptr;
	}

	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();

	UTexture2D* Texture = CreateTextureFromRGBA(TextureName, Width, Height, RawData.GetData(), bTransient);

	if (Texture)
	{
		TextureCache.Add(GRFPath, Texture);
	}

	return Texture;
}

FROImportResult UROAssetImporter::ImportSpritesByPattern(const FString& GRFPattern, const FString& ContentBasePath, int32 FrameIndex)
{
	FROImportResult Result;

	if (!IsGRFOpen())
	{
		Result.Message = TEXT("No GRF archive is open.");
		return Result;
	}

	TArray<FString> MatchedFiles = GRFReader->FindFiles(GRFPattern);
	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Pattern '%s' matched %d files"), *GRFPattern, MatchedFiles.Num());

	for (const FString& FilePath : MatchedFiles)
	{
		// Only process .spr files
		if (!FilePath.EndsWith(TEXT(".spr")))
		{
			continue;
		}

		UTexture2D* Texture = LoadSpriteAsTexture(FilePath, FrameIndex);
		if (Texture)
		{
			FString AssetName = FPaths::GetBaseFilename(FilePath);
			FString AssetPath = ContentBasePath + AssetName;
			Result.ImportedCount++;
			Result.ImportedAssets.Add(AssetPath);
		}
		else
		{
			Result.FailedCount++;
		}
	}

	Result.bSuccess = Result.ImportedCount > 0;
	Result.Message = FString::Printf(TEXT("Imported %d sprites from pattern '%s'"),
		Result.ImportedCount, *GRFPattern);

	return Result;
}

FROImportResult UROAssetImporter::ImportImagesByPattern(const FString& GRFPattern, const FString& ContentBasePath)
{
	FROImportResult Result;

	if (!IsGRFOpen())
	{
		Result.Message = TEXT("No GRF archive is open.");
		return Result;
	}

	TArray<FString> MatchedFiles = GRFReader->FindFiles(GRFPattern);
	UE_LOG(LogTemp, Log, TEXT("ROAssetImporter: Image pattern '%s' matched %d files"), *GRFPattern, MatchedFiles.Num());

	for (const FString& FilePath : MatchedFiles)
	{
		UTexture2D* Texture = LoadImageAsTexture(FilePath);
		if (Texture)
		{
			FString AssetName = FPaths::GetBaseFilename(FilePath);
			FString AssetPath = ContentBasePath + AssetName;
			Result.ImportedCount++;
			Result.ImportedAssets.Add(AssetPath);
		}
		else
		{
			Result.FailedCount++;
		}
	}

	Result.bSuccess = Result.ImportedCount > 0;
	Result.Message = FString::Printf(TEXT("Imported %d images from pattern '%s'"),
		Result.ImportedCount, *GRFPattern);

	return Result;
}
