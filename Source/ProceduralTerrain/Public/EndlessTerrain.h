#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, float Size);
	// NOTE: 2D Ditance ignoring Z coordinate. think about it later
	bool IsInVisibleDistance(FVector2D SourceLocation, float ViewDistance) const;

	int GetSectionIndex() const {
		return SectionIndex;
	}
private:
	NoiseMap Noise;
	FBox2D Rect;
	int SectionIndex;
	// TODO: Will need an `update` function later
	EMapLod MapLod;
};

UCLASS()
class PROCEDURALTERRAIN_API AEndlessTerrain : public AActor
{
	GENERATED_BODY()

	friend FTerrainChunk;

	// TODO: For now, duplicating a lot of stuff from `ProceduranTerrain`. Will delete that class at some point
	static constexpr int VerticesInChunk = 241;
	static constexpr float TileSize = 5.;
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
	TArray<FTerrainChunk> ChunksVisibleLastFrame;

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
