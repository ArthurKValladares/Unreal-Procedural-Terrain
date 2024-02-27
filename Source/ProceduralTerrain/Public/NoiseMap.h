// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

class PROCEDURALTERRAIN_API NoiseMap
{
public:
	NoiseMap();
	~NoiseMap();

	void Init(int Seed, int Width, int Height);
	void Update(float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset);

	FRandomStream RandomStream;

	TArray<float> NoiseValues;

	int Width;
	int Height;

	float MinNoise;
	float MaxNoise;
};
