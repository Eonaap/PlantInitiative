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
	GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("CreateSession completed"));

	if (succeeded)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("Succeeded"));
		GetWorld()->ServerTravel("/Game/EnvMapBlockout_0");
		GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("YaYEEET"));
	}
}

void UMultiplayerGameInstance::OnFindSessionComplete(bool succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete, Succeeded: %d"), succeeded);
	GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("OnFindSessionComplete, Succeeded: "));
	if (succeeded)
	{
		TArray<FOnlineSessionSearchResult> searchResults = sessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Searchresults, server count: %d"), searchResults.Num());
		GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("Searchresults, server count: ") + FString::FromInt(searchResults.Num()));
		//Join first server (0)
		if (searchResults.Num()) {
			GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("Joining session"));
			SessionInterface->JoinSession(0, "My session", searchResults[0]);
			GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("Does this work?  ") + searchResults[0].GetSessionIdStr());
		}
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
	GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Red, TEXT("CreatingServer"));

	FOnlineSessionSettings sessionSettings;
	sessionSettings.bAllowJoinInProgress = true;
	sessionSettings.bIsDedicated = false;
	GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("Seetings created"));

	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL") {
		GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("LanMatch is false"));
		sessionSettings.bIsLANMatch = false;
	}
		
	else {
		GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("LanMatch is true"));
		sessionSettings.bIsLANMatch = true;
	}
		

	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bAllowInvites = true;
	sessionSettings.NumPublicConnections = 5;

	SessionInterface->CreateSession(0, FName("My session"), sessionSettings);

}

void UMultiplayerGameInstance::JoinServer()
{
	UE_LOG(LogTemp, Warning, TEXT("JoinServer")); 
	GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("JoiningServer"));

	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL") {
		sessionSearch->bIsLanQuery = false;
	}
	else {
		sessionSearch->bIsLanQuery = true;
	}
	
	sessionSearch->MaxSearchResults = 10000;
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
	GEngine->AddOnScreenDebugMessage(-1, 15.5f, FColor::Red, TEXT("SessionInterface filled"));
}
