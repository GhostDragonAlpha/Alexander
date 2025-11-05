#include "TradingPostComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UTradingPostComponent::UTradingPostComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Configuration defaults
    DefaultTaxRate = DEFAULT_TAX_RATE;
    DefaultBuyModifier = DEFAULT_BUY_MODIFIER;
    DefaultSellModifier = DEFAULT_SELL_MODIFIER;
    MarketUpdateInterval = DEFAULT_MARKET_UPDATE_INTERVAL;
    PriceFluctuationRate = PRICE_FLUCTUATION_RATE;
    MaxTransactionHistory = DEFAULT_MAX_TRANSACTION_HISTORY;
    bEnableAuctions = true;
    bEnablePlayerTrading = true;
    bEnableMarketEvents = true;
}

void UTradingPostComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTradingPost();
}

void UTradingPostComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTradingPosts(DeltaTime);
    
    if (bEnableAuctions)
    {
        ProcessAuctions(DeltaTime);
    }
    
    if (bEnableMarketEvents)
    {
        UpdateMarketData(DeltaTime);
    }
    
    if (bEnablePlayerTrading)
    {
        ProcessPlayerTrades(DeltaTime);
    }
}

void UTradingPostComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MarketUpdateTimer);
        World->GetTimerManager().ClearTimer(AuctionUpdateTimer);
        World->GetTimerManager().ClearTimer(PriceUpdateTimer);
    }

    // Clear all data
    TradingPosts.Empty();
    Transactions.Empty();
    Auctions.Empty();
    PlayerTrades.Empty();

    Super::EndPlay(EndPlayReason);
}

void UTradingPostComponent::InitializeTradingPost()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Initialize economy system
    EconomySystem = NewObject<UEconomySystem>(this, TEXT("EconomySystem"));

    // Setup timers
    World->GetTimerManager().SetTimer(MarketUpdateTimer, [this]()
    {
        float DeltaTime = MarketUpdateInterval;
        UpdateTradingPosts(DeltaTime);
    }, MarketUpdateInterval, true);

    if (bEnableAuctions)
    {
        World->GetTimerManager().SetTimer(AuctionUpdateTimer, [this]()
        {
            float DeltaTime = DEFAULT_AUCTION_UPDATE_INTERVAL;
            ProcessAuctions(DeltaTime);
        }, DEFAULT_AUCTION_UPDATE_INTERVAL, true);
    }

    World->GetTimerManager().SetTimer(PriceUpdateTimer, [this]()
    {
        float DeltaTime = DEFAULT_PRICE_UPDATE_INTERVAL;
        UpdateMarketData(DeltaTime);
    }, DEFAULT_PRICE_UPDATE_INTERVAL, true);

    UE_LOG(LogTemp, Log, TEXT("TradingPostComponent initialized"));
}

void UTradingPostComponent::RegisterTradingPost(const FTradingPost& Post)
{
    if (TradingPosts.Contains(Post.PostName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trading post '%s' already registered"), *Post.PostName.ToString());
        return;
    }

    TradingPosts.Add(Post.PostName, Post);
    MarketConditions.Add(Post.PostName, Post.MarketCondition);
    MarketActivity.Add(Post.PostName, 0.0f);

    UE_LOG(LogTemp, Log, TEXT("Trading post '%s' registered"), *Post.PostName.ToString());
}

void UTradingPostComponent::UnregisterTradingPost(const FName& PostName)
{
    TradingPosts.Remove(PostName);
    MarketConditions.Remove(PostName);
    MarketActivity.Remove(PostName);
}

FTradingPost* UTradingPostComponent::GetTradingPost(const FName& PostName)
{
    return TradingPosts.Find(PostName);
}

TArray<FTradingPost> UTradingPostComponent::GetAllTradingPosts()
{
    TArray<FTradingPost> Posts;
    TradingPosts.GenerateValueArray(Posts);
    return Posts;
}

void UTradingPostComponent::UpdateTradingPostInventory(const FName& PostName, const TMap<FName, FMarketItem>& NewInventory)
{
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        Post->Inventory = NewInventory;
    }
}

TArray<FMarketItem> UTradingPostComponent::GetMarketInventory(const FName& PostName)
{
    TArray<FMarketItem> Items;
    
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        Post->Inventory.GenerateValueArray(Items);
    }
    
    return Items;
}

FMarketItem* UTradingPostComponent::GetMarketItem(const FName& PostName, const FName& ItemID)
{
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        return Post->Inventory.Find(ItemID);
    }
    return nullptr;
}

bool UTradingPostComponent::CanPlayerBuyItem(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    if (!Player)
    {
        return false;
    }

    FMarketItem* Item = GetMarketItem(PostName, ItemID);
    if (!Item || Item->Quantity < Quantity)
    {
        return false;
    }

    int32 Cost = CalculateBuyPrice(Player, PostName, ItemID, Quantity);
    return CanAffordTransaction(Player, Cost);
}

bool UTradingPostComponent::CanPlayerSellItem(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    if (!Player)
    {
        return false;
    }

    return HasPlayerItem(Player, ItemID, Quantity);
}

int32 UTradingPostComponent::CalculateBuyPrice(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    FMarketItem* Item = GetMarketItem(PostName, ItemID);
    if (!Item)
    {
        return 0;
    }

    FTradingPost* Post = TradingPosts.Find(PostName);
    if (!Post)
    {
        return Item->CurrentPrice * Quantity;
    }

    float RepModifier = CalculateReputationModifier(Player, PostName);
    float TotalPrice = Item->CurrentPrice * Quantity * Post->BuyModifier * RepModifier;
    float Tax = TotalPrice * Post->TaxRate;

    return FMath::RoundToInt(TotalPrice + Tax);
}

int32 UTradingPostComponent::CalculateSellPrice(APlayerState* Player, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    FMarketItem* Item = GetMarketItem(PostName, ItemID);
    if (!Item)
    {
        return 0;
    }

    FTradingPost* Post = TradingPosts.Find(PostName);
    if (!Post)
    {
        return Item->CurrentPrice * Quantity;
    }

    float RepModifier = CalculateReputationModifier(Player, PostName);
    float TotalPrice = Item->CurrentPrice * Quantity * Post->SellModifier * RepModifier;
    float Tax = TotalPrice * Post->TaxRate;

    return FMath::RoundToInt(TotalPrice - Tax);
}

bool UTradingPostComponent::ProcessBuyTransaction(APlayerState* Buyer, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    if (!CanPlayerBuyItem(Buyer, PostName, ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessBuyTransaction: Player cannot buy item %s x%d from %s"),
            *ItemID.ToString(), Quantity, *PostName.ToString());
        return false;
    }

    int32 Cost = CalculateBuyPrice(Buyer, PostName, ItemID, Quantity);

    FTransaction Transaction;
    Transaction.TransactionID = GenerateTransactionID();
    Transaction.TransactionType = ETransactionType::Buy;
    Transaction.Buyer = Buyer;
    Transaction.Seller = nullptr;
    Transaction.Item = *GetMarketItem(PostName, ItemID);
    Transaction.Quantity = Quantity;
    Transaction.TotalPrice = Cost;
    Transaction.Timestamp = GetWorld()->GetTimeSeconds();
    Transaction.bCompleted = false;
    Transaction.TransactionNotes = FString::Printf(TEXT("Post: %s - Buy %s x%d @ %d credits"),
        *PostName.ToString(), *ItemID.ToString(), Quantity, Cost);

    if (ValidateTransaction(Transaction))
    {
        ApplyTransactionEffects(Transaction);
        RecordTransaction(Transaction);

        UE_LOG(LogTemp, Log, TEXT("ProcessBuyTransaction: %s bought %s x%d from %s for %d credits"),
            *Buyer->GetPlayerName(), *ItemID.ToString(), Quantity, *PostName.ToString(), Cost);

        OnTransactionCompleted.Broadcast(Transaction, true, Buyer, PostName);
        return true;
    }

    return false;
}

