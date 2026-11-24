"""Print orientation per part (v2). Each entry is a list of (axis, degrees) rotations applied to the modelled part,
then the part is dropped onto Z=0 and centred on XY. Shared by export_print.py (STL) and step_orient.py (STEP)."""
ORI = {
    # name: (rotations, orientation note)
    'TopShell':     ([('x', 180)], 'top side down: the flat cut face (where the StickCap glues on) and the shell roof on the bed, seam rim up; supports only along the trigger cut-out'),
    'StickCap':     ([('x', 180)], 'dome down (flat rim ring on the bed), cut face and the two alignment pegs up; light supports under the dome flank, brim recommended'),
    'BottomShell':  ([], 'belly down (the edge arcs are limited to 45 deg overhang); open side up, scoop needs supports'),
    'BatteryPanel': ([], '+Z up (as modelled): outer face on the bed'),
    # pouch-cell variant (replaces BottomShell + BatteryPanel + Trigger; TriggerPost and the top-side parts are shared)
    'BottomShell_Pouch': ([], 'belly down (the edge arcs are limited to 45 deg overhang); open side up, scoop needs supports'),
    'Bumper':       ([('x', -90)], 'front face on the bed'),
    'PowerButton':  ([('x', -90)], 'pusher end on the bed, stem up (flange overhangs 0.6 mm per side)'),
    'SideButtonL1': ([('y', 90)], 'flat base on the bed, D stem up'),
    'SideButtonL2': ([('y', 90)], 'flat base on the bed, D stem up'),
    'SideButtonR1': ([('y', -90)], 'flat base on the bed, D stem up'),
    'SideButtonR2': ([('y', -90)], 'flat base on the bed, D stem up'),
    'Trigger':      ([('y', -90)], 'big flat blade face on the bed, spring ear up'),
    'Trigger_Pouch': ([('y', -90)], 'big flat blade face on the bed, spring ear up'),
    'TriggerPost':  ([('x', 90)], 'lying along X on the full-length rear flat (across the collars and neck); no supports'),
}
for _i in range(1, 7):
    ORI['FaceButton%d' % _i] = ([], '+Z up (as modelled): flange on the bed, stem and cap up')
