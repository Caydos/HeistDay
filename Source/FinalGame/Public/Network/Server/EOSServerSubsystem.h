//#pragma once
//
//#include "CoreMinimal.h"
//#include "Network/EOSNetworkSubsystem.h" 
//// NOUVEAU : On inclut l'interface Sessions de l'OSSv2
//#include "Online/Sessions.h" 
//#include "EOSServerSubsystem.generated.h"
//
//UCLASS()
//class FINALGAME_API UEOSServerSubsystem : public UEOSNetworkSubsystem
//{
//	GENERATED_BODY()
//
//public:
//	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
//	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
//
//	UFUNCTION(BlueprintCallable)
//	void StartServerEOS();
//
//	UFUNCTION(BlueprintCallable)
//	void CreateEOSSession();
//
//protected:
//	bool bHasInitializedServer = false;
//
//	// NOUVEAU : Fonction asynchrone OSSv2
//	void OnCreateSessionComplete(const UE::Online::TOnlineResult<UE::Online::FCreateSession>& Result);
//
//private:
//	// NOUVEAU : Identifiant de session propre à l'OSSv2
//	/*UE::Online::FOnlineSessionId CurrentSessionId;*/
//};