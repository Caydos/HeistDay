//#include "Network/Client/EOSMatchmakingSubsystem.h"
//#include "OnlineSubsystem.h"
//#include "OnlineSessionSettings.h"
//
//bool UEOSMatchmakingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
//{
//	return !IsRunningDedicatedServer();
//}
//
//void UEOSMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	if (Subsystem)
//	{
//		SessionInterface = Subsystem->GetSessionInterface();
//	}
//}
//
//void UEOSMatchmakingSubsystem::FindSessions()
//{
//	if (!SessionInterface.IsValid()) return;
//
//	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
//	LastSessionSearch->MaxSearchResults = 20;
//	LastSessionSearch->bIsLanQuery = false;
//	LastSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
//	LastSessionSearch->QuerySettings.Set(FName(TEXT("PRESENCE")), false, EOnlineComparisonOp::Equals);
//	LastSessionSearch->QuerySettings.Set(SETTING_MAPNAME, FString("HeistMap"), EOnlineComparisonOp::Equals);
//
//	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
//		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSMatchmakingSubsystem::OnFindSessionsComplete));
//
//	UE_LOG(LogTemp, Warning, TEXT("CLIENT (Matchmaking) : Recherche de serveurs dedies en cours..."));
//	SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef());
//}
//
//void UEOSMatchmakingSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
//{
//	UE_LOG(LogTemp, Warning, TEXT("CLIENT (Matchmaking) : Recherche terminee."));
//	SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
//}
//
//void UEOSMatchmakingSubsystem::JoinGameSession(int32 Index)
//{
//	if (!SessionInterface.IsValid() || !LastSessionSearch.IsValid()) return;
//
//	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
//		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSMatchmakingSubsystem::OnJoinSessionComplete));
//
//	SessionInterface->JoinSession(0, FName("MainSession"), LastSessionSearch->SearchResults[Index]);
//}
//
//void UEOSMatchmakingSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
//{
//	if (Result == EOnJoinSessionCompleteResult::Success)
//	{
//		FString ConnectString;
//		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
//		{
//			UE_LOG(LogTemp, Warning, TEXT("CLIENT (Matchmaking) : Travel vers %s"), *ConnectString);
//		}
//	}
//	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
//}
//
