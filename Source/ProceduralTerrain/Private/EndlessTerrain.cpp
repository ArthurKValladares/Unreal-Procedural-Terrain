// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

FTerrainChunk::FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size) {
	Position = Point * Size;

	// TODO: Just Creating a plane here, will make better later
	const float HalfSize = (float)Size / 2.;

	TArray<FVector> Vertices;
	Vertices.Add(FVector(-HalfSize, -HalfSize, 0.0));
	Vertices.Add(FVector(HalfSize, -HalfSize, 0.0));
	Vertices.Add(FVector(-HalfSize, HalfSize, 0.0));
	Vertices.Add(FVector(HalfSize, HalfSize, 0.0));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);

	SectionIndex = ParentTerrain->CurrSectionIndex();

	ParentTerrain->GetMesh()->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, {}, {}, {}, {}, false);
}

AEndlessTerrain::AEndlessTerrain()
	: ViewDistance(300.0)
	, Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("EndlessMesh"))
{
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

				}
				else {
					FTerrainChunk Chunk(this, CurrentChunkCoord, AProcuduralTerrain::GetChunkSize());
					TerrainMap.Add(CurrentChunkCoord, Chunk);
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