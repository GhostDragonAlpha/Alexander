// Copyright Alexander Project. All Rights Reserved.

#include "PerformanceProfilerSubsystem.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

// FProfileScope implementation
FProfileScope::FProfileScope(UPerformanceProfilerSubsystem* InProfiler, FName InSystemName)
	: Profiler(InProfiler)
	, SystemName(InSystemName)
	, StartTime(FPlatformTime::Seconds() * 1000.0)
{
}

FProfileScope::~FProfileScope()
{
	if (Profiler && Profiler->IsProfilingEnabled())
	{
		double EndTime = FPlatformTime::Seconds() * 1000.0;
		double ElapsedMs = EndTime - StartTime;
		Profiler->RecordSample(SystemName, ElapsedMs);
	}
}

// UPerformanceProfilerSubsystem implementation
void UPerformanceProfilerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("PerformanceProfilerSubsystem initialized"));
	
	// Reset stats
	ResetAllProfiles();
	PeakMemoryBytes = 0;
	FrameCount = 0;
	AverageFrameTimeMs = 0.0;
}

void UPerformanceProfilerSubsystem::Deinitialize()
{
	// Export final report
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Profiling");
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString FilePath = SaveDir / FString::Printf(TEXT("ProfileReport_%s.json"), *Timestamp);
	
	ExportToJSON(FilePath);
	
	UE_LOG(LogTemp, Log, TEXT("PerformanceProfilerSubsystem deinitialized - Report saved to: %s"), *FilePath);
	
	Super::Deinitialize();
}

void UPerformanceProfilerSubsystem::StartProfiling(FName SystemName)
{
	if (!bProfilingEnabled) return;
	
	double CurrentTime = FPlatformTime::Seconds() * 1000.0;
	ActiveSessions.Add(SystemName, CurrentTime);
}

void UPerformanceProfilerSubsystem::StopProfiling(FName SystemName)
{
	if (!bProfilingEnabled) return;
	
	double* StartTime = ActiveSessions.Find(SystemName);
	if (StartTime)
	{
		double EndTime = FPlatformTime::Seconds() * 1000.0;
		double ElapsedMs = EndTime - *StartTime;
		
		UpdateProfile(SystemName, ElapsedMs);
		ActiveSessions.Remove(SystemName);
	}
}

void UPerformanceProfilerSubsystem::RecordSample(FName SystemName, double TimeMs)
{
	if (!bProfilingEnabled) return;
	
	UpdateProfile(SystemName, TimeMs);
}

void UPerformanceProfilerSubsystem::UpdateProfile(FName SystemName, double TimeMs)
{
	FProfileData* Profile = Profiles.Find(SystemName);
	
	if (!Profile)
	{
		// Create new profile
		FProfileData NewProfile;
		NewProfile.SystemName = SystemName;
		NewProfile.StartTime = FDateTime::Now();
		NewProfile.MinTimeMs = TimeMs;
		NewProfile.MaxTimeMs = TimeMs;
		NewProfile.TotalTimeMs = TimeMs;
		NewProfile.SampleCount = 1;
		NewProfile.AverageTimeMs = TimeMs;
		
		Profiles.Add(SystemName, NewProfile);
	}
	else
	{
		// Update existing profile
		Profile->TotalTimeMs += TimeMs;
		Profile->SampleCount++;
		Profile->AverageTimeMs = Profile->TotalTimeMs / Profile->SampleCount;
		Profile->MinTimeMs = FMath::Min(Profile->MinTimeMs, TimeMs);
		Profile->MaxTimeMs = FMath::Max(Profile->MaxTimeMs, TimeMs);
		Profile->EndTime = FDateTime::Now();
	}
	
	// Track frame time
	FrameCount++;
	AverageFrameTimeMs = (AverageFrameTimeMs * (FrameCount - 1) + TimeMs) / FrameCount;
	
	// Track peak memory
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	PeakMemoryBytes = FMath::Max(PeakMemoryBytes, (int64)MemStats.UsedPhysical);
}

FProfileData UPerformanceProfilerSubsystem::GetSystemProfile(FName SystemName) const
{
	const FProfileData* Profile = Profiles.Find(SystemName);
	return Profile ? *Profile : FProfileData();
}

TArray<FProfileData> UPerformanceProfilerSubsystem::GetAllProfiles() const
{
	TArray<FProfileData> Result;
	Profiles.GenerateValueArray(Result);
	return Result;
}

