// Copyright Epic Games, Inc. All Rights Reserved.

#include "ErosionSimulator.h"
#include "Math/UnrealMathUtility.h"

// ============================================================================
// HYDRAULIC EROSION
// ============================================================================

void UErosionSimulator::ApplyHydraulicErosion(
	TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	const FHydraulicErosionConfig& Config,
	int32 Seed)
{
	FRandomStream Random(Seed);

	// Simulate multiple water droplets
	for (int32 I = 0; I < Config.Iterations; ++I)
	{
		float StartX = Random.FRand();
		float StartY = Random.FRand();

		SimulateWaterDroplet(HeightData, Resolution, TileSize, StartX, StartY, Config);
	}
}

TArray<FVector2D> UErosionSimulator::SimulateWaterDroplet(
	TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	float StartX,
	float StartY,
	const FHydraulicErosionConfig& Config)
{
	TArray<FVector2D> Path;

	// Droplet properties
	float PosX = StartX * (Resolution - 1);
	float PosY = StartY * (Resolution - 1);
	FVector2D Direction(0.0f, 0.0f);
	float Velocity = 1.0f;
	float Water = 1.0f;
	float Sediment = 0.0f;

	float CellSize = TileSize / (Resolution - 1);

	for (int32 Lifetime = 0; Lifetime < Config.MaxDropletLifetime; ++Lifetime)
	{
		// Check bounds
		if (!IsInBounds(PosX, PosY, Resolution))
			break;

		Path.Add(FVector2D(PosX / (Resolution - 1), PosY / (Resolution - 1)));

		// Get current height
		float CurrentHeight = GetHeightInterpolated(HeightData, Resolution, PosX, PosY);

		// Calculate gradient
		FVector2D Gradient = CalculateGradient(HeightData, Resolution, TileSize, PosX, PosY);

		// Update direction with inertia
		Direction = Direction * Config.Inertia + Gradient * (1.0f - Config.Inertia);
		Direction.Normalize();

		// Move droplet
		float NewPosX = PosX + Direction.X;
		float NewPosY = PosY + Direction.Y;

		// Check if moved out of bounds
		if (!IsInBounds(NewPosX, NewPosY, Resolution))
			break;

		// Get new height
		float NewHeight = GetHeightInterpolated(HeightData, Resolution, NewPosX, NewPosY);

		// Calculate height difference
		float HeightDiff = NewHeight - CurrentHeight;

		// Calculate sediment capacity
		float Capacity = FMath::Max(-HeightDiff, Config.MinSlope) * Velocity * Water * Config.SedimentCapacity;

		// Erosion or deposition
		if (Sediment > Capacity || HeightDiff > 0.0f)
		{
			// Deposition
			float AmountToDeposit = (HeightDiff > 0.0f) ?
				FMath::Min(HeightDiff, Sediment) :
				(Sediment - Capacity) * Config.DepositionStrength;

			Sediment -= AmountToDeposit;
			SetHeightInterpolated(HeightData, Resolution, PosX, PosY, AmountToDeposit);
		}
		else
		{
			// Erosion
			float AmountToErode = FMath::Min((Capacity - Sediment) * Config.ErosionStrength, -HeightDiff);
			SetHeightInterpolated(HeightData, Resolution, PosX, PosY, -AmountToErode);
			Sediment += AmountToErode;
		}

		// Update velocity and water
		Velocity = FMath::Sqrt(Velocity * Velocity + HeightDiff * Config.Gravity);
		Water *= (1.0f - Config.EvaporationRate);

		// Move to new position
		PosX = NewPosX;
		PosY = NewPosY;
	}

	return Path;
}

// ============================================================================
// THERMAL EROSION
// ============================================================================

