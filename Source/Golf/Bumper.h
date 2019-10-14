// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bumper.generated.h"

UCLASS()
class GOLF_API ABumper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABumper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Visible Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Bumper;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent * Colider;
	
	// Designer Functionality
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
	float BouncePower;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
