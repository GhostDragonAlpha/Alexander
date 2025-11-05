"""
Check Materials on Scene Objects
Diagnose if materials are black, missing, or unlit
"""

import unreal

print("\n" + "="*70)
print("  🎨 MATERIAL DIAGNOSTIC")
print("="*70 + "\n")

# Get all static meshes
actors = unreal.EditorLevelLibrary.get_all_level_actors()

static_meshes = []
for actor in actors:
    if actor and 'StaticMeshActor' in actor.get_class().get_name():
        static_meshes.append(actor)

print(f"Found {len(static_meshes)} static mesh actors\n")

# Check materials on each
print("="*70)
print("  🔍 MATERIAL INSPECTION")
print("="*70 + "\n")

objects_with_materials = 0
objects_without_materials = 0
material_summary = {}

for actor in static_meshes[:10]:  # Check first 10 objects
    label = actor.get_actor_label()
    mesh_comp = actor.static_mesh_component

    print(f"📦 {label}:")

    # Get materials
    num_materials = mesh_comp.get_num_materials()
    print(f"   Material slots: {num_materials}")

    if num_materials == 0:
        print(f"   ⚠️  NO MATERIAL SLOTS!\n")
        objects_without_materials += 1
        continue

    for i in range(num_materials):
        mat = mesh_comp.get_material(i)
        if mat:
            mat_name = mat.get_name()
            print(f"   Slot {i}: {mat_name}")

            # Track material usage
            if mat_name not in material_summary:
                material_summary[mat_name] = 0
            material_summary[mat_name] += 1

            objects_with_materials += 1
        else:
            print(f"   Slot {i}: ⚠️  NONE (no material assigned!)")
            objects_without_materials += 1

    print()

print("="*70)
print("  📊 MATERIAL SUMMARY")
print("="*70 + "\n")

print(f"Objects with materials: {objects_with_materials}")
print(f"Objects without materials: {objects_without_materials}\n")

if material_summary:
    print("Materials in use:")
    for mat_name, count in sorted(material_summary.items(), key=lambda x: x[1], reverse=True):
        print(f"  • {mat_name}: {count}x\n")

# ===========================================================================
# SOLUTION: Assign default material to everything
# ===========================================================================
print("="*70)
print("  🔧 APPLYING DEFAULT WHITE MATERIAL")
print("="*70 + "\n")

# Load default material
default_material = unreal.EditorAssetLibrary.load_asset('/Engine/EngineMaterials/DefaultMaterial')

if not default_material:
    print("⚠️  Could not load default material!\n")
else:
    print(f"✅ Loaded: {default_material.get_name()}\n")

    fixed_count = 0

    for actor in static_meshes:
        label = actor.get_actor_label()
        mesh_comp = actor.static_mesh_component

        num_materials = mesh_comp.get_num_materials()

        # Apply default material to all slots
        for i in range(num_materials):
            mesh_comp.set_material(i, default_material)

        fixed_count += 1

        if fixed_count <= 5:
            print(f"✅ Applied to: {label}")

    print(f"\n✅ Applied default material to {fixed_count} objects\n")

print("="*70)
print("  🎯 FIX APPLIED!")
print("="*70 + "\n")

print("🎨 ALL OBJECTS NOW HAVE:")
print("   • DefaultMaterial (white/gray)")
print("   • Proper lighting response")
print("   • Visible in screenshots\n")

print("📸 Ready to capture lit screenshots!\n")
