#  File:   export_planet.py
#  Author: Taylor Robbins
#  Date:   09\26\2026
#  Description: 
#  	** This file can be used as a Blender script (developed in Blender 3.6) to
#   ** export the collision data from a model into our own format, as well as
#   ** perform a regular separate file gltf export that can be fed to the
#   ** libdragon mkmodel.exe converter for regular rendering


bl_info = {
	"name": "glTF + Collision Sidecar Export",
	"version": (1, 0, 0),
	"blender": (4, 2, 0),
	"location": "File > Export > glTF + Collision (.gltf)",
	"description": "Builtin glTF (separate) export plus rule-based per-triangle collision sidecar",
	"category": "Import-Export",
}

import json
import math
import os

import bpy
import numpy as np
from bpy.props import BoolProperty, StringProperty
from bpy_extras.io_utils import ExportHelper

# +--------------------------------------------------------------+
# |                            Config                            |
# +--------------------------------------------------------------+
MESHLIKE = {'MESH', 'CURVE', 'SURFACE', 'FONT', 'META'}

SURFACES = ["default", "stone", "wood", "metal", "ice", "water", "grass"]
SURFACE_ID = {n: i for i, n in enumerate(SURFACES)}

F_WALKABLE  = 1 << 0
F_CLIMBABLE = 1 << 1
F_NO_CAMERA = 1 << 2
F_TRIGGER   = 1 << 3
F_ONE_WAY   = 1 << 4
FLAGS = {k[2:]: v for k, v in globals().items() if k.startswith("F_")}

# Extra kwargs passed through to bpy.ops.export_scene.gltf (version-dependent names).
GLTF_KWARGS = {}

SKIP = "skip"  # rule return value: drop triangle

def is_collision_only(obj):
#
	"""Objects exported to sidecar but excluded from glTF."""
	return bool(obj.get("collision_only")) or obj.name.endswith(("_col", "-colonly"))
#

def object_included(src, instancer):
#
	# Object-level filter for collision (cheap, before per-triangle work).
	for obj in (src, instancer):
	#
		if obj is not None and obj.get("collision") == "none":
		#
			return False
		#
	#
	return True
#


