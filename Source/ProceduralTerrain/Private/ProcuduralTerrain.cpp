// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcuduralTerrain.h"
#include "UObject/Object.h"

// Sets default values
AProcuduralTerrain::AProcuduralTerrain()
	: Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("GeneratedMesh"))
	, Material(CreateDefaultSubobject<UMaterial>("NoiseMaterial"))
	, RandomSeed(1)
	, Width(500)
	, Height(500)
	, Scale(60.)
	, Octaves(1)
	, Persistance(0.5)
	, Lacunarity(1.0)
	, NoiseOffset(FVector2D(0., 0.))
{
	check(Mesh);
	check(Material);

	CreateTriangle();

	RootComponent = Mesh;
}

void AProcuduralTerrain::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	Noise.AllocateAndUpdate(RandomSeed, Width, Height, Scale, Octaves, Persistance, Lacunarity, NoiseOffset);
	check(Noise.NoiseTexture);

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, Mesh);
	check(MaterialInstance);
	MaterialInstance->SetTextureParameterValue("NoiseTexture", Noise.NoiseTexture);
	Mesh->SetMaterial(0, MaterialInstance);
}

// Called when the game starts or when spawned
void AProcuduralTerrain::BeginPlay()
{
	Super::BeginPlay();
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

	TArray<FVector2D> Uv0;
	Uv0.Add(FVector2D(0., 0.));
	Uv0.Add(FVector2D(1., 0.));
	Uv0.Add(FVector2D(0., 1.));
	Uv0.Add(FVector2D(1., 1.));

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, {}, Uv0, {}, {}, false);
}


void AProcuduralTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Noise.Update(Scale, Octaves, Persistance, Lacunarity, NoiseOffset);
	
	MaterialInstance->SetTextureParameterValue("NoiseTexture", Noise.NoiseTexture);
}