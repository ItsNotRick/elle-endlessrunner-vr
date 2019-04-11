// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DeckUtils.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCard {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Card Struct")
	FString source;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Card Struct")
	FString dest;
};

UCLASS(BlueprintType)
class ENDLESSRUNNERVR_API UDeckUtils : public UObject
{
	GENERATED_BODY()

	public:
		UDeckUtils() {}
		UFUNCTION(BlueprintCallable)
		static TArray<FCard> extractFromCSV(FString path);

	
};


