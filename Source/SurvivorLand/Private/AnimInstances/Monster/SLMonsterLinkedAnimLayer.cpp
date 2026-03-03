// Ean Sullivan All Rights Reserved


#include "AnimInstances/Monster/SLMonsterLinkedAnimLayer.h"
#include "AnimInstances/Monster/SLMonsterAnimInstance.h"

USLMonsterAnimInstance* USLMonsterLinkedAnimLayer::GetMonsterAnimInstance() const
{
	return GetTypedOuter<USLMonsterAnimInstance>();
}
