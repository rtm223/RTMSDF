// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "URTMSDF_EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

TArray<UClass*> UURTMSDF_EditorUtilityLibrary::FindBlueprintClasses(const FString& searchLocation, const UClass* blueprintClass)
{
	if(!blueprintClass)
		return {};

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FString referenceDirectory = FPaths::GetPath(searchLocation);

	// Form a filter from the paths
	FARFilter filter;
	filter.bRecursivePaths = true;
	filter.bRecursiveClasses = true;
	filter.PackagePaths.Emplace(*referenceDirectory);
	filter.ClassPaths.Add(FTopLevelAssetPath(UBlueprint::StaticClass()));

	TArray<FAssetData> assetList;
	AssetRegistryModule.Get().GetAssets(filter, assetList);

	TArray<UClass*> blueprints;
	for(auto& assetData : assetList)
	{
		if(auto* bp = Cast<UBlueprint>(assetData.FastGetAsset(true)))
		{
			if(bp->bGenerateAbstractClass)
				continue;

			if(bp->GeneratedClass->IsChildOf(blueprintClass))
				blueprints.Add(bp->GeneratedClass);
		}
	}
	return blueprints;
}

void UURTMSDF_EditorUtilityLibrary::SetUtilityWidgetWindowSize(UEditorUtilityWidget* widget, const FVector2D& size)
{
	if(!widget)
		return;

	UEditorUtilitySubsystem* euSS = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	if(!euSS)
		return;

	auto* widgetClass = widget->GetClass();
	for(const auto& [tabId, tabBlueprint] : euSS->RegisteredTabs)
	{
		if(!tabBlueprint || tabBlueprint->GeneratedClass != widgetClass)
			continue;

		FLevelEditorModule& levelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		TSharedPtr<FTabManager> tabManager = levelEditorModule.GetLevelEditorTabManager();
		TSharedPtr<SDockTab> tab = tabManager ? tabManager->FindExistingLiveTab(tabId) : nullptr;
		//TSharedPtr<SDockTab> tab = tabManager ? tabManager->TryInvokeTab(tabId) : nullptr;
		TSharedPtr<SWindow> window = tab ? tab->GetParentWindow() : nullptr;
		if(!window || window == FGlobalTabmanager::Get()->GetRootWindow())
			continue;

		const ETabRole tabRole = tab->GetTabRole();
		if(tabRole == MajorTab || tabRole == NomadTab || tabRole == PanelTab)
			window->Resize(size);
	}
}
