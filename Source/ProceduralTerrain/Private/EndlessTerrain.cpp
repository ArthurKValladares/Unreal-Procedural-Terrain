// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

FTerrainChunk::FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size) {
	FVector2D Center = Point * Size;
	
	const float HalfSize = (float)Size / 2.;
	Rect = FBox2D(Center - HalfSize, Center + HalfSize);

	// TODO: Just Creating a plane here, will make better later
	FVector Position3D = FVector(Center.X, Center.Y, 0.0);
	TArray<FVector> Vertices;
	Vertices.Add(Position3D + FVector(-HalfSize, -HalfSize, 0.0));
	Vertices.Add(Position3D + FVector(HalfSize, -HalfSize, 0.0));
	Vertices.Add(Position3D + FVector(-HalfSize, HalfSize, 0.0));
	Vertices.Add(Position3D + FVector(HalfSize, HalfSize, 0.0));

	TArray<int32> Triangles;
	Triangles.Add(1);
	Triangles.Add(0);
	Triangles.Add(2);
	Triangles.Add(2);
	Triangles.Add(3);
	Triangles.Add(1);

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