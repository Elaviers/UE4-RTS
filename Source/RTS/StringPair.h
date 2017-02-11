#pragma once

#include "StringPair.generated.h"

USTRUCT(BlueprintType)
struct FStringPair {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	FString Property;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Value;
};