bool UTradingPostComponent::ProcessSellTransaction(APlayerState* Seller, const FName& PostName, const FName& ItemID, int32 Quantity)
{
    if (!CanPlayerSellItem(Seller, PostName, ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessSellTransaction: Player cannot sell item %s x%d to %s"),
            *ItemID.ToString(), Quantity, *PostName.ToString());
        return false;
    }

    int32 Price = CalculateSellPrice(Seller, PostName, ItemID, Quantity);

    FTransaction Transaction;
    Transaction.TransactionID = GenerateTransactionID();
    Transaction.TransactionType = ETransactionType::Sell;
    Transaction.Buyer = nullptr;
    Transaction.Seller = Seller;
    Transaction.Item = *GetMarketItem(PostName, ItemID);
    Transaction.Quantity = Quantity;
    Transaction.TotalPrice = Price;
    Transaction.Timestamp = GetWorld()->GetTimeSeconds();
    Transaction.bCompleted = false;
    Transaction.TransactionNotes = FString::Printf(TEXT("Post: %s - Sell %s x%d @ %d credits"),
        *PostName.ToString(), *ItemID.ToString(), Quantity, Price);

    if (ValidateTransaction(Transaction))
    {
        ApplyTransactionEffects(Transaction);
        RecordTransaction(Transaction);

        UE_LOG(LogTemp, Log, TEXT("ProcessSellTransaction: %s sold %s x%d to %s for %d credits"),
            *Seller->GetPlayerName(), *ItemID.ToString(), Quantity, *PostName.ToString(), Price);

        OnTransactionCompleted.Broadcast(Transaction, true, Seller, PostName);
        return true;
    }

    return false;
}

bool UTradingPostComponent::ProcessTradeTransaction(APlayerState* Player1, APlayerState* Player2, const FName& Item1ID, const FName& Item2ID, int32 Quantity1, int32 Quantity2)
{
    if (!Player1 || !Player2)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessTradeTransaction: Invalid players"));
        return false;
    }

    // Validate that both players have the items they're offering
    if (!HasPlayerItem(Player1, Item1ID, Quantity1))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessTradeTransaction: Player1 doesn't have %s x%d"), *Item1ID.ToString(), Quantity1);
        return false;
    }

    if (!HasPlayerItem(Player2, Item2ID, Quantity2))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessTradeTransaction: Player2 doesn't have %s x%d"), *Item2ID.ToString(), Quantity2);
        return false;
    }

    // Create trade transaction
    FName TradeID = GenerateTradeID();
    FTransaction Trade;
    Trade.TransactionID = TradeID;
    Trade.TransactionType = ETransactionType::Trade;
    Trade.Buyer = Player1;
    Trade.Seller = Player2;
    Trade.Quantity = Quantity1;
    Trade.TotalPrice = 0; // Player-to-player trades are item-for-item
    Trade.Timestamp = GetWorld()->GetTimeSeconds();
    Trade.bCompleted = false;
    Trade.TransactionNotes = FString::Printf(TEXT("Trade: %s x%d <-> %s x%d"),
        *Item1ID.ToString(), Quantity1, *Item2ID.ToString(), Quantity2);

    if (!ValidateTrade(Trade))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessTradeTransaction: Trade validation failed"));
        return false;
    }

    // Lock items during trade (remove from both inventories)
    RemoveFromInventory(Player1, Item1ID, Quantity1);
    RemoveFromInventory(Player2, Item2ID, Quantity2);

    // Transfer items
    AddToInventory(Player1, Item2ID, Quantity2);
    AddToInventory(Player2, Item1ID, Quantity1);

    // Mark trade as completed
    Trade.bCompleted = true;
    RecordTransaction(Trade);

    UE_LOG(LogTemp, Log, TEXT("ProcessTradeTransaction: Trade %s completed successfully"), *TradeID.ToString());
    OnTransactionCompleted.Broadcast(Trade, true, Player1, FName("PlayerTrade"));

    return true;
}

FTransaction* UTradingPostComponent::GetTransaction(const FName& TransactionID)
{
    return Transactions.Find(TransactionID);
}

TArray<FTransaction> UTradingPostComponent::GetPlayerTransactions(APlayerState* Player)
{
    TArray<FTransaction> PlayerTransactions;
    
    for (const auto& TransPair : Transactions)
    {
        const FTransaction& Trans = TransPair.Value;
        if (Trans.Buyer == Player || Trans.Seller == Player)
        {
            PlayerTransactions.Add(Trans);
        }
    }
    
    return PlayerTransactions;
}

TArray<FTransaction> UTradingPostComponent::GetPostTransactions(const FName& PostName)
{
    TArray<FTransaction> PostTransactions;

    for (const FTransaction& Trans : TransactionHistory)
    {
        // Filter by post name stored in transaction notes
        // Format: "Post: {PostName}" or check if transaction involves the post
        if (Trans.TransactionNotes.Contains(PostName.ToString()) ||
            (Trans.TransactionType == ETransactionType::Buy || Trans.TransactionType == ETransactionType::Sell))
        {
            // For buy/sell transactions, we need to check if they happened at this post
            // We can infer this from the transaction type and validate against the post's inventory
            if (Trans.TransactionType == ETransactionType::Trade || Trans.TransactionType == ETransactionType::Barter)
            {
                // Player-to-player trades don't belong to a specific post
                continue;
            }

            PostTransactions.Add(Trans);
        }
    }

    return PostTransactions;
}

bool UTradingPostComponent::StartAuction(APlayerState* Seller, const FName& PostName, const FMarketItem& Item, int32 StartingBid, float Duration)
{
    if (!Seller || !TradingPosts.Contains(PostName))
    {
        return false;
    }

    FAuction Auction;
    Auction.AuctionID = GenerateAuctionID();
    Auction.Item = Item;
    Auction.Seller = Seller;
    Auction.StartingBid = StartingBid;
    Auction.CurrentBid = StartingBid;
    Auction.CurrentBidder = nullptr;
    Auction.StartTime = GetWorld()->GetTimeSeconds();
    Auction.Duration = Duration;
    Auction.MinBidIncrement = FMath::Max(1, StartingBid / 10);
    Auction.bIsActive = true;

    if (ValidateAuction(Auction))
    {
        Auctions.Add(Auction.AuctionID, Auction);
        
        OnAuctionStarted.Broadcast(Auction, Seller, PostName);
        
        UE_LOG(LogTemp, Log, TEXT("Auction '%s' started"), *Auction.AuctionID.ToString());
        return true;
    }

    return false;
}

