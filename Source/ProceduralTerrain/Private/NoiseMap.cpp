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
	NoiseTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, TEXT("NoiseTexture"));

	FTexture2DMipMap* MipMap = &NoiseTexture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;

	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	const int PixelSize = 4;
	const int Stride = Width * PixelSize;
	for (int R = 0; R < Height; ++R) {
		for (int C = 0; C < Width; ++C) {
			const int PixelIndex = R * Stride + C * PixelSize;

			const float SampleX = R / Scale;
			const float SampleY = C / Scale;
			const float Noise = Perlin2D(SampleX, SampleY);
			const uint8 Color = static_cast<uint8>(Noise * 255.);

			RawImageData[PixelIndex] = Color;
			RawImageData[PixelIndex + 1] = Color;
			RawImageData[PixelIndex + 2] = Color;
			RawImageData[PixelIndex + 3] = 255;
		}
	}
	ImageData->Unlock();
	
	NoiseTexture->UpdateResource();
}

NoiseMap::~NoiseMap()
{
}
