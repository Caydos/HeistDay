#include "Network/Client/EOSMatchmakingSubsystem.h"
#include "Network/Client/EOSLobbySubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UEOSMatchmakingSubsystem::FindMatch(int32 PartySize)
{
	CurrentPartySize = PartySize;
	OnStatusChanged.Broadcast(TEXT("Searching for matches..."));
	SearchForSession(PartySize);
}

void UEOSMatchmakingSubsystem::SearchForSession(int32 SlotsNeeded)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 10;

			SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);
			SessionSearch->QuerySettings.Set(FName("GameMode"), FString("2v2"), EOnlineComparisonOp::Equals);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnSearchCompleted);
			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
}

void UEOSMatchmakingSubsystem::OnSearchCompleted(bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		Subsystem->GetSessionInterface()->ClearOnFindSessionsCompleteDelegates(this);
	}

	if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
	{
		OnStatusChanged.Broadcast(TEXT("Match found! Joining..."));
		JoinFoundSession(SessionSearch->SearchResults[0]);
	}
	else
	{
		OnStatusChanged.Broadcast(TEXT("No match found. Hosting new session..."));
		CreateMatchmakingSession(CurrentPartySize);
	}
}

void UEOSMatchmakingSubsystem::CreateMatchmakingSession(int32 SlotsNeeded)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FOnlineSessionSettings SessionSettings;

			SessionSettings.bIsLANMatch = false;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.NumPublicConnections = 4;
			SessionSettings.bUsesPresence = true;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bUseLobbiesIfAvailable = false;

			SessionSettings.Set(FName("GameMode"), FString("2v2"), EOnlineDataAdvertisementType::ViaOnlineService);

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnCreateCompleted);
			SessionInterface->CreateSession(0, FName("MyMatchSession"), SessionSettings);
		}
	}
}

void UEOSMatchmakingSubsystem::OnCreateCompleted(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

		if (bWasSuccessful)
		{
			OnStatusChanged.Broadcast(TEXT("Waiting for opponent..."));

			// HOST LOGIC: Start checking the session size every 2 seconds
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(PollingTimer, this, &UEOSMatchmakingSubsystem::PollSessionSize, 2.0f, true);
			}
		}
		else
		{
			OnStatusChanged.Broadcast(TEXT("Failed to create session!"));
		}
	}
}

void UEOSMatchmakingSubsystem::JoinFoundSession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnJoinCompleted);
			SessionInterface->JoinSession(0, FName("MyMatchSession"), SearchResult);
		}
	}
}

void UEOSMatchmakingSubsystem::OnJoinCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		Subsystem->GetSessionInterface()->ClearOnJoinSessionCompleteDelegates(this);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// JOINER LOGIC: Do NOT teleport! 
		// Instead, start the polling timer just like the Host did.
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(PollingTimer, this, &UEOSMatchmakingSubsystem::PollSessionSize, 2.0f, true);
		}
	}
	else
	{
		OnStatusChanged.Broadcast(TEXT("Failed to join match."));
	}
}

void UEOSMatchmakingSubsystem::PollSessionSize()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Read the live data from the session
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(FName("MyMatchSession"));

			if (Session)
			{
				// The bulletproof way to count players in OSSv1
				int32 MaxPlayers = Session->SessionSettings.NumPublicConnections;
				int32 OpenSlots = Session->NumOpenPublicConnections;
				int32 CurrentPlayers = MaxPlayers - OpenSlots;

				// Check if the lobby has hit 4 players
				if (CurrentPlayers >= 4)
				{
					// Stop checking!
					if (GetWorld())
					{
						GetWorld()->GetTimerManager().ClearTimer(PollingTimer);
					}

					OnStatusChanged.Broadcast(TEXT("Lobby is full! Match is starting..."));

					// Teleport the players!
					FString ServerIP = TEXT("10.0.7.4");
					UEOSLobbySubsystem* Lobby = GetGameInstance()->GetSubsystem<UEOSLobbySubsystem>();
					if (Lobby)
					{
						Lobby->StartGame(ServerIP);
					}
				}
				else
				{
					// Update the UI text dynamically with the reliable math
					FString Status = FString::Printf(TEXT("Waiting for players (%d/%d)..."), CurrentPlayers, MaxPlayers);
					OnStatusChanged.Broadcast(Status);
				}
			}
		}
	}
}