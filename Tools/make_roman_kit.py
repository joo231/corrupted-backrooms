"""Generate a Roman Tuscan architectural kit as OBJ meshes (centimetres)."""
import math
import os

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT = os.path.join(ROOT, "RawArt", "Kit")


class Mesh:
    def __init__(self, name):
        self.name = name
        self.positions = []
        self.normals = []
        self.uvs = []
        self.faces = []  # (p,n,t) 1-based later

    def add_vertex(self, p, n, uv):
        self.positions.append(p)
        self.normals.append(n)
        self.uvs.append(uv)
        return len(self.positions) - 1

    def add_tri(self, a, b, c):
        self.faces.append((a, b, c))

    def add_quad(self, a, b, c, d):
        self.add_tri(a, b, c)
        self.add_tri(a, c, d)

    def write(self, path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="ascii") as f:
            f.write("o %s\n" % self.name)
            for x, y, z in self.positions:
                f.write("v %.5f %.5f %.5f\n" % (x, y, z))
            for x, y, z in self.normals:
                f.write("vn %.5f %.5f %.5f\n" % (x, y, z))
            for u, v in self.uvs:
                f.write("vt %.5f %.5f\n" % (u, v))
            for a, b, c in self.faces:
                f.write(
                    "f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n".format(a + 1, b + 1, c + 1)
                )


def lathe(mesh, profile, segments, u_repeat=2.0, v0=0.0, v1=1.0):
    """profile: list of (radius, z, v_override or None)"""
    rings = []
    nprof = len(profile)
    for i, (radius, z, _) in enumerate(profile):
        v = v0 + (v1 - v0) * (i / float(nprof - 1))
        ring = []
        for s in range(segments):
            ang = (2.0 * math.pi * s) / segments
            c, si = math.cos(ang), math.sin(ang)
            p = (radius * c, radius * si, z)
            n = (c, si, 0.0)
            uv = (u_repeat * s / float(segments), v)
            ring.append(mesh.add_vertex(p, n, uv))
        rings.append(ring)
    for i in range(nprof - 1):
        for s in range(segments):
            s2 = (s + 1) % segments
            mesh.add_quad(rings[i][s], rings[i][s2], rings[i + 1][s2], rings[i + 1][s])
    return mesh


def box(mesh, cx, cy, cz, sx, sy, sz, uv_scale=0.002):
    hx, hy, hz = sx * 0.5, sy * 0.5, sz * 0.5
    faces = [
        ((0, 0, 1), [(hx, hy, hz), (-hx, hy, hz), (-hx, -hy, hz), (hx, -hy, hz)]),
        ((0, 0, -1), [(hx, -hy, -hz), (-hx, -hy, -hz), (-hx, hy, -hz), (hx, hy, -hz)]),
        ((1, 0, 0), [(hx, hy, -hz), (hx, hy, hz), (hx, -hy, hz), (hx, -hy, -hz)]),
        ((-1, 0, 0), [(-hx, -hy, -hz), (-hx, -hy, hz), (-hx, hy, hz), (-hx, hy, -hz)]),
        ((0, 1, 0), [(hx, hy, -hz), (-hx, hy, -hz), (-hx, hy, hz), (hx, hy, hz)]),
        ((0, -1, 0), [(hx, -hy, hz), (-hx, -hy, hz), (-hx, -hy, -hz), (hx, -hy, -hz)]),
    ]
    for nrm, corners in faces:
        ids = []
        for x, y, z in corners:
            u = (x + cx) * uv_scale + (y * 0.15 * uv_scale)
            v = (z + cz) * uv_scale + (x * 0.07 * uv_scale)
            ids.append(mesh.add_vertex((cx + x, cy + y, cz + z), nrm, (u, v)))
        mesh.add_quad(ids[0], ids[1], ids[2], ids[3])


def flute_radius(base_r, ang, flutes, depth):
    flute = (1.0 - math.cos(ang * flutes)) * 0.5
    return base_r - depth * flute


