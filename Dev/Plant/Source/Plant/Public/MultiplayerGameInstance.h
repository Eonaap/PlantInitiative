// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		FString ServerName;
	UPROPERTY(BlueprintReadOnly)
		FString PlayerCountStr;
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 MaxPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex;

	void SetPlayerCount() 
	{
		PlayerCountStr = FString(FString::FromInt(CurrentPlayers) + "/" + FString::FromInt(MaxPlayers));
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerDelegate, FServerInfo, ServerListDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerSearchingDelegate, bool, SearchingForServer);

UCLASS()
class PLANT_API UMultiplayerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMultiplayerGameInstance();

protected:
	FName MySessionName;

	UPROPERTY(BlueprintAssignable)
		FServerDelegate ServerListDelegate;
	
	UPROPERTY(BlueprintAssignable)
		FServerSearchingDelegate SearchingForServer;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName sessionName, bool succeeded);

	virtual void OnFindSessionComplete(bool succeeded);

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);

	UFUNCTION(BlueprintCallable)
		void CreateServer(FString serverName, FString hostName);

	UFUNCTION(BlueprintCallable)
		void FindServers();
	
	UFUNCTION(BlueprintCallable)
		void JoinServer(int32 arrayIndex);

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> sessionSearch;
};
