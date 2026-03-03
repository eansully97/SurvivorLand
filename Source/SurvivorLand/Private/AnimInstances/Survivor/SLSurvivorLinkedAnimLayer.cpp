// Ean Sullivan All Rights Reserved


#include "AnimInstances/Survivor/SLSurvivorLinkedAnimLayer.h"
#include "AnimInstances/Survivor/SLSurvivorAnimInstance.h"


USLSurvivorAnimInstance* USLSurvivorLinkedAnimLayer::GetSurvivorAnimInstance() const
{
	return GetTypedOuter<USLSurvivorAnimInstance>();
}
