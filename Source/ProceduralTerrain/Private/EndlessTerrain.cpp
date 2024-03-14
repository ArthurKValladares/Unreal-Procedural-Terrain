// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

FTerrainChunk::FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint ChunkCoord, float  Size)
	: MapLod(EMapLod::One)
	, ChunkCoord(ChunkCoord)
{
	FVector2D Center = ChunkCoord * Size;
	const float HalfSize = (float)Size / 2.;

	Rect = FBox2D(Center - HalfSize, Center + HalfSize);
	SectionIndex = ParentTerrain->CurrSectionIndex();
}

void FTerrainChunk::Init(AEndlessTerrain* ParentTerrain) {
	Noise.Init(ENormalizeMode::Global, ParentTerrain->RandomSeed, ParentTerrain->VerticesInChunk, ParentTerrain->VerticesInChunk, ParentTerrain->Scale, ParentTerrain->Octaves, ParentTerrain->Persistance, ParentTerrain->Lacunarity, ChunkCoord * (ParentTerrain->VerticesInChunk - 1));

	const FVector2D Center = Rect.GetCenter();
	const float HalfSize = Rect.GetSize().X / 2.;
	check(Rect.GetSize().X == Rect.GetSize().Y);

	const float XOffset = -HalfSize;
	const float YOffset = -HalfSize;
	const float ZOffset = 0.0;

	const int Width = AEndlessTerrain::VerticesInChunk;
	const int Height = AEndlessTerrain::VerticesInChunk;

	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			// TODO: Duplicated code here, think about it soon, maybe save normalized noise instead of absolute
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];

			const float XPos = X * AEndlessTerrain::TileSize;
			const float YPos = Y * AEndlessTerrain::TileSize;
			float MultiplierEffectiveness = 1.0;
			if (IsValid(ParentTerrain->ElevationCurve)) {
				MultiplierEffectiveness = ParentTerrain->ElevationCurve->GetFloatValue(NoiseValue);
			}
			Vertices.Add(FVector(Center.X + XPos + XOffset, Center.Y + YPos + YOffset, ZOffset + MultiplierEffectiveness * ParentTerrain->ElevationMultiplier));

			const float U = (float)X / Width;
			const float V = (float)Y / Width;
			Uv0.Add(FVector2D(U, V));
		}
	}
}

void FTerrainChunk::SetLod(AEndlessTerrain* ParentTerrain, EMapLod Lod) {
	if (MapLod == Lod) return;

	MapLod = Lod;
	TArray<int32>* LodTriangles = ParentTerrain->TriangleMap.Find(Lod);
	if (LodTriangles != nullptr) {
		ParentTerrain->MeshMutex.Lock();
		ParentTerrain->GetMesh()->CreateMeshSection_LinearColor(SectionIndex, Vertices, *LodTriangles, {}, Uv0, {}, {}, false);
		ParentTerrain->MeshMutex.Unlock();
	}
	else {
		TArray<int32> Triangles;

		const int Width = AEndlessTerrain::VerticesInChunk;
		const int Height = AEndlessTerrain::VerticesInChunk;

		const int StepSize = static_cast<int>(MapLod);
		const int XStepOffset = StepSize;
		const int YStepOffset = Width * StepSize;

		for (int Y = 0; Y < Height; Y += StepSize) {
			for (int X = 0; X < Width; X += StepSize) {
				if (Y < (Height - StepSize) && X < (Width - StepSize)) {
					const int CurrentIndex = Y * Width + X;
					// Vertex setup
					//   0      1      2      3    ..    W-1
					// (0+W)  (1+W)  (2+W)  (3+W)  .. (2W - 1)
					// ...

					// Both triangles need to have counter-clockwise winding-order
					//     X
					//     |\
					//     | \
					// X+W --- x+W+1
					Triangles.Add(CurrentIndex);
					Triangles.Add(CurrentIndex + YStepOffset);
					Triangles.Add(CurrentIndex + YStepOffset + XStepOffset);

					// X --- X+1    
					//   \ |
					//    \|
					//     X+W+1
					Triangles.Add(CurrentIndex);
					Triangles.Add(CurrentIndex + YStepOffset + XStepOffset);
					Triangles.Add(CurrentIndex + XStepOffset);
				}
			}
		}

		ParentTerrain->MeshMutex.Lock();
		ParentTerrain->GetMesh()->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, {}, Uv0, {}, {}, false);
		ParentTerrain->TriangleMap.Add(Lod, std::move(Triangles));
		ParentTerrain->MeshMutex.Unlock();
	}
}

bool FTerrainChunk::IsInVisibleDistance(FVector2D SourceLocation, float ViewDistance) const {
	const float Distance = FGenericPlatformMath::Sqrt(Rect.ComputeSquaredDistanceToPoint(SourceLocation));
	return Distance <= ViewDistance;
}

