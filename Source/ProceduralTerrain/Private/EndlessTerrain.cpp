// Fill out your copyright notice in the Description page of Project Settings.

#include "EndlessTerrain.h"

#define DEBUG_DRAW false

FTerrainChunk::FTerrainChunk(AEndlessTerrain* ParentTerrain, FIntPoint ChunkCoord, float  Size)
	: MapLod(EMapLod::One)
	, ChunkCoord(ChunkCoord)
{
	FVector2D Center = ChunkCoord * Size;
	const float HalfSize = (float)Size / 2.;

	Rect = FBox2D(Center - HalfSize, Center + HalfSize);
	SectionIndex = ParentTerrain->CurrSectionIndex();

	const FName TextureName = FName(TEXT("NoiseTexture%d"), SectionIndex);
	const int TextureSize = AEndlessTerrain::VerticesInChunk;
	Texture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_B8G8R8A8, TextureName);
	Texture->Filter = TextureFilter::TF_Nearest;
	Texture->AddressX = TextureAddress::TA_Clamp;
	Texture->AddressY = TextureAddress::TA_Clamp;
	check(Texture);
	

	MaterialInstance = UMaterialInstanceDynamic::Create(ParentTerrain->Material, ParentTerrain->Mesh);
	check(MaterialInstance);
	MaterialInstance->SetTextureParameterValue("NoiseTexture", Texture);
	ParentTerrain->Mesh->SetMaterial(SectionIndex, MaterialInstance);
}

void FTerrainChunk::CreateResources(AEndlessTerrain* ParentTerrain) {
	UpdateTexture(ParentTerrain);
	CreateMesh(ParentTerrain);
}

void FTerrainChunk::CreateMesh(AEndlessTerrain* ParentTerrain) {
	const FVector2D Center = Rect.GetCenter();
	const float HalfSize = Rect.GetSize().X / 2.;
	check(Rect.GetSize().X == Rect.GetSize().Y);

	const float XOffset = -HalfSize;
	const float YOffset = -HalfSize;
	const float ZOffset = 0.0;

	const int Width = AEndlessTerrain::VerticesInChunk;
	const int Height = AEndlessTerrain::VerticesInChunk;

	// TODO: This is what `Init` used to be
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			// TODO: Duplicated code here, think about it soon, maybe save normalized noise instead of absolute
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];

			const float XPos = X * AEndlessTerrain::TileSize;
			const float YPos = Y * AEndlessTerrain::TileSize;
			float MultiplierEffectiveness = 1.0;
			if (IsValid(ParentTerrain->ElevationCurve)) {
				MultiplierEffectiveness = ParentTerrain->ElevationCurve->GetFloatValue(NoiseValue);
			}
			Vertices.Add(FVector(Center.X + XPos + XOffset, Center.Y + YPos + YOffset, ZOffset + MultiplierEffectiveness * ParentTerrain->ElevationMultiplier));

			const float U = (float)X / Width;
			const float V = (float)Y / Width;
			Uv0.Add(FVector2D(U, V));
		}
	}

	
	const int StepSize = static_cast<int>(MapLod);
	const int XStepOffset = StepSize;
	const int YStepOffset = Width * StepSize;

	for (int Y = 0; Y < Height; Y += StepSize) {
		for (int X = 0; X < Width; X += StepSize) {
			if (Y < (Height - StepSize) && X < (Width - StepSize)) {
				const int CurrentIndex = Y * Width + X;
				// Vertex setup
				//   0      1      2      3    ..    W-1
				// (0+W)  (1+W)  (2+W)  (3+W)  .. (2W - 1)
				// ...

				// Both triangles need to have counter-clockwise winding-order
				//     X
				//     |\
				//     | \
				// X+W --- x+W+1
				Triangles.Add(CurrentIndex);
				Triangles.Add(CurrentIndex + YStepOffset);
				Triangles.Add(CurrentIndex + YStepOffset + XStepOffset);

				// X --- X+1    
				//   \ |
				//    \|
				//     X+W+1
				Triangles.Add(CurrentIndex);
				Triangles.Add(CurrentIndex + YStepOffset + XStepOffset);
				Triangles.Add(CurrentIndex + XStepOffset);
			}
		}
	}
}

void FTerrainChunk::UpdateTexture(AEndlessTerrain* ParentTerrain) {
	Noise.Init(
		ENormalizeMode::Global,
		ParentTerrain->RandomSeed,
		AEndlessTerrain::VerticesInChunk,
		AEndlessTerrain::VerticesInChunk,
		ParentTerrain->Scale,
		ParentTerrain->Octaves,
		ParentTerrain->Persistance,
		ParentTerrain->Lacunarity,
		ChunkCoord * (AEndlessTerrain::VerticesInChunk)
	);

	const int Width = AEndlessTerrain::VerticesInChunk;
	const int Height = AEndlessTerrain::VerticesInChunk;

	const int PixelSize = 4;
	TextureData.SetNum(Width * Height * PixelSize);
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			const int NoiseIndex = Y * Width + X;
			const float NoiseValue = Noise.NoiseValues[NoiseIndex];

			const int TextureIndex = NoiseIndex * PixelSize;

			for (const FTerrainParams& Param : ParentTerrain->TerrainParams) {
				if (NoiseValue <= Param.MaxHeight) {
					const FColor Color = Param.Color;

					TextureData[TextureIndex] = Color.B;
					TextureData[TextureIndex + 1] = Color.G;
					TextureData[TextureIndex + 2] = Color.R;
					TextureData[TextureIndex + 3] = 255;
					break;
				}
			}
#if DEBUG_DRAW
			if (X == 0 || X == (Width - 1) || Y == 0 || Y == (Height - 1)) {
				TextureData[TextureIndex] = 0;
				TextureData[TextureIndex + 1] = 0;
				TextureData[TextureIndex + 2] = 255;
				TextureData[TextureIndex + 3] = 255;
			}
#endif
		}
	}	
}

