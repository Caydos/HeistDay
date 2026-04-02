#include "Network/Client/EOSMatchmakingSubsystem.h"
#include "Network/Client/EOSLobbySubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"

void UEOSMatchmakingSubsystem::FindMatch(int32 PartySize)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] FindMatch called with PartySize: %d"), PartySize);
	CurrentPartySize = PartySize;
	OnStatusChanged.Broadcast(TEXT("Searching for matches..."));
	SearchForSession(PartySize);
}

void UEOSMatchmakingSubsystem::SearchForSession(int32 SlotsNeeded)
{
	UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Starting SearchForSession..."));
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

			// --- NEW ID LOGIC ---
			TSharedPtr<const FUniqueNetId> UserId = GetGameInstance()->GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			if (UserId.IsValid())
			{
				SessionInterface->FindSessions(*UserId, SessionSearch.ToSharedRef());
				UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] FindSessions request sent to EOS using valid NetId."));
			}
		}
	}
}
void UEOSMatchmakingSubsystem::OnSearchCompleted(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] OnSearchCompleted fired! Success: %d"), bWasSuccessful);

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		Subsystem->GetSessionInterface()->ClearOnFindSessionsCompleteDelegates(this);
	}

	bool bFoundValidSession = false;

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Found %d sessions matching the 2v2 criteria."), SessionSearch->SearchResults.Num());

		// Loop through all found sessions to find one with enough empty slots!
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			if (Result.IsValid() && Result.Session.NumOpenPublicConnections >= CurrentPartySize)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] Found a session with %d open slots! Joining..."), Result.Session.NumOpenPublicConnections);
				OnStatusChanged.Broadcast(TEXT("Match found! Joining..."));

				JoinFoundSession(Result);
				bFoundValidSession = true;
				break; // Stop searching, we found our match!
			}
		}
	}

	// If we found absolutely nothing, OR all found sessions were already full (4/4)
	if (!bFoundValidSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] No valid sessions with enough space. Hosting our own..."));
		OnStatusChanged.Broadcast(TEXT("Hosting new session..."));
		CreateMatchmakingSession(CurrentPartySize);
	}
}

void UEOSMatchmakingSubsystem::CreateMatchmakingSession(int32 SlotsNeeded)
{
	UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Creating new Matchmaking Session..."));
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

			// --- NEW ID LOGIC ---
			TSharedPtr<const FUniqueNetId> UserId = GetGameInstance()->GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			if (UserId.IsValid())
			{
				SessionInterface->CreateSession(*UserId, FName("MyMatchSession"), SessionSettings);
			}
		}
	}
}

void UEOSMatchmakingSubsystem::OnCreateCompleted(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] OnCreateCompleted fired! Success: %d"), bWasSuccessful);

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Binding Participant Delegates for HOST..."));
		SessionInterface->OnSessionParticipantJoinedDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnParticipantJoined);
		SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnParticipantLeft);

		UpdateSessionState();
	}
	else
	{
		OnStatusChanged.Broadcast(TEXT("Failed to create session!"));
	}
}

void UEOSMatchmakingSubsystem::JoinFoundSession(const FOnlineSessionSearchResult& SearchResult)
{
	UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Attempting to Join Found Session..."));
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnJoinCompleted);

			// --- NEW ID LOGIC ---
			TSharedPtr<const FUniqueNetId> UserId = GetGameInstance()->GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			if (UserId.IsValid())
			{
				SessionInterface->JoinSession(*UserId, FName("MyMatchSession"), SearchResult);
			}
		}
	}
}

void UEOSMatchmakingSubsystem::OnJoinCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] OnJoinCompleted fired! Result Code: %d"), (int32)Result);

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Binding Participant Delegates for JOINER..."));
		SessionInterface->OnSessionParticipantJoinedDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnParticipantJoined);
		SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(this, &UEOSMatchmakingSubsystem::OnParticipantLeft);

		UpdateSessionState();
	}
	else
	{
		OnStatusChanged.Broadcast(TEXT("Failed to join match."));
	}
}

void UEOSMatchmakingSubsystem::OnParticipantJoined(FName SessionName, const FUniqueNetId& ParticipantId)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] DELEGATE FIRED: OnParticipantJoined! Someone entered the session."));
	UpdateSessionState();
}

void UEOSMatchmakingSubsystem::OnParticipantLeft(FName SessionName, const FUniqueNetId& ParticipantId, EOnSessionParticipantLeftReason Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] DELEGATE FIRED: OnParticipantLeft! Someone left the session."));

	// Re-run the math to update the UI (e.g., drops from 2/4 back down to 1/4)
	UpdateSessionState();
}

void UEOSMatchmakingSubsystem::UpdateSessionState()
{
	UE_LOG(LogTemp, Log, TEXT("[MATCHMAKING] Running UpdateSessionState..."));

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(FName("MyMatchSession"));
	if (Session)
	{
		int32 MaxPlayers = Session->SessionSettings.NumPublicConnections;
		int32 OpenSlots = Session->NumOpenPublicConnections;
		int32 CurrentPlayers = MaxPlayers - OpenSlots;

		// --- THE FIX FOR THE "0/4" HOST DELAY BUG ---
		// If Epic hasn't registered us yet, force the math to at least include our own party!
		if (CurrentPlayers < CurrentPartySize)
		{
			CurrentPlayers = CurrentPartySize;
		}

		UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] Session Math -> Max: %d | Open: %d | UI Showing: %d"), MaxPlayers, OpenSlots, CurrentPlayers);

		// --- THE 4-PLAYER TRIGGER ---
		if (CurrentPlayers >= 4)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MATCHMAKING] LOBBY FULL! Triggering Server Travel!"));
			OnStatusChanged.Broadcast(TEXT("Lobby is full! Match is starting..."));

			SessionInterface->ClearOnSessionParticipantJoinedDelegates(this);
			SessionInterface->ClearOnSessionParticipantLeftDelegates(this);

			FString ServerIP = TEXT("10.0.7.4");
			UEOSLobbySubsystem* Lobby = GetGameInstance()->GetSubsystem<UEOSLobbySubsystem>();
			if (Lobby)
			{
				Lobby->StartGame(ServerIP);
			}
		}
		else
		{
			// If it is 1/4, 2/4, or 3/4, we just update the UI and wait patiently!
			FString Status = FString::Printf(TEXT("Waiting for players (%d/%d)..."), CurrentPlayers, MaxPlayers);
			OnStatusChanged.Broadcast(Status);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MATCHMAKING] GetNamedSession returned null!"));
	}
}