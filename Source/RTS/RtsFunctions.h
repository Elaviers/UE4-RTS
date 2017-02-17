#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImageWrapper.h"
#include "RtsFunctions.generated.h"

UCLASS()
class RTS_API URtsFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable, Category = "File", meta = (CompactNodeTitle = "WRITE"))
		static void WriteBinaryFile(TArray<uint8> data, FString File, bool& Success);

	UFUNCTION(BlueprintCallable, Category = "File", meta = (CompactNodeTitle = "READ"))
		static void ReadBinaryFile(FString File, TArray<uint8>& data, bool & Success);

	UFUNCTION(BlueprintCallable, Category = "File")
		static void WriteStringToFile(FString String, FString File, bool& Success);

	UFUNCTION(BlueprintPure, Category = "File")
		static void ReadStringFromFile(FString File, FString& String);

	//
	UFUNCTION(BlueprintPure, Category = "File")
		static void SeperateLines(TArray<FString> &Array, FString String, bool CullEmpty = true) { String.ParseIntoArrayLines(Array, CullEmpty); };

	//
	UFUNCTION(BluePrintCallable)
		static void ReadPNGRaw(FString file, TArray<uint8>& Data, int32 &Width, int32 &Height);

	UFUNCTION(BlueprintPure, Category = "File")
		static UTexture2D* ReadPNGFile(FString File);
	
	//
	UFUNCTION(BlueprintPure, Category = "File")
		static TArray<FString> FindAllFilesInDirectory(FString Directory,FString FileName);

	//
	UFUNCTION(BlueprintPure, Category = "Rendering")
		static UTexture2D* ConstructRuntimeTexture2D(UTextureRenderTarget2D *target,int Width = 256,int Height = 256);

	//
	UFUNCTION(BlueprintPure)
		static UMaterial* FindMaterial(FString Path);

	UFUNCTION(BlueprintPure)
		static UStaticMesh* FindStaticMesh(FString Path);

	//
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="->", BlueprintAutocast),Category="Math|Conversions")
		static TArray<uint8> FloatToBytes(float Float);

	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "->", BlueprintAutocast), Category = "Math|Conversions")
		static float BytesToFloat(TArray<uint8> Bytes);

	//
	UFUNCTION(BlueprintCallable, meta = (CompactNodeTitle = "LOG"))
		static void LogString(FString string) { GLog->Log(string); };
};
