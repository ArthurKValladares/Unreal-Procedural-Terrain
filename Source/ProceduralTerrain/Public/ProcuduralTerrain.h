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
			0.4, 
			FColor(0, 0, 255)
		};
		Params[1] = FTerrainParams{
			ETerrainType::Sand,
			0.5,
			FColor(255, 255, 0)
		};
		Params[2] = FTerrainParams{ 
			ETerrainType::Grass, 
			0.6, 
			FColor(0, 255, 0)
		};
		Params[3] = FTerrainParams{
			ETerrainType::Mountain,
			0.95,
			FColor(69, 50, 0)
		};
		Params[4] = FTerrainParams{
			ETerrainType::Snow,
			1.0,
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

UCLASS()
class PROCEDURALTERRAIN_API AProcuduralTerrain : public AActor
{
	GENERATED_BODY()
	
	void CreateTriangle();

	NoiseMap Noise;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere)
	UMaterial* Material;
	UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(EditAnywhere)
	int Width;
	UPROPERTY(EditAnywhere)
	int Height;
	UPROPERTY(EditAnywhere)
	float TileSize;

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

	UTexture2D* Texture;

	void SetDisplayTexture();
	void UpdateNoise();
public:	
	AProcuduralTerrain();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
