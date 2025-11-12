// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SystemSelfTestInterface.h"
#include "VerificationGameMode.h"
#include "VerificationReportGenerator.generated.h"

// Report format types
UENUM(BlueprintType)
enum class EReportFormat : uint8
{
	JSON		UMETA(DisplayName = "JSON"),
	XML			UMETA(DisplayName = "XML"),
	CSV			UMETA(DisplayName = "CSV"),
	PlainText	UMETA(DisplayName = "Plain Text"),
	HTML		UMETA(DisplayName = "HTML")
};

// Report sections
USTRUCT(BlueprintType)
struct FReportSection
{
	GENERATED_BODY()

	// Section title
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString Title;

	// Section content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString Content;

	// Section priority (lower = higher priority)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	int32 Priority;

	FReportSection()
	{
		Priority = 100;
	}
};

// Report generator for creating verification reports in various formats
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVerificationReportGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	UVerificationReportGenerator();

protected:
	virtual void BeginPlay() override;

public:
	// Generate report from verification result
	UFUNCTION(BlueprintCallable, Category = "Report")
	void GenerateReport(const FVerificationResult& Result, const FVerificationConfig& Config);

	// Save report to file
	UFUNCTION(BlueprintCallable, Category = "Report")
	bool SaveReportToFile(const FString& FilePath, EReportFormat Format = EReportFormat::JSON);

	// Get report as string
	UFUNCTION(BlueprintCallable, Category = "Report")
	FString GetReportAsString() const { return GeneratedReport; }

	// Get report in specific format
	UFUNCTION(BlueprintCallable, Category = "Report")
	FString GetReportInFormat(EReportFormat Format) const;

	// Add custom section to report
	UFUNCTION(BlueprintCallable, Category = "Report")
	void AddCustomSection(const FString& Title, const FString& Content, int32 Priority = 100);

	// Clear all report data
	UFUNCTION(BlueprintCallable, Category = "Report")
	void ClearReport();

protected:
	// Generate JSON report
	FString GenerateJSONReport() const;

	// Generate XML report
	FString GenerateXMLReport() const;

	// Generate CSV report
	FString GenerateCSVReport() const;

	// Generate plain text report
	FString GeneratePlainTextReport() const;

	// Generate HTML report
	FString GenerateHTMLReport() const;

	// Generate summary section
	FString GenerateSummarySection() const;

	// Generate system results section
	FString GenerateSystemResultsSection() const;

	// Generate performance section
	FString GeneratePerformanceSection() const;

	// Generate errors section
	FString GenerateErrorsSection() const;

	// Generate warnings section
	FString GenerateWarningsSection() const;

	// Helper functions
	FString EscapeJSONString(const FString& Input) const;
	FString EscapeXMLString(const FString& Input) const;
	FString GetCurrentTimestamp() const;

	// Report data
	UPROPERTY()
	FVerificationResult LastResult;

	UPROPERTY()
	FVerificationConfig LastConfig;

	UPROPERTY()
	FString GeneratedReport;

	UPROPERTY()
	TArray<FReportSection> CustomSections;

	// Report statistics
	UPROPERTY()
	int32 TotalReportsGenerated;

	UPROPERTY()
	TMap<EReportFormat, int32> ReportsByFormat;
};