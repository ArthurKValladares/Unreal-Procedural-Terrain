// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

AEndlessTerrain::AEndlessTerrain()
{
}

AEndlessTerrain::~AEndlessTerrain()
{
}

int AEndlessTerrain::NumChunksInViewDistance() const {
	return FGenericPlatformMath::RoundToInt(ViewDistance / AProcuduralTerrain::ChunkSize());
}

void AEndlessTerrain::UpdateVisibleChunks() {
	const auto* Player = GetWorld()->GetFirstPlayerController();
	if (Player) {
		const FVector Location = Player->GetPawnOrSpectator()->GetActorLocation();
		const int CurrentChunkCoordX =
			FGenericPlatformMath::RoundToInt(Location.X / AProcuduralTerrain::ChunkSize());
		const int CurrentChunkCoordY =
			FGenericPlatformMath::RoundToInt(Location.Y / AProcuduralTerrain::ChunkSize());

		const int ChunksInViewDistance = NumChunksInViewDistance();

		for (int YOffset = -ChunksInViewDistance; YOffset < ChunksInViewDistance; ++YOffset) {
			for (int XOffset = -ChunksInViewDistance; XOffset < ChunksInViewDistance; ++XOffset) {
				const FIntPoint CurrentChunkCoord = FIntPoint(CurrentChunkCoordX + XOffset, CurrentChunkCoordY + YOffset);
				
				if (TerrainMap.Contains(CurrentChunkCoord)) {

				}
				else {
					// TODO: I need to move all the modifiable params from AProcuduralTerrain to AEndlessTerrain,
					// And pass them in here in the constructor
					TerrainMap.Add(CurrentChunkCoord, AProcuduralTerrain());
				}
			}
		}
	}
}