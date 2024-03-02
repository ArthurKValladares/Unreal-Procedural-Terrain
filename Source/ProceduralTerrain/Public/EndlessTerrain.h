#pragma once

#include "CoreMinimal.h"
#include <ProcuduralTerrain.h>
#include "EndlessTerrain.generated.h"

UCLASS()
class PROCEDURALTERRAIN_API AEndlessTerrain : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ViewDistance;

	TMap<FIntPoint, AProcuduralTerrain> TerrainMap;

	int NumChunksInViewDistance() const;
	void UpdateVisibleChunks();
public:
	AEndlessTerrain();
	~AEndlessTerrain();
};
