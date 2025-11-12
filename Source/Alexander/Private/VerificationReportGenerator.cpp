// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerificationReportGenerator.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

UVerificationReportGenerator::UVerificationReportGenerator()
{
	PrimaryComponentTick.bCanEverTick = false;
	TotalReportsGenerated = 0;
}

void UVerificationReportGenerator::BeginPlay()
{
	Super::BeginPlay();
	ClearReport();
}

void UVerificationReportGenerator::GenerateReport(const FVerificationResult& Result, const FVerificationConfig& Config)
{
	LastResult = Result;
	LastConfig = Config;
	TotalReportsGenerated++;

	// Sort custom sections by priority
	CustomSections.Sort([](const FReportSection& A, const FReportSection& B) {
		return A.Priority < B.Priority;
	});

	UE_LOG(LogTemp, Log, TEXT("Generated verification report for %d systems"), Result.SystemsTested);
}

bool UVerificationReportGenerator::SaveReportToFile(const FString& FilePath, EReportFormat Format)
{
	if (GeneratedReport.IsEmpty())
	{
		// Generate the report first
		GeneratedReport = GetReportInFormat(Format);
	}

	FString FullPath = FPaths::ProjectSavedDir() / FilePath;
	FString Directory = FPaths::GetPath(FullPath);

	// Ensure directory exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	// Save the file
	bool bSuccess = FFileHelper::SaveStringToFile(GeneratedReport, *FullPath);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Saved verification report to %s"), *FullPath);
		ReportsByFormat.FindOrAdd(Format)++;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save verification report to %s"), *FullPath);
	}

	return bSuccess;
}

FString UVerificationReportGenerator::GetReportInFormat(EReportFormat Format) const
{
	switch (Format)
	{
	case EReportFormat::JSON:
		return GenerateJSONReport();
	case EReportFormat::XML:
		return GenerateXMLReport();
	case EReportFormat::CSV:
		return GenerateCSVReport();
	case EReportFormat::HTML:
		return GenerateHTMLReport();
	case EReportFormat::PlainText:
	default:
		return GeneratePlainTextReport();
	}
}

void UVerificationReportGenerator::AddCustomSection(const FString& Title, const FString& Content, int32 Priority)
{
	FReportSection NewSection;
	NewSection.Title = Title;
	NewSection.Content = Content;
	NewSection.Priority = Priority;
	CustomSections.Add(NewSection);
}

void UVerificationReportGenerator::ClearReport()
{
	GeneratedReport.Empty();
	CustomSections.Empty();
	LastResult = FVerificationResult();
}

FString UVerificationReportGenerator::GenerateJSONReport() const
{
	FString Report = TEXT("{\n");
	Report += TEXT("  \"verification_report\": {\n");
	Report += TEXT("    \"timestamp\": \"") + GetCurrentTimestamp() + TEXT("\",\n");
	Report += TEXT("    \"summary\": {\n");
	Report += TEXT("      \"overall_success\": ") + FString(LastResult.bAllTestsPassed ? TEXT("true") : TEXT("false")) + TEXT(",\n");
	Report += TEXT("      \"systems_tested\": ") + FString::FromInt(LastResult.SystemsTested) + TEXT(",\n");
	Report += TEXT("      \"systems_passed\": ") + FString::FromInt(LastResult.SystemsPassed) + TEXT(",\n");
	Report += TEXT("      \"systems_failed\": ") + FString::FromInt(LastResult.SystemsFailed) + TEXT(",\n");
	Report += TEXT("      \"total_duration\": ") + FString::Printf(TEXT("%.2f"), LastResult.TotalDuration) + TEXT("\n");
	Report += TEXT("    },\n");

	// System results
	Report += TEXT("    \"system_results\": [\n");
	for (int32 i = 0; i < LastResult.SystemResults.Num(); i++)
	{
		const FSystemTestResult& SystemResult = LastResult.SystemResults[i];
		Report += TEXT("      {\n");
		Report += TEXT("        \"system_name\": \"") + EscapeJSONString(SystemResult.SystemName) + TEXT("\",\n");
		Report += TEXT("        \"passed\": ") + FString(SystemResult.bPassed ? TEXT("true") : TEXT("false")) + TEXT(",\n");
		Report += TEXT("        \"duration\": ") + FString::Printf(TEXT("%.2f"), SystemResult.TestDuration);
		
		if (!SystemResult.ErrorMessage.IsEmpty())
		{
			Report += TEXT(",\n        \"error\": \"") + EscapeJSONString(SystemResult.ErrorMessage) + TEXT("\"");
		}

		if (SystemResult.WarningMessages.Num() > 0)
		{
			Report += TEXT(",\n        \"warnings\": [\n");
			for (int32 j = 0; j < SystemResult.WarningMessages.Num(); j++)
			{
				Report += TEXT("          \"") + EscapeJSONString(SystemResult.WarningMessages[j]) + TEXT("\"");
				if (j < SystemResult.WarningMessages.Num() - 1)
				{
					Report += TEXT(",");
				}
				Report += TEXT("\n");
			}
			Report += TEXT("        ]");
		}

		Report += TEXT("\n      }");
		if (i < LastResult.SystemResults.Num() - 1)
		{
			Report += TEXT(",");
		}
		Report += TEXT("\n");
	}
	Report += TEXT("    ],\n");

	// Performance metrics
	if (LastResult.PerformanceMetrics.Num() > 0)
	{
		Report += TEXT("    \"performance_metrics\": {\n");
		int32 MetricIndex = 0;
		for (const auto& Metric : LastResult.PerformanceMetrics)
		{
			Report += TEXT("      \"") + Metric.Key + TEXT("\": ") + FString::Printf(TEXT("%.2f"), Metric.Value);
			if (MetricIndex < LastResult.PerformanceMetrics.Num() - 1)
			{
				Report += TEXT(",");
			}
			Report += TEXT("\n");
			MetricIndex++;
		}
		Report += TEXT("    }");
	}

	Report += TEXT("\n  }\n");
	Report += TEXT("}\n");

	return Report;
}

