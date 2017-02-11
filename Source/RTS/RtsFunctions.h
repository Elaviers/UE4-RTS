#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImageWrapper.h"
#include "StringPair.h"
#include "RtsFunctions.generated.h"

UCLASS()
class RTS_API URtsFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

	UFUNCTION(BlueprintCallable, Category = "File")
		static void WriteStringToFile(FString String, FString File, bool& Success);

	UFUNCTION(BlueprintPure, Category = "File")
		static void ReadStringFromFile(FString File, FString& String);

	UFUNCTION(BlueprintPure, Category = "File")
		static UTexture2D* ReadPNGFile(FString File);
	
	UFUNCTION(BluePrintPure, Category = "File")
		static TArray<FString> FindAllFilesInDirectory(FString Directory,FString FileName);

	UFUNCTION(BluePrintPure, Category = "Utilities|Array|StringPair")
		static void GetValueOfProperty(TArray<FStringPair> Array,FString Property, FString Default,FString& Value,bool& Found);

	UFUNCTION(BluePrintPure, Category = "Utilities|CustomObject")
		static bool Equals(TArray<FStringPair> one, TArray<FStringPair> two);

	UFUNCTION(BluePrintPure, Category = "Rendering")
		static UTexture2D* ConstructRuntimeTexture2D(UTextureRenderTarget2D *target,int Width = 256,int Height = 256);

	UFUNCTION(BluePrintCallable)
		static void ReadPNGRaw(FString file, TArray<uint8>& Data, int32 &Width,int32 &Height);

	static void ReadPNGRaw16(FString file, TArray<uint8>& Data, int32 &Width, int32 &Height);

	UFUNCTION(BluePrintPure)
		static TArray<FVector> LoadHeightmapFromPNG(FString Path,float Width,float Length,float Height);

	UFUNCTION(BluePrintPure)
		static UMaterial* FindMaterial(FString Path);

	UFUNCTION(BluePrintPure)
		static UStaticMesh* FindStaticMesh(FString Path);
};
