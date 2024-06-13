// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthWidget.h"
#include "Components/ProgressBar.h"

void UEnemyHealthWidget::NativeConstruct(){
	Super::NativeConstruct();
}

void UEnemyHealthWidget::SetHealthPercent(float Delta){
	HP->SetPercent(Delta);
}
