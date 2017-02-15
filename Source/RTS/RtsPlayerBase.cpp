
#include "RTS.h"
#include "RtsPlayerBase.h"


// Sets default values
ARtsPlayerBase::ARtsPlayerBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RotLock = true;

	Sensitivity = 2;
	LowerLimit = -90;
	UpperLimit = 90;

	////
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	
	SetRootComponent(Collision);

	Camera->SetupAttachment(Collision);

	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_Pawn);

	TraceChannel = ECC_Camera;
}

// Called every frame
void ARtsPlayerBase::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	APlayerController *pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FHitResult hit;
	pc->GetHitResultUnderCursor(TraceChannel, false, hit);
	Process(DeltaTime,hit);
}

void ARtsPlayerBase::moveX(float axis) {
	AddMovementInput(Camera->GetForwardVector(),axis);
}

void ARtsPlayerBase::moveY(float axis) {
	AddMovementInput(Camera->GetRightVector(), axis);
}

void ARtsPlayerBase::moveZ(float axis) {
	AddMovementInput(Camera->GetUpVector(), axis);
}

void ARtsPlayerBase::mouseX(float axis) {
	if (!RotLock)
		AddActorLocalRotation(FRotator(0, (axis * Sensitivity), 0));
}

void ARtsPlayerBase::mouseY(float axis) {
	if (!RotLock) {
		Camera->SetRelativeRotation(FRotator(FMath::ClampAngle(Camera->RelativeRotation.Pitch + (axis * Sensitivity),LowerLimit,UpperLimit),0,0));
	}
}

// Called to bind functionality to input
void ARtsPlayerBase::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ARtsPlayerBase::moveX);
	InputComponent->BindAxis("MoveRight", this, &ARtsPlayerBase::moveY);
	InputComponent->BindAxis("MoveUp", this, &ARtsPlayerBase::moveZ);

	InputComponent->BindAxis("MouseRight", this, &ARtsPlayerBase::mouseX);
	InputComponent->BindAxis("MouseUp", this, &ARtsPlayerBase::mouseY);
}