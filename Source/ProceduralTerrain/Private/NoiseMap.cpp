// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseMap.h"
#include "Math/UnrealMathUtility.h"

namespace {
	float Perlin2D(float X, float Y) {
		return FMath::PerlinNoise2D(FVector2D(X, Y));
	}
}

NoiseMap::NoiseMap() {
}

NoiseMap::NoiseMap(int Width, int Height, float Scale)
{
	NoiseValues.SetNum(Width * Height);
	NoiseTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "NoiseTexture");

	FTexture2DMipMap* MipMap = &NoiseTexture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;

	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	const int PixelSize = 4;
	const int Stride = Width;
	for (int R = 0; R < Height; ++R) {
		for (int C = 0; C < Width; ++C) {
			const int NoiseIndex = R * Stride + C;
			const int TextureIndex = NoiseIndex * PixelSize;

			const float SampleX = C / Scale;
			const float SampleY = R / Scale;
			const float Noise = Perlin2D(SampleX, SampleY);
			NoiseValues[NoiseIndex] = Noise;

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