void UErosionSimulator::ApplyThermalErosion(
	TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	const FThermalErosionConfig& Config)
{
	float CellSize = TileSize / (Resolution - 1);
	float MaxHeightDiff = FMath::Tan(FMath::DegreesToRadians(Config.TalusAngle)) * CellSize;

	TArray<float> TempHeightData = HeightData;

	for (int32 Iteration = 0; Iteration < Config.Iterations; ++Iteration)
	{
		for (int32 Y = 0; Y < Resolution; ++Y)
		{
			for (int32 X = 0; X < Resolution; ++X)
			{
				int32 Index = Y * Resolution + X;
				float CurrentHeight = HeightData[Index];

				// Get neighbors
				TArray<int32> Neighbors;
				GetNeighbors(Index, Resolution, Neighbors);

				float TotalHeightDiff = 0.0f;
				int32 ValidNeighbors = 0;

				// Calculate total height difference exceeding talus angle
				for (int32 NeighborIndex : Neighbors)
				{
					float NeighborHeight = HeightData[NeighborIndex];
					float HeightDiff = CurrentHeight - NeighborHeight;

					if (HeightDiff > MaxHeightDiff)
					{
						TotalHeightDiff += HeightDiff - MaxHeightDiff;
						ValidNeighbors++;
					}
				}

				// Distribute material to lower neighbors
				if (ValidNeighbors > 0 && TotalHeightDiff > Config.MinHeightDifference)
				{
					float MaterialToMove = TotalHeightDiff * Config.ErosionRate;
					TempHeightData[Index] -= MaterialToMove;

					float MaterialPerNeighbor = MaterialToMove / ValidNeighbors;

					for (int32 NeighborIndex : Neighbors)
					{
						float NeighborHeight = HeightData[NeighborIndex];
						float HeightDiff = CurrentHeight - NeighborHeight;

						if (HeightDiff > MaxHeightDiff)
						{
							TempHeightData[NeighborIndex] += MaterialPerNeighbor;
						}
					}
				}
			}
		}

		// Update height data
		HeightData = TempHeightData;
	}
}

// ============================================================================
// WIND EROSION
// ============================================================================

void UErosionSimulator::ApplyWindErosion(
	TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	const FWindErosionConfig& Config)
{
	TArray<float> TempHeightData = HeightData;
	FVector2D WindDir = Config.WindDirection;
	WindDir.Normalize();

	for (int32 Iteration = 0; Iteration < Config.Iterations; ++Iteration)
	{
		for (int32 Y = 0; Y < Resolution; ++Y)
		{
			for (int32 X = 0; X < Resolution; ++X)
			{
				int32 Index = Y * Resolution + X;

				// Calculate exposure to wind
				FVector2D Gradient = CalculateGradient(HeightData, Resolution, TileSize, X, Y);
				float Exposure = FMath::Max(0.0f, FVector2D::DotProduct(Gradient, WindDir));

				// Abrasion on windward slopes
				float Abrasion = Exposure * Config.WindStrength * Config.AbrasionRate;
				TempHeightData[Index] -= Abrasion;

				// Deposition on leeward slopes
				int32 LeewardX = X + FMath::RoundToInt(WindDir.X);
				int32 LeewardY = Y + FMath::RoundToInt(WindDir.Y);

				if (LeewardX >= 0 && LeewardX < Resolution && LeewardY >= 0 && LeewardY < Resolution)
				{
					int32 LeewardIndex = LeewardY * Resolution + LeewardX;
					float Deposition = Abrasion * Config.DepositionRate;
					TempHeightData[LeewardIndex] += Deposition;
				}
			}
		}

		HeightData = TempHeightData;
	}
}

// ============================================================================
// DRAINAGE PATTERNS
// ============================================================================

