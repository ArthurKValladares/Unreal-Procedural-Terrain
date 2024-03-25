// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseMap.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProcuduralTerrain.generated.h"

UENUM()
enum class ETerrainType : uint8 {
	Water,
	Sand,
	Grass,
	Mountain,
	Snow,
	Count
};

USTRUCT()
struct FTerrainParams {
	GENERATED_BODY()

	static TArray<FTerrainParams> GetParams() {
		TArray<FTerrainParams> Params;
		Params.SetNum(static_cast<int>(ETerrainType::Count));
		Params[0] = FTerrainParams{
			ETerrainType::Water, 
			0.1, 
			FColor(0, 0, 255)
		};
		Params[1] = FTerrainParams{
			ETerrainType::Sand,
			0.3,
			FColor(255, 255, 0)
		};
		Params[2] = FTerrainParams{ 
			ETerrainType::Grass, 
			0.6, 
			FColor(0, 255, 0)
		};
		Params[3] = FTerrainParams{
			ETerrainType::Mountain,
			0.9,
			FColor(69, 50, 0)
		};
		Params[4] = FTerrainParams{
			ETerrainType::Snow,
			999.0,
			FColor(255, 255, 255)
		};
		// TODO: Assert some params stuff later, like height is always increasing. On change too
		return Params;
	}

	UPROPERTY(VisibleAnywhere)
	ETerrainType Type;
	UPROPERTY(EditAnywhere)
	float MaxHeight;
	UPROPERTY(EditAnywhere)
	FColor Color;
};

UENUM()
enum class EDisplayTexture : uint8 {
	Noise,
	Color
};

// TODO: Better way to get like a static array of multiples you can choose from
UENUM()
enum class EMapLod : uint8 {
	One = 1,
	Two = 2,
	Four = 4,
	Six = 6,
	Eight = 8,
	Ten = 10,
	Twelve = 12
};

inline EMapLod LodFromDistance(int Distance) {
	if (Distance == 0) return EMapLod::One;
	const int ClampedDistance = std::clamp(Distance, 1,  6);
	return static_cast<EMapLod>(ClampedDistance * 2);
}

UCLASS()
class PROCEDURALTERRAIN_API AProcuduralTerrain : public AActor
{
	GENERATED_BODY()
	
	NoiseMap Noise;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
	UMaterial* Material;
	UMaterialInstanceDynamic* MaterialInstance;

	static constexpr int ChunkSize = 241;
	static constexpr float TileSize = 5.;

	UPROPERTY(EditAnywhere)
	EMapLod MapLod;
	UPROPERTY(EditAnywhere)
	float ElevationMultiplier;
	UPROPERTY(EditAnywhere)
	UCurveFloat* ElevationCurve;

	UPROPERTY(EditAnywhere)
	int RandomSeed;
	UPROPERTY(EditAnywhere)
	float Scale;
	UPROPERTY(EditAnywhere)
	int Octaves;
	UPROPERTY(EditAnywhere)
	float Persistance;
	UPROPERTY(EditAnywhere)
	float Lacunarity;
	UPROPERTY(EditAnywhere)
	FVector2D NoiseOffset;

	UPROPERTY(EditAnywhere)
	EDisplayTexture DisplayTexture;

	UPROPERTY(EditAnywhere)
	TArray<FTerrainParams> TerrainParams;

	// TODO: Will use multiple textures
	UTexture2D* Texture;

	void CreateMesh();
	void UpdateTexture();
public:	
	AProcuduralTerrain();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
