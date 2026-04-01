//#pragma once
//
//#include "CoreMinimal.h"
//#include "Network/EOSNetworkSubsystem.h"
//#include "Interfaces/OnlineSessionInterface.h"
//#include <Online/OnlineSessionNames.h>
//
//#include "EOSMatchmakingSubsystem.generated.h"
//
//UCLASS()
//class FINALGAME_API UEOSMatchmakingSubsystem : public UEOSNetworkSubsystem
//{
//	GENERATED_BODY()
//
//public:
//	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
//	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
//
//	UFUNCTION(BlueprintCallable, Category = "EOS|Matchmaking")
//	void FindSessions();
//
//	UFUNCTION(BlueprintCallable, Category = "EOS|Matchmaking")
//	void JoinGameSession(int32 Index);
//
//protected:
//	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
//
//	void OnFindSessionsComplete(bool bWasSuccessful);
//	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
//
//private:
//	IOnlineSessionPtr SessionInterface;
//};