TArray<FDrainagePattern> UErosionSimulator::GenerateDrainagePatterns(
	const TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	float MinStreamLength)
{
	TArray<FDrainagePattern> Patterns;

	// Calculate flow accumulation
	TArray<float> FlowAccumulation = CalculateFlowAccumulation(HeightData, Resolution);

	// Find stream starting points (high accumulation areas)
	float Threshold = Resolution * Resolution * 0.01f; // 1% of total cells

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;

			if (FlowAccumulation[Index] > Threshold)
			{
				// Trace stream path
				FDrainagePattern Pattern;
				Pattern.FlowStrength = FlowAccumulation[Index];
				Pattern.StreamOrder = 1;

				int32 CurrentX = X;
				int32 CurrentY = Y;

				// Follow flow downhill
				for (int32 Step = 0; Step < Resolution; ++Step)
				{
					Pattern.StreamPath.Add(FVector2D(CurrentX, CurrentY));

					// Find lowest neighbor
					TArray<int32> Neighbors;
					GetNeighbors(CurrentY * Resolution + CurrentX, Resolution, Neighbors);

					int32 LowestNeighbor = -1;
					float LowestHeight = HeightData[CurrentY * Resolution + CurrentX];

					for (int32 NeighborIndex : Neighbors)
					{
						if (HeightData[NeighborIndex] < LowestHeight)
						{
							LowestHeight = HeightData[NeighborIndex];
							LowestNeighbor = NeighborIndex;
						}
					}

					if (LowestNeighbor == -1)
						break; // Reached local minimum

					CurrentX = LowestNeighbor % Resolution;
					CurrentY = LowestNeighbor / Resolution;
				}

				// Check if stream is long enough
				float StreamLength = Pattern.StreamPath.Num() * (TileSize / Resolution);
				if (StreamLength >= MinStreamLength)
				{
					Patterns.Add(Pattern);
				}
			}
		}
	}

	return Patterns;
}

TArray<float> UErosionSimulator::CalculateFlowAccumulation(
	const TArray<float>& HeightData,
	int32 Resolution)
{
	TArray<float> FlowAccumulation;
	FlowAccumulation.SetNumZeroed(Resolution * Resolution);

	// Initialize all cells with 1 (self-contribution)
	for (int32 I = 0; I < FlowAccumulation.Num(); ++I)
	{
		FlowAccumulation[I] = 1.0f;
	}

	// Process cells from highest to lowest
	TArray<int32> SortedIndices;
	SortedIndices.SetNum(Resolution * Resolution);
	for (int32 I = 0; I < SortedIndices.Num(); ++I)
	{
		SortedIndices[I] = I;
	}

	SortedIndices.Sort([&HeightData](int32 A, int32 B) {
		return HeightData[A] > HeightData[B];
	});

	// Flow accumulation calculation
	for (int32 Index : SortedIndices)
	{
		TArray<int32> Neighbors;
		GetNeighbors(Index, Resolution, Neighbors);

		// Find lowest neighbor
		int32 LowestNeighbor = -1;
		float LowestHeight = HeightData[Index];

		for (int32 NeighborIndex : Neighbors)
		{
			if (HeightData[NeighborIndex] < LowestHeight)
			{
				LowestHeight = HeightData[NeighborIndex];
				LowestNeighbor = NeighborIndex;
			}
		}

		// Flow to lowest neighbor
		if (LowestNeighbor != -1)
		{
			FlowAccumulation[LowestNeighbor] += FlowAccumulation[Index];
		}
	}

	return FlowAccumulation;
}

// ============================================================================
// SEDIMENT TRANSPORT
// ============================================================================

