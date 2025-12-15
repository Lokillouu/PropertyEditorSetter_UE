// All rights reserved to Jorge David López Caraballo (https://github.com/Lokillouu)

#pragma once

#include "Engine/DeveloperSettings.h"
#include "PropertyEditorSetterSettings.generated.h"

/* FORWARD DECLARES */

class UPropertyEditorSetterHelper;

/**
 * @brief Settings for automatic property setter invocation in the editor.
 * When enabled, properties with Setter metadata will automatically invoke their setter function when modified in the Details panel,
 * even when the property is nested inside structs, instanced structs, arrays, maps or sets.
 * This ensures setter validation logic runs consistently for all property edits.
 */
UCLASS(config=Editor, defaultconfig, MinimalAPI, meta=(DisplayName="Property Setter Settings"))
class UPropertyEditorSetterSettings : public UDeveloperSettings {
	
	GENERATED_BODY()

	/* PROPERTIES */

	#pragma region CONFIG
	private:

	// When enabled, properties with Setter will automatically invoke their setter when modified in the Details panel, even for nested properties.
	UPROPERTY(config, EditAnywhere, meta = (Category = "Property Editor Setter", DisplayName = "Forcefully Call Setters on Property Edit"))
	bool bForceCallSettersOnPropertyEdit;

	// Enables logging of surface properties by name for debugging.
	UPROPERTY(config, EditAnywhere, meta = (Category = "Property Editor Setter", DisplayName = "Log Surface Properties"))
	bool bLogSurfaceProperties;

	// Object classes that will not execute the editor setter logic even if bForceCallSettersOnPropertyEdit is true and whether this applies to subclasses (true) or not (false).
	UPROPERTY(config, EditAnywhere, meta = (Category = "Property Editor Setter", DisplayName = "Force Call Setters Blacklist"))
	TMap<TSubclassOf<UObject>, bool> ForceCallSettersBlacklist;
	#pragma endregion

	/* FUNCTIONALITY */

	#pragma region INIT
	public:

	/**
	 * @brief Default constructor.
	 */
	UPropertyEditorSetterSettings();
	#pragma endregion

	#pragma region GETTERS
	public:

	/**
	 * @brief Gets whether properties with Setter will automatically invoke their setter when modified in the Details panel (even nested properties).
	 * @return Stored bForceCallSettersOnPropertyEdit value.
	 */
	UFUNCTION(BlueprintPure, meta = (Category = "Getters"))
	FORCEINLINE bool GetForceCallSettersOnPropertyEdit() const { return bForceCallSettersOnPropertyEdit; }

	/**
	 * @brief Gets whether logging of surface properties by name for debugging is enabled.
	 * @return Stored bLogSurfaceProperties value.
	 */
	UFUNCTION(BlueprintPure, meta = (Category = "Getters"))
	FORCEINLINE bool GetLogSurfaceProperties() const { return bLogSurfaceProperties; }

	/**
	 * @brief Gets the map of object classes that will not execute the editor setter logic even if bForceCallSettersOnPropertyEdit is true and whether this applies to subclasses (true) or not (false).
	 * @return Stored ForceCallSettersBlacklist map.
	 */
	UFUNCTION(BlueprintPure, meta = (Category = "Getters"))
	FORCEINLINE TMap<TSubclassOf<UObject>, bool> GetForceCallSettersBlacklist() const { return ForceCallSettersBlacklist; }
	#pragma endregion

	#pragma region INTERNAL
	protected:

	#if WITH_EDITOR
	/**
	 * @brief Ensures that runtime flags and console variables are updated whenever relevant UPROPERTY members are changed.
	 * @param PropertyChangedEvent: Struct containing information about the changed property.
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif
	#pragma endregion

};