AEndlessTerrain::AEndlessTerrain()
	: Scale(60.)
	, Octaves(1)
	, Persistance(0.5)
	, Lacunarity(1.0)
	, ViewDistance(1000.0)
	, Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("EndlessMesh"))
	, Material(CreateDefaultSubobject<UMaterial>("EndlessMaterial"))
	, ElevationMultiplier(AEndlessTerrain::VerticesInChunk / 2.)
	, ElevationCurve(CreateDefaultSubobject<UCurveFloat>("ElevationCurve"))
	, RandomSeed(0)
	, RandomStream(FRandomStream(RandomSeed))
{
	check(Mesh);
	check(Material);

	RootComponent = Mesh;

	PrimaryActorTick.bCanEverTick = true;
}

AEndlessTerrain::~AEndlessTerrain()
{
}

void AEndlessTerrain::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	//UpdateVisibleChunks();
}

int AEndlessTerrain::NumChunksInViewDistance() const {
	return FGenericPlatformMath::RoundToInt(ViewDistance / ((VerticesInChunk - 1) * TileSize));
}

int AEndlessTerrain::CurrSectionIndex() const {
	return TerrainMap.Num();
}

void AEndlessTerrain::UpdateVisibleChunks() {
	const auto* Player = GetWorld()->GetFirstPlayerController();
	FVector Location;
	if (Player) {
		Location = Player->GetPawnOrSpectator()->GetActorLocation();
	}
	else {
		Location = FVector(0.);
	}
	const FVector2D Location2D = FVector2D(Location.X, Location.Y);

	// Update Chunks visible last frame
	for (const FIntPoint& ChunkCoord : ChunksVisibleLastFrame) {
		const FTerrainChunk& ChunkRef = TerrainMap[ChunkCoord];
		if (!ChunkRef.IsInVisibleDistance(Location2D, ViewDistance)) {
			Mesh->SetMeshSectionVisible(ChunkRef.GetSectionIndex(), false);
		}
	}
	ChunksVisibleLastFrame.Empty();

	// Test Chunks around Player Location
	TArray<FIntPoint> NewChunksThisFrame;
	const FIntPoint OriginChunkCoord = FIntPoint(FGenericPlatformMath::RoundToInt(Location.X / ChunkSize()), FGenericPlatformMath::RoundToInt(Location.Y / ChunkSize()));
	const int ChunksInViewDistance = NumChunksInViewDistance();
	for (int YOffset = -ChunksInViewDistance; YOffset <= ChunksInViewDistance; ++YOffset) {
		for (int XOffset = -ChunksInViewDistance; XOffset <= ChunksInViewDistance; ++XOffset) {
			const FIntPoint CurrentChunkOffset = FIntPoint(XOffset, YOffset);
			const FIntPoint CurrentChunkCoord = OriginChunkCoord + CurrentChunkOffset;
				
			const int DistanceInBlocksToOrigin = CurrentChunkOffset.Size();
			const EMapLod Lod = LodFromDistance(DistanceInBlocksToOrigin);

			if (TerrainMap.Contains(CurrentChunkCoord)) {
				FTerrainChunk& Chunk = TerrainMap[CurrentChunkCoord];
				// TODO: Redeuce duplication below
				if (Chunk.IsInVisibleDistance(Location2D, ViewDistance)) {
					Chunk.SetLod(this, Lod);
					if (!Mesh->IsVisible()) {
						Mesh->SetMeshSectionVisible(Chunk.GetSectionIndex(), true);
					}
					ChunksVisibleLastFrame.Add(CurrentChunkCoord);
				}
			}
			else {
				UE_LOG(LogTemp, Display, TEXT("Creating Chunk: (%d, %d)"), CurrentChunkCoord.X, CurrentChunkCoord.Y);
				FTerrainChunk Chunk(this, CurrentChunkCoord, ChunkSize());
				Mesh->SetMaterial(Chunk.GetSectionIndex(), Material);
				if (Chunk.IsInVisibleDistance(Location2D, ViewDistance)) {					
					if (!Mesh->IsVisible()) {
						Mesh->SetMeshSectionVisible(Chunk.GetSectionIndex(), true);
					}
					ChunksVisibleLastFrame.Add(CurrentChunkCoord);
				}
				TerrainMap.Add(CurrentChunkCoord, std::move(Chunk));
				NewChunksThisFrame.Add(CurrentChunkCoord);
			}
		}
	}

	for (int I = 0; I < NewChunksThisFrame.Num(); ++I) {
		const FIntPoint ChunkCoord = NewChunksThisFrame[I];
		AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [ChunkCoord, this]() {
			UE_LOG(LogTemp, Display, TEXT("Initting Chunk: (%d, %d)"), ChunkCoord.X, ChunkCoord.Y);
			FTerrainChunk& ChunkRef = TerrainMap[ChunkCoord];
			ChunkRef.Init(this);
			ChunkRef.SetLod(this, EMapLod::One);
		});
	}
}

void AEndlessTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void AEndlessTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVisibleChunks();
}