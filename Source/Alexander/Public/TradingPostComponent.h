#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "EconomySystem.h"
#include "PerformanceProfiler.h"
#include "TradingPostComponent.generated.h"

UENUM(BlueprintType)
enum class ETradingPostType : uint8
{
    General,
    Weapons,
    Armor,
    Electronics,
    Medical,
    Food,
    Materials,
    Ships,
    Modules,
    BlackMarket
};

UENUM(BlueprintType)
enum class ETransactionType : uint8
{
    Buy,
    Sell,
    Trade,
    Auction,
    Barter
};

UENUM(BlueprintType)
enum class EMarketCondition : uint8
{
    Normal,
    HighDemand,
    LowSupply,
    Surplus,
    Shortage,
    Boom,
    Bust
};

USTRUCT(BlueprintType)
struct FMarketItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BasePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Quality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ItemProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredFactions; // Changed from TArray to FString for serialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLegal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRare;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PriceModifier;
};

USTRUCT(BlueprintType)
struct FTransaction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TransactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETransactionType TransactionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* Buyer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* Seller;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMarketItem Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TransactionNotes;
};

USTRUCT(BlueprintType)
struct FAuction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AuctionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMarketItem Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* Seller;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StartingBid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentBid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* CurrentBidder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinBidIncrement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<APlayerState*> Bidders;
};

USTRUCT(BlueprintType)
struct FTradingPost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PostName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETradingPostType PostType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FMarketItem> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAuction> ActiveAuctions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TaxRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BuyModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SellModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMarketCondition MarketCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> FactionReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OpenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloseTime;
};

USTRUCT(BlueprintType)
struct FPriceHistoryData
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<float> PricePoints;
};

USTRUCT(BlueprintType)
struct FItemPriceHistory
{
    GENERATED_BODY()

    UPROPERTY()
    TMap<FName, FPriceHistoryData> ItemHistory;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UTradingPostComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTradingPostComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Trading Post Management
    UFUNCTION(BlueprintCallable, Category = "Trading Post")
    void InitializeTradingPost();

    UFUNCTION(BlueprintCallable, Category = "Trading Post")
    void RegisterTradingPost(const FTradingPost& Post);

    UFUNCTION(BlueprintCallable, Category = "Trading Post")
    void UnregisterTradingPost(const FName& PostName);

