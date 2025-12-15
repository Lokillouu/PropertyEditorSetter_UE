// All rights reserved to Jorge David L�pez Caraballo (https://github.com/Lokillouu)

#include "PropertyEditorSetterModule.h"
#include "PropertyEditorSetter/PropertyEditorSetter.h"
#include "PropertyEditorSetter/PropertyEditorSetterSettings.h"

// Mandatory Unreal's module implementation.
IMPLEMENT_MODULE(FPropertyEditorSetterModule, PropertyEditorSetter)

// Declares a custom float stat for tracking the execution time of property setter calls in the editor.
DECLARE_FLOAT_COUNTER_STAT(TEXT("Property Editor Setter"), STAT_PropertyEditorSetter, STATGROUP_Game);

/* INTERFACE */

void FPropertyEditorSetterModule::StartupModule() {
	// Bind OnPreObjectPropertyChanged and OnObjectPropertyChanged and store the binding handles.
	PreHandle = FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddRaw(this, &FPropertyEditorSetterModule::OnPreObjectPropertyChanged);
	PostHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FPropertyEditorSetterModule::OnPostObjectPropertyChanged);

    // Load the property editor setter settings from the default config object and ensure it is valid.
    const UPropertyEditorSetterSettings* PropertyEditorSetterSettings = GetDefault<UPropertyEditorSetterSettings>();
    ensureAlways(PropertyEditorSetterSettings);
	
	// Load settings values to the global PropertyEditorSetter state for runtime use.
	PropertyEditorSetter::bForceCallSettersOnPropertyEdit = PropertyEditorSetterSettings->GetForceCallSettersOnPropertyEdit();
	PropertyEditorSetter::bLogSurfaceProperties = PropertyEditorSetterSettings->GetLogSurfaceProperties();
	PropertyEditorSetter::ForceCallSettersBlacklist = PropertyEditorSetterSettings->GetForceCallSettersBlacklist();

	// Retrieve the console variable for forcing setter calls and validate it.
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Editor.ForceCallSettersOnPropertyEdit"));
	ensureAlways(CVar);

	// Apply the runtime setting to the console variable for forcing setter calls.
	CVar->Set(PropertyEditorSetter::bForceCallSettersOnPropertyEdit ? 1 : 0, ECVF_SetByCode);

	// Retrieve the console variable for logging and validate it.
	CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Editor.LogSurfaceProperties"));
	ensureAlways(CVar);

	// Apply the runtime setting to the console variable for logging.
	CVar->Set(PropertyEditorSetter::bLogSurfaceProperties ? 1 : 0, ECVF_SetByCode);

	return;
}

void FPropertyEditorSetterModule::ShutdownModule() {
	// Unbind OnPreObjectPropertyChanged and OnObjectPropertyChanged.
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.Remove(PreHandle);
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PostHandle);

	// Loop through all setter property instances and free the original value snapshots.
	for (FPropertyEditorSetterInstance& Instance : SetterPropertyInstances) {
		// Get the property the instance represents.
		FProperty* SurfaceProperty = Instance.Get<2>().Get();
    
		// Get the original value snapshot and validate it.
		void* OriginalValue = Instance.Get<3>();
		if (!OriginalValue) { continue; }

		// Call the property's destructor to clean up internal resources if the FProperty is still valid.
		if (SurfaceProperty) { SurfaceProperty->DestroyValue(OriginalValue); }

		// Free the memory allocated for the value.
		FMemory::Free(OriginalValue);
	}

	// Remove all stored instances.
	SetterPropertyInstances.Empty();

	return;
}

/* INTERNAL */

