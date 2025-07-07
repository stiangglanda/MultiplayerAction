#include "PatrolPath.h"

APatrolPath::APatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;

}

FVector APatrolPath::GetPatrolPoint(int32 Index) const
{
    if (PatrolPoints.IsValidIndex(Index))
    {
        return GetActorTransform().TransformPosition(PatrolPoints[Index]);
    }
    return FVector::ZeroVector;
}

int32 APatrolPath::GetNumPoints() const
{
    return PatrolPoints.Num();
}

void APatrolPath::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    for (int32 i = 0; i < PatrolPoints.Num(); ++i)
    {
        FVector WorldPoint = GetActorTransform().TransformPosition(PatrolPoints[i]);
        DrawDebugSphere(GetWorld(), WorldPoint, 32.f, 12, FColor::Green, false, -1.f, 0, 2.f);

        // Draw line to next point (looped)
        if (PatrolPoints.Num() > 1)
        {
            FVector NextWorldPoint = GetActorTransform().TransformPosition(PatrolPoints[(i + 1) % PatrolPoints.Num()]);
            DrawDebugLine(GetWorld(), WorldPoint, NextWorldPoint, FColor::Green, false, -1.f, 0, 2.f);
        }
    }
}
