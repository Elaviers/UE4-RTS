#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Terrain.generated.h"

USTRUCT(BlueprintType)
struct FMeshData {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FVector> Vertices;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<int> Triangles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FVector> Normals;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FVector2D> UVs;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FProcMeshTangent> Tangents;

	UPROPERTY()
		bool Loaded;
};

UCLASS()
class RTS_API ATerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerrain();

protected:
	TArray<uint8> HM_Data;
	int HM_Width, HM_Height;

	TArray<FMeshData> AllData;
	int LoadedSections;

	FTimerHandle checkTimer;

	void CreateMeshes();
	void CreateMesh(int section);
public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UProceduralMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain") float Width;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain") float Length;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain") float Height;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain") float ZOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain") UMaterial* Material;

	UFUNCTION(BlueprintCallable)
	void LoadHeightmap(FString Path);

	UFUNCTION(BlueprintCallable)
		void Build(int SectionSize = 64, float CheckDelay = 1);
	UFUNCTION(BlueprintCallable)
		void BuildOnMainThread(bool GenerateCollision = false);
	
	UFUNCTION(BlueprintCallable)
	void SetSize(FVector Size) {
		Width = Size.X;
		Length = Size.Y;
		Height = Size.Z;
	}
};
