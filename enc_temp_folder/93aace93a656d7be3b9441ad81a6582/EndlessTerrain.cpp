// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

namespace {
	float InverseLerp(float X, float Y, float V)
	{
		return (V - X) / (Y - X);
	}
}

FTerrainChunk::FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size) {
	FVector2D Center = Point * Size;

	NoiseMap Noise;
	Noise.Init(ParentTerrain->RandomSeed, 241, 241, 60., 1, 0.5, 1.0, Center);

	//
	const int StepSize = 1;
	const int Width = 241;
	const int Height = 241;

	const int VerticesPerRow = (Width - 1) / StepSize + 1;
	const int NumIndices = (VerticesPerRow - 1) * (VerticesPerRow - 1) * 6;

	const float TileSize = 5.;
	const float TotalWidth = Width * TileSize;
	const float TotalHeight = Height * TileSize;

	const float XOffset = -TotalWidth / 2.;
	const float YOffset = -TotalHeight / 2.;
	const float ZOffset = 10.0;

	TArray<FVector> Vertices;
	TArray<FVector2D> Uv0;
	TArray<int32> Triangles;
	for (int Y = 0; Y < Height; Y += StepSize) {
		const int YSteps = Y / StepSize;
		const int YIndexOffset = YSteps * VerticesPerRow;
		for (int X = 0; X < Width; X += StepSize) {
			const int XSteps = X / StepSize;

			// TODO: Duplicated code here, think about it soon, maybe save normalized noise instead of absolute
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];
			const float NormalizedNoise = InverseLerp(Noise.MinNoise, Noise.MaxNoise, NoiseValue);

			const float XPos = X * TileSize;
			const float YPos = Y * TileSize;
			float MultiplierEffectiveness = 1.0;
			if (IsValid(ParentTerrain->ElevationCurve)) {
				MultiplierEffectiveness = ParentTerrain->ElevationCurve->GetFloatValue(NormalizedNoise);
			}
			Vertices.Add(FVector(XPos + XOffset, YPos + YOffset, ZOffset + MultiplierEffectiveness * ParentTerrain->ElevationMultiplier));

			const float U = (float)X / Width;
			const float V = (float)Y / Width;
			Uv0.Add(FVector2D(U, V));

			if (Y < (Height - 1) && X < (Width - 1)) {
				const int CurrentIndex = YIndexOffset + XSteps;
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
				Triangles.Add(CurrentIndex + VerticesPerRow);
				Triangles.Add(CurrentIndex + VerticesPerRow + 1);

				// X --- X+1    
				//   \ |
				//    \|
				//     X+W+1
				Triangles.Add(CurrentIndex);
				Triangles.Add(CurrentIndex + VerticesPerRow + 1);
				Triangles.Add(CurrentIndex + 1);
			}
		}
	}
	check(Triangles.Num() == NumIndices);

	// TODO: Cleanup
	SectionIndex = ParentTerrain->CurrSectionIndex();
	ParentTerrain->GetMesh()->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, {}, {}, {}, {}, false);
	ParentTerrain->GetMesh()->SetMaterial(SectionIndex, ParentTerrain->GetMaterial());
	ParentTerrain->GetMesh()->SetMeshSectionVisible(SectionIndex, false);
}

bool FTerrainChunk::IsInVisibleDistance(FVector2D SourceLocation, float ViewDistance) const {
	const float Distance = FGenericPlatformMath::Sqrt(Rect.ComputeSquaredDistanceToPoint(SourceLocation));
	return Distance <= ViewDistance;
}

AEndlessTerrain::AEndlessTerrain()
	: ViewDistance(6000.0)
	, Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("EndlessMesh"))
	, Material(CreateDefaultSubobject<UMaterial>("EndlessMaterial"))
	, ElevationMultiplier(AProcuduralTerrain::GetChunkSize() / 3.)
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
}

int AEndlessTerrain::NumChunksInViewDistance() const {
	return FGenericPlatformMath::RoundToInt(ViewDistance / AProcuduralTerrain::GetChunkSize());
}

int AEndlessTerrain::CurrSectionIndex() const {
	return TerrainMap.Num();
}

void AEndlessTerrain::UpdateVisibleChunks() {
	for (const FTerrainChunk& Chunk : ChunksVisibleLastFrame) {
		Mesh->SetMeshSectionVisible(Chunk.GetSectionIndex(), false);
	}
	ChunksVisibleLastFrame.Empty();

	const auto* Player = GetWorld()->GetFirstPlayerController();
	if (Player) {
		const FVector Location = Player->GetPawnOrSpectator()->GetActorLocation();

		const int CurrentChunkCoordX =
			FGenericPlatformMath::RoundToInt(Location.X / AProcuduralTerrain::GetChunkSize());
		const int CurrentChunkCoordY =
			FGenericPlatformMath::RoundToInt(Location.Y / AProcuduralTerrain::GetChunkSize());

		const int ChunksInViewDistance = NumChunksInViewDistance();

		for (int YOffset = -ChunksInViewDistance; YOffset < ChunksInViewDistance; ++YOffset) {
			for (int XOffset = -ChunksInViewDistance; XOffset < ChunksInViewDistance; ++XOffset) {
				const FIntPoint CurrentChunkCoord = FIntPoint(CurrentChunkCoordX + XOffset, CurrentChunkCoordY + YOffset);
				
				if (TerrainMap.Contains(CurrentChunkCoord)) {
					const FTerrainChunk& Chunk = TerrainMap[CurrentChunkCoord];
					// TODO: Redeuce duplication below
					if (Chunk.IsInVisibleDistance(FVector2D(Location.X, Location.Y), ViewDistance)) {
						Mesh->SetMeshSectionVisible(Chunk.GetSectionIndex(), true);
						ChunksVisibleLastFrame.Add(Chunk);
					}
				}
				else {
					FTerrainChunk Chunk(this, CurrentChunkCoord, AProcuduralTerrain::GetChunkSize());
					if (Chunk.IsInVisibleDistance(FVector2D(Location.X, Location.Y), ViewDistance)) {
						Mesh->SetMeshSectionVisible(Chunk.GetSectionIndex(), true);
						ChunksVisibleLastFrame.Add(Chunk);
					}
					TerrainMap.Add(CurrentChunkCoord, std::move(Chunk));
				}
			}
		}
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