#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size);
	// NOTE: 2D Ditance ignoring Z coordinate. think about it later
	bool IsInVisibleDistance(FVector2D SourceLocation, float ViewDistance) const;

	int GetSectionIndex() const {
		return SectionIndex;
	}
private:
	FBox2D Rect;
	int SectionIndex;
};

UCLASS()
class PROCEDURALTERRAIN_API AEndlessTerrain : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ViewDistance;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
	UMaterial* Material;

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
