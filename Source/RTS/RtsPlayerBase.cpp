
#include "RTS.h"
#include "RtsPlayerBase.h"


// Sets default values
ARtsPlayerBase::ARtsPlayerBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RotLock = true;

	MoveSpeed = 250;
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

	Collision->SetIsReplicated(true);
	Camera->SetIsReplicated(true);

	TraceChannel = ECC_Camera;
}

void ARtsPlayerBase::SendTransformToServer_Implementation(const FTransform &t, const float camPitch) {
	SetActorTransform(t);
	Camera->SetRelativeRotation(FRotator(camPitch,0,0));
}

bool ARtsPlayerBase::SendTransformToServer_Validate(const FTransform &t,const float c) {
	return true;
}

// Called when the game starts or when spawned
void ARtsPlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARtsPlayerBase::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (IsLocallyControlled())SendTransformToServer(GetActorTransform(),Camera->RelativeRotation.Pitch);

	APlayerController *pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FHitResult hit;
	pc->GetHitResultUnderCursor(TraceChannel, false, hit);
	Process(DeltaTime,hit);
}

void ARtsPlayerBase::moveX(float axis) {
	AddMovementInput(Camera->GetForwardVector(),axis * MoveSpeed);
}

void ARtsPlayerBase::moveY(float axis) {
	AddMovementInput(Camera->GetRightVector(), axis * MoveSpeed);
}

void ARtsPlayerBase::moveZ(float axis) {
	AddMovementInput(Camera->GetUpVector(), axis * MoveSpeed);
}

void ARtsPlayerBase::mouseX(float axis) {
	if (!RotLock)
		AddActorLocalRotation(FRotator(0, (axis * Sensitivity * GetWorld()->DeltaTimeSeconds), 0));
}

void ARtsPlayerBase::mouseY(float axis) {
	if (!RotLock) {
		Camera->SetRelativeRotation(FRotator(FMath::ClampAngle(Camera->RelativeRotation.Pitch + (axis * Sensitivity * GetWorld()->DeltaTimeSeconds),LowerLimit,UpperLimit),0,0));
	}
}

// Called to bind functionality to input
void ARtsPlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ARtsPlayerBase::moveX);
	InputComponent->BindAxis("MoveRight", this, &ARtsPlayerBase::moveY);
	InputComponent->BindAxis("MoveUp", this, &ARtsPlayerBase::moveZ);

	InputComponent->BindAxis("MouseRight", this, &ARtsPlayerBase::mouseX);
	InputComponent->BindAxis("MouseUp", this, &ARtsPlayerBase::mouseY);
}