void UErosionSimulator::SimulateSedimentTransport(
	TArray<float>& HeightData,
	int32 Resolution,
	const TArray<float>& FlowAccumulation,
	float TransportCapacity,
	float DepositionRate)
{
	TArray<float> Sediment;
	Sediment.SetNumZeroed(Resolution * Resolution);

	// Calculate sediment capacity based on flow
	for (int32 I = 0; I < Resolution * Resolution; ++I)
	{
		float Capacity = FlowAccumulation[I] * TransportCapacity;
		Sediment[I] = FMath::Min(Capacity, HeightData[I] * 0.1f); // Erode up to 10% of height
		HeightData[I] -= Sediment[I];
	}

	// Deposit sediment in low-flow areas
	for (int32 I = 0; I < Resolution * Resolution; ++I)
	{
		if (FlowAccumulation[I] < 2.0f) // Low flow areas
		{
			float Deposition = Sediment[I] * DepositionRate;
			HeightData[I] += Deposition;
		}
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float UErosionSimulator::GetHeightInterpolated(
	const TArray<float>& HeightData,
	int32 Resolution,
	float X,
	float Y)
{
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 X1 = FMath::Min(X0 + 1, Resolution - 1);
	int32 Y1 = FMath::Min(Y0 + 1, Resolution - 1);

	float Fx = X - X0;
	float Fy = Y - Y0;

	float H00 = HeightData[Y0 * Resolution + X0];
	float H10 = HeightData[Y0 * Resolution + X1];
	float H01 = HeightData[Y1 * Resolution + X0];
	float H11 = HeightData[Y1 * Resolution + X1];

	float H0 = FMath::Lerp(H00, H10, Fx);
	float H1 = FMath::Lerp(H01, H11, Fx);

	return FMath::Lerp(H0, H1, Fy);
}

void UErosionSimulator::SetHeightInterpolated(
	TArray<float>& HeightData,
	int32 Resolution,
	float X,
	float Y,
	float DeltaHeight)
{
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 X1 = FMath::Min(X0 + 1, Resolution - 1);
	int32 Y1 = FMath::Min(Y0 + 1, Resolution - 1);

	float Fx = X - X0;
	float Fy = Y - Y0;

	// Distribute height change to neighboring cells
	HeightData[Y0 * Resolution + X0] += DeltaHeight * (1.0f - Fx) * (1.0f - Fy);
	HeightData[Y0 * Resolution + X1] += DeltaHeight * Fx * (1.0f - Fy);
	HeightData[Y1 * Resolution + X0] += DeltaHeight * (1.0f - Fx) * Fy;
	HeightData[Y1 * Resolution + X1] += DeltaHeight * Fx * Fy;
}

FVector2D UErosionSimulator::CalculateGradient(
	const TArray<float>& HeightData,
	int32 Resolution,
	float TileSize,
	float X,
	float Y)
{
	float CellSize = TileSize / (Resolution - 1);

	// Sample heights around position
	float HeightL = GetHeightInterpolated(HeightData, Resolution, X - 1.0f, Y);
	float HeightR = GetHeightInterpolated(HeightData, Resolution, X + 1.0f, Y);
	float HeightD = GetHeightInterpolated(HeightData, Resolution, X, Y - 1.0f);
	float HeightU = GetHeightInterpolated(HeightData, Resolution, X, Y + 1.0f);

	// Calculate gradient
	FVector2D Gradient;
	Gradient.X = (HeightR - HeightL) / (2.0f * CellSize);
	Gradient.Y = (HeightU - HeightD) / (2.0f * CellSize);

	return Gradient;
}

void UErosionSimulator::GetNeighbors(int32 Index, int32 Resolution, TArray<int32>& OutNeighbors)
{
	OutNeighbors.Empty();

	int32 X = Index % Resolution;
	int32 Y = Index / Resolution;

	// 8-connected neighbors
	for (int32 DY = -1; DY <= 1; ++DY)
	{
		for (int32 DX = -1; DX <= 1; ++DX)
		{
			if (DX == 0 && DY == 0)
				continue;

			int32 NX = X + DX;
			int32 NY = Y + DY;

			if (NX >= 0 && NX < Resolution && NY >= 0 && NY < Resolution)
			{
				OutNeighbors.Add(NY * Resolution + NX);
			}
		}
	}
}

float UErosionSimulator::CalculateSlopeAngle(float Height1, float Height2, float Distance)
{
	float HeightDiff = FMath::Abs(Height1 - Height2);
	return FMath::RadiansToDegrees(FMath::Atan2(HeightDiff, Distance));
}

bool UErosionSimulator::IsInBounds(float X, float Y, int32 Resolution)
{
	return X >= 0.0f && X < Resolution - 1 && Y >= 0.0f && Y < Resolution - 1;
}
