#include "RTS.h"
#include "RtsPlayerControllerBase.h"

void ARtsPlayerControllerBase::Tick(float dt) {
	Super::Tick(dt);

	if (_locked) {
		if (_hide && bShowMouseCursor) { //If the curson should be hidden but it's visible
			float tempx, tempy;
			GetMousePosition(tempx, tempy);
			if (tempx != lockedMousePosition.X && tempy != lockedMousePosition.Y) //If the mouse is being moved from the locked position
				bShowMouseCursor = false;
		}
		SetMouseLocation(lockedMousePosition.X, lockedMousePosition.Y);
	}
}

void ARtsPlayerControllerBase::SetMouseLocked(bool value,bool hide) {
	_locked = value;
	_hide = hide;

	if (_locked)
		GetMousePosition(lockedMousePosition.X,lockedMousePosition.Y);

	if (!hide) {
		bShowMouseCursor = true;
		SetMouseLocation(lockedMousePosition.X, lockedMousePosition.Y);
	}
}