def make_column():
    mesh = Mesh("SM_RomanColumn")
    segments = 48
    flutes = 20
    depth = 3.2
    # Plinth
    box(mesh, 0, 0, 8, 95, 95, 16)
    # Base torus-ish
    profile = []
    for z, r in [(16, 48), (22, 52), (28, 46), (36, 42), (44, 40)]:
        profile.append((r, z, None))
    # Shaft
    h0, h1 = 44.0, 470.0
    steps = 28
    for i in range(steps + 1):
        t = i / float(steps)
        z = h0 + (h1 - h0) * t
        # slight entasis
        r = 40.0 - 4.0 * t + 1.2 * math.sin(t * math.pi)
        profile.append((r, z, None))
    # Neck
    profile += [(38, 478, None), (42, 486, None), (46, 494, None)]
    # Capital echinus + abacus
    profile += [(52, 502, None), (58, 510, None), (50, 516, None)]
    # Custom lathe with flutes on shaft portion
    rings = []
    nprof = len(profile)
    for i, (radius, z, _) in enumerate(profile):
        v = z / 550.0
        ring = []
        shaft = 44.0 <= z <= 470.0
        for s in range(segments):
            ang = (2.0 * math.pi * s) / segments
            r = flute_radius(radius, ang, flutes, depth) if shaft else radius
            c, si = math.cos(ang), math.sin(ang)
            p = (r * c, r * si, z)
            n = (c, si, 0.0)
            uv = (2.0 * s / float(segments), v)
            ring.append(mesh.add_vertex(p, n, uv))
        rings.append(ring)
    for i in range(nprof - 1):
        for s in range(segments):
            s2 = (s + 1) % segments
            mesh.add_quad(rings[i][s], rings[i][s2], rings[i + 1][s2], rings[i + 1][s])
    box(mesh, 0, 0, 528, 108, 108, 18)
    return mesh


def make_pedestal():
    mesh = Mesh("SM_RomanPedestal")
    box(mesh, 0, 0, 8, 90, 90, 16)
    box(mesh, 0, 0, 55, 70, 70, 78)
    box(mesh, 0, 0, 102, 88, 88, 16)
    return mesh


def make_entablature():
    mesh = Mesh("SM_RomanEntablature")
    # 4.5 m span piece to place between columns
    box(mesh, 0, 0, 20, 460, 90, 40)
    box(mesh, 0, 0, 52, 470, 98, 24)
    box(mesh, 0, 0, 78, 490, 110, 28)
    return mesh


def make_baluster():
    mesh = Mesh("SM_RomanBaluster")
    profile = [
        (10, 0, None),
        (12, 8, None),
        (8, 18, None),
        (16, 40, None),
        (7, 70, None),
        (11, 88, None),
        (12, 96, None),
    ]
    return lathe(mesh, profile, 16, u_repeat=1.0)


def make_floor():
    mesh = Mesh("SM_RomanFloor")
    # 80m x 22m x 0.6m, UVs tile marble
    sx, sy, sz = 8000.0, 2200.0, 60.0
    hx, hy, hz = sx * 0.5, sy * 0.5, sz * 0.5
    tiles_u, tiles_v = 16.0, 4.5
    # top
    n = (0, 0, 1)
    a = mesh.add_vertex((-hx, -hy, hz), n, (0, 0))
    b = mesh.add_vertex((hx, -hy, hz), n, (tiles_u, 0))
    c = mesh.add_vertex((hx, hy, hz), n, (tiles_u, tiles_v))
    d = mesh.add_vertex((-hx, hy, hz), n, (0, tiles_v))
    mesh.add_quad(a, b, c, d)
    # sides + bottom simplified
    faces = [
        ((0, 0, -1), [(-hx, hy, -hz), (hx, hy, -hz), (hx, -hy, -hz), (-hx, -hy, -hz)]),
        ((0, -1, 0), [(-hx, -hy, -hz), (hx, -hy, -hz), (hx, -hy, hz), (-hx, -hy, hz)]),
        ((0, 1, 0), [(hx, hy, -hz), (-hx, hy, -hz), (-hx, hy, hz), (hx, hy, hz)]),
        ((1, 0, 0), [(hx, -hy, -hz), (hx, hy, -hz), (hx, hy, hz), (hx, -hy, hz)]),
        ((-1, 0, 0), [(-hx, hy, -hz), (-hx, -hy, -hz), (-hx, -hy, hz), (-hx, hy, hz)]),
    ]
    for nrm, corners in faces:
        ids = []
        for x, y, z in corners:
            ids.append(mesh.add_vertex((x, y, z), nrm, (x / 400.0, z / 400.0)))
        mesh.add_quad(ids[0], ids[1], ids[2], ids[3])
    return mesh


