// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerGameInstance::UMultiplayerGameInstance()
{
	MySessionName = FName("PlantSession");
}

void UMultiplayerGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get()) {
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid()) {
			//Bind delegates
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnJoinSessionComplete);
		}
	}
		
}

void UMultiplayerGameInstance::OnCreateSessionComplete(FName sessionName, bool succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OncreateSessionComplete, Succeeded: %d"), succeeded);

	if (succeeded)
	{
		GetWorld()->ServerTravel("/Game/EnvMapBlockout_0?listen");
	}
}

void UMultiplayerGameInstance::OnFindSessionComplete(bool succeeded)
{
	SearchingForServer.Broadcast(false);

	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete, Succeeded: %d"), succeeded);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FindSessionComplete"));
	if (succeeded)
	{
		TArray<FOnlineSessionSearchResult> searchResults = sessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Searchresults, server count: %d"), searchResults.Num());
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Succeeded founds sessions: ") + FString::FromInt(searchResults.Num()));

		int32 arrayIndex = 0;
		for (FOnlineSessionSearchResult result : searchResults)
		{
			if (!result.IsValid())
				continue;
			
			FServerInfo info;
			FString serverName = "";
			FString hostName = "";

			result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), serverName);
			result.Session.SessionSettings.Get(FName("SERVER_HOSTNAME_KEY"), hostName);

			info.ServerName = serverName;
			info.MaxPlayers = result.Session.NumOpenPublicConnections;
			info.CurrentPlayers = info.MaxPlayers - result.Session.NumOpenPublicConnections;
			info.ServerArrayIndex = arrayIndex;
			info.SetPlayerCount();

			ServerListDelegate.Broadcast(info);
			++arrayIndex;
		}
		
		//Join first server (0)
		if (searchResults.Num()) {
			//SessionInterface->JoinSession(0, "PlantSession", searchResults[0]);

		}
	}

}

void UMultiplayerGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete"));
	if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Controller found, player can start client travel"));
		FString joinAdress = "";
		SessionInterface->GetResolvedConnectString(SessionName, joinAdress);

		if (joinAdress != "")
			PController->ClientTravel(joinAdress, ETravelType::TRAVEL_Absolute);
	}
}

void UMultiplayerGameInstance::CreateServer(FString serverName, FString hostName)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateServer"));

	FOnlineSessionSettings sessionSettings;
	sessionSettings.bAllowJoinInProgress = true;
	sessionSettings.bIsDedicated = false;

	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
		sessionSettings.bIsLANMatch = false;
	else
		sessionSettings.bIsLANMatch = true;

	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bAllowInvites = true;
	sessionSettings.NumPublicConnections = 5;

	sessionSettings.Set(FName("SERVER_NAME_KEY"), serverName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	sessionSettings.Set(FName("SERVER_HOSTNAME_KEY"), hostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, MySessionName, sessionSettings);

}

void UMultiplayerGameInstance::FindServers()
{
	SearchingForServer.Broadcast(true);

	UE_LOG(LogTemp, Warning, TEXT("JoinServer"));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Join server"));

	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
		sessionSearch->bIsLanQuery = false;
	else
		sessionSearch->bIsLanQuery = true;
	
	sessionSearch->MaxSearchResults = 10000;
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UMultiplayerGameInstance::JoinServer(int32 arrayIndex)
{
	FOnlineSessionSearchResult result = sessionSearch->SearchResults[arrayIndex];
	if (result.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Joining server at index: ") + FString::FromInt(arrayIndex));
		SessionInterface->JoinSession(0, MySessionName, result);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Joining server failed, session invalid"));
	}
}
