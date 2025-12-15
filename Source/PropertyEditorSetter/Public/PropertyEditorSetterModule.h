// All rights reserved to Jorge David L�pez Caraballo (https://github.com/Lokillouu)

#pragma once

#include "Modules/ModuleInterface.h"

/* FORWARD DECLARES */

class UObject;
class FEditPropertyChain;

/**
 * @brief Module for tracking and forcing property setter calls in the editor.
 * Captures original property values for surface properties with a valid setter and invokes the setter after changes have been cached and restored.
 */
class FPropertyEditorSetterModule : public IModuleInterface {

	/* TYPEDEFS */

	// Defines a cached instance used to track property edits and force setter calls,
	// storing weak references to the owning object and relevant properties, along
	// with a raw snapshot of the original value.
	typedef TTuple<
		TWeakObjectPtr<UObject> /* OwningObject */,
		TWeakFieldPtr<FProperty> /* ChangingProperty */,
		TWeakFieldPtr<FProperty> /* SurfaceProperty */,
		void* /* OriginalValueSnapshot */
	> FPropertyEditorSetterInstance;

	/* PROPERTIES */

	#pragma region DATA
	private:

	// Delegate handle for the pre-property-change editor callback.
	FDelegateHandle PreHandle;

	// Delegate handle for the post-property-change editor callback.
	FDelegateHandle PostHandle;
	#pragma endregion

	#pragma region REFERENCES
	private:

	// Stores active property edit instances awaiting post-change processing.
	TArray<FPropertyEditorSetterInstance> SetterPropertyInstances;
	#pragma endregion

	/* FUNCTIONALITY */

	#pragma region INTERFACE
	public:

	/**
	 * @brief Binds editor delegates for pre-change and post-change UObject property otifications and stores their handles for later removal.
	 */
    virtual void StartupModule() override;

	/**
	 * @brief Unbinds editor property change delegates, destroys and frees any stored original property value snapshots and clears all stored setter instances.
	 */
    virtual void ShutdownModule() override;
	#pragma endregion

	#pragma region INTERNAL
	private:

	/**
	 * @brief Captures the original value of a surface property with a valid Setter before it is edited in the property editor, forcing setter invocation later on.
	 * @param InObject: The UObject whose property is about to change.
	 * @param PropertyChain: The edit chain describing the property hierarchy.
	 */
    void OnPreObjectPropertyChanged(UObject* InObject, const FEditPropertyChain& PropertyChain);

	/**
	 * @brief Restores the original property value of a captured surface property and manually invokes the property setter with the desired property value.
	 * @param InObject: The UObject whose property was modified.
	 * @param PropertyChangedEvent: Information describing the property change.
	 */
    void OnPostObjectPropertyChanged(class UObject* InObject, FPropertyChangedEvent& PropertyChangedEvent);
	#pragma endregion

};
