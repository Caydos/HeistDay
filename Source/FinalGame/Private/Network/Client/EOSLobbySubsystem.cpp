#include "Network/Client/EOSLobbySubsystem.h"
#include "Network/Client/EOSIdentitySubsystem.h"

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

			/*UIJoinRequestedHandle = Lobbies->OnUILobbyJoinRequested().Add([this](const FUILobbyJoinRequested& Event) {
				OnLobbyJoinRequested(Event);
				});*/
		}
	}
}
//void UEOSLobbySubsystem::OnLobbyJoinRequested(const FUILobbyJoinRequested& Event)
//{
//	UE_LOG(LogTemp, Warning, TEXT("EOSLobbySubsystem: L'Overlay demande de rejoindre un lobby !"));
//
//	IOnlineServicesPtr Services = GetServices();
//	if (!Services || !Services->GetLobbiesInterface()) return;
//
//	FJoinLobby::Params JoinParams;
//	JoinParams.LocalAccountId = Event.LocalAccountId; // The user who accepted the invite
//	JoinParams.LobbyId = Event.LobbyId;               // The lobby they are trying to join
//	JoinParams.bPresenceEnabled = true;               // Must be true to see other players in the overlay!
//
//	Services->GetLobbiesInterface()->JoinLobby(MoveTemp(JoinParams))
//		.OnComplete(this, &UEOSLobbySubsystem::OnJoinLobbyComplete);
//}
void UEOSLobbySubsystem::Deinitialize()
{
	MemberJoinedHandle.Unbind();
	MemberLeftHandle.Unbind();
	//UIJoinRequestedHandle.Unbind();

	Super::Deinitialize();
}

void UEOSLobbySubsystem::CreateLobby()
{
	UEOSIdentitySubsystem* IdentitySubsystem = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
	if (!IdentitySubsystem || !IdentitySubsystem->IsLoggedIn()) return;

	IOnlineServicesPtr Services = GetServices();
	if (!Services || !Services->GetLobbiesInterface()) return;


		auto Lobbies = Services->GetLobbiesInterface();
		if (!Lobbies)
		{
			UE_LOG(LogTemp, Error, TEXT("Lobbies interface is NULL"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Lobbies interface OK"));
		}
	
		FCreateLobby::Params Params;
		Params.LocalAccountId = IdentitySubsystem->GetLocalAccountId();
		Params.LocalName = TEXT("PartyLobby");
		Params.SchemaId = FName(TEXT("GameLobby")); // Must match the ID in the .ini exactly
		Params.MaxMembers = 4;
		Params.bPresenceEnabled = true;
	// Debug to verify schema name at runtime
	UE_LOG(LogTemp, Warning, TEXT("EOSLobbySubsystem: Creating lobby with schema [%s]"), *Params.SchemaId.ToString());

	Services->GetLobbiesInterface()->CreateLobby(MoveTemp(Params))
		.OnComplete(this, &UEOSLobbySubsystem::OnCreateLobbyComplete);
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

		Services->GetLobbiesInterface()->LeaveLobby(MoveTemp(Params))
			.OnComplete(this, &UEOSLobbySubsystem::OnLeaveLobbyComplete);
	}
}

void UEOSLobbySubsystem::OnLeaveLobbyComplete(const TOnlineResult<FLeaveLobby>& Result)
{
	CurrentLobbyId = UE::Online::FLobbyId();
	CurrentPlayerCount = 0;
	NotifyLobbyStateChanged();
}

bool UEOSLobbySubsystem::IsInLobby() const
{
	return CurrentLobbyId.IsValid();
}

int32 UEOSLobbySubsystem::GetLobbyPlayerCount() const
{
	return CurrentPlayerCount;
}

void UEOSLobbySubsystem::OnLobbyMemberJoined(const FLobbyMemberJoined& Event)
{
	if (Event.Lobby->LobbyId == CurrentLobbyId)
	{
		CurrentPlayerCount++;
		FString PrintMsg = FString::Printf(TEXT("Un joueur a rejoint ! Il y a maintenant %d joueurs."), CurrentPlayerCount);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, PrintMsg);
		UE_LOG(LogTemp, Warning, TEXT("EOSLobbySubsystem: %s"), *PrintMsg);

		NotifyLobbyStateChanged();
	}
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