FString UVerificationReportGenerator::GenerateXMLReport() const
{
	FString Report = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	Report += TEXT("<verification_report>\n");
	Report += TEXT("  <timestamp>") + EscapeXMLString(GetCurrentTimestamp()) + TEXT("</timestamp>\n");
	Report += TEXT("  <summary>\n");
	Report += TEXT("    <overall_success>") + FString(LastResult.bAllTestsPassed ? TEXT("true") : TEXT("false")) + TEXT("</overall_success>\n");
	Report += TEXT("    <systems_tested>") + FString::FromInt(LastResult.SystemsTested) + TEXT("</systems_tested>\n");
	Report += TEXT("    <systems_passed>") + FString::FromInt(LastResult.SystemsPassed) + TEXT("</systems_passed>\n");
	Report += TEXT("    <systems_failed>") + FString::FromInt(LastResult.SystemsFailed) + TEXT("</systems_failed>\n");
	Report += TEXT("    <total_duration>") + FString::Printf(TEXT("%.2f"), LastResult.TotalDuration) + TEXT("</total_duration>\n");
	Report += TEXT("  </summary>\n");

	// System results
	Report += TEXT("  <system_results>\n");
	for (const FSystemTestResult& SystemResult : LastResult.SystemResults)
	{
		Report += TEXT("    <system>\n");
		Report += TEXT("      <name>") + EscapeXMLString(SystemResult.SystemName) + TEXT("</name>\n");
		Report += TEXT("      <passed>") + FString(SystemResult.bPassed ? TEXT("true") : TEXT("false")) + TEXT("</passed>\n");
		Report += TEXT("      <duration>") + FString::Printf(TEXT("%.2f"), SystemResult.TestDuration) + TEXT("</duration>\n");
		
		if (!SystemResult.ErrorMessage.IsEmpty())
		{
			Report += TEXT("      <error>") + EscapeXMLString(SystemResult.ErrorMessage) + TEXT("</error>\n");
		}

		if (SystemResult.WarningMessages.Num() > 0)
		{
			Report += TEXT("      <warnings>\n");
			for (const FString& Warning : SystemResult.WarningMessages)
			{
				Report += TEXT("        <warning>") + EscapeXMLString(Warning) + TEXT("</warning>\n");
			}
			Report += TEXT("      </warnings>\n");
		}
		Report += TEXT("    </system>\n");
	}
	Report += TEXT("  </system_results>\n");

	// Performance metrics
	if (LastResult.PerformanceMetrics.Num() > 0)
	{
		Report += TEXT("  <performance_metrics>\n");
		for (const auto& Metric : LastResult.PerformanceMetrics)
		{
			Report += TEXT("    <metric name=\"") + Metric.Key + TEXT("\">") + 
				FString::Printf(TEXT("%.2f"), Metric.Value) + TEXT("</metric>\n");
		}
		Report += TEXT("  </performance_metrics>\n");
	}

	Report += TEXT("</verification_report>\n");
	return Report;
}

