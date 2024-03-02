#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size);

private:
	FIntPoint Position;
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

	TMap<FIntPoint, FTerrainChunk> TerrainMap;

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

	virtual void Tick(float DeltaTime) override;
};