def make_step():
    mesh = Mesh("SM_RomanStep")
    box(mesh, 0, 0, 12, 8200, 240, 24)
    box(mesh, 0, 12, 24, 8080, 180, 24)
    return mesh


def make_roof():
    mesh = Mesh("SM_RomanRoof")
    box(mesh, 0, 0, 36, 460, 1980, 20)
    box(mesh, 0, 0, 62, 90, 1980, 28)
    for iy in range(-4, 5):
        box(mesh, 0, iy * 220.0, 18, 460, 34, 38)
    for ix in (-190.0, 0.0, 190.0):
        box(mesh, ix, 0, 18, 34, 1980, 38)
    for ix in (-145.0, 145.0):
        for iy in range(-3, 4):
            box(mesh, ix, iy * 220.0, 8, 112, 168, 10)
    for iy in range(-4, 5):
        box(mesh, -230, iy * 220.0, 52, 20, 30, 24)
        box(mesh, 230, iy * 220.0, 52, 20, 30, 24)
    return mesh


def make_pediment():
    mesh = Mesh("SM_RomanPediment")
    for i, w in enumerate((520, 420, 320, 220, 120, 40)):
        box(mesh, 0, 0, 18 + i * 26, w, 90, 26)
    box(mesh, 0, 0, 8, 540, 100, 16)
    return mesh


def make_stair_chunk(name="SM_RomanStairChunk", going_up=False):
    mesh = Mesh(name)
    steps = 10
    rise, run, width = 20.0, 40.0, 360.0
    sign = 1.0 if going_up else -1.0
    # Flat landing at the mouth so turns still have something to stand on.
    box(mesh, 0, 20, sign * 6.0, width, 80.0, 12.0)
    for i in range(steps):
        z = sign * (i * rise + rise * 0.5)
        y = i * run + run * 0.5
        box(mesh, 0, y, z, width, run + 6.0, rise)
    return mesh


def main():
    os.makedirs(OUT, exist_ok=True)
    make_column().write(os.path.join(OUT, "SM_RomanColumn.obj"))
    make_pedestal().write(os.path.join(OUT, "SM_RomanPedestal.obj"))
    make_entablature().write(os.path.join(OUT, "SM_RomanEntablature.obj"))
    make_baluster().write(os.path.join(OUT, "SM_RomanBaluster.obj"))
    make_floor().write(os.path.join(OUT, "SM_RomanFloor.obj"))
    make_step().write(os.path.join(OUT, "SM_RomanStep.obj"))
    make_roof().write(os.path.join(OUT, "SM_RomanRoof.obj"))
    make_pediment().write(os.path.join(OUT, "SM_RomanPediment.obj"))
    make_stair_chunk("SM_RomanStairChunk", False).write(os.path.join(OUT, "SM_RomanStairChunk.obj"))
    make_stair_chunk("SM_RomanStairUp", True).write(os.path.join(OUT, "SM_RomanStairUp.obj"))
    print("kit written to", OUT)


if __name__ == "__main__":
    main()
