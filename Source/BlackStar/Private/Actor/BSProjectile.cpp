#include "Actor/BSProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/BSBaseAttributeSet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayCueInterface.h"
#include "NiagaraComponent.h"
#include "Utility/BSGameplayTags.h"

ABSProjectile::ABSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(CollisionComponent);
	NiagaraComponent->bAutoActivate = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = LifeSpan;
}

void ABSProjectile::InitProjectile(UAbilitySystemComponent* InSourceASC, float InDamageMultiplier)
{
	SourceASC = InSourceASC;
	DamageMultiplier = InDamageMultiplier;
}

void ABSProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed;
	}

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ABSProjectile::OnHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABSProjectile::OnOverlapBegin);

		if (AActor* OwnerActor = GetOwner())
		{
			CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		}
	}
}

void ABSProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	bHasHit = true;
	ExecuteCue(HitCueTag, OtherActor, Hit);
	Destroy();
}

void ABSProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasHit || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	bHasHit = true;
	ApplyDamageToTarget(OtherActor, SweepResult);
	ExecuteCue(OverlapCueTag, OtherActor, SweepResult);
	Destroy();
}

void ABSProjectile::ApplyDamageToTarget(AActor* TargetActor, const FHitResult& HitResult)
{
	if (!TargetActor || !DamageEffect)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	UAbilitySystemComponent* SpecSourceASC = SourceASC ? SourceASC.Get() : TargetASC;
	FGameplayEffectContextHandle ContextHandle = SpecSourceASC->MakeEffectContext();
	ContextHandle.AddHitResult(HitResult);
	ContextHandle.AddInstigator(GetOwner(), this);

	FGameplayEffectSpecHandle SpecHandle = SpecSourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	float FinalDamage = BaseDamage;
	if (SourceASC)
	{
		if (const UBSBaseAttributeSet* SourceAttributeSet = SourceASC->GetSet<UBSBaseAttributeSet>())
		{
			FinalDamage *= 1.0f + SourceAttributeSet->GetAttackPower() / 100.0f;
		}
	}
	FinalDamage *= DamageMultiplier;

	SpecHandle.Data->SetSetByCallerMagnitude(BSGameplayTags::Data_Damage, -FinalDamage);
	SpecSourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void ABSProjectile::ExecuteCue(FGameplayTag CueTag, AActor* TargetActor, const FHitResult& HitResult)
{
	if (!CueTag.IsValid() || !SourceASC)
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = HitResult.ImpactPoint;
	CueParameters.Normal = HitResult.ImpactNormal;
	CueParameters.PhysicalMaterial = HitResult.PhysMaterial;
	CueParameters.EffectCauser = this;
	CueParameters.Instigator = GetOwner();
	CueParameters.SourceObject = TargetActor;
	SourceASC->ExecuteGameplayCue(CueTag, CueParameters);
}
