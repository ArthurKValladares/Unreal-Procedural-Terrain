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

NoiseMap::NoiseMap(int Width, int Height, float Scale, int Octaves, float Persistance, float Lacunarity)
{
	NoiseValues.SetNum(Width * Height);
	NoiseTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "NoiseTexture");

	FTexture2DMipMap* MipMap = &NoiseTexture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;

	float MaxNoise = std::numeric_limits<float>::min();
	float MinNoise = std::numeric_limits<float>::max();
	const int Stride = Width;
	for (int R = 0; R < Height; ++R) {
		for (int C = 0; C < Width; ++C) {
			const int NoiseIndex = R * Stride + C;
			//const int TextureIndex = NoiseIndex * PixelSize;
			float Amplitude = 1.;
			float Frequency = 1.;
			float NoiseHeight = 0.;

			for (int I = 0; I < Octaves; ++I) {
				const float SampleX = C / Scale * Frequency;
				const float SampleY = R / Scale * Frequency;

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
