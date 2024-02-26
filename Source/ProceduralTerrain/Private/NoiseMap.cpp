// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseMap.h"
#include "Math/UnrealMathUtility.h"

namespace {
	float Perlin2D(float X, float Y) {
		return FMath::PerlinNoise2D(FVector2D(X, Y));
	}

	float InverseLerp(float X, float Y, float V)
	{
		return (V - X) / (Y - X);
	}
}

NoiseMap::NoiseMap() {
}

void NoiseMap::AllocateAndUpdate(int Seed, int W, int H, float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset)
{
	RandomStream = FRandomStream(Seed);

	Width = W;
	Height = H;
	NoiseValues.SetNum(Width * Height);
	NoiseTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "NoiseTexture");

	Update(Scale, Octaves, Persistance, Lacunarity, NoiseOffset);
}

void NoiseMap::Update(float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset) {
	FTexture2DMipMap* MipMap = &NoiseTexture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;

	TArray<FVector2D> OctaveOffsets;
	OctaveOffsets.SetNum(Octaves);
	for (int I = 0; I < Octaves;  ++I) {
		const float X = RandomStream.FRandRange(-100000, 100000);
		const float Y = RandomStream.FRandRange(-100000, 100000);
		OctaveOffsets[I] = FVector2D(X, Y);
	}

	const float HalfWidth = Width / 2.;
	const float HalfHeight = Height / 2.;
	float MaxNoise = std::numeric_limits<float>::min();
	float MinNoise = std::numeric_limits<float>::max();
	const int Stride = Width;
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			const int NoiseIndex = Y * Stride + X;
			//const int TextureIndex = NoiseIndex * PixelSize;
			float Amplitude = 1.;
			float Frequency = 1.;
			float NoiseHeight = 0.;

			for (int I = 0; I < Octaves; ++I) {
				const float SampleX = (X - HalfWidth) / Scale * Frequency + OctaveOffsets[I].X + NoiseOffset.X;
				const float SampleY = (Y - HalfHeight) / Scale * Frequency + OctaveOffsets[I].Y + NoiseOffset.Y;

				const float Noise = Perlin2D(SampleX, SampleY) * 2. - 1.;
				NoiseHeight += Noise * Amplitude;

				Amplitude *= Persistance;
				Frequency *= Lacunarity;
			}

			MaxNoise = std::max(MaxNoise, NoiseHeight);
			MinNoise = std::min(MinNoise, NoiseHeight);

			NoiseValues[NoiseIndex] = NoiseHeight;
		}
	}

	const int PixelSize = 4;
	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	for (int R = 0; R < Height; ++R) {
		for (int C = 0; C < Width; ++C) {
			const int NoiseIndex = R * Stride + C;
			const int TextureIndex = NoiseIndex * PixelSize;

			float Noise = InverseLerp(MinNoise, MaxNoise, NoiseValues[NoiseIndex]);

			const uint8 Color = Noise * 255.;
			RawImageData[TextureIndex] = Color;
			RawImageData[TextureIndex + 1] = Color;
			RawImageData[TextureIndex + 2] = Color;
			RawImageData[TextureIndex + 3] = 255;
		}
	}
	ImageData->Unlock();

	NoiseTexture->UpdateResource();
}

NoiseMap::~NoiseMap()
{
}
