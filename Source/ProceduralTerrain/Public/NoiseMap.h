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
	NoiseMap(int Width, int Height, float Scale, int Octaves, float Persistance, float Lacunarity);
	~NoiseMap();

	UPROPERTY()
	TArray<float> NoiseValues;
	UTexture2D* NoiseTexture;
};