# +--------------------------------------------------------------+
# |                     Per-Triangle Context                     |
# +--------------------------------------------------------------+
# Mutable view of the current triangle. Reused per triangle; do not store.
class Tri:
#
	def __init__(self, srcObject, instancer, eval_obj, mesh):
	#
		self.obj = srcObject        # original object (custom props)
		self.instancer = instancer  # original instancing object or None
		self.eval_obj = eval_obj
		self.mesh = mesh            # evaluated mesh (modifiers applied)
		self.index = 0              # loop_triangle index
		self.poly = 0               # source polygon index (face-domain attrs)
		self.material = None
		self.vidx = None            # (3,) vertex indices
		self.verts = None           # (3,3) world space, Blender Z-up
		self.normal = None          # (3,) unit, world space, Z-up
		self._fa = {}
		self._vg = {}
	#

	# +==============================+
	# |           geometry           |
	# +==============================+
	@property
	def slope_deg(self):
	#
		return math.degrees(math.acos(max(-1.0, min(1.0, float(self.normal[2])))))
	#

	# +==============================+
	# |      custom properties       |
	# +==============================+
	def obj_prop(self, key, default=None):
	#
		if key in self.obj:
		#
			return self.obj[key]
		#
		if self.instancer is not None and key in self.instancer:
		#
			return self.instancer[key]
		#
		return default
	#

	def mat_prop(self, key, default=None):
	#
		m = self.material
		return m.get(key, default) if m is not None else default
	#

	# +========================================================+
	# | face attributes (Data Props > Attributes, domain=Face) |
	# +========================================================+
	def face_attr(self, name, default=None):
	#
		arr = self._face_array(name)
		return default if arr is None else arr[self.poly]
	#

	def _face_array(self, name):
	#
		if name in self._fa:
		#
			return self._fa[name]
		#
		a = self.mesh.attributes.get(name)
		arr = None
		if a is not None and a.domain == 'FACE':
		#
			n = len(a.data)
			t = a.data_type
			if t in ('FLOAT', 'INT', 'INT8', 'BOOLEAN'):
			#
				dt = {'FLOAT': np.float32, 'INT': np.int32, 'INT8': np.int8, 'BOOLEAN': bool}[t]
				arr = np.empty(n, dt)
				a.data.foreach_get("value", arr)
			#
			elif t == 'FLOAT_VECTOR':
			#
				arr = np.empty(n * 3, np.float32)
				a.data.foreach_get("vector", arr)
				arr = arr.reshape(n, 3)
			#
			elif t in ('FLOAT_COLOR', 'BYTE_COLOR'):
			#
				arr = np.empty(n * 4, np.float32)
				a.data.foreach_get("color", arr)
				arr = arr.reshape(n, 4)
			#
			elif t == 'STRING':
			#
				arr = [d.value for d in a.data]
			#
		#
		self._fa[name] = arr
		return arr
	#

	# +==================================================================+
	# | vertex groups (falls back to float point attribute of same name) |
	# +==================================================================+
	def vg_weights(self, name):
	#
		return self._vg_array(name)[self.vidx]
	#

	def in_group(self, name, threshold=0.5, mode="all"):
	#
		hit = self.vg_weights(name) >= threshold
		return bool(hit.all() if mode == "all" else hit.any())
	#

	def _vg_array(self, name):
	#
		if name in self._vg:
		#
			return self._vg[name]
		#
		nv = len(self.mesh.vertices)
		w = np.zeros(nv, np.float32)
		vg = self.eval_obj.vertex_groups.get(name)
		if vg is not None:
		#
			gi = vg.index
			for v in self.mesh.vertices:
			#
				for g in v.groups:
				#
					if g.group == gi:
					#
						w[v.index] = g.weight
						break
					#
				#
			#
		#	
		else:
		#
			a = self.mesh.attributes.get(name)
			if a is not None and a.domain == 'POINT' and a.data_type == 'FLOAT':
			#
				a.data.foreach_get("value", w)
			#
		#
		self._vg[name] = w
		return w
	#
#


# +--------------------------------------------------------------+
# |                            Rules                             |
# +--------------------------------------------------------------+
# signature: rule(t: Tri, r: dict) -> None | SKIP ; mutate r in place
# Order = priority; later rules override earlier ones.
def rule_object(t, r):
#
	s = t.obj_prop("surface")
	if s in SURFACE_ID:
	#
		r["surface"] = s
	#
	if t.obj_prop("trigger"):
	#
		r["flags"] |= F_TRIGGER
	#
	if t.obj_prop("one_way"):
	#
		r["flags"] |= F_ONE_WAY
	#
#


def rule_material(t, r):
#
	m = t.material
	if m is None:
	#
		return
	#
	if m.get("no_collide") or m.name.upper().startswith("NOCOL"):
	#
		return SKIP
	#
	s = m.get("surface")
	if s in SURFACE_ID:
	#
		r["surface"] = s
	#
	if m.get("no_camera"):
	#
		r["flags"] |= F_NO_CAMERA
	#
#


def rule_slope(t, r):
#
	if t.slope_deg <= float(t.obj_prop("walkable_slope", 45.0)):
	#
		r["flags"] |= F_WALKABLE
	#
#


def rule_vertex_groups(t, r):
#
	if t.in_group("col_skip"):
	#
		return SKIP
	#
	if t.in_group("col_ladder"):
	#
		r["flags"] |= F_CLIMBABLE
	#
#


def rule_face_attrs(t, r):
#
	if t.face_attr("col_skip", False):
	#
		return SKIP
	#
	r["flags"] |= int(t.face_attr("col_flags", 0))
	sid = int(t.face_attr("col_surface", -1))
	if 0 <= sid < len(SURFACES):
	#
		r["surface"] = SURFACES[sid]
	#
#


RULES = [rule_object, rule_material, rule_slope, rule_vertex_groups, rule_face_attrs]


