#include "Character/BSNormalEnemyCharacter.h"

#include "AI/BSPatrolRoute.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Controller/BSAIController.h"
#include "GAS/BSAbilitySystemComponent.h"
#include "GAS/BSBaseAttributeSet.h"
#include "UI/NameplateWidget.h"


ABSNormalEnemyCharacter::ABSNormalEnemyCharacter()
{
    AIControllerClass = ABSAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    NameplateWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidgetComponent"));
    NameplateWidgetComponent->SetupAttachment(GetCapsuleComponent());
    NameplateWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    NameplateWidgetComponent->SetDrawSize(NameplateSize);
    NameplateWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABSNormalEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();
    InitializePatrolPoint();
    InitializeNameplate();
}

void ABSNormalEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UBSBaseAttributeSet::GetCurrentHPAttribute()).Remove(CurrentHPDelegateHandle);

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UBSBaseAttributeSet::GetMaxHPAttribute()).Remove(MaxHPDelegateHandle);
    }

    CurrentHPDelegateHandle.Reset();
    MaxHPDelegateHandle.Reset();

    Super::EndPlay(EndPlayReason);
}

// ===== 이름표 =====

void ABSNormalEnemyCharacter::InitializeNameplate()
{
    if (!NameplateWidgetComponent)
    {
        return;
    }

    const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    NameplateWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, CapsuleHalfHeight * NameplateHeight));
    // BP에서 지정한 크기를 실행 시점에도 반영.
    NameplateWidgetComponent->SetDrawSize(NameplateSize);
    if (NameplateWidgetClass)
    {
        NameplateWidgetComponent->SetWidgetClass(NameplateWidgetClass);
        NameplateWidgetComponent->InitWidget();
    }
    RefreshNameplate();

    if (!AbilitySystemComponent)
    {
        return;
    }

    // 초기화를 다시 호출하더라도 델리게이트가 중복 등록되지 않도록 해제.
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UBSBaseAttributeSet::GetCurrentHPAttribute()).Remove(CurrentHPDelegateHandle);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UBSBaseAttributeSet::GetMaxHPAttribute()).Remove(MaxHPDelegateHandle);

    CurrentHPDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UBSBaseAttributeSet::GetCurrentHPAttribute()).AddUObject(this,&ABSNormalEnemyCharacter::OnHPChanged);

    MaxHPDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UBSBaseAttributeSet::GetMaxHPAttribute()).AddUObject(this, &ABSNormalEnemyCharacter::OnHPChanged);
}

void ABSNormalEnemyCharacter::RefreshNameplate()
{
    if (!NameplateWidgetComponent)
    {
        return;
    }

    UNameplateWidget* Nameplate = Cast<UNameplateWidget>(NameplateWidgetComponent->GetUserWidgetObject());

    if (!Nameplate)
    {
        return;
    }

    // EnemyName은 부모 ABSEnemyCharacter에 선언된 값을 사용.
    Nameplate->UpdateName(EnemyName);

    if (AttributeSet)
    {
        Nameplate->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
    }
}

void ABSNormalEnemyCharacter::OnHPChanged(const FOnAttributeChangeData& Data)
{
    RefreshNameplate();
}

void ABSNormalEnemyCharacter::SetLastKnownTargetLocation(const FVector& Location)
{
    LastKnownTargetLocation = Location;
    bHasLastKnownTargetLocation = true;
}

void ABSNormalEnemyCharacter::ClearLastKnownTargetLocation()
{
    LastKnownTargetLocation = FVector::ZeroVector;
    bHasLastKnownTargetLocation = false;
}

bool ABSNormalEnemyCharacter::HasPatrolRoute() const
{
    return IsValid(PatrolRoute) && PatrolRoute->GetNumPatrolPoints() > 0;
}

FVector ABSNormalEnemyCharacter::GetCurrentPatrolPointLocation() const
{
    if (!HasPatrolRoute())
    {
        return GetActorLocation();
    }
    return PatrolRoute->GetPatrolPointLocation(CurrentPatrolPointIndex);
}

void ABSNormalEnemyCharacter::InitializePatrolPoint()
{
    CurrentPatrolPointIndex = 0;
    PatrolDirection = 1;

    if (!HasPatrolRoute())
    {
        return;
    }
    // 배치된 위치에서 가장 가까운 순찰 지점을 시작점으로 사용.
    const int32 ClosestIndex = PatrolRoute->FindClosestPatrolPointIndex(GetActorLocation());
    if (ClosestIndex != INDEX_NONE)
    {
        CurrentPatrolPointIndex = ClosestIndex;
    }
}

void ABSNormalEnemyCharacter::AdvancePatrolPoint()
{
    if (!HasPatrolRoute())
    {
        CurrentPatrolPointIndex = 0;
        PatrolDirection = 1;
        return;
    }

    const int32 PointCount = PatrolRoute->GetNumPatrolPoints();

    if (PointCount <= 1)
    {
        CurrentPatrolPointIndex = 0;
        PatrolDirection = 1;
        return;
    }

    switch (PatrolRoute->GetPatrolMode())
    {
    case EBSPatrolRouteMode::Loop:
        {
            // 마지막 지점 다음에는 첫 지점으로 이동.
            CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PointCount;
            break;
        }
    case EBSPatrolRouteMode::PingPong:
        {
            // 양 끝에 도착하면 진행 방향을 반전.
            CurrentPatrolPointIndex += PatrolDirection;
            if (CurrentPatrolPointIndex >= PointCount)
            {
                CurrentPatrolPointIndex = PointCount - 2;
                PatrolDirection = -1;
            }
            else if (CurrentPatrolPointIndex < 0)
            {
                CurrentPatrolPointIndex = 1;
                PatrolDirection = 1;
            }
            break;
        }
    default:
        break;
    }
}

void ABSNormalEnemyCharacter::OnDeathStarted(AActor* Killer)
{
    if (NameplateWidgetComponent)
    {
        NameplateWidgetComponent->SetVisibility(false);
    }
    Super::OnDeathStarted(Killer);
}