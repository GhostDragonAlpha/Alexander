// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NakamaIntegration.generated.h"

// Forward declarations
class UNakamaClient;
class UNakamaSession;
class UNakamaRealtimeClient;
class UNakamaSocket;

/**
 * Nakama connection states
 */
UENUM(BlueprintType)
enum class ENakamaConnectionState : uint8
{
	Disconnected UMETA(DisplayName = "Disconnected"),
	Connecting UMETA(DisplayName = "Connecting"),
	Connected UMETA(DisplayName = "Connected"),
	Authenticating UMETA(DisplayName = "Authenticating"),
	Authenticated UMETA(DisplayName = "Authenticated"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Nakama match types
 */
UENUM(BlueprintType)
enum class ENakamaMatchType : uint8
{
	Relayed UMETA(DisplayName = "Relayed"),
	Authoritative UMETA(DisplayName = "Authoritative"),
	Dynamic UMETA(DisplayName = "Dynamic")
};

/**
 * Nakama player data
 */
USTRUCT(BlueprintType)
struct FNakamaPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	UPROPERTY(BlueprintReadOnly)
	FString Username;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	int32 Score;

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline;

	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	UPROPERTY(BlueprintReadOnly)
	FString PresenceID;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Metadata;

	FNakamaPlayerData()
	{
		PlayerID = TEXT("");
		Username = TEXT("");
		DisplayName = TEXT("");
		Score = 0;
		bIsOnline = false;
		SessionID = TEXT("");
		PresenceID = TEXT("");
		Metadata.Empty();
	}
};

/**
 * Nakama match data
 */
USTRUCT(BlueprintType)
struct FNakamaMatchData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString MatchID;

	UPROPERTY(BlueprintReadOnly)
	ENakamaMatchType MatchType;

	UPROPERTY(BlueprintReadOnly)
	FString MatchName;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPrivate;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FNakamaPlayerData> Players;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> MatchData;

	UPROPERTY(BlueprintReadOnly)
	FString CreatorID;

	FNakamaMatchData()
	{
		MatchID = TEXT("");
		MatchType = ENakamaMatchType::Relayed;
		MatchName = TEXT("");
		MaxPlayers = 64;
		CurrentPlayers = 0;
		bIsPrivate = false;
		Players.Empty();
		MatchData.Empty();
		CreatorID = TEXT("");
	}
};

