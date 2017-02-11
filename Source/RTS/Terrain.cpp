#include "RTS.h"
#include "Terrain.h"
#include "RtsFunctions.h"
#include "PhysicsEngine/BodySetup.h"

class FMeshDataTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FMeshDataTask>;

public:
	FMeshDataTask(FMeshData& Dest, TArray<uint8>& Data, int DataW, int DataH, int Width, int Length, int Height, int Segment, int SegmentSize) :
		Output(&Dest), raw(&Data), datawidth(DataW), dataheight(DataH), width(Width), length(Length), height(Height), segment(Segment), segsize(SegmentSize) {};

protected:
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMeshDataTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	FMeshData* Output;
	TArray<uint8>* raw;
	int datawidth, dataheight, width, length, height, segment, segsize;

	void DoWork() {
		int sr = 0, sc = 0;
		for (int i = 0; i < segment; i++) {
			sc += segsize;
			if (sc >= datawidth) {
				sc = 0;
				sr += segsize;
			}
		}

		int w = datawidth, h = dataheight;

		if (datawidth > segsize) {
			if (sc + segsize > datawidth)
				w = datawidth - sc;
			else
				w = segsize;
		}
		if (dataheight > segsize) {
			if (sr + segsize > dataheight)
				h = dataheight - sr;
			else
				h = segsize;
		}

		float stepx = width / (float)datawidth, stepy = length / (float)dataheight;

		if (sc + segsize < datawidth)w++;
		if (sr + segsize < dataheight)h++;

		//Generate Verts

		for (int r = 0; r < h; r++) {
			for (int c = 0; c < w; c++) {
				Output->Vertices.Add(FVector((sr + r) * stepx - (width / 2), (sc + c) * stepy - (length / 2), ((*raw)[(sr + r) * datawidth + (sc + c)] - 128) * (height / 256)));

				if (r < h - 1 && c < w - 1) {
					Output->Triangles.Add(r * w + c);
					Output->Triangles.Add(r * w + c + 1);
					Output->Triangles.Add(r * w + c + w);
					Output->Triangles.Add(r * w + c + w + 1);
					Output->Triangles.Add(r * w + c + w);
					Output->Triangles.Add(r * w + c + 1);
				}

				Output->UVs.Add(FVector2D(c, r));

				if (sr + r == 0 || sc + c == 0 || sr + r >= dataheight - 2 || sc + c >= datawidth - 2)
					Output->Normals.Add(FVector(0, 0, 1)); //Just add this if we're on a border (TEMP)
				else {
					Output->Normals.Add(((
						FVector::CrossProduct(r == 0 ?
							FVector((sr + r - 1) * stepx - (width / 2), (sc + c) * stepy - (length / 2), ((*raw)[(sr + r - 1) * datawidth + (sc + c)] - 128) * (height / 256)) - Output->Vertices[r * w + c]
							: Output->Vertices[(r - 1) * w + c] - Output->Vertices[r * w + c], c == 0 ?
							FVector((sr + r) * stepx - (width / 2), (sc + c - 1) * stepy - (length / 2), ((*raw)[(sr + r) * datawidth + (sc + c - 1)] - 128) * (height / 256)) - Output->Vertices[r * w + c]
							: Output->Vertices[r * w + c - 1] - Output->Vertices[r * w + c])
						+ FVector::CrossProduct(FVector((sr + r + 1) * stepx - (width / 2), (sc + c) * stepy - (length / 2), ((*raw)[(sr + r + 1) * datawidth + (sc + c)] - 128) * (height / 256)) - Output->Vertices[r * w + c], FVector((sr + r) * stepx - (width / 2), (sc + c + 1) * stepy - (length / 2), ((*raw)[(sr + r) * datawidth + (sc + c + 1)] - 128) * (height / 256)) - Output->Vertices[r * w + c])
						) / 2).GetSafeNormal());
				}

				//Output->Tangents.Add(FProcMeshTangent((((1 / FVector2D::Distance(bUV, aUV))*(b - a)) + (((bUV.Y - aUV.Y) / FVector2D::Distance(bUV, aUV))*FVector::CrossProduct(Output->Normals[Output->Normals.Num() - 1], (b - a)))).GetSafeNormal(),true));
				//((((bUV.X - aUV.X) / FVector2D::Distance(bUV, aUV))*(b - a)) + (((bUV.Y - aUV.Y) / FVector2D::Distance(bUV, aUV))*FVector::CrossProduct(normal, (b - a)))
			}
		}

		Output->Loaded = true;
	}
};

