#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint ChunkCoord, float Size);

	void CreateResources(AEndlessTerrain* ParentTerrain);
	void UploadResources(AEndlessTerrain* ParentTerrain);

	int GetSectionIndex() const {
		return SectionIndex;
	}

	bool IsReadyToUpload() const {
		return ReadyToUpload;
	}

private:
	void CreateMesh(AEndlessTerrain* ParentTerrain);
	void UpdateTexture(AEndlessTerrain* ParentTerrain);

	EMapLod MapLod;
	FIntPoint ChunkCoord;
	FBox2D Rect;
	int SectionIndex;

	NoiseMap Noise;

	UMaterialInstanceDynamic* MaterialInstance;

	TArray<uint8> TextureData;
	UTexture2D* Texture;

	// Mesh Data
	TArray<FVector> Vertices;
	TArray<FVector2D> Uv0;
	TArray<int32> Triangles;

	FThreadSafeBool ReadyToUpload = false;
};

struct FAsyncChunkGenerator : public FNonAbandonableTask {
public:
	FAsyncChunkGenerator(FTerrainChunk* Chunk, AEndlessTerrain* ParentTerrain) : 
		Chunk(Chunk), 
		ParentTerrain(ParentTerrain)
	{}

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncChunkGenerator, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork();
private:
	FTerrainChunk* Chunk;
	AEndlessTerrain* ParentTerrain;
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
	int ChunksInViewDistance;

	FCriticalSection MeshMutex;
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
	UMaterial* Material;

	UPROPERTY(EditAnywhere)
	TArray<FTerrainParams> TerrainParams;
	UPROPERTY(EditAnywhere)
	float ElevationMultiplier;
	UPROPERTY(EditAnywhere)
	UCurveFloat* ElevationCurve;

	UPROPERTY(EditAnywhere)
	int RandomSeed;
	FRandomStream RandomStream;

	TMap<FIntPoint, FTerrainChunk> TerrainMap;

	TArray<FIntPoint> ChunksVisibleLastFrame;

	void UpdateVisibleChunks();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

public:
	AEndlessTerrain();
	~AEndlessTerrain();

	int CurrSectionIndex() const;

	virtual void Tick(float DeltaTime) override;
};
