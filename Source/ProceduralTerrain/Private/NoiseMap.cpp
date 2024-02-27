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

void NoiseMap::Init(int Seed, int W, int H)
{
	RandomStream = FRandomStream(Seed);

	Width = W;
	Height = H;
	NoiseValues.SetNum(Width * Height);
}

void NoiseMap::Update(float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset) {
	TArray<FVector2D> OctaveOffsets;
	OctaveOffsets.SetNum(Octaves);
	for (int I = 0; I < Octaves;  ++I) {
		const float X = RandomStream.FRandRange(-100000, 100000);
		const float Y = RandomStream.FRandRange(-100000, 100000);
		OctaveOffsets[I] = FVector2D(X, Y);
	}

	const float HalfWidth = Width / 2.;
	const float HalfHeight = Height / 2.;
	MaxNoise = std::numeric_limits<float>::min();
	MinNoise = std::numeric_limits<float>::max();
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			const int NoiseIndex = Y * Width + X;

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
}

NoiseMap::~NoiseMap()
{
}
