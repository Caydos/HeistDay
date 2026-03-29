//#include "Network/Server/EOSServerSubsystem.h"
//#include "Network/Client/EOSIdentitySubsystem.h"
//
//using namespace UE::Online;
//
//bool UEOSServerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
//{
//	return !IsRunningClientOnly(); // Autorisé pour Listen Server / Dedicated Server
//}
//
//void UEOSServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//}
//
//void UEOSServerSubsystem::StartServerEOS()
//{
//	CreateEOSSession();
//}
//
//void UEOSServerSubsystem::CreateEOSSession()
//{
//	IOnlineServicesPtr Services = GetServices();
//	if (!Services) return;
//
//	ISessionsPtr Sessions = Services->GetSessionsInterface();
//	if (!Sessions) return;
//
//	UEOSIdentitySubsystem* IdentitySubsystem = GetGameInstance()->GetSubsystem<UEOSIdentitySubsystem>();
//
//	FCreateSession::Params Params;
//	Params.SessionName = FName(TEXT("MainServerSession"));
//
//	// --- CORRECTIONS OSSv2 ---
//	// 1. Les paramètres généraux vont directement dans "Params" (Pas de SessionSettings ici !)
//	Params.bIsLANSession = false;
//	Params.bPresenceEnabled = true;
//
//	// 2. Les règles de la session vont dans "SessionSettings"
//	Params.SessionSettings.NumMaxConnections = 16;
//	Params.SessionSettings.bAllowNewMembers = true; // Remplace bAllowJoinInProgress
//	Params.SessionSettings.JoinPolicy = UE::Online::ESessionJoinPolicy::Public; // Définit l'ouverture du serveur
//
//	if (IdentitySubsystem && IdentitySubsystem->IsLoggedIn())
//	{
//		Params.LocalAccountId = IdentitySubsystem->GetLocalAccountId();
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("EOSServerSubsystem: Création de la Session OSSv2..."));
//	Sessions->CreateSession(MoveTemp(Params)).OnComplete(this, &UEOSServerSubsystem::OnCreateSessionComplete);
//}
//
//void UEOSServerSubsystem::OnCreateSessionComplete(const UE::Online::TOnlineResult<UE::Online::FCreateSession>& Result)
//{
//	if (Result.IsOk())
//	{
//		// CORRECTION : Le Result est vide, on se fie juste au succès de l'opération !
//		UE_LOG(LogTemp, Warning, TEXT("EOSServerSubsystem: Session créée avec succès (Nom: MainServerSession) !"));
//		bHasInitializedServer = true;
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("EOSServerSubsystem: Échec de création de session: %s"), *Result.GetErrorValue().GetLogString());
//	}
//}