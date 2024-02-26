// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

class PROCEDURALTERRAIN_API NoiseMap
{
public:
	NoiseMap();
	~NoiseMap();

	void AllocateAndUpdate(int Seed, int Width, int Height, float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset);
	void Update(float Scale, int Octaves, float Persistance, float Lacunarity, FVector2D NoiseOffset);

	FRandomStream RandomStream;

	UPROPERTY()
	TArray<float> NoiseValues;
	UTexture2D* NoiseTexture;

	int Width;
	int Height;
};
