// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseMap.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProcuduralTerrain.generated.h"

UCLASS()
class PROCEDURALTERRAIN_API AProcuduralTerrain : public AActor
{
	GENERATED_BODY()
	
	void CreateTriangle();

	NoiseMap Noise;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	UMaterial* Material;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* MaterialInstance;

public:	
	// Sets default values for this actor's properties
	AProcuduralTerrain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