FString UVerificationReportGenerator::GenerateCSVReport() const
{
	FString Report = TEXT("System Name,Passed,Duration (s),Error Message\n");

	for (const FSystemTestResult& SystemResult : LastResult.SystemResults)
	{
		Report += EscapeXMLString(SystemResult.SystemName) + TEXT(",");
		Report += FString(SystemResult.bPassed ? TEXT("PASSED") : TEXT("FAILED")) + TEXT(",");
		Report += FString::Printf(TEXT("%.2f,"), SystemResult.TestDuration);
		
		if (!SystemResult.ErrorMessage.IsEmpty())
		{
			Report += EscapeXMLString(SystemResult.ErrorMessage);
		}
		
		Report += TEXT("\n");
	}

	Report += TEXT("\nSummary\n");
	Report += TEXT("Total Systems,") + FString::FromInt(LastResult.SystemsTested) + TEXT("\n");
	Report += TEXT("Passed,") + FString::FromInt(LastResult.SystemsPassed) + TEXT("\n");
	Report += TEXT("Failed,") + FString::FromInt(LastResult.SystemsFailed) + TEXT("\n");
	Report += TEXT("Total Duration (s),") + FString::Printf(TEXT("%.2f"), LastResult.TotalDuration) + TEXT("\n");

	return Report;
}

FString UVerificationReportGenerator::GeneratePlainTextReport() const
{
	FString Report = TEXT("=== ALEXANDER VERIFICATION REPORT ===\n");
	Report += TEXT("Generated: ") + GetCurrentTimestamp() + TEXT("\n");
	Report += TEXT("=====================================\n\n");

	// Summary
	Report += TEXT("SUMMARY\n");
	Report += TEXT("-------\n");
	Report += FString::Printf(TEXT("Overall Result: %s\n"), LastResult.bAllTestsPassed ? TEXT("PASSED") : TEXT("FAILED"));
	Report += FString::Printf(TEXT("Systems Tested: %d\n"), LastResult.SystemsTested);
	Report += FString::Printf(TEXT("Systems Passed: %d\n"), LastResult.SystemsPassed);
	Report += FString::Printf(TEXT("Systems Failed: %d\n"), LastResult.SystemsFailed);
	Report += FString::Printf(TEXT("Total Duration: %.2f seconds\n"), LastResult.TotalDuration);
	Report += TEXT("\n");

	// System results
	Report += TEXT("SYSTEM RESULTS\n");
	Report += TEXT("--------------\n");
	for (const FSystemTestResult& SystemResult : LastResult.SystemResults)
	{
		Report += FString::Printf(TEXT("%s: %s (%.2f s)\n"), 
			*SystemResult.SystemName, 
			SystemResult.bPassed ? TEXT("PASSED") : TEXT("FAILED"),
			SystemResult.TestDuration);

		if (!SystemResult.ErrorMessage.IsEmpty())
		{
			Report += FString::Printf(TEXT("  Error: %s\n"), *SystemResult.ErrorMessage);
		}

		if (SystemResult.WarningMessages.Num() > 0)
		{
			Report += TEXT("  Warnings:\n");
			for (const FString& Warning : SystemResult.WarningMessages)
			{
				Report += FString::Printf(TEXT("    - %s\n"), *Warning);
			}
		}
	}
	Report += TEXT("\n");

	// Performance metrics
	if (LastResult.PerformanceMetrics.Num() > 0)
	{
		Report += TEXT("PERFORMANCE METRICS\n");
		Report += TEXT("------------------\n");
		for (const auto& Metric : LastResult.PerformanceMetrics)
		{
			Report += FString::Printf(TEXT("%s: %.2f\n"), *Metric.Key, Metric.Value);
		}
		Report += TEXT("\n");
	}

	// Custom sections
	if (CustomSections.Num() > 0)
	{
		for (const FReportSection& Section : CustomSections)
		{
			Report += Section.Title + TEXT("\n");
			Report += FString::ChrN(Section.Title.Len(), '-') + TEXT("\n");
			Report += Section.Content + TEXT("\n\n");
		}
	}

	return Report;
}