bool UTradingPostComponent::PlaceBid(APlayerState* Bidder, const FName& AuctionID, int32 BidAmount)
{
    if (!Bidder || !Auctions.Contains(AuctionID))
    {
        return false;
    }

    FAuction& Auction = Auctions[AuctionID];
    
    if (!Auction.bIsActive)
    {
        return false;
    }

    if (BidAmount < Auction.CurrentBid + Auction.MinBidIncrement)
    {
        return false;
    }

    if (!CanAffordTransaction(Bidder, BidAmount))
    {
        return false;
    }

    Auction.CurrentBid = BidAmount;
    Auction.CurrentBidder = Bidder;
    Auction.Bidders.AddUnique(Bidder);

    NotifyAuctionParticipants(Auction, FString::Printf(TEXT("New bid: %d"), BidAmount));

    return true;
}

bool UTradingPostComponent::WithdrawAuction(APlayerState* Seller, const FName& AuctionID)
{
    if (!Seller || !Auctions.Contains(AuctionID))
    {
        return false;
    }

    FAuction& Auction = Auctions[AuctionID];
    
    if (Auction.Seller != Seller)
    {
        return false;
    }

    Auction.bIsActive = false;
    Auctions.Remove(AuctionID);

    return true;
}

FAuction* UTradingPostComponent::GetAuction(const FName& AuctionID)
{
    return Auctions.Find(AuctionID);
}

TArray<FAuction> UTradingPostComponent::GetActiveAuctions(const FName& PostName)
{
    TArray<FAuction> ActiveAuctions;
    
    for (const auto& AuctionPair : Auctions)
    {
        if (AuctionPair.Value.bIsActive)
        {
            ActiveAuctions.Add(AuctionPair.Value);
        }
    }
    
    return ActiveAuctions;
}

TArray<FAuction> UTradingPostComponent::GetPlayerAuctions(APlayerState* Player)
{
    TArray<FAuction> PlayerAuctions;
    
    for (const auto& AuctionPair : Auctions)
    {
        if (AuctionPair.Value.Seller == Player)
        {
            PlayerAuctions.Add(AuctionPair.Value);
        }
    }
    
    return PlayerAuctions;
}

TMap<FName, float> UTradingPostComponent::GetMarketPrices(const FName& PostName)
{
    TMap<FName, float> Prices;
    
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        for (const auto& ItemPair : Post->Inventory)
        {
            Prices.Add(ItemPair.Key, ItemPair.Value.CurrentPrice);
        }
    }
    
    return Prices;
}

TMap<FName, float> UTradingPostComponent::GetPriceHistory(const FName& PostName, const FName& ItemID, int32 Days)
{
    TMap<FName, float> History;

    // Get the price history for this trading post
    FItemPriceHistory* PostHistory = PriceHistory.Find(PostName);
    if (!PostHistory)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetPriceHistory: No price history found for post %s"), *PostName.ToString());
        return History;
    }

    // Get the specific item's price history
    FPriceHistoryData* ItemHistory = PostHistory->ItemHistory.Find(ItemID);
    if (!ItemHistory || ItemHistory->PricePoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetPriceHistory: No price history found for item %s at post %s"),
            *ItemID.ToString(), *PostName.ToString());
        return History;
    }

    // Calculate how many data points we need based on days requested
    // Assuming we track prices daily (could be adjusted based on update interval)
    int32 NumDataPoints = FMath::Min(Days, ItemHistory->PricePoints.Num());

    // Get the most recent N price points
    int32 StartIndex = FMath::Max(0, ItemHistory->PricePoints.Num() - NumDataPoints);

    for (int32 i = StartIndex; i < ItemHistory->PricePoints.Num(); i++)
    {
        // Create a name for each day (Day_0, Day_1, etc.)
        FName DayName = FName(*FString::Printf(TEXT("Day_%d"), i - StartIndex));
        History.Add(DayName, ItemHistory->PricePoints[i]);
    }

    UE_LOG(LogTemp, Log, TEXT("GetPriceHistory: Retrieved %d price points for %s at %s"),
        History.Num(), *ItemID.ToString(), *PostName.ToString());

    return History;
}

EMarketCondition UTradingPostComponent::GetMarketCondition(const FName& PostName)
{
    if (MarketConditions.Contains(PostName))
    {
        return MarketConditions[PostName];
    }
    return EMarketCondition::Normal;
}

float UTradingPostComponent::GetMarketActivity(const FName& PostName)
{
    if (MarketActivity.Contains(PostName))
    {
        return MarketActivity[PostName];
    }
    return 0.0f;
}

