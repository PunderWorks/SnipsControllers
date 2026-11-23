"""Raw Fusion STEP -> print-orientation STEP (v2), same orientations as export_print.py.
Needs build123d. Run from this folder: python3 step_orient.py"""
import os
from build123d import import_step, export_step, Pos, Rot, Compound
import export_print_cfg as C
import paths as P

def rot(s, rots):
    for ax, d in rots:
        s = Rot(**{ax.upper(): d}) * s
    return s

def drop(s):
    bb = s.bounding_box()
    return Pos(-(bb.min.X + bb.max.X) / 2, -(bb.min.Y + bb.max.Y) / 2, -bb.min.Z) * s

os.makedirs(P.OUT_STEP, exist_ok=True)
for n, (rots, note) in C.ORI.items():
    s = drop(rot(import_step(os.path.join(P.SRC_STEP, n + '.step')), rots))
    b = s.bounding_box(); export_step(s, os.path.join(P.OUT_STEP, n + '.step'))
    print('%-13s %.1f x %.1f x %.1f  vol %.1f' % (n, b.size.X, b.size.Y, b.size.Z, s.volume))
