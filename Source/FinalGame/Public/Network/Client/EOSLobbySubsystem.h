#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/EOSNetworkSubsystem.h"
#include "EOSLobbySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyStateChanged);

UCLASS()
class FINALGAME_API UEOSLobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void CreateLobby();

	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void LeaveLobby();

	UFUNCTION(BlueprintPure, Category = "EOS|Lobby")
	bool IsInLobby() const;

	UFUNCTION(BlueprintPure, Category = "EOS|Lobby")
	int32 GetLobbyPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void NotifyLobbyStateChanged();

	UPROPERTY(BlueprintAssignable, Category = "EOS|Lobby")
	FOnLobbyStateChanged OnLobbyStateChanged;

private:
	void OnCreateLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FCreateLobby>& Result);
	void OnLeaveLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FLeaveLobby>& Result);

	//UE::Online::FOnlineEventDelegateHandle UIJoinRequestedHandle;

	//void OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& Event);
	//void OnJoinLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FJoinLobby>& Result);

	void OnLobbyMemberJoined(const UE::Online::FLobbyMemberJoined& Event);
	void OnLobbyMemberLeft(const UE::Online::FLobbyMemberLeft& Event);

	UE::Online::FLobbyId CurrentLobbyId;
	UE::Online::FOnlineEventDelegateHandle MemberJoinedHandle;
	UE::Online::FOnlineEventDelegateHandle MemberLeftHandle;

	int32 CurrentPlayerCount = 0;
};