#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint ChunkCoord, float Size);
	void Init(AEndlessTerrain* ParentTerrain);
	void SetLod(AEndlessTerrain* ParentTerrain, EMapLod MapLod);

	// NOTE: 2D Ditance ignoring Z coordinate. think about it later
	bool IsInVisibleDistance(FVector2D SourceLocation, float ViewDistance) const;

	int GetSectionIndex() const {
		return SectionIndex;
	}
private:
	EMapLod MapLod;
	FIntPoint ChunkCoord;
	FBox2D Rect;
	int SectionIndex;
	NoiseMap Noise;
	// TODO: Will need an `update` function later

	// Mesh Data
	TArray<FVector> Vertices;
	TArray<FVector2D> Uv0;
	TMap<EMapLod, TArray<int32>> TriangleMap;
};

UCLASS()
class PROCEDURALTERRAIN_API AEndlessTerrain : public AActor
{
	GENERATED_BODY()

	friend FTerrainChunk;

	// TODO: For now, duplicating a lot of stuff from `ProceduranTerrain`. Will delete that class at some point
	static constexpr int VerticesInChunk = 241;
	static constexpr float TileSize = 3.0;
	static constexpr float ChunkSize() {
		return (VerticesInChunk - 1)* TileSize;
	}

	UPROPERTY(EditAnywhere)
	float Scale;
	UPROPERTY(EditAnywhere)
	int Octaves;
	UPROPERTY(EditAnywhere)
	float Persistance;
	UPROPERTY(EditAnywhere)
	float Lacunarity;

	UPROPERTY(EditAnywhere)
	float ViewDistance;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
	UMaterial* Material;

	UPROPERTY(EditAnywhere)
	float ElevationMultiplier;
	UPROPERTY(EditAnywhere)
	UCurveFloat* ElevationCurve;

	UPROPERTY(EditAnywhere)
	int RandomSeed;
	FRandomStream RandomStream;

	TMap<FIntPoint, FTerrainChunk> TerrainMap;
	TArray<FIntPoint> ChunksVisibleLastFrame;

	int NumChunksInViewDistance() const;

	void UpdateVisibleChunks();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

public:
	AEndlessTerrain();
	~AEndlessTerrain();

	int CurrSectionIndex() const;
	UProceduralMeshComponent* GetMesh() {
		return Mesh;
	}
	UMaterial* GetMaterial() {
		return Material;
	}

	virtual void Tick(float DeltaTime) override;
};
