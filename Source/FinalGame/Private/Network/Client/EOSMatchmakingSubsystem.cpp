//#include "Network/Client/EOSMatchmakingSubsystem.h"
//#include "Network/Client/EOSIdentitySubsystem.h"
//#include "Network/Client/EOSLobbySubsystem.h"
//
//using namespace UE::Online;
//
//void UEOSMatchmakingSubsystem::FindMatch(int32 PartySize)
//{
//	OnStatusChanged.Broadcast(TEXT("Searching for opponents..."));
//	SearchForLobbies();
//}
//
//void UEOSMatchmakingSubsystem::SearchForLobbies()
//{
//	IOnlineServicesPtr Services = UE::Online::GetServices();
//	UEOSIdentitySubsystem* Identity = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
//
//	if (Services && Identity)
//	{
//		ILobbiesPtr Lobbies = Services->GetLobbiesInterface();
//		FFindLobbies::Params Params;
//		Params.LocalAccountId = Identity->GetLocalAccountId();
//		Params.MaxResults = 10;
//
//		// Find Lobbies tagged as 2v2
//		FFindLobbySearchFilter Filter;
//		Filter.AttributeName = FSchemaAttributeId(TEXT("GameMode"));
//		Filter.ComparisonOp = ESchemaAttributeComparisonOp::Equals;
//		Filter.ComparisonValue = FSchemaVariant(FString(TEXT("2v2")));
//		Params.Filters.Add(Filter);
//
//		Lobbies->FindLobbies(MoveTemp(Params)).OnComplete(this, &UEOSMatchmakingSubsystem::OnSearchCompleted);
//	}
//}
//
//void UEOSMatchmakingSubsystem::OnSearchCompleted(const TOnlineResult<FFindLobbies>& Result)
//{
//	if (Result.IsOk() && Result.GetOkValue().Lobbies.Num() > 0)
//	{
//		OnStatusChanged.Broadcast(TEXT("Match found! Joining..."));
//
//		// We found an opponent's lobby! Let's join it.
//		IOnlineServicesPtr Services = UE::Online::GetServices();
//		UEOSIdentitySubsystem* Identity = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
//
//		FJoinLobby::Params Params;
//		Params.LocalAccountId = Identity->GetLocalAccountId();
//		Params.LocalName = FName(TEXT("PartyLobby")); // Must match the name in CreateLobby!
//		Params.LobbyId = Result.GetOkValue().Lobbies[0]->LobbyId;
//		Params.bPresenceEnabled = true;
//
//		Services->GetLobbiesInterface()->JoinLobby(MoveTemp(Params)).OnComplete(this, &UEOSMatchmakingSubsystem::OnJoinCompleted);
//	}
//	else
//	{
//		// No opponents found. We will host the match by making our Party Lobby public!
//		MakePartyLobbyPublic();
//	}
//}
//
//void UEOSMatchmakingSubsystem::MakePartyLobbyPublic()
//{
//	IOnlineServicesPtr Services = UE::Online::GetServices();
//	UEOSIdentitySubsystem* Identity = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
//	UEOSLobbySubsystem* LobbySub = GetGameInstance()->GetSubsystem<UEOSLobbySubsystem>();
//
//	if (Services && Identity && LobbySub->IsInLobby())
//	{
//		FModifyLobbyAttributes::Params Params;
//		Params.LocalAccountId = Identity->GetLocalAccountId();
//		//Params.LobbyId = LobbySub->CurrentLobbyId; // Use the Lobby we are already inside!
//
//		// Add the 2v2 tag so others can find us
//		Params.UpdatedAttributes.Add(FSchemaAttributeId(TEXT("GameMode")), FSchemaVariant(FString(TEXT("2v2"))));
//
//		Services->GetLobbiesInterface()->ModifyLobbyAttributes(MoveTemp(Params));
//
//		// Make the lobby Publicly Searchable
//		FModifyLobbyJoinPolicy::Params JoinParams;
//		JoinParams.LocalAccountId = Identity->GetLocalAccountId();
//		/*JoinParams.LobbyId = LobbySub->CurrentLobbyId;*/
//		JoinParams.JoinPolicy = ELobbyJoinPolicy::PublicAdvertised;
//
//		Services->GetLobbiesInterface()->ModifyLobbyJoinPolicy(MoveTemp(JoinParams));
//
//		OnStatusChanged.Broadcast(TEXT("Waiting for opponents..."));
//	}
//}
//
//void UEOSMatchmakingSubsystem::OnJoinCompleted(const TOnlineResult<FJoinLobby>& Result)
//{
//	if (Result.IsOk())
//	{
//		OnStatusChanged.Broadcast(TEXT("Successfully joined the match!"));
//		// The teleport is handled automatically by OnLobbyMemberJoined in EOSLobbySubsystem!
//	}
//	else
//	{
//		OnStatusChanged.Broadcast(TEXT("Failed to join match."));
//	}
//}