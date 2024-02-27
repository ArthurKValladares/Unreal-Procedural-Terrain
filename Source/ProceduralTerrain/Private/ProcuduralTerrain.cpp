// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcuduralTerrain.h"
#include "UObject/Object.h"

namespace {
	float InverseLerp(float X, float Y, float V)
	{
		return (V - X) / (Y - X);
	}
}

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
	, DisplayTexture(EDisplayTexture::Color)
	, TerrainParams(FTerrainParams::GetParams())
{
	check(Mesh);
	check(Material);

	CreateTriangle();

	RootComponent = Mesh;
}

void AProcuduralTerrain::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	NoiseTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "NoiseTexture");
	check(NoiseTexture);
	ColorTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "ColorTexture");
	check(ColorTexture);

	Noise.Init(RandomSeed, Width, Height);
	UpdateNoise();

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, Mesh);
	check(MaterialInstance);
	SetDisplayTexture();
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

	UpdateNoise();
	SetDisplayTexture();
}

void AProcuduralTerrain::SetDisplayTexture() {
	switch (DisplayTexture)
	{
		case EDisplayTexture::Noise: {
			MaterialInstance->SetTextureParameterValue("NoiseTexture", NoiseTexture);
			break;
		}
		case EDisplayTexture::Color: {
			MaterialInstance->SetTextureParameterValue("NoiseTexture", ColorTexture);
			break;
		}
	}
}

void AProcuduralTerrain::UpdateNoise() {
	Noise.Update(Scale, Octaves, Persistance, Lacunarity, NoiseOffset);

	FTexture2DMipMap* NoiseMipMap = &NoiseTexture->GetPlatformData()->Mips[0];
	FByteBulkData* NoiseImageData = &NoiseMipMap->BulkData;
	uint8* RawNoiseImageData = (uint8*)NoiseImageData->Lock(LOCK_READ_WRITE);

	FTexture2DMipMap* ColorMipMap = &ColorTexture->GetPlatformData()->Mips[0];
	FByteBulkData* ColorImageData = &ColorMipMap->BulkData;
	uint8* RawColorImageData = (uint8*)ColorImageData->Lock(LOCK_READ_WRITE);

	const int PixelSize = 4;
	for (int R = 0; R < Height; ++R) {
		for (int C = 0; C < Width; ++C) {
			const int NoiseIndex = R * Width + C;
			const int TextureIndex = NoiseIndex * PixelSize;

			const float NoiseValue = Noise.NoiseValues[NoiseIndex];
			const float NormalizedNoise = InverseLerp(Noise.MinNoise, Noise.MaxNoise, NoiseValue);

			const uint8 NoiseTexColor = NormalizedNoise * 255.;
			RawNoiseImageData[TextureIndex] = NoiseTexColor;
			RawNoiseImageData[TextureIndex + 1] = NoiseTexColor;
			RawNoiseImageData[TextureIndex + 2] = NoiseTexColor;
			RawNoiseImageData[TextureIndex + 3] = 255;

			for (const FTerrainParams& Param : TerrainParams) {
				if (NormalizedNoise <= Param.MaxHeight) {
					const FColor Color = Param.Color;

					RawColorImageData[TextureIndex]     = Color.B;
					RawColorImageData[TextureIndex + 1] = Color.G;
					RawColorImageData[TextureIndex + 2] = Color.R;
					RawColorImageData[TextureIndex + 3] = 255;
					break;
				}
			}
		}
	}

	NoiseImageData->Unlock();
	ColorImageData->Unlock();

	NoiseTexture->UpdateResource();
	ColorTexture->UpdateResource();
}