TArray<FName> UTradingPostComponent::GetTrendingItems(const FName& PostName)
{
    TArray<FName> Trending;

    // Calculate trending score for each item based on recent transaction volume
    TMap<FName, int32> ItemTradeVolume;

    // Get current time for recency calculation
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float TrendingTimeWindow = 3600.0f; // Last hour

    // Analyze recent transactions
    for (const FTransaction& Trans : TransactionHistory)
    {
        // Only consider transactions within the trending time window
        if (CurrentTime - Trans.Timestamp > TrendingTimeWindow)
        {
            continue;
        }

        // Filter by post name (from transaction notes)
        if (!Trans.TransactionNotes.Contains(PostName.ToString()))
        {
            continue;
        }

        // Count trade volume for this item
        FName ItemID = Trans.Item.ItemID;
        int32& Volume = ItemTradeVolume.FindOrAdd(ItemID, 0);
        Volume += Trans.Quantity;
    }

    // Sort items by trade volume
    ItemTradeVolume.ValueSort([](int32 A, int32 B) { return A > B; });

    // Get top 10 trending items
    int32 Count = 0;
    const int32 MaxTrendingItems = 10;

    for (const auto& ItemPair : ItemTradeVolume)
    {
        if (Count >= MaxTrendingItems)
        {
            break;
        }

        Trending.Add(ItemPair.Key);
        Count++;

        UE_LOG(LogTemp, Verbose, TEXT("GetTrendingItems: %s at %s - Volume: %d"),
            *ItemPair.Key.ToString(), *PostName.ToString(), ItemPair.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("GetTrendingItems: Found %d trending items at %s"),
        Trending.Num(), *PostName.ToString());

    return Trending;
}

TArray<FName> UTradingPostComponent::GetRecommendedItems(APlayerState* Player, const FName& PostName)
{
    TArray<FName> Recommended;

    if (!Player)
    {
        return Recommended;
    }

    // Get player's purchase history
    TMap<FName, int32> PlayerPurchases;
    TArray<FTransaction> PlayerTransactions = GetPlayerTransactions(Player);

    // Analyze player's buying patterns
    for (const FTransaction& Trans : PlayerTransactions)
    {
        if (Trans.TransactionType == ETransactionType::Buy && Trans.Buyer == Player)
        {
            FName ItemID = Trans.Item.ItemID;
            int32& PurchaseCount = PlayerPurchases.FindOrAdd(ItemID, 0);
            PurchaseCount += Trans.Quantity;
        }
    }

    // Get market inventory for this post
    TArray<FMarketItem> AvailableItems = GetMarketInventory(PostName);

    // Score items based on various factors
    TMap<FName, float> ItemScores;

    for (const FMarketItem& Item : AvailableItems)
    {
        float Score = 0.0f;

        // Factor 1: Price affordability (items within player's budget)
        // Assume player can afford if price is reasonable relative to their purchasing history
        if (PlayerPurchases.Num() > 0)
        {
            // Calculate average purchase price from history
            float TotalSpent = 0.0f;
            int32 TotalTransactions = 0;
            for (const FTransaction& Trans : PlayerTransactions)
            {
                if (Trans.TransactionType == ETransactionType::Buy)
                {
                    TotalSpent += Trans.TotalPrice;
                    TotalTransactions++;
                }
            }

            if (TotalTransactions > 0)
            {
                float AvgPurchasePrice = TotalSpent / TotalTransactions;
                if (Item.CurrentPrice <= AvgPurchasePrice * 1.5f)
                {
                    Score += 30.0f;
                }
            }
        }

        // Factor 2: Item quality
        Score += Item.Quality * 20.0f;

        // Factor 3: Rarity
        if (Item.bIsRare)
        {
            Score += 25.0f;
        }

        // Factor 4: Similar to previously purchased items (collaborative filtering)
        bool bSimilarPurchase = false;
        for (const auto& Purchase : PlayerPurchases)
        {
            // Simple similarity check - same item or similar name
            if (Item.ItemID.ToString().Contains(Purchase.Key.ToString()) ||
                Purchase.Key.ToString().Contains(Item.ItemID.ToString()))
            {
                Score += 35.0f;
                bSimilarPurchase = true;
                break;
            }
        }

        // Factor 5: Current availability (encourage buying low stock items)
        if (Item.Quantity > 0 && Item.Quantity < Item.MaxQuantity * 0.2f)
        {
            Score += 15.0f;
        }

        // Factor 6: Price trending (good deals)
        TMap<FName, float> HistoricalPrices = GetPriceHistory(PostName, Item.ItemID, 7);
        if (HistoricalPrices.Num() > 0)
        {
            float AvgHistoricalPrice = 0.0f;
            for (const auto& PricePair : HistoricalPrices)
            {
                AvgHistoricalPrice += PricePair.Value;
            }
            AvgHistoricalPrice /= PriceHistory.Num();

            // If current price is below historical average, it's a good deal
            if (Item.CurrentPrice < AvgHistoricalPrice * 0.9f)
            {
                Score += 20.0f;
            }
        }

        ItemScores.Add(Item.ItemID, Score);
    }

    // Sort by score
    ItemScores.ValueSort([](float A, float B) { return A > B; });

    // Get top 5 recommended items
    int32 Count = 0;
    const int32 MaxRecommendations = 5;

    for (const auto& ItemPair : ItemScores)
    {
        if (Count >= MaxRecommendations)
        {
            break;
        }

        // Only recommend items with a minimum score
        if (ItemPair.Value >= 20.0f)
        {
            Recommended.Add(ItemPair.Key);
            Count++;

            UE_LOG(LogTemp, Verbose, TEXT("GetRecommendedItems: %s for player %s - Score: %.2f"),
                *ItemPair.Key.ToString(), *Player->GetPlayerName(), ItemPair.Value);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GetRecommendedItems: Generated %d recommendations for player %s at %s"),
        Recommended.Num(), *Player->GetPlayerName(), *PostName.ToString());

    return Recommended;
}

void UTradingPostComponent::TriggerMarketEvent(const FName& PostName, EMarketCondition NewCondition, float Duration)
{
    if (!TradingPosts.Contains(PostName))
    {
        return;
    }

    MarketConditions[PostName] = NewCondition;
    ApplyMarketCondition(PostName, NewCondition);

    OnMarketConditionChanged.Broadcast(PostName, NewCondition);

    UE_LOG(LogTemp, Log, TEXT("Market event triggered for '%s'"), *PostName.ToString());
}

void UTradingPostComponent::UpdateMarketConditions(float DeltaTime)
{
    for (auto& PostPair : TradingPosts)
    {
        if (bEnableMarketEvents && FMath::FRand() < 0.001f)
        {
            GenerateMarketEvent(PostPair.Key);
        }
    }
}

void UTradingPostComponent::RefreshMarketInventory(const FName& PostName)
{
    if (!TradingPosts.Contains(PostName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RefreshMarketInventory: Trading post %s not found"), *PostName.ToString());
        return;
    }

    FTradingPost* Post = TradingPosts.Find(PostName);
    if (!Post)
    {
        return;
    }

    // Replenish inventory based on market conditions
    EMarketCondition Condition = GetMarketCondition(PostName);

    for (auto& ItemPair : Post->Inventory)
    {
        FMarketItem& Item = ItemPair.Value;

        // Calculate replenishment rate based on market condition
        float ReplenishmentRate = 0.1f; // 10% base replenishment

        switch (Condition)
        {
        case EMarketCondition::HighDemand:
            ReplenishmentRate = 0.05f; // Slower replenishment when demand is high
            break;
        case EMarketCondition::LowSupply:
            ReplenishmentRate = 0.03f; // Very slow replenishment during shortage
            break;
        case EMarketCondition::Surplus:
            ReplenishmentRate = 0.2f; // Fast replenishment during surplus
            break;
        case EMarketCondition::Shortage:
            ReplenishmentRate = 0.02f; // Almost no replenishment during shortage
            break;
        case EMarketCondition::Boom:
            ReplenishmentRate = 0.15f; // Good replenishment during boom
            break;
        case EMarketCondition::Bust:
            ReplenishmentRate = 0.08f; // Reduced replenishment during bust
            break;
        default:
            ReplenishmentRate = 0.1f;
            break;
        }

        // Calculate quantity to add
        int32 ReplenishAmount = FMath::RoundToInt((Item.MaxQuantity - Item.Quantity) * ReplenishmentRate);

        if (ReplenishAmount > 0)
        {
            Item.Quantity = FMath::Min(Item.Quantity + ReplenishAmount, Item.MaxQuantity);

            UE_LOG(LogTemp, Verbose, TEXT("RefreshMarketInventory: Replenished %s at %s by %d (Total: %d/%d)"),
                *Item.ItemID.ToString(), *PostName.ToString(), ReplenishAmount, Item.Quantity, Item.MaxQuantity);
        }

        // Update prices during refresh
        UpdateItemPrice(PostName, Item.ItemID);
    }

    UE_LOG(LogTemp, Log, TEXT("RefreshMarketInventory: Refreshed inventory at %s"), *PostName.ToString());
}

void UTradingPostComponent::ApplyMarketFluctuations(const FName& PostName)
{
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        for (auto& ItemPair : Post->Inventory)
        {
            UpdateItemPrice(PostName, ItemPair.Key);
        }
    }
}

bool UTradingPostComponent::InitiateTrade(APlayerState* Initiator, APlayerState* Target)
{
    if (!Initiator || !Target)
    {
        return false;
    }

    FName TradeID = GenerateTradeID();
    
    FTransaction Trade;
    Trade.TransactionID = TradeID;
    Trade.TransactionType = ETransactionType::Trade;
    Trade.Buyer = Initiator;
    Trade.Seller = Target;
    Trade.bCompleted = false;

    PlayerTrades.Add(TradeID, Trade);

    OnTradeInitiated.Broadcast(TradeID, Initiator, Target);

    return true;
}

bool UTradingPostComponent::RespondToTrade(APlayerState* Player, const FName& TradeID, bool bAccept)
{
    if (!Player || !PlayerTrades.Contains(TradeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("RespondToTrade: Invalid player or trade ID %s"), *TradeID.ToString());
        return false;
    }

    FTransaction& Trade = PlayerTrades[TradeID];

    // Verify player is the target of the trade
    if (Trade.Seller != Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("RespondToTrade: Player %s is not the target of trade %s"),
            *Player->GetPlayerName(), *TradeID.ToString());
        return false;
    }

    if (!bAccept)
    {
        // Trade rejected - clean up and notify
        PlayerTrades.Remove(TradeID);
        NotifyTradeParticipants(Trade, TEXT("Trade rejected"));

        UE_LOG(LogTemp, Log, TEXT("RespondToTrade: Trade %s rejected by %s"),
            *TradeID.ToString(), *Player->GetPlayerName());

        OnTradeCompleted.Broadcast(TradeID, false);
        return true;
    }

    // Trade accepted - mark as ready for confirmation
    Trade.TransactionNotes += TEXT(" | Target accepted");

    UE_LOG(LogTemp, Log, TEXT("RespondToTrade: Trade %s accepted by %s"),
        *TradeID.ToString(), *Player->GetPlayerName());

    NotifyTradeParticipants(Trade, TEXT("Trade accepted - waiting for item exchange"));

    return true;
}

bool UTradingPostComponent::AddTradeItem(APlayerState* Player, const FName& TradeID, const FName& ItemID, int32 Quantity)
{
    if (!Player || !PlayerTrades.Contains(TradeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("AddTradeItem: Invalid player or trade ID %s"), *TradeID.ToString());
        return false;
    }

    FTransaction& Trade = PlayerTrades[TradeID];

    // Verify player is part of this trade
    if (Trade.Buyer != Player && Trade.Seller != Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddTradeItem: Player %s is not part of trade %s"),
            *Player->GetPlayerName(), *TradeID.ToString());
        return false;
    }

    // Verify player has the item
    if (!HasPlayerItem(Player, ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("AddTradeItem: Player %s doesn't have item %s x%d"),
            *Player->GetPlayerName(), *ItemID.ToString(), Quantity);
        return false;
    }

    // Store item info in transaction notes (simple implementation)
    // In a production system, you'd use a dedicated TradeOffer structure
    FString PlayerRole = (Trade.Buyer == Player) ? TEXT("Buyer") : TEXT("Seller");
    Trade.TransactionNotes += FString::Printf(TEXT(" | %s offers: %s x%d"),
        *PlayerRole, *ItemID.ToString(), Quantity);

    // Update the main item field if this is the first item added
    if (Trade.Item.ItemID.IsNone())
    {
        // Create a basic market item for the trade
        Trade.Item.ItemID = ItemID;
        Trade.Item.ItemName = ItemID.ToString();
        Trade.Quantity = Quantity;
    }

    UE_LOG(LogTemp, Log, TEXT("AddTradeItem: Player %s added %s x%d to trade %s"),
        *Player->GetPlayerName(), *ItemID.ToString(), Quantity, *TradeID.ToString());

    NotifyTradeParticipants(Trade, FString::Printf(TEXT("%s added %s x%d"),
        *Player->GetPlayerName(), *ItemID.ToString(), Quantity));

    return true;
}