    FTradingPost* GetTradingPost(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Trading Post")
    TArray<FTradingPost> GetAllTradingPosts();

    UFUNCTION(BlueprintCallable, Category = "Trading Post")
    void UpdateTradingPostInventory(const FName& PostName, const TMap<FName, FMarketItem>& NewInventory);

    // Market Operations
    UFUNCTION(BlueprintCallable, Category = "Market Operations")
    TArray<FMarketItem> GetMarketInventory(const FName& PostName);

    FMarketItem* GetMarketItem(const FName& PostName, const FName& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Market Operations")
    bool CanPlayerBuyItem(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Market Operations")
    bool CanPlayerSellItem(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Market Operations")
    int32 CalculateBuyPrice(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Market Operations")
    int32 CalculateSellPrice(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity);

    // Transactions
    UFUNCTION(BlueprintCallable, Category = "Transactions")
    bool ProcessBuyTransaction(APlayerState* Buyer, const FName& PostName, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Transactions")
    bool ProcessSellTransaction(APlayerState* Seller, const FName& PostName, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Transactions")
    bool ProcessTradeTransaction(APlayerState* Player1, APlayerState* Player2, const FName& Item1ID, const FName& Item2ID, int32 Quantity1, int32 Quantity2);

    FTransaction* GetTransaction(const FName& TransactionID);

    UFUNCTION(BlueprintCallable, Category = "Transactions")
    TArray<FTransaction> GetPlayerTransactions(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Transactions")
    TArray<FTransaction> GetPostTransactions(const FName& PostName);

    // Auctions
    UFUNCTION(BlueprintCallable, Category = "Auctions")
    bool StartAuction(APlayerState* Seller, const FName& PostName, const FMarketItem& Item, int32 StartingBid, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Auctions")
    bool PlaceBid(APlayerState* Bidder, const FName& AuctionID, int32 BidAmount);

    UFUNCTION(BlueprintCallable, Category = "Auctions")
    bool WithdrawAuction(APlayerState* Seller, const FName& AuctionID);

    FAuction* GetAuction(const FName& AuctionID);

    UFUNCTION(BlueprintCallable, Category = "Auctions")
    TArray<FAuction> GetActiveAuctions(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Auctions")
    TArray<FAuction> GetPlayerAuctions(APlayerState* Player);

    // Market Analysis
    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    TMap<FName, float> GetMarketPrices(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    TMap<FName, float> GetPriceHistory(const FName& PostName, const FName& ItemID, int32 Days);

    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    EMarketCondition GetMarketCondition(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    float GetMarketActivity(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    TArray<FName> GetTrendingItems(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Market Analysis")
    TArray<FName> GetRecommendedItems(APlayerState* Player, const FName& PostName);

    // Market Events
    UFUNCTION(BlueprintCallable, Category = "Market Events")
    void TriggerMarketEvent(const FName& PostName, EMarketCondition NewCondition, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Market Events")
    void UpdateMarketConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Market Events")
    void RefreshMarketInventory(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Market Events")
    void ApplyMarketFluctuations(const FName& PostName);

    // Player Trading
    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool InitiateTrade(APlayerState* Initiator, APlayerState* Target);

    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool RespondToTrade(APlayerState* Player, const FName& TradeID, bool bAccept);

    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool AddTradeItem(APlayerState* Player, const FName& TradeID, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool RemoveTradeItem(APlayerState* Player, const FName& TradeID, const FName& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool ConfirmTrade(APlayerState* Player, const FName& TradeID);

    UFUNCTION(BlueprintCallable, Category = "Player Trading")
    bool CancelTrade(const FName& TradeID);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsTradingPostOpen(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetTaxRate(const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetFactionReputation(APlayerState* Player, const FName& PostName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void UpdateFactionReputation(APlayerState* Player, const FName& PostName, float Change);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TMap<FName, int32> GetPlayerInventory(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool HasPlayerItem(APlayerState* Player, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void AddItemToPlayerInventory(APlayerState* Player, const FName& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void RemoveItemFromPlayerInventory(APlayerState* Player, const FName& ItemID, int32 Quantity);

protected:
    // Trading Posts
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trading Posts")
    TMap<FName, FTradingPost> TradingPosts;

    // Transactions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transactions")
    TMap<FName, FTransaction> Transactions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transactions")
    TArray<FTransaction> TransactionHistory;

    // Auctions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auctions")
    TMap<FName, FAuction> Auctions;

    // Player Trades
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Trades")
    TMap<FName, FTransaction> PlayerTrades;

    // Market Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Market Data")
    TMap<FName, FItemPriceHistory> PriceHistory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Market Data")
    TMap<FName, EMarketCondition> MarketConditions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Market Data")
    TMap<FName, float> MarketActivity;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultTaxRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultBuyModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultSellModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MarketUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PriceFluctuationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxTransactionHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAuctions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePlayerTrading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableMarketEvents;

    // System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UEconomySystem* EconomySystem;

private:
    // Internal Functions
    void UpdateTradingPosts(float DeltaTime);
    void ProcessAuctions(float DeltaTime);
    void UpdateMarketData(float DeltaTime);
    void ProcessPlayerTrades(float DeltaTime);

    // Transaction Internal
    FName GenerateTransactionID();
    void RecordTransaction(const FTransaction& Transaction);
    void ApplyTransactionEffects(const FTransaction& Transaction);
    bool ValidateTransaction(const FTransaction& Transaction);

    // Auction Internal
    FName GenerateAuctionID();
    void ProcessAuctionEnd(const FName& AuctionID);
    void NotifyAuctionParticipants(const FAuction& Auction, const FString& Message);
    bool ValidateAuction(const FAuction& Auction);

    // Market Internal
    void UpdateItemPrice(const FName& PostName, const FName& ItemID);
    void ApplyMarketCondition(const FName& PostName, EMarketCondition Condition);
    void GenerateMarketEvent(const FName& PostName);
    void UpdatePriceHistory(const FName& PostName, const FName& ItemID, float Price);

    // Player Trading Internal
    FName GenerateTradeID();
    bool ValidateTrade(const FTransaction& Trade);
    void ExecuteTrade(const FTransaction& Trade);
    void NotifyTradeParticipants(const FTransaction& Trade, const FString& Message);

    // Inventory Internal
    bool RemoveFromInventory(APlayerState* Player, const FName& ItemID, int32 Quantity);
    bool AddToInventory(APlayerState* Player, const FName& ItemID, int32 Quantity);
    bool CanAffordTransaction(APlayerState* Player, int32 Cost);

    // Reputation Internal
    float CalculateReputationModifier(APlayerState* Player, const FName& PostName);
    void UpdateReputation(APlayerState* Player, const FName& PostName, float Change);

    // Timers
    UPROPERTY()
    FTimerHandle MarketUpdateTimer;

    UPROPERTY()
    FTimerHandle AuctionUpdateTimer;

    UPROPERTY()
    FTimerHandle PriceUpdateTimer;

    // Performance Profiler
    UPROPERTY()
    UPerformanceProfiler* PerformanceProfiler;

    // Constants
    static constexpr float DEFAULT_MARKET_UPDATE_INTERVAL = 60.0f; // 1 minute
    static constexpr float DEFAULT_AUCTION_UPDATE_INTERVAL = 10.0f; // 10 seconds
    static constexpr float DEFAULT_PRICE_UPDATE_INTERVAL = 300.0f; // 5 minutes
    static constexpr float DEFAULT_TAX_RATE = 0.05f; // 5%
    static constexpr float DEFAULT_BUY_MODIFIER = 1.1f; // 10% markup
    static constexpr float DEFAULT_SELL_MODIFIER = 0.9f; // 10% markdown
    static constexpr float PRICE_FLUCTUATION_RATE = 0.02f; // 2%
    static constexpr int32 DEFAULT_MAX_TRANSACTION_HISTORY = 1000;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTransactionCompleted, const FTransaction&, Transaction, bool, bSuccess, APlayerState*, Player, const FName&, PostName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAuctionStarted, const FAuction&, Auction, APlayerState*, Seller, const FName&, PostName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAuctionEnded, const FAuction&, Auction, APlayerState*, Winner, const FName&, PostName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarketConditionChanged, const FName&, PostName, EMarketCondition, NewCondition);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTradeInitiated, const FName&, TradeID, APlayerState*, Initiator, APlayerState*, Target);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, const FName&, TradeID, bool, bSuccess);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnTransactionCompleted OnTransactionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnAuctionStarted OnAuctionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnAuctionEnded OnAuctionEnded;

    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnMarketConditionChanged OnMarketConditionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnTradeInitiated OnTradeInitiated;

    UPROPERTY(BlueprintAssignable, Category = "Trading Events")
    FOnTradeCompleted OnTradeCompleted;
};