# +--------------------------------------------------------------+
# |                       Collision Gather                       |
# +--------------------------------------------------------------+
def _process_mesh(src, instancer, ob, mesh, mw, yup):
#
	mesh.calc_loop_triangles()
	nt, nv = len(mesh.loop_triangles), len(mesh.vertices)
	if nt == 0:
	#
		return None
	#

	co = np.empty(nv * 3, np.float32)
	mesh.vertices.foreach_get("co", co)
	co = co.reshape(nv, 3).astype(np.float64) @ mw[:3, :3].T + mw[:3, 3]

	tv = np.empty(nt * 3, np.int32)
	mesh.loop_triangles.foreach_get("vertices", tv)
	tv = tv.reshape(nt, 3)
	tp = np.empty(nt, np.int32)
	mesh.loop_triangles.foreach_get("polygon_index", tp)
	tm = np.empty(nt, np.int32)
	mesh.loop_triangles.foreach_get("material_index", tm)

	if np.linalg.det(mw[:3, :3]) < 0:  # negative scale flips winding
	#
		tv = tv[:, [0, 2, 1]]
	#

	tri = co[tv]
	n = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
	ln = np.linalg.norm(n, axis=1)
	valid = ln > 1e-12
	n[valid] /= ln[valid, None]

	mats = [s.material for s in ob.material_slots] or list(mesh.materials)
	t = Tri(src, instancer, ob, mesh)
	keep, surf, flg = [], [], []

	for i in range(nt):
	#
		if not valid[i]:
		#
			continue  # degenerate
		#
		mi = int(tm[i])
		t.index, t.poly = i, int(tp[i])
		t.material = mats[mi] if mi < len(mats) else None
		t.vidx, t.verts, t.normal = tv[i], tri[i], n[i]
		r = {"surface": "default", "flags": 0}
		for rule in RULES:
		#
			if rule(t, r) is SKIP:
			#
				break
			#
		#
		else:
		#
			keep.append(i)
			surf.append(SURFACE_ID.get(r["surface"], 0))
			flg.append(int(r["flags"]))
		#
	#

	if not keep:
	#
		return None
	#
	used, inv = np.unique(tv[keep], return_inverse=True)
	idx = inv.reshape(-1, 3)
	pos = co[used]
	if yup:  # Blender Z-up -> glTF Y-up: (x, z, -y)
	#
		pos = pos[:, [0, 2, 1]] * np.array([1.0, 1.0, -1.0])
	#
	return {
		"source": src.name,
		"positions": np.round(pos, 6).ravel().tolist(),
		"indices": idx.ravel().tolist(),
		"surface": surf,
		"flags": flg,
	}


def collect_collision(depsgraph, use_selection, yup):
#
	out = []
	for inst in depsgraph.object_instances:
	#
		ob = inst.object  # evaluated
		if ob.type not in MESHLIKE:
		#
			continue
		#
		src = ob.original
		instancer = inst.parent.original if (inst.is_instance and inst.parent) else None
		owner = instancer or src
		if use_selection and not owner.select_get():
		#
			continue
		#
		if not object_included(src, instancer):
		#
			continue
		#
		mw = np.array(inst.matrix_world, dtype=np.float64)
		name = f"{owner.name}/{src.name}#{len(out)}" if instancer else src.name
		mesh = ob.to_mesh(preserve_all_data_layers=True, depsgraph=depsgraph)
		try:
		#
			entry = _process_mesh(src, instancer, ob, mesh, mw, yup)
		#
		finally:
		#
			ob.to_mesh_clear()
		#
		if entry:
		#
			entry["name"] = name
			out.append(entry)
		#
	return out
#


