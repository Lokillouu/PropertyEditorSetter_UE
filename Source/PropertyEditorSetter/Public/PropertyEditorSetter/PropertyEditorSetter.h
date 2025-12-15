// All rights reserved to Jorge David López Caraballo (https://github.com/Lokillouu)

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

/* DEFINES */

#define PROPERTYEDITORSETTER_DEFAULT_FLAG_ENABLED true
#define PROPERTYEDITORSETTER_DEFAULT_FLAG_DEBUG false
#define PROPERTYEDITORSETTER_DEFAULT_BLACKLIST {}

/* NAMESPACE */

/**
 * @brief Settings for automatic property setter invocation in the editor.
 * When enabled, properties with Setter metadata will automatically invoke their setter function when modified in the Details panel,
 * even when the property is nested inside structs, instanced structs, arrays, maps or sets.
 * This ensures setter validation logic runs consistently for all property edits.
 */
namespace PropertyEditorSetter {
	// When enabled, properties with Setter will automatically invoke their setter when modified in the Details panel, even for nested properties (toggles on the feature).
	extern PROPERTYEDITORSETTER_API bool bForceCallSettersOnPropertyEdit;

	// Enables logging of surface properties by name for debugging.
	extern PROPERTYEDITORSETTER_API bool bLogSurfaceProperties;

	// Object classes that will not execute the editor setter logic even if bForceCallSettersOnPropertyEdit is true and whether this applies to subclasses (true) or not (false).
	extern PROPERTYEDITORSETTER_API TMap<TSubclassOf<UObject>, bool> ForceCallSettersBlacklist;
}

/* CVARS */

// Enable flag console variable.
static FAutoConsoleVariableRef CVarForceCallSettersOnPropertyEdit(
	TEXT("Editor.ForceCallSettersOnPropertyEdit"),
	PropertyEditorSetter::bForceCallSettersOnPropertyEdit,
	TEXT("When enabled, properties with Setter will automatically invoke their setter when modified in the Details panel, even for nested properties.\n")
	TEXT("0: Disabled, 1: Enabled (default)"),
	ECVF_Default
);

// Debug flag console variable.
static FAutoConsoleVariableRef CVarLogSurfaceProperties(
	TEXT("Editor.LogSurfaceProperties"),
	PropertyEditorSetter::bLogSurfaceProperties,
	TEXT("Enables logging of surface properties by name for debugging.\n")
	TEXT("0: Disabled (default), 1: Enabled"),
	ECVF_Default
);

/**
 * @brief A collection of useful methods related to the property editor setter system.
 * This class provides helper functions for working with property hierarchies.
 * Currently, it is not actively used by the core system. Might be deprecated in the future.
 */
class UPropertyEditorSetterHelper {

	/* FUNCTIONALITY */

	#pragma region INIT
	public:

	/*
	 * @brief Default constructor.
	*/
	UPropertyEditorSetterHelper();
	#pragma endregion

	#pragma region INTERFACE
	public:

	/*
	 * @brief Recursive method that checks if a property contains a nested child property.
	 * @param ParentProp: Parent property to check.
	 * @param ChildProp: Child property to look for.
	 * @return True if the child property is in the parent property's hierarchy; false otherwise.
	 */
	static bool EnsurePropertyContainsChild(FProperty* ParentProp, FProperty* ChildProp);

	/**
	 * @brief Finds the surface (top-level) property in a class that owns or contains a given property.
	 * @param OwnerClass: The UClass to search for surface properties.
	 * @param Property: The property for which the owning surface property is being resolved.
	 * @return The surface property that directly owns or contains the given property, or nullptr if none is found.
	 */
	static FProperty* FindSurfaceProperty(UClass* OwnerClass, FProperty* Property);
	#pragma endregion

};
