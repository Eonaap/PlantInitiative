// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PLANT_API UMultiplayerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMultiplayerGameInstance();

protected:
	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName serverName, bool succeeded);

	virtual void OnFindSessionComplete(bool succeeded);

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);

	UFUNCTION(BlueprintCallable)
		void CreateServer();

	UFUNCTION(BlueprintCallable)
		void JoinServer();

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> sessionSearch;
};
