// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerGameInstance::UMultiplayerGameInstance()
{
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

void UMultiplayerGameInstance::OnCreateSessionComplete(FName serverName, bool succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OncreateSessionComplete, Succeeded: %d"), succeeded);

	if (succeeded)
	{
		GetWorld()->ServerTravel("/Game/EnvMapBlockout_0?listen");
	}
}

void UMultiplayerGameInstance::OnFindSessionComplete(bool succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete, Succeeded: %d"), succeeded);
	
	if (succeeded)
	{
		TArray<FOnlineSessionSearchResult> searchResults = sessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Searchresults, server count: %d"), searchResults.Num());
		
		//Join first server (0)
		if (searchResults.Num())
			SessionInterface->JoinSession(0, "My session", searchResults[0]);
	}

}

void UMultiplayerGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete"));
	if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString joinAdress = "";
		SessionInterface->GetResolvedConnectString(SessionName, joinAdress);

		if (joinAdress != "")
			PController->ClientTravel(joinAdress, ETravelType::TRAVEL_Absolute);
	}
}

void UMultiplayerGameInstance::CreateServer()
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

	SessionInterface->CreateSession(0, FName("My session"), sessionSettings);

}

void UMultiplayerGameInstance::JoinServer()
{
	UE_LOG(LogTemp, Warning, TEXT("JoinServer"));

	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
		sessionSearch->bIsLanQuery = false;
	else
		sessionSearch->bIsLanQuery = true;
	
	sessionSearch->MaxSearchResults = 10000;
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}
