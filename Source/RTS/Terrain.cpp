#include "RTS.h"
#include "Terrain.h"
#include "RtsFunctions.h"
#include "PhysicsEngine/BodySetup.h"

static void GenerateTangentsFromMeshData(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector2D>& UVs, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents) {

	if (Vertices.Num() == 0)
	{
		return;
	}

	// Number of triangles
	const int32 NumTris = Triangles.Num() / 3;
	// Number of verts
	const int32 NumVerts = Vertices.Num();

	// Map of vertex to triangles in Triangles array
	TMultiMap<int32, int32> VertToTriMap;
	// Map of vertex to triangles to consider for normal calculation
	TMultiMap<int32, int32> VertToTriSmoothMap;

	// Normal/tangents for each face
	TArray<FVector> FaceTangentX, FaceTangentY, FaceTangentZ;
	FaceTangentX.AddUninitialized(NumTris);
	FaceTangentY.AddUninitialized(NumTris);
	FaceTangentZ.AddUninitialized(NumTris);

	// Iterate over triangles
	for (int TriIdx = 0; TriIdx < NumTris; TriIdx++)
	{
		int32 CornerIndex[3];
		FVector P[3];

		for (int32 CornerIdx = 0; CornerIdx < 3; CornerIdx++)
		{
			// Find vert index (clamped within range)
			int32 VertIndex = FMath::Min(Triangles[(TriIdx * 3) + CornerIdx], NumVerts - 1);

			CornerIndex[CornerIdx] = VertIndex;
			P[CornerIdx] = Vertices[VertIndex];

			// Find/add this vert to index buffer
			TArray<int32> VertOverlaps;

			// Check if Verts is empty or test is outside range
			if (VertIndex < Vertices.Num())
			{
				const FVector TestVert = Vertices[VertIndex];

				for (int32 VertIdx = 0; VertIdx < Vertices.Num(); VertIdx++)
				{
					// First see if we overlap, and smoothing groups are the same
					if (TestVert.Equals(Vertices[VertIdx]))
					{
						// If it, so we are at least considered an 'overlap' for normal gen
						VertOverlaps.Add(VertIdx);
					}
				}
			}

			// Remember which triangles map to this vert
			VertToTriMap.AddUnique(VertIndex, TriIdx);
			VertToTriSmoothMap.AddUnique(VertIndex, TriIdx);

			// Also update map of triangles that 'overlap' this vert (ie don't match UV, but do match smoothing) and should be considered when calculating normal
			for (int32 OverlapIdx = 0; OverlapIdx < VertOverlaps.Num(); OverlapIdx++)
			{
				// For each vert we overlap..
				int32 OverlapVertIdx = VertOverlaps[OverlapIdx];

				// Add this triangle to that vert
				VertToTriSmoothMap.AddUnique(OverlapVertIdx, TriIdx);

				// And add all of its triangles to us
				TArray<int32> OverlapTris;
				VertToTriMap.MultiFind(OverlapVertIdx, OverlapTris);
				for (int32 OverlapTriIdx = 0; OverlapTriIdx < OverlapTris.Num(); OverlapTriIdx++)
				{
					VertToTriSmoothMap.AddUnique(VertIndex, OverlapTris[OverlapTriIdx]);
				}
			}
		}

		// Calculate triangle edge vectors and normal
		const FVector Edge21 = P[1] - P[2];
		const FVector Edge20 = P[0] - P[2];
		const FVector TriNormal = (Edge21 ^ Edge20).GetSafeNormal();

		// If we have UVs, use those to calc 
		if (UVs.Num() == Vertices.Num())
		{
			const FVector2D T1 = UVs[CornerIndex[0]];
			const FVector2D T2 = UVs[CornerIndex[1]];
			const FVector2D T3 = UVs[CornerIndex[2]];

			FMatrix	ParameterToLocal(
				FPlane(P[1].X - P[0].X, P[1].Y - P[0].Y, P[1].Z - P[0].Z, 0),
				FPlane(P[2].X - P[0].X, P[2].Y - P[0].Y, P[2].Z - P[0].Z, 0),
				FPlane(P[0].X, P[0].Y, P[0].Z, 0),
				FPlane(0, 0, 0, 1)
			);

			FMatrix ParameterToTexture(
				FPlane(T2.X - T1.X, T2.Y - T1.Y, 0, 0),
				FPlane(T3.X - T1.X, T3.Y - T1.Y, 0, 0),
				FPlane(T1.X, T1.Y, 1, 0),
				FPlane(0, 0, 0, 1)
			);

			// Use InverseSlow to catch singular matrices.  Inverse can miss this sometimes.
			const FMatrix TextureToLocal = ParameterToTexture.Inverse() * ParameterToLocal;

			FaceTangentX[TriIdx] = TextureToLocal.TransformVector(FVector(1, 0, 0)).GetSafeNormal();
			FaceTangentY[TriIdx] = TextureToLocal.TransformVector(FVector(0, 1, 0)).GetSafeNormal();
		}
		else
		{
			FaceTangentX[TriIdx] = Edge20.GetSafeNormal();
			FaceTangentY[TriIdx] = (FaceTangentX[TriIdx] ^ TriNormal).GetSafeNormal();
		}

		FaceTangentZ[TriIdx] = TriNormal;
	}


	// Arrays to accumulate tangents into
	TArray<FVector> VertexTangentXSum, VertexTangentYSum, VertexTangentZSum;
	VertexTangentXSum.AddZeroed(NumVerts);
	VertexTangentYSum.AddZeroed(NumVerts);
	VertexTangentZSum.AddZeroed(NumVerts);

	// For each vertex..
	for (int VertxIdx = 0; VertxIdx < Vertices.Num(); VertxIdx++)
	{
		// Find relevant triangles for normal
		TArray<int32> SmoothTris;
		VertToTriSmoothMap.MultiFind(VertxIdx, SmoothTris);

		for (int i = 0; i < SmoothTris.Num(); i++)
		{
			int32 TriIdx = SmoothTris[i];
			VertexTangentZSum[VertxIdx] += FaceTangentZ[TriIdx];
		}

		// Find relevant triangles for tangents
		TArray<int32> TangentTris;
		VertToTriMap.MultiFind(VertxIdx, TangentTris);

		for (int i = 0; i < TangentTris.Num(); i++)
		{
			int32 TriIdx = TangentTris[i];
			VertexTangentXSum[VertxIdx] += FaceTangentX[TriIdx];
			VertexTangentYSum[VertxIdx] += FaceTangentY[TriIdx];
		}
	}

	// Finally, normalize tangents and build output arrays

	Normals.Reset();
	Normals.AddUninitialized(NumVerts);

	Tangents.Reset();
	Tangents.AddUninitialized(NumVerts);

	for (int VertxIdx = 0; VertxIdx < NumVerts; VertxIdx++)
	{
		FVector& TangentX = VertexTangentXSum[VertxIdx];
		FVector& TangentY = VertexTangentYSum[VertxIdx];
		FVector& TangentZ = VertexTangentZSum[VertxIdx];

		TangentX.Normalize();
		TangentZ.Normalize();

		Normals[VertxIdx] = TangentZ;

		// Use Gram-Schmidt orthogonalization to make sure X is orth with Z
		TangentX -= TangentZ * (TangentZ | TangentX);
		TangentX.Normalize();

		// See if we need to flip TangentY when generating from cross product
		const bool bFlipBitangent = ((TangentZ ^ TangentX) | TangentY) < 0.f;

		Tangents[VertxIdx] = FProcMeshTangent(TangentX, bFlipBitangent);
	}
}

static void GeneerateNormalsAndTangents(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector2D>& UVs, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents) {
	
}

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