#include "PatrolPath.h"

APatrolPath::APatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;

}

FVector APatrolPath::GetPatrolPoint(int32 Index) const
{
    if (PatrolPoints.IsValidIndex(Index))
    {
        return PatrolPoints[Index];
    }
    return FVector::ZeroVector;
}

int32 APatrolPath::GetNumPoints() const
{
    return PatrolPoints.Num();
}

