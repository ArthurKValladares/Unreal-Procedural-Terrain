// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcuduralTerrain.h"
#include "UObject/Object.h"

#include <cassert>
#include <HAL/FileManagerGeneric.h>

// Sets default values
AProcuduralTerrain::AProcuduralTerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Noise = NoiseMap(500, 500, 0.5);

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));

	static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/Script/Engine.Material'/Game/TestMaterial.TestMaterial'"));
	assert(FoundMaterial.Succeeded());
	Material = FoundMaterial.Object;
}

// Called when the game starts or when spawned
void AProcuduralTerrain::BeginPlay()
{
	Super::BeginPlay();
	CreateTriangle();

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, Mesh);
	assert(MaterialInstance);
	Mesh->SetMaterial(0, MaterialInstance);
	assert(Noise.NoiseTexture);
	MaterialInstance->SetTextureParameterValue(TEXT("NoiseTexture"), Noise.NoiseTexture);
}

// Called every frame
void AProcuduralTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProcuduralTerrain::CreateTriangle() {
	TArray<FVector> Vertices;
	Vertices.Add(FVector(0, 0, 0));
	Vertices.Add(FVector(0, 100, 0));
	Vertices.Add(FVector(0, 0, 100));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	TArray<FVector> Normals;
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));

	TArray<FVector2D> Uv0;
	Uv0.Add(FVector2D(0, 0));
	Uv0.Add(FVector2D(10, 0));
	Uv0.Add(FVector2D(0, 10));


	TArray<FProcMeshTangent> Tangents;
	Tangents.Add(FProcMeshTangent(0, 1, 0));
	Tangents.Add(FProcMeshTangent(0, 1, 0));
	Tangents.Add(FProcMeshTangent(0, 1, 0));

	TArray<FLinearColor> VertexColors;
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, Uv0, VertexColors, Tangents, true);

	// Enable collision data
	Mesh->ContainsPhysicsTriMeshData(true);
}