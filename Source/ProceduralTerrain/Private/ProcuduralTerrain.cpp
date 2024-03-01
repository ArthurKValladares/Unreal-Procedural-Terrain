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
	, MapLod(EMapLod::One)
	, TileSize(5.)
	, ElevationMultiplier( (AProcuduralTerrain::ChunkSize * TileSize) / 3.)
	, ElevationCurve(CreateDefaultSubobject<UCurveFloat>("ElevationCurve"))
	, RandomSeed(1)
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

	RootComponent = Mesh;
}

void AProcuduralTerrain::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	const int Width = AProcuduralTerrain::ChunkSize;
	const int Height = AProcuduralTerrain::ChunkSize;
	check((Width - 1) % static_cast<int>(MapLod) == 0);

	Noise.Init(RandomSeed, Width, Height, Scale, Octaves, Persistance, Lacunarity, NoiseOffset);

	Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "Texture");
	Texture->Filter = TextureFilter::TF_Nearest;
	Texture->AddressX = TextureAddress::TA_Clamp;
	Texture->AddressY = TextureAddress::TA_Clamp;
	check(Texture);
	UpdateTexture();

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, Mesh);
	check(MaterialInstance);
	MaterialInstance->SetTextureParameterValue("NoiseTexture", Texture);
	Mesh->SetMaterial(0, MaterialInstance);

	CreateMesh();
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

void AProcuduralTerrain::CreateMesh() {
	const int StepSize = static_cast<int>(MapLod);
	const int Width = AProcuduralTerrain::ChunkSize;
	const int Height = AProcuduralTerrain::ChunkSize;

	const int VerticesPerRow = (Width - 1) / StepSize + 1;
	const int NumIndices = (VerticesPerRow - 1) * (VerticesPerRow - 1) * 6;

	const float TotalWidth = Width * TileSize;
	const float TotalHeight = Height * TileSize;

	const float XOffset = -TotalWidth / 2.;
	const float YOffset = -TotalHeight / 2.;
	const float ZOffset = 10.0;

	TArray<FVector> Vertices;
	TArray<FVector2D> Uv0;
	TArray<int32> Triangles;
	for (int Y = 0; Y < Height; Y += StepSize) {
		const int YSteps = Y / StepSize;
		const int YIndexOffset = YSteps * VerticesPerRow;
		for (int X = 0; X < Width; X += StepSize) {
			const int XSteps = X / StepSize;

			// TODO: Duplicated code here, think about it soon, maybe save normalized noise instead of absolute
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];
			const float NormalizedNoise = InverseLerp(Noise.MinNoise, Noise.MaxNoise, NoiseValue);

			const float XPos = X * TileSize;
			const float YPos = Y * TileSize;
			float MultiplierEffectiveness = 1.0;
			if (IsValid(ElevationCurve)) {
				MultiplierEffectiveness = ElevationCurve->GetFloatValue(NormalizedNoise);
			}
			Vertices.Add(FVector(XPos + XOffset, YPos + YOffset, ZOffset + MultiplierEffectiveness * ElevationMultiplier));

			const float U = (float)X / Width;
			const float V = (float)Y / Width;
			Uv0.Add(FVector2D(U, V));

			if (Y < (Height - 1) && X < (Width - 1)) {				
				const int CurrentIndex = YIndexOffset + XSteps;
				// Vertex setup
				//   0      1      2      3    ..    W-1
				// (0+W)  (1+W)  (2+W)  (3+W)  .. (2W - 1)
				// ...
				
				// Both triangles need to have counter-clockwise winding-order
				//     X
				//     |\
				//     | \
				// X+W --- x+W+1
				Triangles.Add(CurrentIndex);
				Triangles.Add(CurrentIndex + VerticesPerRow);
				Triangles.Add(CurrentIndex + VerticesPerRow + 1);

				// X --- X+1    
				//   \ |
				//    \|
				//     X+W+1
				Triangles.Add(CurrentIndex);
				Triangles.Add(CurrentIndex + VerticesPerRow + 1);
				Triangles.Add(CurrentIndex + 1);
			}
		}
	}
	check(Triangles.Num() == NumIndices);

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, {}, Uv0, {}, {}, false);
}

void AProcuduralTerrain::UpdateTexture() {
	const int Width = AProcuduralTerrain::ChunkSize;
	const int Height = AProcuduralTerrain::ChunkSize;

	FTexture2DMipMap* MipMap = &Texture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;
	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	const int PixelSize = 4;
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];
			const float NormalizedNoise = InverseLerp(Noise.MinNoise, Noise.MaxNoise, NoiseValue);

			const int TextureIndex = NoiseIndex * PixelSize;
			switch (DisplayTexture)
			{
				case EDisplayTexture::Noise: {
					const uint8 NoiseTexColor = NormalizedNoise * 255.;
					RawImageData[TextureIndex] = NoiseTexColor;
					RawImageData[TextureIndex + 1] = NoiseTexColor;
					RawImageData[TextureIndex + 2] = NoiseTexColor;
					RawImageData[TextureIndex + 3] = 255;
					break;
				}
				case EDisplayTexture::Color: {
					for (const FTerrainParams& Param : TerrainParams) {
						if (NormalizedNoise <= Param.MaxHeight) {
							const FColor Color = Param.Color;

							RawImageData[TextureIndex] = Color.B;
							RawImageData[TextureIndex + 1] = Color.G;
							RawImageData[TextureIndex + 2] = Color.R;
							RawImageData[TextureIndex + 3] = 255;
							break;
						}
					}
					break;
				}
			}
		}
	}
	ImageData->Unlock();
	Texture->UpdateResource();
}

void AProcuduralTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
}