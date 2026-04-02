#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Online/OnlineServices.h"
#include "Online/Lobbies.h"
#include "EOSMatchmakingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStatusChanged, FString, StatusMessage);

UCLASS()
class FINALGAME_API UEOSMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EOS|Matchmaking")
	void FindMatch(int32 PartySize);

	UPROPERTY(BlueprintAssignable, Category = "EOS|Matchmaking")
	FOnMatchmakingStatusChanged OnStatusChanged;

private:
	void SearchForLobbies();
	void MakePartyLobbyPublic();

	void OnSearchCompleted(const UE::Online::TOnlineResult<UE::Online::FFindLobbies>& Result);
	void OnJoinCompleted(const UE::Online::TOnlineResult<UE::Online::FJoinLobby>& Result);
};