ATerrain::ATerrain()
{
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	PrimaryActorTick.bCanEverTick = false;

	Width = Length = 10000;
	Height = 2500;
}

void ATerrain::LoadHeightmap(FString path) {
	URtsFunctions::ReadPNGRaw(path,HM_Data,HM_Width,HM_Height);
}

void ATerrain::Build(int SectionSize,float QueryTime) {
	int segmentCount = 0;

	for (int r = 0; r < HM_Height; r += SectionSize)
		for (int c = 0; c < HM_Width; c += SectionSize)
			segmentCount++;

	GLog->Log("(Terrain) THREADED LOAD FOR " + FString::FromInt(segmentCount) + " SEGMENTS");

	AllData.Empty();
	AllData.SetNum(segmentCount);

	for (int i = 0; i < AllData.Num(); i++)
		AllData[i].Loaded = false;

	for (int i = 0; i < segmentCount; i++)
		(new FAutoDeleteAsyncTask<FMeshDataTask>(AllData[i], HM_Data, HM_Width, HM_Height, Width, Length, Height, i, SectionSize))->StartBackgroundTask();

	Mesh->ClearAllMeshSections();
	LoadedSections = 0;

	GetWorldTimerManager().SetTimer(checkTimer,this,&ATerrain::CreateMeshes, QueryTime,true);
}

void ATerrain::CreateMeshes() {
	for (int i = 0; i < AllData.Num(); i++) {
		if (AllData[i].Loaded)
			CreateMesh(i);
	}

	if (AllData.Num() == LoadedSections) {
		GetWorldTimerManager().ClearTimer(checkTimer);

		//Mesh->GetProcMeshSection(0)->bEnableCollision = true;
		//Mesh->ClearCollisionConvexMeshes();//Prompt update

		GLog->Log("Load Complete!");
	}
}

void ATerrain::CreateMesh(int segment) {
	AllData[segment].Loaded = false;
	LoadedSections++;

	Mesh->CreateMeshSection(segment, 
		AllData[segment].Vertices, AllData[segment].Triangles, AllData[segment].Normals, AllData[segment].UVs, TArray<FColor>(), AllData[segment].Tangents, 
		false);

	Mesh->SetMaterial(segment, Material);

	GLog->Log("Created segment " + FString::FromInt(segment));
}

void ATerrain::BuildOnMainThread(bool collision) {
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UVs;
	TArray<FVector> Normals;

	float stepx = Width / (float)HM_Width;
	float stepy = Length / (float)HM_Height;

	for (int r = 0; r < HM_Height; r++) {
		for (int c = 0; c < HM_Width; c++) {
			Vertices.Add(FVector(r * stepx - (Width / 2), c * stepy - (Length / 2), (HM_Data[r * HM_Width + c] - ZOffset) * (Height / 256)));

			if (r < HM_Height - 1 && c < HM_Width - 1) {
				Triangles.Add(r * HM_Width + c);
				Triangles.Add(r * HM_Width + c + 1);
				Triangles.Add(r * HM_Width + c + HM_Width);
				Triangles.Add(r * HM_Width + c + HM_Width + 1);
				Triangles.Add(r * HM_Width + c + HM_Width);
				Triangles.Add(r * HM_Width + c + 1);
			}

			UVs.Add(FVector2D(c, r));

			if (r == 0 || c == 0 || r >= HM_Height - 2 || c >= HM_Width - 2)
				Normals.Add(FVector(0, 0, 1)); //Just add this if we're on a border (TEMP)
			else {
				Normals.Add(((
					FVector::CrossProduct(Vertices[(r - 1) * HM_Width + c] - Vertices[r * HM_Width + c], Vertices[r * HM_Width + c - 1] - Vertices[r * HM_Width + c])
					+ 
					FVector::CrossProduct(FVector((r + 1) * stepx - (Width / 2), c * stepy - (Length / 2), (HM_Data[(r + 1) * HM_Width + c] - ZOffset) * (Height / 256)) - Vertices[r * HM_Width + c], FVector(r * stepx - (Width / 2), (c + 1) * stepy - (Length / 2), (HM_Data[r * HM_Width + c + 1] - ZOffset) * (Height / 256)) - Vertices[r * HM_Width + c])
				)).GetSafeNormal());
			}

		}
	}

	Mesh->ClearAllMeshSections();

	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), collision);

	Mesh->SetMaterial(0, Material);

}