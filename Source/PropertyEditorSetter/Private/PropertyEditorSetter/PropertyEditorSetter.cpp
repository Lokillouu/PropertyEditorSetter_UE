// All rights reserved to Jorge David López Caraballo (https://github.com/Lokillouu)

#include "PropertyEditorSetter/PropertyEditorSetter.h"
#include "UObject/UnrealTypePrivate.h"
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27) || (ENGINE_MAJOR_VERSION == 5 && (ENGINE_MINOR_VERSION >= 0 && ENGINE_MINOR_VERSION < 5))
#include "InstancedStruct.h"
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
#include "CoreUObject/InstancedStruct.h"
#endif

/* NAMESPACE */

namespace PropertyEditorSetter {
	bool bForceCallSettersOnPropertyEdit = PROPERTYEDITORSETTER_DEFAULT_FLAG_ENABLED;
	bool bLogSurfaceProperties = PROPERTYEDITORSETTER_DEFAULT_FLAG_DEBUG;
	TMap<TSubclassOf<UObject>, bool> ForceCallSettersBlacklist = PROPERTYEDITORSETTER_DEFAULT_BLACKLIST;
}

/* INIT */

UPropertyEditorSetterHelper::UPropertyEditorSetterHelper() { return; }

/* INTERFACE */

bool UPropertyEditorSetterHelper::EnsurePropertyContainsChild(FProperty* ParentProp, FProperty* ChildProp) {
	// Validate input properties to ensure neither is null.
	if (!ParentProp || !ChildProp) { return false; }

	// Get the parent property owner and validate it.
	UObject* PropertyOwner = ParentProp->GetOwnerUObject();
	if (!PropertyOwner) { ensureAlways(0); return false; }

	// If the parent is a struct, search its fields.
	if (FStructProperty* StructProp = CastField<FStructProperty>(ParentProp)) {
		// Get the script struct from the property.
		const UScriptStruct* ScriptStruct = StructProp->Struct;
	
		// If the struct is an FInstancedStruct, get the script struct of the actually instanced struct.
		if (ScriptStruct->IsChildOf(FInstancedStruct::StaticStruct())) {
			// Get a pointer to the current value and validate it.
			FInstancedStruct* ValueAddress = StructProp->ContainerPtrToValuePtr<FInstancedStruct>(PropertyOwner);
			if (!ValueAddress) { return false; }
			
			// Get the script struct of the actually instanced struct. Will be null if the FInstancedStruct is uninitialized.
			ScriptStruct = ValueAddress->GetScriptStruct();
			if (!ScriptStruct) { return false; }
		}

		// Search the struct fields.
		for (TFieldIterator<FProperty> It(StructProp->Struct); It; ++It)
		{ if (*It == ChildProp || EnsurePropertyContainsChild(*It, ChildProp)) { return true; }}
	}

	// If the parent is an array, search its inner property.
	if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(ParentProp))
	{ return ArrayProp->Inner == ChildProp || EnsurePropertyContainsChild(ArrayProp->Inner, ChildProp); }

	// If the parent is a map, search its key/value properties.
	if (FMapProperty* MapProp = CastField<FMapProperty>(ParentProp)) {
		return (MapProp->KeyProp == ChildProp || MapProp->ValueProp == ChildProp) ||
		(EnsurePropertyContainsChild(MapProp->KeyProp, ChildProp) || EnsurePropertyContainsChild(MapProp->ValueProp, ChildProp));
	}
			
	// If the parent is a set, search its element property.
	if (FSetProperty* SetProp = CastField<FSetProperty>(ParentProp))
	{ return SetProp->ElementProp == ChildProp || EnsurePropertyContainsChild(SetProp->ElementProp, ChildProp); }
			
	// Else, fail.
	return false;
}

FProperty* UPropertyEditorSetterHelper::FindSurfaceProperty(UClass* OwnerClass, FProperty* Property) {
	// Validate input.
	if (!OwnerClass || !Property) { ensureAlways(0); return nullptr; }

	// Create a pointer to store the the surface property (property directly owned by the object).
	FProperty* SurfaceProperty = nullptr;

	// Iterate all the class surface properties looking for the property.
	for (TFieldIterator<FProperty> It(OwnerClass); It; ++It) { if (*It == Property || EnsurePropertyContainsChild(*It, Property)) { SurfaceProperty = *It; break; } }
	
	// Return the surface property.
	return SurfaceProperty;
}
