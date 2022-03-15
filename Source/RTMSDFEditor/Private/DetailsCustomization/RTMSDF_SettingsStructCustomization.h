// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

class FRTMSDF_SettingsStructCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> structPropertyHandle, FDetailWidgetRow& headerRow, IPropertyTypeCustomizationUtils& structCustomizationUtils) override {}
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> structPropertyHandle, IDetailChildrenBuilder& structBuilder, IPropertyTypeCustomizationUtils& structCustomizationUtils) override;
};