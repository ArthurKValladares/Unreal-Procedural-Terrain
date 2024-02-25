// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROCEDURALTERRAIN_API NoiseMap
{
public:
	NoiseMap();
	~NoiseMap();

	void AllocateAndUpdate(int Width, int Height, float Scale, int Octaves, float Persistance, float Lacunarity);
	void Update(float Scale, int Octaves, float Persistance, float Lacunarity);

	UPROPERTY()
	TArray<float> NoiseValues;
	UTexture2D* NoiseTexture;
	int Width;
	int Height;
};
