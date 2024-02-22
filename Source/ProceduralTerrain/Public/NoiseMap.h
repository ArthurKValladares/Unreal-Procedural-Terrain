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
	NoiseMap(int Width, int Height, float Scale);
	~NoiseMap();

	UTexture2D* NoiseTexture;
};
