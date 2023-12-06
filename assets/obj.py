# Export as Z forward, -Y up
import numpy as np
import parse
import struct
import sys
import os

# TODO: fixed point
# object space coord = int16

class Vec2:
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

    def pack(self):
        # Texture coordinates, 16 bits ?
        return struct.pack('<hh', int(self.x), int(self.y))

    def __str__(self):
        return '{:.6f} {:.6f}'.format(self.x, self.y)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y


class Vec3:
    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z

    def pack(self):
        return struct.pack('<hhhh', int(self.x), int(self.y), int(self.z), 0) # SVECTOR are padded

    def __str__(self):
        return '{:.6f} {:.6f} {:.6f}'.format(self.x, self.y, self.z)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y and self.z == other.z


class Vertex:
    def __init__(self, p=Vec3(), n=Vec3(), t=Vec2()):
        self.position = p
        self.normal = n
        self.uv = t

    def pack(self):
        return self.position.pack() + self.normal.pack() + self.uv.pack()

    def __str__(self):
        return '({}) ({}) ({})'.format(self.position, self.normal, self.uv)

    def __eq__(self, other):
        return self.position == other.position and self.normal == other.normal and self.uv == other.uv


class Triangle:
    def __init__(self, vertIndices=[]):
        self.vertIndices = vertIndices

    def pack(self):
        return struct.pack('<III', self.vertIndices[0], self.vertIndices[1], self.vertIndices[2])

    def __str__(self):
        return '{} {} {}'.format(
            self.vertIndices[0], self.vertIndices[1], self.vertIndices[2])


class ObjVert:
    def __init__(self, vi=0, ni=0, ti=0):
        self.position = vi - 1
        self.normal = ni - 1
        self.uv = ti - 1

    def __str__(self):
        return '{}/{}/{}'.format(self.position, self.normal, self.uv)


class ObjFace:
    def __init__(self, v0=ObjVert(), v1=ObjVert(), v2=ObjVert()):
        self.v0 = v0
        self.v1 = v1
        self.v2 = v2

    def __str__(self):
        return '{} {} {}'.format(self.v0, self.v1, self.v2)


class Subset:
    MAX_TEX_CHAR = 20
    def __init__(self, texture='', start=0):
        self.start = start
        self.count = 0
        tex = os.path.splitext(os.path.basename(texture))[0]
        self.texture = tex[:Subset.MAX_TEX_CHAR-1] # limit to 20 chars

    def __str__(self):
        return '{} {} [{}] ({})'.format(self.start, self.count, len(self.texture), self.texture)

    def pack(self):
        data = struct.pack('<II', self.start, self.count)

        return data + bytes(self.texture.ljust(Subset.MAX_TEX_CHAR, '\0'), 'ascii')

class Mesh:
    ONE = 4096
    def __init__(self):
        self.vertices = []
        self.tris = []
        self.subsets = []

    def from_file(self, filename):
        positions = []
        normals = []
        uvs = []
        faces = []

        subset = None
        start = 0

        with open(filename) as rawfile:
            while True:
                line = rawfile.readline()
                if not line:
                    break

                if line.startswith('v '):
                    data = parse.search('v {px:g} {py:g} {pz:g}', line)

                    x = data['px'] * Mesh.ONE
                    y = data['py'] * Mesh.ONE
                    z = data['pz'] * Mesh.ONE
                    positions.append(Vec3(x, y, z))

                elif line.startswith('vt '):
                    data = parse.search('vt {tu:g} {tv:g}', line)
                    # TODO: determine image size
                    tex_w = 96
                    tex_h = 64
                    u = round(data['tu'] * tex_w)
                    v = round((1 - data['tv']) * tex_h)
                    uvs.append(Vec2(u, v))

                elif line.startswith('vn '):
                    data = parse.search('vn {nx:g} {ny:g} {nz:g}', line)
                    # TODO: transform
                    x = data['nx'] * Mesh.ONE
                    x = data['ny'] * Mesh.ONE
                    x = data['nz'] * Mesh.ONE
                    normals.append(Vec3(x, y, z))

                elif line.startswith('usemtl'):
                    data = parse.search('usemtl {:S}', line)
                    self.subsets.append(Subset(data[0], start))
                    subset = len(self.subsets) - 1

                elif line.startswith('f '):
                    data = parse.search('f {v0_vi:d}/{v0_vti:d}/{v0_vni:d} {v1_vi:d}/{v1_vti:d}/{v1_vni:d} {v2_vi:d}/{v2_vti:d}/{v2_vni:d}', line)
                    v0 = ObjVert(data['v0_vi'], data['v0_vni'], data['v0_vti'])
                    v1 = ObjVert(data['v1_vi'], data['v1_vni'], data['v1_vti'])
                    v2 = ObjVert(data['v2_vi'], data['v2_vni'], data['v2_vti'])
                    faces.append(ObjFace(v0, v1, v2))

                    if subset != None:
                        self.subsets[subset].count += 1
                        start += 1

        for f in faces:
            # ---- v0
            v0 = Vertex(positions[f.v0.position],
                        normals[f.v0.normal],
                        uvs[f.v0.uv])

            try:
                v0_i = self.vertices.index(v0)
            except ValueError:
                self.vertices.append(v0)
                v0_i = len(self.vertices) - 1

            # ---- v1
            v1 = Vertex(positions[f.v1.position],
                        normals[f.v1.normal],
                        uvs[f.v1.uv])

            try:
                v1_i = self.vertices.index(v1)
            except ValueError:
                self.vertices.append(v1)
                v1_i = len(self.vertices) - 1

            # ---- v2
            v2 = Vertex(positions[f.v2.position],
                        normals[f.v2.normal],
                        uvs[f.v2.uv])

            try:
                v2_i = self.vertices.index(v2)
            except ValueError:
                self.vertices.append(v2)
                v2_i = len(self.vertices) - 1

            self.tris.append(Triangle([v2_i, v1_i, v0_i]))


    def pack(self, outfile):
        data = bytearray()

        print('*** Header ***')
        print(len(self.vertices), len(self.tris), len(self.subsets))
        data += struct.pack('<III', len(self.vertices), len(self.tris), len(self.subsets))

        print('*** Vertices ***')
        for v in self.vertices:
            print(v)
            data += v.pack()

        print('*** Triangles ***')
        for t in self.tris:
            print(t)
            data += t.pack()

        print('*** Subsets ***')
        for s in self.subsets:
            print(s)
            data += s.pack()

        with open(outfile, 'wb') as f:
            f.write(data)

if __name__ == '__main__':


    outfile = 'model.bin'

    if len(sys.argv) > 2:
        infile = sys.argv[1]
        outfile = sys.argv[2]
    elif len(sys.argv) > 1:
        infile = sys.argv[1]
    else:
        print('usage: obj.py infile [outfile]')
        exit()

    print(infile, outfile)

    m = Mesh()
    m.from_file(infile)
    m.pack(outfile)