void FPropertyEditorSetterModule::OnPreObjectPropertyChanged(UObject* InObject, const FEditPropertyChain& PropertyChain) {
	// Validate the target object.
	if (!InObject) { return; }
	
	// Cache pointers to the head (surface) and tail (active) nodes of the property edit chain.
	auto* Tail = PropertyChain.GetTail();
	auto* Head = PropertyChain.GetHead();

	// Validate both the surface and active property nodes.
	if (!Head || !Tail) { return; }

	// Retrieve the property that is actively being modified.
	FProperty* ChangingProperty = Tail->GetValue();

	// Retrieve the top-level (surface) property from the property chain.
	FProperty* SurfaceProperty = Head->GetValue();

	// Validate both the changing and surface property pointers.
	if (!ChangingProperty || !SurfaceProperty) { return; }

	// Optionally log the surface property and owning object for debugging purposes.
	if (PropertyEditorSetter::bLogSurfaceProperties) {
		// Resolve readable names for logging, handling invalid surface properties.
		FString ChangingPropertyName = !ChangingProperty ? "Invalid" : ChangingProperty->GetFName().ToString();
		FString SurfacePropertyName = !SurfaceProperty ? "Invalid" : SurfaceProperty->GetFName().ToString();
		FString PropertyOwnerName = InObject->GetFName().ToString();

		// Log the changing and surface properties and owner.
		UE_LOG(
			LogTemp,
			Warning, 
			TEXT("FPropertyEditorSetterModule::OnPreObjectPropertyChanged - ChangingProperty = %s, SurfaceProperty = %s, Owner = %s"),
			*ChangingPropertyName,
			*SurfacePropertyName,
			*PropertyOwnerName
		);
	}

	// If the surface property is valid, has a setter and forced setter execution is enabled, proceed.
	if (SurfaceProperty && SurfaceProperty->HasSetter() && PropertyEditorSetter::bForceCallSettersOnPropertyEdit) {
		// Resolve and validate the class of the modified object.
		UClass* ObjClass = InObject->GetClass();
		if (!ObjClass) { return; }

		// Reference the blacklist used to prevent forced setter calls on specific classes.
		auto& Blacklist = PropertyEditorSetter::ForceCallSettersBlacklist;
		
		// Determine whether the object class is allowed to have setters force-called.
		bool bIsAllowed = true;
		for (auto& BlacklistEntry : Blacklist) {
			// Resolve the blacklisted class from its weak reference.
			UClass* BlacklistClass = BlacklistEntry.Key.Get();
			if (!BlacklistClass) { continue; }

			// Check if the object class is banned directly or via inheritance, based on entry settings.
			bIsAllowed = !(BlacklistEntry.Value ? ObjClass->IsChildOf(BlacklistClass) : ObjClass == BlacklistClass);

			// Stop evaluating once a ban condition is met.
			if (!bIsAllowed) { break; }
		}

		// If the class is allowed, create and store an FPropertyEditorSetterInstance for the property.
		if (bIsAllowed) {
			// Resolve a pointer to the current surface property value within the object.
			const void* ValueAddress = SurfaceProperty->ContainerPtrToValuePtr<void>(InObject);

			// Query the byte size of the property value.
			size_t ValueSize = SurfaceProperty->GetSize();

			// Allocate raw memory to store a snapshot of the original value.
			void* OriginalValueSnapshot = FMemory::Malloc(ValueSize);

			// Initialize the allocated memory as a valid instance of the changing property.
			SurfaceProperty->InitializeValue(OriginalValueSnapshot);

			// Copy the pre-change value into the snapshot buffer.
			SurfaceProperty->CopyCompleteValue(OriginalValueSnapshot, ValueAddress);
			
			// Create an instance by storing weak references and the original value snapshot.
			FPropertyEditorSetterInstance PropertyInstance = {
				MakeWeakObjectPtr<UObject>(InObject),
				MakeWeakFieldPtr<FProperty>(ChangingProperty),
				MakeWeakFieldPtr<FProperty>(SurfaceProperty),
				OriginalValueSnapshot
			};

			// Store the instance for post-change processing.
			SetterPropertyInstances.Add(PropertyInstance);
		}
	}

	return;
}

void FPropertyEditorSetterModule::OnPostObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& PropertyChangedEvent) {
	// Validate object.
	if (!InObject) { return; }

	// Get a pointer to the changed property and validate it.
	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (!PropertyThatChanged) { return; }

	// Cached pointers to the matching surface property, its original value snapshot, and the index of the corresponding setter instance.
	FProperty* SurfaceProperty = nullptr;
	void* OriginalValue = nullptr;
	int32 InstanceID = -1;

	// Iterate over all stored setter property instances to find the matching one.
	for (int32 i = 0; i < SetterPropertyInstances.Num(); i++) {
		// Get a reference to the current instance.
		FPropertyEditorSetterInstance& Instance = SetterPropertyInstances[i];
		
		// Resolve the weak UObject pointer associated with the instance.
		UObject* ObjectPtr = Instance.Get<0>().Get();
		
		// Skip the instance if the object is invalid or does not match the object being modified.
		if (!ObjectPtr || ObjectPtr != InObject) { continue; }

		// Resolve the weak pointer to the property that is currently changing.
		FProperty* ChangingProperty = Instance.Get<1>().Get();
		
		// Skip if the property is invalid or is not the property that triggered the change.
		if (!ChangingProperty || ChangingProperty != PropertyThatChanged) { continue; }

		// Retrieve the surface (target) property that has a setter associated.
		SurfaceProperty = Instance.Get<2>().Get();

		// Retrieve the stored snapshot of the original value for restoration.
		OriginalValue = Instance.Get<3>();

		// Store the index of the matching instance for later use.
		InstanceID = i;
	}

	// If the surface property is invalid, it means the property didn't get cached because it doesn't need a forced Setter call, so abort.
	if (!SurfaceProperty) { return; }

	ensureAlways(OriginalValue != nullptr);

	// Get the size of the value.
	size_t ValueSize = SurfaceProperty->GetSize();

	// Get a raw pointer to the current value.
	void* ValueAddress = SurfaceProperty->ContainerPtrToValuePtr<void>(InObject);

	// Allocate the value size into a post-change value snapshot.
	void* NewValueSnapshot = FMemory::Malloc(ValueSize);
    
	// Initialize the memory as if it's a new instance of the property.
    SurfaceProperty->InitializeValue(NewValueSnapshot);

	// Copy the changed value into the snapshot.
	SurfaceProperty->CopyCompleteValue(NewValueSnapshot, ValueAddress);

    // Restore old value to the object.
    SurfaceProperty->CopyCompleteValue(ValueAddress, OriginalValue);

	// Take a snapshot of the current time to measure setter performance.
	double SetterTime = FPlatformTime::Seconds();

    // Called the property setter manually with the new value.
    SurfaceProperty->CallSetter(InObject, NewValueSnapshot);

	// Calculate time passed since setter was called and convert to ms.
	SetterTime = (FPlatformTime::Seconds() - SetterTime) * 1000.0; 

	// Update the custom stat value for debug.
	SET_FLOAT_STAT(STAT_PropertyEditorSetter, SetterTime);

    // Properly destroy the struct data before freeing memory.
    SurfaceProperty->DestroyValue(OriginalValue);
    SurfaceProperty->DestroyValue(NewValueSnapshot);

    // Free the allocated memory from both snapshots.
    FMemory::Free(OriginalValue);
	FMemory::Free(NewValueSnapshot);

	// Remove the cached property data.
    SetterPropertyInstances.RemoveAt(InstanceID);

	return;
}