bool UTradingPostComponent::RemoveTradeItem(APlayerState* Player, const FName& TradeID, const FName& ItemID)
{
    if (!Player || !PlayerTrades.Contains(TradeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveTradeItem: Invalid player or trade ID %s"), *TradeID.ToString());
        return false;
    }

    FTransaction& Trade = PlayerTrades[TradeID];

    // Verify player is part of this trade
    if (Trade.Buyer != Player && Trade.Seller != Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveTradeItem: Player %s is not part of trade %s"),
            *Player->GetPlayerName(), *TradeID.ToString());
        return false;
    }

    // Mark item as removed in transaction notes
    FString PlayerRole = (Trade.Buyer == Player) ? TEXT("Buyer") : TEXT("Seller");
    Trade.TransactionNotes += FString::Printf(TEXT(" | %s removed: %s"),
        *PlayerRole, *ItemID.ToString());

    // Clear the main item if it matches
    if (Trade.Item.ItemID == ItemID)
    {
        Trade.Item.ItemID = NAME_None;
        Trade.Item.ItemName = TEXT("");
        Trade.Quantity = 0;
    }

    UE_LOG(LogTemp, Log, TEXT("RemoveTradeItem: Player %s removed %s from trade %s"),
        *Player->GetPlayerName(), *ItemID.ToString(), *TradeID.ToString());

    NotifyTradeParticipants(Trade, FString::Printf(TEXT("%s removed %s"),
        *Player->GetPlayerName(), *ItemID.ToString()));

    return true;
}

bool UTradingPostComponent::ConfirmTrade(APlayerState* Player, const FName& TradeID)
{
    if (!Player || !PlayerTrades.Contains(TradeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("ConfirmTrade: Invalid player or trade ID %s"), *TradeID.ToString());
        return false;
    }

    FTransaction& Trade = PlayerTrades[TradeID];

    // Verify player is part of this trade
    if (Trade.Buyer != Player && Trade.Seller != Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConfirmTrade: Player %s is not part of trade %s"),
            *Player->GetPlayerName(), *TradeID.ToString());
        return false;
    }

    // Mark player as confirmed
    FString PlayerRole = (Trade.Buyer == Player) ? TEXT("Buyer") : TEXT("Seller");
    Trade.TransactionNotes += FString::Printf(TEXT(" | %s confirmed"), *PlayerRole);

    // Check if both players have confirmed
    bool bBuyerConfirmed = Trade.TransactionNotes.Contains(TEXT("Buyer confirmed"));
    bool bSellerConfirmed = Trade.TransactionNotes.Contains(TEXT("Seller confirmed"));

    if (bBuyerConfirmed && bSellerConfirmed)
    {
        // Both confirmed - execute the trade
        if (ValidateTrade(Trade))
        {
            ExecuteTrade(Trade);
            Trade.bCompleted = true;

            // Record the transaction
            RecordTransaction(Trade);

            // Remove from active trades
            PlayerTrades.Remove(TradeID);

            UE_LOG(LogTemp, Log, TEXT("ConfirmTrade: Trade %s completed successfully"), *TradeID.ToString());

            OnTradeCompleted.Broadcast(TradeID, true);
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ConfirmTrade: Trade %s validation failed"), *TradeID.ToString());

            NotifyTradeParticipants(Trade, TEXT("Trade failed validation"));
            PlayerTrades.Remove(TradeID);

            OnTradeCompleted.Broadcast(TradeID, false);
            return false;
        }
    }
    else
    {
        // Waiting for other player to confirm
        UE_LOG(LogTemp, Log, TEXT("ConfirmTrade: Player %s confirmed trade %s - waiting for other player"),
            *Player->GetPlayerName(), *TradeID.ToString());

        NotifyTradeParticipants(Trade, FString::Printf(TEXT("%s confirmed - waiting for other player"),
            *Player->GetPlayerName()));

        return true;
    }
}