FString UVerificationReportGenerator::GenerateHTMLReport() const
{
	FString Report = TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
	Report += TEXT("  <title>Alexander Verification Report</title>\n");
	Report += TEXT("  <style>\n");
	Report += TEXT("    body { font-family: Arial, sans-serif; margin: 20px; }\n");
	Report += TEXT("    .header { background-color: #f0f0f0; padding: 20px; border-radius: 5px; }\n");
	Report += TEXT("    .passed { color: green; font-weight: bold; }\n");
	Report += TEXT("    .failed { color: red; font-weight: bold; }\n");
	Report += TEXT("    .system-result { margin: 10px 0; padding: 10px; border: 1px solid #ddd; }\n");
	Report += TEXT("    .metrics { background-color: #f9f9f9; padding: 15px; margin: 10px 0; }\n");
	Report += TEXT("  </style>\n");
	Report += TEXT("</head>\n<body>\n");
	Report += TEXT("  <div class=\"header\">\n");
	Report += TEXT("    <h1>Alexander Verification Report</h1>\n");
	Report += TEXT("    <p>Generated: ") + GetCurrentTimestamp() + TEXT("</p>\n");
	Report += TEXT("  </div>\n");

	// Summary
	Report += TEXT("  <h2>Summary</h2>\n");
	Report += TEXT("  <p>Overall Result: <span class=\"") + 
		FString(LastResult.bAllTestsPassed ? TEXT("passed") : TEXT("failed")) + TEXT("\">") +
		FString(LastResult.bAllTestsPassed ? TEXT("PASSED") : TEXT("FAILED")) + TEXT("</span></p>\n");
	Report += TEXT("  <p>Systems Tested: ") + FString::FromInt(LastResult.SystemsTested) + TEXT("</p>\n");
	Report += TEXT("  <p>Systems Passed: ") + FString::FromInt(LastResult.SystemsPassed) + TEXT("</p>\n");
	Report += TEXT("  <p>Systems Failed: ") + FString::FromInt(LastResult.SystemsFailed) + TEXT("</p>\n");
	Report += TEXT("  <p>Total Duration: ") + FString::Printf(TEXT("%.2f seconds"), LastResult.TotalDuration) + TEXT("</p>\n");

	// System results
	Report += TEXT("  <h2>System Results</h2>\n");
	for (const FSystemTestResult& SystemResult : LastResult.SystemResults)
	{
		Report += TEXT("  <div class=\"system-result\">\n");
		Report += TEXT("    <h3>") + SystemResult.SystemName + TEXT(": <span class=\"") + 
			FString(SystemResult.bPassed ? TEXT("passed") : TEXT("failed")) + TEXT("\">") +
			FString(SystemResult.bPassed ? TEXT("PASSED") : TEXT("FAILED")) + TEXT("</span></h3>\n");
		Report += TEXT("    <p>Duration: ") + FString::Printf(TEXT("%.2f seconds"), SystemResult.TestDuration) + TEXT("</p>\n");
		
		if (!SystemResult.ErrorMessage.IsEmpty())
		{
			Report += TEXT("    <p><strong>Error:</strong> ") + EscapeXMLString(SystemResult.ErrorMessage) + TEXT("</p>\n");
		}

		if (SystemResult.WarningMessages.Num() > 0)
		{
			Report += TEXT("    <p><strong>Warnings:</strong></p>\n");
			Report += TEXT("    <ul>\n");
			for (const FString& Warning : SystemResult.WarningMessages)
			{
				Report += TEXT("      <li>") + EscapeXMLString(Warning) + TEXT("</li>\n");
			}
			Report += TEXT("    </ul>\n");
		}
		Report += TEXT("  </div>\n");
	}

	// Performance metrics
	if (LastResult.PerformanceMetrics.Num() > 0)
	{
		Report += TEXT("  <h2>Performance Metrics</h2>\n");
		Report += TEXT("  <div class=\"metrics\">\n");
		for (const auto& Metric : LastResult.PerformanceMetrics)
		{
			Report += TEXT("    <p><strong>") + Metric.Key + TEXT(":</strong> ") + 
				FString::Printf(TEXT("%.2f"), Metric.Value) + TEXT("</p>\n");
		}
		Report += TEXT("  </div>\n");
	}

	Report += TEXT("</body>\n</html>\n");
	return Report;
}

FString UVerificationReportGenerator::EscapeJSONString(const FString& Input) const
{
	FString Output = Input;
	Output.ReplaceInline(TEXT("\""), TEXT("\\\""));
	Output.ReplaceInline(TEXT("\n"), TEXT("\\n"));
	Output.ReplaceInline(TEXT("\r"), TEXT("\\r"));
	Output.ReplaceInline(TEXT("\t"), TEXT("\\t"));
	return Output;
}

FString UVerificationReportGenerator::EscapeXMLString(const FString& Input) const
{
	FString Output = Input;
	Output.ReplaceInline(TEXT("&"), TEXT("&amp;"));
	Output.ReplaceInline(TEXT("<"), TEXT("&lt;"));
	Output.ReplaceInline(TEXT(">"), TEXT("&gt;"));
	Output.ReplaceInline(TEXT("\""), TEXT("&quot;"));
	Output.ReplaceInline(TEXT("'"), TEXT("&apos;"));
	return Output;
}

FString UVerificationReportGenerator::GetCurrentTimestamp() const
{
	return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}