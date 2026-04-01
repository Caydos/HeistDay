#include "Network/Client/EOSLobbySubsystem.h"
#include "Network/Client/EOSIdentitySubsystem.h"
#include <eos_presence_types.h>

using namespace UE::Online;

bool UEOSLobbySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !IsRunningDedicatedServer();
}

void UEOSLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineServicesPtr Services = GetServices();
	if (Services)
	{
		ILobbiesPtr Lobbies = Services->GetLobbiesInterface();
		if (Lobbies)
		{
			MemberJoinedHandle = Lobbies->OnLobbyMemberJoined().Add([this](const FLobbyMemberJoined& Event) {
				OnLobbyMemberJoined(Event);
				});

			MemberLeftHandle = Lobbies->OnLobbyMemberLeft().Add([this](const FLobbyMemberLeft& Event) {
				OnLobbyMemberLeft(Event);
				});

			UIJoinRequestedHandle = Lobbies->OnUILobbyJoinRequested().Add([this](const FUILobbyJoinRequested& Event) {
				OnLobbyJoinRequested(Event);
				});

			AttributesChangedHandle = Lobbies->OnLobbyAttributesChanged().Add([this](const UE::Online::FLobbyAttributesChanged& Event) {
				OnLobbyAttributesUpdate(Event);
				});
		}
	}
}

void UEOSLobbySubsystem::OnLobbyAttributesUpdate(const UE::Online::FLobbyAttributesChanged& Event)
{
	FName ServerIPKey(TEXT("ServerIP"));
	FString TargetIP = TEXT("");

	// Check if the ServerIP was just added or changed by the leader
	if (Event.AddedAttributes.Contains(ServerIPKey))
	{
		TargetIP = Event.AddedAttributes[ServerIPKey].GetString();
	}
	else if (Event.ChangedAttributes.Contains(ServerIPKey))
	{
		TargetIP = Event.ChangedAttributes[ServerIPKey].Value.GetString();
	}

	// If we found a valid IP, execute the travel!
	if (!TargetIP.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Lobby Broadcast received! Travelling to server: %s"), *TargetIP);

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(TargetIP, TRAVEL_Absolute);
		}
	}
}

void UEOSLobbySubsystem::OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& Event)
{
	if (Event.Result.IsOk())
	{
		TSharedRef<const UE::Online::FLobby> TargetLobby = Event.Result.GetOkValue();
		UE_LOG(LogTemp, Log, TEXT("Overlay requested to join lobby: %s"), *ToLogString(TargetLobby->LobbyId));

		if (IsInLobby())
		{
			PendingLobbyId = TargetLobby->LobbyId;
			PendingAccountId = Event.LocalAccountId;
			bHasPendingJoin = true;

			UE_LOG(LogTemp, Warning, TEXT("Already in a lobby. Leaving current lobby before joining the new one..."));
			LeaveLobby();
		}
		else
		{
			ProcessJoinLobby(Event.LocalAccountId, TargetLobby->LobbyId);
		}
	}
	else
	{
		FString ErrorMessage = ToLogString(Event.Result.GetErrorValue());
		UE_LOG(LogTemp, Error, TEXT("Failed to join lobby via UI: %s"), *ErrorMessage);
	}
}

void UEOSLobbySubsystem::ProcessJoinLobby(UE::Online::FAccountId InLocalAccountId, UE::Online::FLobbyId InLobbyId)
{
	UE::Online::IOnlineServicesPtr Services = UE::Online::GetServices();
	if (Services)
	{
		UE::Online::ILobbiesPtr LobbiesInterface = Services->GetLobbiesInterface();
		if (LobbiesInterface)
		{
			UE::Online::FJoinLobby::Params JoinParams;
			JoinParams.LocalAccountId = InLocalAccountId;
			JoinParams.LobbyId = InLobbyId;
			JoinParams.bPresenceEnabled = true;

			LobbiesInterface->JoinLobby(MoveTemp(JoinParams)).OnComplete(
				[this](const UE::Online::TOnlineResult<UE::Online::FJoinLobby>& JoinResult)
				{
					OnJoinLobbyComplete(JoinResult);
				}
			);
		}
	}
}

void UEOSLobbySubsystem::OnJoinLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FJoinLobby>& Result)
{
	if (Result.IsOk())
	{
		const TSharedPtr<const UE::Online::FLobby>& Lobby = Result.GetOkValue().Lobby;

		if (Lobby.IsValid())
		{
			CurrentLobbyId = Lobby->LobbyId;
			CurrentPlayerCount = Lobby->Members.Num();

			UE_LOG(LogTemp, Log, TEXT("Successfully joined the lobby! %s. Players in lobby: %d"),
				*ToLogString(Lobby->LobbyId), CurrentPlayerCount);
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Successfully Joined Lobby!"));
		}

		NotifyLobbyStateChanged();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join lobby: %s"),
			*Result.GetErrorValue().GetLogString());
	}
}

void UEOSLobbySubsystem::OnLobbyMemberJoined(const UE::Online::FLobbyMemberJoined& Event)
{
	CurrentPlayerCount++;
	FString JoinMsg = FString::Printf(TEXT("A player joined! Current Count: %d"), CurrentPlayerCount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, JoinMsg);
	}

	NotifyLobbyStateChanged();
}

void UEOSLobbySubsystem::Deinitialize()
{
	MemberJoinedHandle.Unbind();
	MemberLeftHandle.Unbind();
	UIJoinRequestedHandle.Unbind();
	AttributesChangedHandle.Unbind(); 

	Super::Deinitialize();
}