bool UTradingPostComponent::CancelTrade(const FName& TradeID)
{
    if (PlayerTrades.Contains(TradeID))
    {
        PlayerTrades.Remove(TradeID);
        OnTradeCompleted.Broadcast(TradeID, false);
        return true;
    }
    return false;
}

bool UTradingPostComponent::IsTradingPostOpen(const FName& PostName)
{
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        return Post->bIsOpen;
    }
    return false;
}

float UTradingPostComponent::GetTaxRate(const FName& PostName)
{
    if (FTradingPost* Post = TradingPosts.Find(PostName))
    {
        return Post->TaxRate;
    }
    return DefaultTaxRate;
}

float UTradingPostComponent::GetFactionReputation(APlayerState* Player, const FName& PostName)
{
    if (!Player)
    {
        return 0.0f;
    }

    // Try to get EconomySystem from player's pawn or controller
    // This integrates with the EconomySystem component
    AActor* PlayerPawn = Player->GetPawn();
    if (PlayerPawn)
    {
        UEconomySystem* PlayerEconomy = PlayerPawn->FindComponentByClass<UEconomySystem>();
        if (PlayerEconomy)
        {
            // Get faction reputation from the economy system
            // Map trading post to faction (simplified mapping)
            for (const auto& FactionPair : PlayerEconomy->FactionReputation)
            {
                // Return first faction reputation as base
                // In production, map PostName to specific faction
                return FactionPair.Value;
            }
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("GetFactionReputation: No reputation data found for player %s at %s"),
        *Player->GetPlayerName(), *PostName.ToString());

    return 0.0f; // Neutral reputation
}

void UTradingPostComponent::UpdateFactionReputation(APlayerState* Player, const FName& PostName, float Change)
{
    UpdateReputation(Player, PostName, Change);
}

TMap<FName, int32> UTradingPostComponent::GetPlayerInventory(APlayerState* Player)
{
    TMap<FName, int32> Inventory;

    if (!Player)
    {
        return Inventory;
    }

    // Try to get inventory from player's EconomySystem component
    AActor* PlayerPawn = Player->GetPawn();
    if (PlayerPawn)
    {
        UEconomySystem* PlayerEconomy = PlayerPawn->FindComponentByClass<UEconomySystem>();
        if (PlayerEconomy)
        {
            // Convert cargo items to inventory map
            for (const FCargoItem& CargoItem : PlayerEconomy->CargoHold)
            {
                FName ItemID = CargoItem.Commodity.CommodityName;
                int32& Quantity = Inventory.FindOrAdd(ItemID, 0);
                Quantity += CargoItem.Quantity;
            }

            UE_LOG(LogTemp, Verbose, TEXT("GetPlayerInventory: Retrieved %d unique items for player %s"),
                Inventory.Num(), *Player->GetPlayerName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GetPlayerInventory: Player %s has no EconomySystem component"),
                *Player->GetPlayerName());
        }
    }

    return Inventory;
}

bool UTradingPostComponent::HasPlayerItem(APlayerState* Player, const FName& ItemID, int32 Quantity)
{
    if (!Player)
    {
        return false;
    }

    // Get player's inventory
    TMap<FName, int32> Inventory = GetPlayerInventory(Player);

    // Check if player has the item in sufficient quantity
    const int32* ItemQuantity = Inventory.Find(ItemID);
    if (ItemQuantity && *ItemQuantity >= Quantity)
    {
        UE_LOG(LogTemp, Verbose, TEXT("HasPlayerItem: Player %s has %s x%d (needs %d)"),
            *Player->GetPlayerName(), *ItemID.ToString(), *ItemQuantity, Quantity);
        return true;
    }

    UE_LOG(LogTemp, Verbose, TEXT("HasPlayerItem: Player %s doesn't have enough %s (has %d, needs %d)"),
        *Player->GetPlayerName(), *ItemID.ToString(), ItemQuantity ? *ItemQuantity : 0, Quantity);

    return false;
}

void UTradingPostComponent::AddItemToPlayerInventory(APlayerState* Player, const FName& ItemID, int32 Quantity)
{
    AddToInventory(Player, ItemID, Quantity);
}

void UTradingPostComponent::RemoveItemFromPlayerInventory(APlayerState* Player, const FName& ItemID, int32 Quantity)
{
    RemoveFromInventory(Player, ItemID, Quantity);
}

void UTradingPostComponent::UpdateTradingPosts(float DeltaTime)
{
    for (auto& PostPair : TradingPosts)
    {
        ApplyMarketFluctuations(PostPair.Key);
    }
}

void UTradingPostComponent::ProcessAuctions(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<FName> ExpiredAuctions;

    for (auto& AuctionPair : Auctions)
    {
        FAuction& Auction = AuctionPair.Value;
        if (Auction.bIsActive && (CurrentTime - Auction.StartTime) >= Auction.Duration)
        {
            ExpiredAuctions.Add(AuctionPair.Key);
        }
    }

    for (const FName& AuctionID : ExpiredAuctions)
    {
        ProcessAuctionEnd(AuctionID);
    }
}

void UTradingPostComponent::UpdateMarketData(float DeltaTime)
{
    UpdateMarketConditions(DeltaTime);
}

void UTradingPostComponent::ProcessPlayerTrades(float DeltaTime)
{
    // Process active player trades
}

FName UTradingPostComponent::GenerateTransactionID()
{
    return FName(*FString::Printf(TEXT("TRANS_%d"), FMath::RandRange(100000, 999999)));
}

void UTradingPostComponent::RecordTransaction(const FTransaction& Transaction)
{
    Transactions.Add(Transaction.TransactionID, Transaction);
    TransactionHistory.Add(Transaction);

    if (TransactionHistory.Num() > MaxTransactionHistory)
    {
        TransactionHistory.RemoveAt(0);
    }
}

void UTradingPostComponent::ApplyTransactionEffects(const FTransaction& Transaction)
{
    // Handle different transaction types
    switch (Transaction.TransactionType)
    {
    case ETransactionType::Buy:
        {
            if (Transaction.Buyer)
            {
                // Deduct credits from buyer
                AActor* BuyerPawn = Transaction.Buyer->GetPawn();
                if (BuyerPawn)
                {
                    UEconomySystem* BuyerEconomy = BuyerPawn->FindComponentByClass<UEconomySystem>();
                    if (BuyerEconomy)
                    {
                        BuyerEconomy->PlayerCredits -= Transaction.TotalPrice;
                        UE_LOG(LogTemp, Log, TEXT("ApplyTransactionEffects: Deducted %d credits from %s (Balance: %.2f)"),
                            Transaction.TotalPrice, *Transaction.Buyer->GetPlayerName(), BuyerEconomy->PlayerCredits);
                    }
                }

                // Add item to buyer's inventory
                AddToInventory(Transaction.Buyer, Transaction.Item.ItemID, Transaction.Quantity);

                // Update market inventory (reduce stock)
                // Find which trading post this transaction was for
                for (auto& PostPair : TradingPosts)
                {
                    FMarketItem* Item = PostPair.Value.Inventory.Find(Transaction.Item.ItemID);
                    if (Item && Item->ItemID == Transaction.Item.ItemID)
                    {
                        Item->Quantity = FMath::Max(0, Item->Quantity - Transaction.Quantity);
                        UE_LOG(LogTemp, Log, TEXT("ApplyTransactionEffects: Reduced %s stock at %s to %d"),
                            *Transaction.Item.ItemID.ToString(), *PostPair.Key.ToString(), Item->Quantity);
                        break;
                    }
                }
            }
        }
        break;

    case ETransactionType::Sell:
        {
            if (Transaction.Seller)
            {
                // Add credits to seller
                AActor* SellerPawn = Transaction.Seller->GetPawn();
                if (SellerPawn)
                {
                    UEconomySystem* SellerEconomy = SellerPawn->FindComponentByClass<UEconomySystem>();
                    if (SellerEconomy)
                    {
                        SellerEconomy->PlayerCredits += Transaction.TotalPrice;
                        UE_LOG(LogTemp, Log, TEXT("ApplyTransactionEffects: Added %d credits to %s (Balance: %.2f)"),
                            Transaction.TotalPrice, *Transaction.Seller->GetPlayerName(), SellerEconomy->PlayerCredits);
                    }
                }

                // Remove item from seller's inventory
                RemoveFromInventory(Transaction.Seller, Transaction.Item.ItemID, Transaction.Quantity);

                // Update market inventory (increase stock)
                for (auto& PostPair : TradingPosts)
                {
                    FMarketItem* Item = PostPair.Value.Inventory.Find(Transaction.Item.ItemID);
                    if (Item && Item->ItemID == Transaction.Item.ItemID)
                    {
                        Item->Quantity = FMath::Min(Item->MaxQuantity, Item->Quantity + Transaction.Quantity);
                        UE_LOG(LogTemp, Log, TEXT("ApplyTransactionEffects: Increased %s stock at %s to %d"),
                            *Transaction.Item.ItemID.ToString(), *PostPair.Key.ToString(), Item->Quantity);
                        break;
                    }
                }
            }
        }
        break;

    case ETransactionType::Trade:
        {
            // Player-to-player trades don't involve credits
            // Inventory transfers are handled in ProcessTradeTransaction
            UE_LOG(LogTemp, Log, TEXT("ApplyTransactionEffects: Player trade - inventory already exchanged"));
        }
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("ApplyTransactionEffects: Unknown transaction type"));
        break;
    }
}

