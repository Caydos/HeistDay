// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSNetworkSubsystem.generated.h"

/**
 *
 */
UCLASS()
class FINALGAME_API UEOSNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Login with Dev Auth Tool
	void LoginWithDevAuth();
	void StartServerEOS();

protected:
	// Callback when login is complete
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void OnServerLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

private:
	// Online Identity Interface pointer
	IOnlineIdentityPtr IdentityInterface;
};