void UEOSLobbySubsystem::CreateLobby()
{
	UEOSIdentitySubsystem* IdentitySubsystem = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
	if (!IdentitySubsystem || !IdentitySubsystem->IsLoggedIn()) return;

	IOnlineServicesPtr Services = GetServices();
	if (!Services || !Services->GetLobbiesInterface()) return;

	FCreateLobby::Params Params;
	Params.LocalAccountId = IdentitySubsystem->GetLocalAccountId();
	Params.LocalName = TEXT("PartyLobby");
	Params.SchemaId = FName(TEXT("GameLobby"));
	Params.MaxMembers = 4;
	Params.bPresenceEnabled = true;
	Params.JoinPolicy = UE::Online::ELobbyJoinPolicy::PublicAdvertised;

	UE_LOG(LogTemp, Warning, TEXT("EOSLobbySubsystem: Creating lobby..."));

	Services->GetLobbiesInterface()->CreateLobby(MoveTemp(Params)).OnComplete(
		[this](const UE::Online::TOnlineResult<UE::Online::FCreateLobby>& Result)
		{
			OnCreateLobbyComplete(Result);
		}
	);
}

void UEOSLobbySubsystem::OnCreateLobbyComplete(const TOnlineResult<FCreateLobby>& Result)
{
	if (Result.IsOk())
	{
		CurrentLobbyId = Result.GetOkValue().Lobby->LobbyId;
		CurrentPlayerCount = 1;

		UE_LOG(LogTemp, Warning, TEXT("EOSLobbySubsystem: Lobby créé avec succès ! ID: %s"), *UE::Online::ToLogString(CurrentLobbyId));
		NotifyLobbyStateChanged();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSLobbySubsystem: Échec création Lobby: %s"), *Result.GetErrorValue().GetLogString());
	}
}

void UEOSLobbySubsystem::LeaveLobby()
{
	if (!IsInLobby()) return;

	UEOSIdentitySubsystem* IdentitySubsystem = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
	IOnlineServicesPtr Services = GetServices();

	if (Services && IdentitySubsystem)
	{
		FLeaveLobby::Params Params;
		Params.LocalAccountId = IdentitySubsystem->GetLocalAccountId();
		Params.LobbyId = CurrentLobbyId;

		Services->GetLobbiesInterface()->LeaveLobby(MoveTemp(Params)).OnComplete(
			[this](const UE::Online::TOnlineResult<UE::Online::FLeaveLobby>& Result)
			{
				OnLeaveLobbyComplete(Result);
			}
		);
	}
}

void UEOSLobbySubsystem::StartGame(FString ServerIP)
{
	UEOSIdentitySubsystem* IdentitySubsystem = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
	UE::Online::IOnlineServicesPtr Services = UE::Online::GetServices();

	if (Services && IdentitySubsystem && IsInLobby())
	{
		UE::Online::FModifyLobbyAttributes::Params Params;
		Params.LocalAccountId = IdentitySubsystem->GetLocalAccountId();
		Params.LobbyId = CurrentLobbyId;

		// Add the Server IP to the Lobby Attributes
		Params.UpdatedAttributes.Add(FName(TEXT("ServerIP")), UE::Online::FSchemaVariant(ServerIP));

		UE_LOG(LogTemp, Log, TEXT("Leader is broadcasting Server IP to lobby..."));

		Services->GetLobbiesInterface()->ModifyLobbyAttributes(MoveTemp(Params)).OnComplete(
			[this, ServerIP](const UE::Online::TOnlineResult<UE::Online::FModifyLobbyAttributes>& Result)
			{
				if (Result.IsOk())
				{
					UE_LOG(LogTemp, Log, TEXT("Successfully broadcasted the Server IP! Hosting player travelling now."));

					// Force the Host to travel too, in case EOS doesn't fire OnLobbyAttributesChanged for them
					if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
					{
						PC->ClientTravel(ServerIP, TRAVEL_Absolute);
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to broadcast Server IP: %s"), *Result.GetErrorValue().GetLogString());
				}
			}
		);
	}
}

void UEOSLobbySubsystem::OnLeaveLobbyComplete(const TOnlineResult<FLeaveLobby>& Result)
{
	CurrentLobbyId = UE::Online::FLobbyId();
	CurrentPlayerCount = 0;
	NotifyLobbyStateChanged();

	UE_LOG(LogTemp, Log, TEXT("Successfully left the lobby."));

	if (bHasPendingJoin)
	{
		bHasPendingJoin = false;
		UE_LOG(LogTemp, Log, TEXT("Executing pending join request..."));
		ProcessJoinLobby(PendingAccountId, PendingLobbyId);
	}
}

bool UEOSLobbySubsystem::IsInLobby() const
{
	return CurrentLobbyId.IsValid();
}

int32 UEOSLobbySubsystem::GetLobbyPlayerCount() const
{
	return CurrentPlayerCount;
}

void UEOSLobbySubsystem::OnLobbyMemberLeft(const FLobbyMemberLeft& Event)
{
	if (Event.Lobby->LobbyId == CurrentLobbyId)
	{
		CurrentPlayerCount--;
		FString PrintMsg = FString::Printf(TEXT("Un joueur est parti ! Reste %d joueurs."), CurrentPlayerCount);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, PrintMsg);

		NotifyLobbyStateChanged();
	}
}

void UEOSLobbySubsystem::NotifyLobbyStateChanged()
{
	OnLobbyStateChanged.Broadcast();
}