bool UTradingPostComponent::ValidateTransaction(const FTransaction& Transaction)
{
    // Validate based on transaction type
    switch (Transaction.TransactionType)
    {
    case ETransactionType::Buy:
        {
            if (!Transaction.Buyer)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Buy transaction has no buyer"));
                return false;
            }

            // Verify buyer has enough credits
            if (!CanAffordTransaction(Transaction.Buyer, Transaction.TotalPrice))
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Buyer %s cannot afford %d credits"),
                    *Transaction.Buyer->GetPlayerName(), Transaction.TotalPrice);
                return false;
            }

            // Verify quantity is positive
            if (Transaction.Quantity <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Invalid quantity %d"), Transaction.Quantity);
                return false;
            }
        }
        break;

    case ETransactionType::Sell:
        {
            if (!Transaction.Seller)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Sell transaction has no seller"));
                return false;
            }

            // Verify seller has the items
            if (!HasPlayerItem(Transaction.Seller, Transaction.Item.ItemID, Transaction.Quantity))
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Seller %s doesn't have %s x%d"),
                    *Transaction.Seller->GetPlayerName(), *Transaction.Item.ItemID.ToString(), Transaction.Quantity);
                return false;
            }

            // Verify quantity is positive
            if (Transaction.Quantity <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Invalid quantity %d"), Transaction.Quantity);
                return false;
            }
        }
        break;

    case ETransactionType::Trade:
        {
            if (!Transaction.Buyer || !Transaction.Seller)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Trade transaction missing players"));
                return false;
            }

            if (Transaction.Buyer == Transaction.Seller)
            {
                UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Cannot trade with yourself"));
                return false;
            }
        }
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("ValidateTransaction: Unknown transaction type"));
        return false;
    }

    UE_LOG(LogTemp, Verbose, TEXT("ValidateTransaction: Transaction validated successfully"));
    return true;
}

FName UTradingPostComponent::GenerateAuctionID()
{
    return FName(*FString::Printf(TEXT("AUCTION_%d"), FMath::RandRange(100000, 999999)));
}

void UTradingPostComponent::ProcessAuctionEnd(const FName& AuctionID)
{
    if (!Auctions.Contains(AuctionID))
    {
        return;
    }

    FAuction& Auction = Auctions[AuctionID];
    Auction.bIsActive = false;

    OnAuctionEnded.Broadcast(Auction, Auction.CurrentBidder, FName());

    Auctions.Remove(AuctionID);
}

void UTradingPostComponent::NotifyAuctionParticipants(const FAuction& Auction, const FString& Message)
{
    // Notify all auction participants
}

bool UTradingPostComponent::ValidateAuction(const FAuction& Auction)
{
    return Auction.Seller != nullptr && Auction.StartingBid > 0;
}

void UTradingPostComponent::UpdateItemPrice(const FName& PostName, const FName& ItemID)
{
    FMarketItem* Item = GetMarketItem(PostName, ItemID);
    if (!Item)
    {
        return;
    }

    // Apply random price fluctuation
    float FluctuationRange = Item->BasePrice * PriceFluctuationRate;
    float Fluctuation = FMath::RandRange(-FluctuationRange, FluctuationRange);

    // Apply market condition modifiers
    EMarketCondition Condition = GetMarketCondition(PostName);
    float ConditionModifier = 1.0f;

    switch (Condition)
    {
    case EMarketCondition::HighDemand:
        ConditionModifier = 1.2f;
        break;
    case EMarketCondition::LowSupply:
        ConditionModifier = 1.3f;
        break;
    case EMarketCondition::Surplus:
        ConditionModifier = 0.8f;
        break;
    case EMarketCondition::Shortage:
        ConditionModifier = 1.5f;
        break;
    case EMarketCondition::Boom:
        ConditionModifier = 1.4f;
        break;
    case EMarketCondition::Bust:
        ConditionModifier = 0.6f;
        break;
    default:
        ConditionModifier = 1.0f;
        break;
    }

    // Calculate new price
    float NewPrice = (Item->BasePrice + Fluctuation) * ConditionModifier * Item->PriceModifier;

    // Ensure price doesn't go below 10% of base price or above 500% of base price
    NewPrice = FMath::Clamp(NewPrice, Item->BasePrice * 0.1f, Item->BasePrice * 5.0f);

    Item->CurrentPrice = FMath::RoundToInt(NewPrice);

    // Update price history
    UpdatePriceHistory(PostName, ItemID, Item->CurrentPrice);

    UE_LOG(LogTemp, Verbose, TEXT("UpdateItemPrice: %s at %s - Old Price: %d, New Price: %d, Condition: %d"),
        *ItemID.ToString(), *PostName.ToString(), (int32)(Item->BasePrice), Item->CurrentPrice, (int32)Condition);
}