/**
 * Nakama Integration - Distributed server integration using Nakama
 * Provides scalable multiplayer backend with real-time features
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UNakamaIntegration : public UObject
{
	GENERATED_BODY()

public:
	UNakamaIntegration();

protected:
	virtual void BeginDestroy() override;

public:
	// Connection management
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	bool ConnectToNakama(const FString& ServerURL, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void DisconnectFromNakama();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	ENakamaConnectionState GetConnectionState() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	bool IsConnected() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	bool IsAuthenticated() const;

	// Authentication
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateWithEmail(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateWithDevice(const FString& DeviceID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateWithFacebook(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateWithSteam(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateWithGoogle(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AuthenticateAsGuest();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void Logout();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	FString GetCurrentUserID() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	FString GetCurrentUsername() const;

	// Match management
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void CreateMatch(ENakamaMatchType MatchType, const FString& MatchName, int32 MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void JoinMatch(const FString& MatchID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void LeaveMatch(const FString& MatchID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	TArray<FNakamaMatchData> GetAvailableMatches();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	FNakamaMatchData GetCurrentMatch() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	bool IsInMatch() const;

	// Real-time communication
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SendMatchData(const FString& MatchID, const TArray<uint8>& Data);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SendChatMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SendPrivateMessage(const FString& UserID, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SendPartyInvite(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AcceptPartyInvite(const FString& PartyID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void DeclinePartyInvite(const FString& PartyID);

	// Leaderboards and rankings
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SubmitScore(const FString& LeaderboardID, int64 Score, const FString& Subscore = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void GetLeaderboard(const FString& LeaderboardID, int32 Limit = 100);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void GetLeaderboardAroundUser(const FString& LeaderboardID, int32 Limit = 10);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void GetFriendsLeaderboard(const FString& LeaderboardID, int32 Limit = 100);

	// Friends and social
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void AddFriend(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void RemoveFriend(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void BlockUser(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void UnblockUser(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	TArray<FNakamaPlayerData> GetFriendsList();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	TArray<FNakamaPlayerData> GetBlockedUsers();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SearchUsers(const FString& Query);

	// Storage and cloud data
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void WriteStorageData(const FString& Collection, const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	FString ReadStorageData(const FString& Collection, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void DeleteStorageData(const FString& Collection, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void ListStorageData(const FString& Collection);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SyncStorageData();

	// Parties and groups
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void CreateParty(const FString& PartyName, bool bIsOpen);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void JoinParty(const FString& PartyID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void LeaveParty(const FString& PartyID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void PromotePartyMember(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void KickPartyMember(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	TArray<FNakamaPlayerData> GetPartyMembers(const FString& PartyID);

	// Notifications
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SendNotification(const FString& UserID, const FString& Content, int32 Code);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void ListNotifications();

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void MarkNotificationRead(const FString& NotificationID);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void DeleteNotification(const FString& NotificationID);

	// Server configuration
	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	void SetServerConfiguration(const FString& ServerKey, const FString& ServerURL, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	FString GetServerURL() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	int32 GetServerPort() const;

	UFUNCTION(BlueprintCallable, Category = "Nakama Integration")
	bool IsProductionMode() const;

	// Events and delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, ENakamaConnectionState, NewState);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnConnectionStateChanged OnConnectionStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthenticated, const FString&, UserID);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnAuthenticated OnAuthenticated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchJoined, const FString&, MatchID, const FNakamaMatchData&, MatchData);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnMatchJoined OnMatchJoined;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchLeft, const FString&, MatchID, const FString&, Reason);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnMatchLeft OnMatchLeft;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMatchDataReceived, const FString&, MatchID, const TArray<uint8>&, Data, const FString&, SenderID);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnMatchDataReceived OnMatchDataReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceived, const FString&, SenderID, const FString&, Message);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnChatMessageReceived OnChatMessageReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendAdded, const FString&, UserID);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnFriendAdded OnFriendAdded;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRemoved, const FString&, UserID);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnFriendRemoved OnFriendRemoved;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeaderboardUpdated, const FString&, LeaderboardID, const TArray<FNakamaPlayerData>&, Players);
	UPROPERTY(BlueprintAssignable, Category = "Nakama Integration")
	FOnLeaderboardUpdated OnLeaderboardUpdated;

protected:
	// Nakama clients - commented out until Nakama plugin is integrated
	//	UPROPERTY()
	//	UNakamaClient* NakamaClient;

	//	UPROPERTY()
	//	UNakamaSession* NakamaSession;

	//	UPROPERTY()
	//	UNakamaRealtimeClient* RealtimeClient;

	//	UPROPERTY()
	//	UNakamaSocket* NakamaSocket;
	
	// Temporary placeholder pointers
	UPROPERTY()
	class UObject* NakamaClient;
	
	UPROPERTY()
	class UObject* NakamaSession;
	
	UPROPERTY()
	class UObject* RealtimeClient;
	
	UPROPERTY()
	class UObject* NakamaSocket;

	// Connection state
	UPROPERTY()
	ENakamaConnectionState ConnectionState;

	// Server configuration
	UPROPERTY()
	FString ServerURL;

	UPROPERTY()
	int32 ServerPort;

	UPROPERTY()
	FString ServerKey;

	UPROPERTY()
	bool bProductionMode;

	// Current match
	UPROPERTY()
	FNakamaMatchData CurrentMatch;

	// Player data
	UPROPERTY()
	FNakamaPlayerData CurrentPlayer;

	// Friends and social
	UPROPERTY()
	TArray<FNakamaPlayerData> FriendsList;

	UPROPERTY()
	TArray<FNakamaPlayerData> BlockedUsers;

	// Match list
	UPROPERTY()
	TArray<FNakamaMatchData> AvailableMatches;

private:
	void InitializeNakamaClient();
	void ConnectRealtimeClient();
	void HandleConnectionError(const FString& ErrorMessage);
	void HandleAuthenticationSuccess(const FString& UserID);
	void HandleMatchJoin(const FString& MatchID);
	void HandleMatchLeave(const FString& MatchID, const FString& Reason);
	void HandleMatchData(const FString& MatchID, const TArray<uint8>& Data, const FString& SenderID);
	void HandleChatMessage(const FString& SenderID, const FString& Message);
	void UpdateConnectionState(ENakamaConnectionState NewState);
	FString GenerateDeviceID() const;
};