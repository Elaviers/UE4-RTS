#pragma once

#include "GameFramework/PlayerController.h"
#include "RtsPlayerControllerBase.generated.h"

UCLASS()
class RTS_API ARtsPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

	private:
		bool _locked,_hide;
		FVector2D lockedMousePosition;
	public:
		virtual void Tick(float deltaTime) override;

		UFUNCTION(BlueprintCallable, Category = "Game|Player")
			void SetMouseLocked(bool LockMouse, bool hide);
};