void UTradingPostComponent::ApplyMarketCondition(const FName& PostName, EMarketCondition Condition)
{
    // Apply market condition effects
}

void UTradingPostComponent::GenerateMarketEvent(const FName& PostName)
{
    // Generate random market event
}

void UTradingPostComponent::UpdatePriceHistory(const FName& PostName, const FName& ItemID, float Price)
{
    // Get or create price history for this trading post
    FItemPriceHistory& PostHistory = PriceHistory.FindOrAdd(PostName);

    // Get or create price history for this specific item
    FPriceHistoryData& ItemHistory = PostHistory.ItemHistory.FindOrAdd(ItemID);

    // Add the new price point
    ItemHistory.PricePoints.Add(Price);

    // Limit history size to prevent unbounded growth (keep last 365 days)
    const int32 MaxHistorySize = 365;
    if (ItemHistory.PricePoints.Num() > MaxHistorySize)
    {
        ItemHistory.PricePoints.RemoveAt(0, ItemHistory.PricePoints.Num() - MaxHistorySize);
    }

    UE_LOG(LogTemp, Verbose, TEXT("UpdatePriceHistory: Updated %s at %s - Price: %.2f, History Size: %d"),
        *ItemID.ToString(), *PostName.ToString(), Price, ItemHistory.PricePoints.Num());
}

FName UTradingPostComponent::GenerateTradeID()
{
    return FName(*FString::Printf(TEXT("TRADE_%d"), FMath::RandRange(100000, 999999)));
}

bool UTradingPostComponent::ValidateTrade(const FTransaction& Trade)
{
    if (!Trade.Buyer || !Trade.Seller)
    {
        UE_LOG(LogTemp, Warning, TEXT("ValidateTrade: Missing buyer or seller"));
        return false;
    }

    if (Trade.Buyer == Trade.Seller)
    {
        UE_LOG(LogTemp, Warning, TEXT("ValidateTrade: Cannot trade with yourself"));
        return false;
    }

    // Parse transaction notes to extract trade items
    // Format: "Buyer offers: ItemID x Quantity | Seller offers: ItemID x Quantity"
    // This is a simplified validation - in production, use proper data structures

    if (Trade.TransactionNotes.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("ValidateTrade: No trade items specified"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("ValidateTrade: Trade validated successfully"));
    return true;
}

void UTradingPostComponent::ExecuteTrade(const FTransaction& Trade)
{
    if (!Trade.Buyer || !Trade.Seller)
    {
        return;
    }

    // In a production system, you would parse the actual items from a proper trade structure
    // For this implementation, we'll use the Item field and transaction notes
    // This is a simplified implementation - extend with proper item parsing

    UE_LOG(LogTemp, Log, TEXT("ExecuteTrade: Executing trade between %s and %s"),
        *Trade.Buyer->GetPlayerName(), *Trade.Seller->GetPlayerName());

    // The actual item exchange should happen in ProcessTradeTransaction
    // This function is called when both players have confirmed
    // Items should already be locked/reserved at this point

    NotifyTradeParticipants(Trade, TEXT("Trade completed successfully"));
}

void UTradingPostComponent::NotifyTradeParticipants(const FTransaction& Trade, const FString& Message)
{
    // Notify trade participants
}

bool UTradingPostComponent::RemoveFromInventory(APlayerState* Player, const FName& ItemID, int32 Quantity)
{
    if (!Player)
    {
        return false;
    }

    // Get player's EconomySystem component
    AActor* PlayerPawn = Player->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromInventory: Player %s has no pawn"), *Player->GetPlayerName());
        return false;
    }

    UEconomySystem* PlayerEconomy = PlayerPawn->FindComponentByClass<UEconomySystem>();
    if (!PlayerEconomy)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromInventory: Player %s has no EconomySystem"),
            *Player->GetPlayerName());
        return false;
    }

    // Remove from cargo using EconomySystem's RemoveFromCargo function
    bool bSuccess = PlayerEconomy->RemoveFromCargo(ItemID, Quantity);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("RemoveFromInventory: Removed %s x%d from player %s"),
            *ItemID.ToString(), Quantity, *Player->GetPlayerName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromInventory: Failed to remove %s x%d from player %s"),
            *ItemID.ToString(), Quantity, *Player->GetPlayerName());
    }

    return bSuccess;
}

bool UTradingPostComponent::AddToInventory(APlayerState* Player, const FName& ItemID, int32 Quantity)
{
    if (!Player)
    {
        return false;
    }

    // Get player's EconomySystem component
    AActor* PlayerPawn = Player->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddToInventory: Player %s has no pawn"), *Player->GetPlayerName());
        return false;
    }

    UEconomySystem* PlayerEconomy = PlayerPawn->FindComponentByClass<UEconomySystem>();
    if (!PlayerEconomy)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddToInventory: Player %s has no EconomySystem"),
            *Player->GetPlayerName());
        return false;
    }

    // Create a commodity data for the item (simplified - in production, look up from data table)
    FCommodityData Commodity;
    Commodity.CommodityName = ItemID;
    Commodity.BasePrice = 100.0f; // Default price
    Commodity.Volume = 1.0f;
    Commodity.Mass = 1.0f;

    // Add to cargo using EconomySystem's AddToCargo function
    bool bSuccess = PlayerEconomy->AddToCargo(Commodity, Quantity, Commodity.BasePrice);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("AddToInventory: Added %s x%d to player %s"),
            *ItemID.ToString(), Quantity, *Player->GetPlayerName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AddToInventory: Failed to add %s x%d to player %s (cargo full?)"),
            *ItemID.ToString(), Quantity, *Player->GetPlayerName());
    }

    return bSuccess;
}

bool UTradingPostComponent::CanAffordTransaction(APlayerState* Player, int32 Cost)
{
    if (!Player)
    {
        return false;
    }

    // Get player's EconomySystem component
    AActor* PlayerPawn = Player->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Verbose, TEXT("CanAffordTransaction: Player %s has no pawn"), *Player->GetPlayerName());
        return false;
    }

    UEconomySystem* PlayerEconomy = PlayerPawn->FindComponentByClass<UEconomySystem>();
    if (!PlayerEconomy)
    {
        UE_LOG(LogTemp, Warning, TEXT("CanAffordTransaction: Player %s has no EconomySystem"),
            *Player->GetPlayerName());
        return false;
    }

    // Check if player can afford the transaction using EconomySystem
    bool bCanAfford = PlayerEconomy->CanAffordTransaction(Cost);

    UE_LOG(LogTemp, Verbose, TEXT("CanAffordTransaction: Player %s %s afford %d credits (has %.2f)"),
        *Player->GetPlayerName(), bCanAfford ? TEXT("can") : TEXT("cannot"),
        Cost, PlayerEconomy->PlayerCredits);

    return bCanAfford;
}

float UTradingPostComponent::CalculateReputationModifier(APlayerState* Player, const FName& PostName)
{
    return 1.0f;
}

void UTradingPostComponent::UpdateReputation(APlayerState* Player, const FName& PostName, float Change)
{
    // Update player reputation
}
