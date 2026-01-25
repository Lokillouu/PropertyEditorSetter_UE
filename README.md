# Property Editor Setter

## Overview

**Property Editor Setter** is an Unreal Engine editor module that ensures surface (top-level) property setters are invoked when any nested property is modified in the Details panel, including properties inside structs, instanced structs, arrays, maps, and sets.

## Problem Statement

In Unreal Engine, properties can be marked with the `Setter` specifier to define a setter function. However, **Unreal's property editor is inconsistent about invoking these setters**:

- Some container types (arrays, maps, sets) may invoke setters.
- Many property types **never invoke their setters**.
- Even when supported, not all edit actions trigger setter calls.

**There's no reliable pattern - it varies by property type and edit context.**

This means that even though your `UPROPERTY` has valid `Setter` specifier, **the setter may never be called** when editing in the Details panel. This leads to:
- Validation logic being bypassed.
- Dependent properties not updating.
- Inconsistent object state.

## Solution

This module **takes complete control** of setter invocation by:

1. **Intercepting ALL property edits** in the editor via `OnPreObjectPropertyChanged` / `OnObjectPropertyChanged` delegates.
2. **Capturing** the original value of any surface property that has a valid setter.
3. **Allowing** the engine to apply the change normally.
4. **Restoring** the original value temporarily.
5. **Manually invoking** the property setter with the new value.
6. **Cleaning up** all allocated memory.

The module provides **consistent, predictable setter invocation** across all property types and edit scenarios.

## Key Features

### Automatic Setter Invocation
- Intercepts **all property edits** in the editor.
- Manually calls setters for **any property type** with valid `Setter` specifier.
- Works regardless of nesting (structs, instanced structs, containers, containers of containers, etc).
- Works for **all edit actions** (direct edit, add, remove, reset to default, etc).

### Performance Tracking
- Measures setter execution time via custom stat: `STAT_PropertyEditorSetter`.
- Useful for identifying performance bottlenecks in setter functions.

### Configurable Settings
Settings accessible via **Project Settings → Property Setter Settings**:

| Setting | Description | Default |
|---------|-------------|---------|
| **Force Call Setters on Property Edit** | Enable/disable automatic setter invocation | `true` |
| **Log Surface Properties** | Debug logging of property changes | `false` |
| **Force Call Setters Blacklist** | Classes excluded from automatic setter calls (with inheritance toggle) | Empty |

### Console Variables
```cpp
Editor.ForceCallSettersOnPropertyEdit  // Toggle feature on/off
Editor.LogSurfaceProperties            // Toggle debug logging
```

### Class Blacklist
Allows excluding specific classes from automatic setter invocation:
- Can exclude exact classes only.
- Can exclude classes and all subclasses.
- Useful for performance-sensitive or incompatible classes.

## Use Example

```cpp
UPROPERTY(EditAnywhere, Setter = "SetMyStruct")
FMyStructure MyStruct;

UFUNCTION(BlueprintCallable)
void SetMyStruct(FMyStructure& MyNewStruct);
```

MyStruct will have its setter called, where data validation, sanitization and even rejection can happen. Your code is the limit!
Note: Setters need to be BlueprintCallable!

## Architecture

### Module Structure

```
PropertyEditorSetter/
├── PropertyEditorSetterModule.h/cpp    // Core module implementation
├── PropertyEditorSetter.h/cpp          // Helper utilities and namespace
├── PropertyEditorSetterSettings.h/cpp  // Configuration settings
└── PropertyEditorSetter.Build.cs       // Build configuration
```

## Workflow

### Pre-Change (OnPreObjectPropertyChanged)
1. Validates the object and property chain.
2. Extracts the **changing property** (tail) and **surface property** (head).
3. Checks if the surface property has a setter.
4. Verifies the object's class isn't blacklisted.
5. Allocates memory and captures the current surface property value.
6. Stores a data instance for post-change processing.

### Post-Change (OnPostObjectPropertyChanged)
1. Finds the matching cached instance.
2. Captures the new (modified) value of the surface property.
3. Restores the original value to the surface property.
4. Calls the surface property's setter with the new value.
5. Measures execution time for performance tracking.
6. Cleans up memory and removes the instance.

## Engine Compatibility

| Engine Version | Support | Notes |
|----------------|---------|-------|
| UE 4.27+ | ✅ | Requires StructUtils module |
| UE 5.0 - 5.4 | ✅ | Requires StructUtils module |
| UE 5.5+ | ✅ | Uses CoreUObject for InstancedStruct |

The build configuration automatically adjusts module dependencies based on engine version.

## Considerations

- Only works in the editor (not at runtime).
- Requires properties to have `Setter` specifier.
- Some performance overhead (allocates/deallocates temporary snapshots for each property edit, copies data a few times per edit). Not noticeable in most cases, but worth noting.

## Credits

Created by Jorge David López Caraballo
GitHub: [Lokillouu](https://github.com/Lokillouu)

## License

All rights reserved to Jorge David López Caraballo
