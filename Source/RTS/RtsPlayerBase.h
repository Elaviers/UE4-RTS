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
	
	

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	//////////////////////////////////////
	UFUNCTION(Server,Unreliable,WithValidation) 
		void SendTransformToServer(const FTransform &t,const float camPitch);

	UPROPERTY(EditAnywhere) USphereComponent *Collision;
	UPROPERTY(EditAnywhere) UCameraComponent *Camera;
	UPROPERTY(EditAnywhere) UFloatingPawnMovement *Movement;
	
	UPROPERTY(EditAnywhere) float MoveSpeed; //Speed in cm/s
	UPROPERTY(EditAnywhere, Category = "Camera") float Sensitivity;
	UPROPERTY(EditAnywhere, Category = "Camera") float UpperLimit;
	UPROPERTY(EditAnywhere, Category = "Camera") float LowerLimit;

	UPROPERTY(EditAnywhere) TEnumAsByte<ECollisionChannel> TraceChannel;

	UFUNCTION(BlueprintImplementableEvent) void Process(const float DeltaSeconds, const FHitResult& Hit);
	UFUNCTION(BlueprintCallable, Category = "Game|Player") void SetLockRotation(bool Value) { RotLock = Value; };

	void moveX(float);
	void moveY(float);
	void moveZ(float);
	void mouseX(float);
	void mouseY(float);
};
