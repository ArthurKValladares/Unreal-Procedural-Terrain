#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"


struct FTerrainChunk {
	FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint Point, int Size);
	int GetSectionIndex() const {
		return SectionIndex;
	}
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
