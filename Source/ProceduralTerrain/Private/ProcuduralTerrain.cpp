// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcuduralTerrain.h"
#include "UObject/Object.h"

// Sets default values
AProcuduralTerrain::AProcuduralTerrain()
	: Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("GeneratedMesh"))
	, Material(CreateDefaultSubobject<UMaterial>("NoiseMaterial"))
{
	check(Mesh);
	check(Material);
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void AProcuduralTerrain::BeginPlay()
{
	Super::BeginPlay();

	Mesh->SetMaterial(0, Material);

	Noise = NoiseMap(500, 500, 0.5);
	check(Noise.NoiseTexture);

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, Mesh);
	check(MaterialInstance);
	//MaterialInstance->SetTextureParameterValue("NoiseTexture", Noise.NoiseTexture);

	CreateTriangle();
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
	Vertices.Add(FVector(0, 100, 100));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);

	TArray<FVector> Normals;
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));

	TArray<FVector2D> Uv0;
	Uv0.Add(FVector2D(0, 0));
	Uv0.Add(FVector2D(1, 0));
	Uv0.Add(FVector2D(0, 1));
	Uv0.Add(FVector2D(1, 1));

	TArray<FProcMeshTangent> Tangents;
	Tangents.Add(FProcMeshTangent(0, 1, 0));
	Tangents.Add(FProcMeshTangent(0, 1, 0));
	Tangents.Add(FProcMeshTangent(0, 1, 0));
	Tangents.Add(FProcMeshTangent(0, 1, 0));

	TArray<FLinearColor> VertexColors;
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	VertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, Uv0, VertexColors, Tangents, true);

	// Enable collision data
	Mesh->ContainsPhysicsTriMeshData(true);
}