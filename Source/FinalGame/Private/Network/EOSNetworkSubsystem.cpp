// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/EOSNetworkSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UEOSNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IdentityInterface = Subsystem->GetIdentityInterface();
		/*SessionInterface = Subsystem->GetSessionInterface();*/

		if (IsRunningDedicatedServer())
		{
			UE_LOG(LogTemp, Warning, TEXT("SERVER : EOS Dedicated server initialization ..."));
			StartServerEOS();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CLIENT : Client initialization..."));

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UEOSNetworkSubsystem::LoginWithDevAuth, 5.0f, false);
		}
	}
}
void UEOSNetworkSubsystem::StartServerEOS()
{
	if (!IdentityInterface.IsValid()) return;

	FOnlineAccountCredentials Credentials;
	Credentials.Type = TEXT("accountportal");

	IdentityInterface->AddOnLoginCompleteDelegate_Handle(0,
		FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnServerLoginComplete));

	UE_LOG(LogTemp, Warning, TEXT("SERVEUR : Tentative d'authentification auprès d'Epic..."));
	IdentityInterface->Login(0, Credentials);
}

void UEOSNetworkSubsystem::OnServerLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVEUR : Authentification EOS réussie !"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SERVEUR : Écheccc de l'authentification : %s"), *Error);
	}
}
void UEOSNetworkSubsystem::LoginWithDevAuth()
{
	if (!IdentityInterface.IsValid()) return;

	FOnlineAccountCredentials Credentials;

	Credentials.Type = TEXT("developer");
	Credentials.Id = TEXT("localhost:8081");
	Credentials.Token = TEXT("DevUser");

	UE_LOG(LogTemp, Warning, TEXT("Vérification avant Login : Type='%s', Id='%s', Token='%s'"),
		*Credentials.Type, *Credentials.Id, *Credentials.Token);

	IdentityInterface->AddOnLoginCompleteDelegate_Handle(0,
		FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));

	if (!IdentityInterface->Login(0, Credentials))
	{
		UE_LOG(LogTemp, Error, TEXT("L'appel à IdentityInterface->Login a échoué immédiatement !"));
	}
}

void UEOSNetworkSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("=== LOGIN RÉUSSI ! ==="));
		UE_LOG(LogTemp, Warning, TEXT("Utilisateur EOS connecté : %s"), *UserId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("=== ÉCHEC DU LOGIN EOS ==="));
		UE_LOG(LogTemp, Error, TEXT("Raison de l'erreur : %s"), *Error);
	}
}