TArray<FProfileData> UPerformanceProfilerSubsystem::GetTopBottlenecks(int32 Count) const
{
	TArray<FProfileData> AllProfiles = GetAllProfiles();
	
	// Sort by average time (descending)
	AllProfiles.Sort([](const FProfileData& A, const FProfileData& B) {
		return A.AverageTimeMs > B.AverageTimeMs;
	});
	
	// Return top N
	if (AllProfiles.Num() > Count)
	{
		AllProfiles.SetNum(Count);
	}
	
	return AllProfiles;
}

void UPerformanceProfilerSubsystem::ResetProfile(FName SystemName)
{
	Profiles.Remove(SystemName);
	ActiveSessions.Remove(SystemName);
}

void UPerformanceProfilerSubsystem::ResetAllProfiles()
{
	Profiles.Empty();
	ActiveSessions.Empty();
	FrameCount = 0;
	AverageFrameTimeMs = 0.0;
}

void UPerformanceProfilerSubsystem::ExportToJSON(const FString& FilePath)
{
	// Create JSON structure
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	
	// Add summary
	RootObject->SetNumberField(TEXT("TotalFrames"), FrameCount);
	RootObject->SetNumberField(TEXT("AverageFrameTimeMs"), AverageFrameTimeMs);
	RootObject->SetNumberField(TEXT("AverageFPS"), GetCurrentFPS());
	RootObject->SetNumberField(TEXT("PeakMemoryMB"), GetPeakMemoryUsageMB());
	RootObject->SetNumberField(TEXT("CurrentMemoryMB"), GetCurrentMemoryUsageMB());
	RootObject->SetStringField(TEXT("ExportTime"), FDateTime::Now().ToString());
	
	// Add profiles
	TArray<TSharedPtr<FJsonValue>> ProfilesArray;
	for (const auto& Pair : Profiles)
	{
		const FProfileData& Profile = Pair.Value;
		
		TSharedPtr<FJsonObject> ProfileObj = MakeShareable(new FJsonObject);
		ProfileObj->SetStringField(TEXT("SystemName"), Profile.SystemName.ToString());
		ProfileObj->SetNumberField(TEXT("AverageTimeMs"), Profile.AverageTimeMs);
		ProfileObj->SetNumberField(TEXT("MinTimeMs"), Profile.MinTimeMs);
		ProfileObj->SetNumberField(TEXT("MaxTimeMs"), Profile.MaxTimeMs);
		ProfileObj->SetNumberField(TEXT("TotalTimeMs"), Profile.TotalTimeMs);
		ProfileObj->SetNumberField(TEXT("SampleCount"), Profile.SampleCount);
		ProfileObj->SetStringField(TEXT("StartTime"), Profile.StartTime.ToString());
		ProfileObj->SetStringField(TEXT("EndTime"), Profile.EndTime.ToString());
		
		ProfilesArray.Add(MakeShareable(new FJsonValueObject(ProfileObj)));
	}
	RootObject->SetArrayField(TEXT("Profiles"), ProfilesArray);
	
	// Write to file
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
	
	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	IFileManager::Get().MakeDirectory(*Directory, true);
	
	// Save file
	if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Profile report exported to: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export profile report to: %s"), *FilePath);
	}
}

void UPerformanceProfilerSubsystem::LogTopBottlenecks(int32 Count)
{
	TArray<FProfileData> TopBottlenecks = GetTopBottlenecks(Count);
	
	UE_LOG(LogTemp, Warning, TEXT("========== Top %d Performance Bottlenecks =========="), Count);
	UE_LOG(LogTemp, Warning, TEXT("Overall: %.2f ms/frame (%.1f FPS)"), AverageFrameTimeMs, GetCurrentFPS());
	UE_LOG(LogTemp, Warning, TEXT("Memory: %lld MB (Peak: %lld MB)"), GetCurrentMemoryUsageMB(), GetPeakMemoryUsageMB());
	UE_LOG(LogTemp, Warning, TEXT(""));
	
	for (int32 i = 0; i < TopBottlenecks.Num(); i++)
	{
		const FProfileData& Profile = TopBottlenecks[i];
		UE_LOG(LogTemp, Warning, TEXT("%d. %s: %.3f ms avg (min: %.3f, max: %.3f, samples: %d)"),
			i + 1,
			*Profile.SystemName.ToString(),
			Profile.AverageTimeMs,
			Profile.MinTimeMs,
			Profile.MaxTimeMs,
			Profile.SampleCount
		);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("===================================================="));
}

int64 UPerformanceProfilerSubsystem::GetCurrentMemoryUsageMB() const
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return (int64)(MemStats.UsedPhysical / (1024 * 1024));
}

int64 UPerformanceProfilerSubsystem::GetPeakMemoryUsageMB() const
{
	return PeakMemoryBytes / (1024 * 1024);
}