void FTerrainChunk::UploadResources(AEndlessTerrain * ParentTerrain) {
	// TODO: Not using Normal values atm
	ParentTerrain->Mesh->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, {}, Uv0, {}, {}, false);

	const int Width = AEndlessTerrain::VerticesInChunk;
	const int Height = AEndlessTerrain::VerticesInChunk;

	FTexture2DMipMap* MipMap = &Texture->GetPlatformData()->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;
	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	for (int I = 0; I < TextureData.Num(); ++I) {
		RawImageData[I] = TextureData[I];
	}
	ImageData->Unlock();
	Texture->UpdateResource();
}

AEndlessTerrain::AEndlessTerrain()
	: Scale(60.)
	, Octaves(1)
	, Persistance(0.5)
	, Lacunarity(1.0)
	, ChunksInViewDistance(2)
	, Mesh(CreateDefaultSubobject<UProceduralMeshComponent>("EndlessMesh"))
	, Material(CreateDefaultSubobject<UMaterial>("EndlessMaterial"))
	, TerrainParams(FTerrainParams::GetParams())
	, ElevationMultiplier(AEndlessTerrain::VerticesInChunk)
	, ElevationCurve(CreateDefaultSubobject<UCurveFloat>("ElevationCurve"))
	, RandomSeed(0)
	, RandomStream(FRandomStream(RandomSeed))
{
	check(Mesh);
	check(Material);

	RootComponent = Mesh;

	PrimaryActorTick.bCanEverTick = true;
}

AEndlessTerrain::~AEndlessTerrain()
{
}

void AEndlessTerrain::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	UpdateVisibleChunks();
}

int AEndlessTerrain::CurrSectionIndex() const {
	return TerrainMap.Num();
}

void AEndlessTerrain::UpdateVisibleChunks() {
	const FVector2D Location2D = [&]() {
		const auto* Player = GetWorld()->GetFirstPlayerController();
		FVector Location;
		if (Player) {
			Location = Player->GetPawnOrSpectator()->GetActorLocation();
		}
		else {
			Location = FVector(0.);
		}
		return FVector2D(Location.X, Location.Y);
	}();
	const FIntPoint OriginChunkCoord = FIntPoint(FGenericPlatformMath::RoundToInt(Location2D.X / ChunkSize()), FGenericPlatformMath::RoundToInt(Location2D.Y / ChunkSize()));

	UE_LOG(LogTemp, Display, TEXT("------------"));
	for (const FIntPoint ChunkCoord : ChunksVisibleLastFrame) {
		if (abs(ChunkCoord.X - OriginChunkCoord.X) > ChunksInViewDistance || 
			abs(ChunkCoord.Y - OriginChunkCoord.Y) > ChunksInViewDistance) {
			const FTerrainChunk& ChunkRef = TerrainMap[ChunkCoord];
			Mesh->SetMeshSectionVisible(ChunkRef.GetSectionIndex(), false);
		}
	}
	ChunksVisibleLastFrame.Empty();

	// Test Chunks around Player Location
	for (int YOffset = -ChunksInViewDistance; YOffset <= ChunksInViewDistance; ++YOffset) {
		for (int XOffset = -ChunksInViewDistance; XOffset <= ChunksInViewDistance; ++XOffset) {
			const FIntPoint CurrentChunkOffset = FIntPoint(XOffset, YOffset);
			const FIntPoint CurrentChunkCoord = OriginChunkCoord + CurrentChunkOffset;
				
			const int DistanceInBlocksToOrigin = CurrentChunkOffset.Size();
			const EMapLod Lod = LodFromDistance(DistanceInBlocksToOrigin);

			if (TerrainMap.Contains(CurrentChunkCoord)) {
				UE_LOG(LogTemp, Display, TEXT("Updating Chunk: (%d, %d)"), CurrentChunkCoord.X, CurrentChunkCoord.Y);

				// TODO: Doing nothing atm
			}
			else {
				UE_LOG(LogTemp, Display, TEXT("Creating Chunk: (%d, %d)"), CurrentChunkCoord.X, CurrentChunkCoord.Y);

				// TODO: For now, all work in Chunk creation is done syncronously on main thread.
				FTerrainChunk Chunk(this, CurrentChunkCoord, ChunkSize());
				Chunk.CreateResources(this);
				Chunk.UploadResources(this);

				TerrainMap.Add(CurrentChunkCoord, std::move(Chunk));
			}

			ChunksVisibleLastFrame.Add(CurrentChunkCoord);
		}
	}
}

void AEndlessTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void AEndlessTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVisibleChunks();
}