# +----------------------------------------------------------------------+
# | glTF EXPORT (builtin), temporarily excluding collision-only objects  |
# +----------------------------------------------------------------------+
def export_gltf(context, filepath, use_selection, yup, exclude_col_only):
#
	vl = context.view_layer
	objs = list(vl.objects)
	prev_sel = {o.name: o.select_get() for o in objs}
	prev_active = vl.objects.active
	swap = exclude_col_only and any(is_collision_only(o) for o in objs)
	try:
	#
		if swap:
		#
			for o in objs:
			#
				want = (prev_sel[o.name] or not use_selection) and not is_collision_only(o)
				try:
				#
					o.select_set(want)
				#
				except RuntimeError:
				#
					pass
				#
			#
		#
		bpy.ops.export_scene.gltf(
			filepath=filepath,
			export_format='GLTF_SEPARATE',
			export_apply=True,
			export_yup=yup,
			use_selection=use_selection or swap,
			**GLTF_KWARGS,
		)
	finally:
	#
		if swap:
		#
			for o in objs:
			#
				try:
				#
					o.select_set(prev_sel[o.name])
				#
				except RuntimeError:
				#
					pass
				#
			#
			vl.objects.active = prev_active
		#
	#
#


def export_all(context, filepath, use_selection=False, yup=True,
			   exclude_collision_only=True, suffix=".collision.json"):
#
	if context.mode != 'OBJECT':
	#
		bpy.ops.object.mode_set(mode='OBJECT')  # flush edit-mode data
	#
	export_gltf(context, filepath, use_selection, yup, exclude_collision_only)

	meshes = collect_collision(context.evaluated_depsgraph_get(), use_selection, yup)
	side = os.path.splitext(filepath)[0] + suffix
	data = {
		"format": "collision-sidecar",
		"version": 1,
		"gltf": os.path.basename(filepath),
		"up": "+Y" if yup else "+Z",
		"space": "world",
		"surfaces": SURFACES,
		"flags": FLAGS,
		"meshes": meshes,
	}
	with open(side, "w", encoding="utf-8") as f:
	#
		json.dump(data, f, separators=(",", ":"))
	#
	return side, len(meshes), sum(len(m["flags"]) for m in meshes)
#


# +--------------------------------------------------------------+
# |                   Operator / Registration                    |
# +--------------------------------------------------------------+
class EXPORT_SCENE_OT_gltf_collision(bpy.types.Operator, ExportHelper):
#
	bl_idname = "export_scene.gltf_collision"
	bl_label = "Export glTF + Collision"
	bl_options = {'PRESET'}

	filename_ext = ".gltf"
	filter_glob: StringProperty(default="*.gltf", options={'HIDDEN'})
	use_selection: BoolProperty(name="Selected Only", default=False)
	export_yup: BoolProperty(name="+Y Up", default=True)
	exclude_collision_only: BoolProperty(name="Exclude Collision-Only from glTF", default=True)
	sidecar_suffix: StringProperty(name="Sidecar Suffix", default=".collision.json")

	def execute(self, context):
	#
		try:
		#
			side, nm, nt = export_all(context, self.filepath, self.use_selection,
									  self.export_yup, self.exclude_collision_only,
									  self.sidecar_suffix)
		#
		except Exception as e:
		#
			self.report({'ERROR'}, f"Export failed: {e}")
			return {'CANCELLED'}
		#
		self.report({'INFO'}, f"{nm} meshes, {nt} tris -> {side}")
		return {'FINISHED'}
	#
#


def menu_func(self, context):
#
	self.layout.operator(EXPORT_SCENE_OT_gltf_collision.bl_idname,
						 text="glTF + Collision (.gltf)")
#


def register():
#
	bpy.utils.register_class(EXPORT_SCENE_OT_gltf_collision)
	bpy.types.TOPBAR_MT_file_export.append(menu_func)
#


def unregister():
#
	bpy.types.TOPBAR_MT_file_export.remove(menu_func)
	bpy.utils.unregister_class(EXPORT_SCENE_OT_gltf_collision)
#


def _cli():
#
	# blender -b scene.blend --python gltf_collision_export.py -- out/scene.gltf
	import sys
	if "--" not in sys.argv:
	#
		return False
	#
	args = sys.argv[sys.argv.index("--") + 1:]
	if not args:
	#
		return False
	#
	print(export_all(bpy.context, os.path.abspath(args[0])))
	return True
#


if __name__ == "__main__":
#
	if not _cli():
	#
		register()
	#
#
