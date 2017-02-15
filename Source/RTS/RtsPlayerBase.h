// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "RtsPlayerBase.generated.h"

UCLASS()
class RTS_API ARtsPlayerBase : public APawn
{
	GENERATED_BODY()
protected:
	bool RotLock;
public:
	// Sets default values for this pawn's properties
	ARtsPlayerBase();
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	//////////////////////////////////////
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USphereComponent *Collision;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UCameraComponent *Camera;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UFloatingPawnMovement *Movement;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera") float Sensitivity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera") float UpperLimit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera") float LowerLimit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TEnumAsByte<ECollisionChannel> TraceChannel;

	UFUNCTION(BlueprintImplementableEvent) void Process(const float DeltaSeconds, const FHitResult& Hit);
	UFUNCTION(BlueprintCallable, Category = "Game|Player") void SetLockRotation(bool Value) { RotLock = Value; };

	void moveX(float);
	void moveY(float);
	void moveZ(float);
	void mouseX(float);
	void mouseY(float);
};
