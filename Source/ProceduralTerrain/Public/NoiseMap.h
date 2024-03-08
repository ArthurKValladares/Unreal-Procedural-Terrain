// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

enum class ENormalizeMode {
	Local,
	Global
};

class PROCEDURALTERRAIN_API NoiseMap
{
public:
	NoiseMap();
	~NoiseMap();

	void Init(ENormalizeMode NormalizeMode, int Seed, int Width, int Height, float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset);

	FRandomStream RandomStream;

	TArray<float> NoiseValues;

	int Width;
	int Height;

	float MinNoise;
	float MaxNoise;
};
