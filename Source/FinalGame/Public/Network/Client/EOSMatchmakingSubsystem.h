#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "EOSMatchmakingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStatusChanged, FString, StatusMessage);

UCLASS()
class FINALGAME_API UEOSMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Starts the matchmaking process
	UFUNCTION(BlueprintCallable, Category = "EOS|Matchmaking")
	void FindMatch(int32 PartySize);

	UPROPERTY(BlueprintAssignable, Category = "EOS|Matchmaking")
	FOnMatchmakingStatusChanged OnStatusChanged;

private:
	void SearchForSession(int32 SlotsNeeded);
	void CreateMatchmakingSession(int32 SlotsNeeded);
	void JoinFoundSession(const FOnlineSessionSearchResult& SearchResult);

	// Core OSSv1 Callbacks
	void OnSearchCompleted(bool bWasSuccessful);
	void OnCreateCompleted(FName SessionName, bool bWasSuccessful);
	void OnJoinCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// THE REAL MATCHMAKING DELEGATES
	void OnParticipantJoined(FName SessionName, const FUniqueNetId& ParticipantId);
	void OnParticipantLeft(FName SessionName, const FUniqueNetId& ParticipantId, EOnSessionParticipantLeftReason Reason);

	// Helper function to evaluate the math and trigger travel
	void UpdateSessionState();

	int32 CurrentPartySize = 1;

	// Holds the search results in memory
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
};