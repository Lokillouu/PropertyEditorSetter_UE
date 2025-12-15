// All rights reserved to Jorge David López Caraballo (https://github.com/Lokillouu)

#include "PropertyEditorSetter/PropertyEditorSetterSettings.h"
#include "PropertyEditorSetter/PropertyEditorSetter.h"
#include "HAL/IConsoleManager.h"

/* INIT */

UPropertyEditorSetterSettings::UPropertyEditorSetterSettings() {
	// Set inherited config.
	SectionName = "Property Editor Setter";

	// Set default config.
	bForceCallSettersOnPropertyEdit = PROPERTYEDITORSETTER_DEFAULT_FLAG_ENABLED;
	bLogSurfaceProperties = PROPERTYEDITORSETTER_DEFAULT_FLAG_DEBUG;
	ForceCallSettersBlacklist = PROPERTYEDITORSETTER_DEFAULT_BLACKLIST;

	return;
}

/* INTERNAL */

#if WITH_EDITOR
void UPropertyEditorSetterSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) {
	// Get the changed property and validate it.
	FProperty* Property = PropertyChangedEvent.Property;
	if (!Property) { return; }

	// Get the name of the property.
	FName PropertyName = Property->GetFName();

	// Update the enable flag variable if it changed.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPropertyEditorSetterSettings, bForceCallSettersOnPropertyEdit)) {
		// Update the runtime variable.
		PropertyEditorSetter::bForceCallSettersOnPropertyEdit = bForceCallSettersOnPropertyEdit;
		
		// Also update the console variable for consistency.
		IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Editor.ForceCallSettersOnPropertyEdit"));
		if (CVar) { CVar->Set(bForceCallSettersOnPropertyEdit ? 1 : 0, ECVF_SetByCode); }
	}

	// Update the debug flag variable if it changed.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPropertyEditorSetterSettings, bLogSurfaceProperties)) {
		// Update the runtime variable.
		PropertyEditorSetter::bLogSurfaceProperties = bLogSurfaceProperties;
		
		// Also update the console variable for consistency.
		IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Editor.LogSurfaceProperties"));
		if (CVar) { CVar->Set(bLogSurfaceProperties ? 1 : 0, ECVF_SetByCode); }
	}

	// Update the blacklist variable if it changed.
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPropertyEditorSetterSettings, ForceCallSettersBlacklist))
	{ PropertyEditorSetter::ForceCallSettersBlacklist = ForceCallSettersBlacklist; }

	return;
}
#endif
