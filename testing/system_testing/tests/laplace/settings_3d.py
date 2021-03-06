# Laplace 3D
#
# command arguments: <name> <number elements>

import numpy as np
#import scipy.integrate
import sys

nx = 3   # number of elements
ny = 3   
nz = 3
name = ""

if len(sys.argv) > 1:
  if "check_results.py" not in sys.argv[0]:
    name = sys.argv[0]
    n = int(sys.argv[1])
    nx = int(n**(1./3))
    ny = nx+2
    nz = nx+1
    
    print("name: \"{}\", nx,ny,nz: {},{},{}".format(name, nx, ny, nz))

# boundary conditions
bc = {}
if "linear" in name or "hermite" in name:
  n_nodes_x = nx + 1
  n_nodes_y = ny + 1
  n_nodes_z = nz + 1

if "quadratic" in name:
  n_nodes_x = 2*nx + 1
  n_nodes_y = 2*ny + 1
  n_nodes_z = 2*nz + 1

stride = 1
if "hermite" in name:
  stride = 8

for iy in range(int(n_nodes_y)):
  # y position
  y = float(iy)/(n_nodes_y-1.)
    
  for ix in range(int(n_nodes_x)):
    # x position
    x = float(ix)/(n_nodes_x-1.)
    
    # bottom (z-)
    i = iy*n_nodes_x + ix
    bc[stride*i] = np.sin(x*np.pi)*np.sin(y*np.pi)
    
    # top (z+)
    i = n_nodes_x*n_nodes_y*(n_nodes_z-1) + iy*n_nodes_x + ix
    bc[stride*i] = np.sin(x*np.pi)*np.sin(y*np.pi)
    
    # left (x-)
    #i = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
    i = ix*n_nodes_y*n_nodes_x + iy*n_nodes_x
    bc[stride*i] = 0.0
    
    # right (x+)
    i = ix*n_nodes_y*n_nodes_x + iy*n_nodes_x + (n_nodes_x-1)
    bc[stride*i] = 0.0
    
    # front (y-)
    i = ix*n_nodes_y*n_nodes_x + iy
    bc[stride*i] = 0.0
    
    # back (y+)
    i = ix*n_nodes_y*n_nodes_x + (n_nodes_y-1)*n_nodes_x + iy
    bc[stride*i] = 0.0

physicalExtent = [4.0, 3.0, 2.0]

if "fixed" in name:
  physicalExtent = [nx, ny, nz]

hx = physicalExtent[0]/(n_nodes_x-1.0)
hy = physicalExtent[1]/(n_nodes_y-1.0)
hz = physicalExtent[2]/(n_nodes_z-1.0)

# set node positions for unstructured mesh
node_positions = []
for z in list(np.linspace(0,physicalExtent[2],n_nodes_z)):
  for y in list(np.linspace(0,physicalExtent[1],n_nodes_y)):
    for x in list(np.linspace(0,physicalExtent[0],n_nodes_x)):
      node_positions.append([x,y,z])
  
# move node positions
move_factor = 0.1
for i in range(len(node_positions)):
  node_positions[i][0] += move_factor * hx * np.sin(float(i%n_nodes_x)/n_nodes_x*4*np.pi)
  node_positions[i][1] += move_factor * hy * np.sin(float(int(i%(n_nodes_x*n_nodes_y)/n_nodes_x))/n_nodes_y*4*np.pi)
  node_positions[i][2] += move_factor * hz * np.sin(float(int(i/(n_nodes_x*n_nodes_y)))/n_nodes_z*4*np.pi)

# set elements for unstructured mesh
elements = []
if "linear" in name or "hermite" in name:
  for iz in range(n_nodes_z-1):
    for iy in range(n_nodes_y-1):
      for ix in range(n_nodes_x-1):
        i0 = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
        i1 = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+1
        i2 = iz*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix
        i3 = iz*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+1
        i4 = (iz+1)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
        i5 = (iz+1)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+1
        i6 = (iz+1)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix
        i7 = (iz+1)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+1
      
        elements.append([i0, i1, i2, i3, i4, i5, i6, i7])
      
elif "quadratic" in name:
  for iz in range(n_nodes_z-2):
    for iy in range(n_nodes_y-2):
      for ix in range(n_nodes_x-2):
        i0 = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
        i1 = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+1
        i2 = iz*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+2
        i3 = iz*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix
        i4 = iz*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+1
        i5 = iz*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+2
        i6 = iz*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix
        i7 = iz*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+1
        i8 = iz*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+2
        
        i9 = (iz+1)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
        i10 = (iz+1)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+1
        i11 = (iz+1)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+2
        i12 = (iz+1)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix
        i13 = (iz+1)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+1
        i14 = (iz+1)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+2
        i15 = (iz+1)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix
        i16 = (iz+1)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+1
        i17 = (iz+1)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+2
        
        i18 = (iz+2)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix
        i19 = (iz+2)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+1
        i20 = (iz+2)*n_nodes_y*n_nodes_x + iy*n_nodes_x + ix+2
        i21 = (iz+2)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix
        i22 = (iz+2)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+1
        i23 = (iz+2)*n_nodes_y*n_nodes_x + (iy+1)*n_nodes_x + ix+2
        i24 = (iz+2)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix
        i25 = (iz+2)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+1
        i26 = (iz+2)*n_nodes_y*n_nodes_x + (iy+2)*n_nodes_x + ix+2
        
        elements.append([i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26])
      
config = {
  "FiniteElementMethod" : {
    "nElements": [nx, ny, nz],
    "nodePositions": node_positions,
    "elements": elements,
    "physicalExtent": physicalExtent,
    "prefactor": 0.6,
    "dirichletBoundaryConditions": bc,
    "relativeTolerance": 1e-15,
    "maxIterations": 50000,
    "OutputWriter" : [
      {"format": "Paraview", "outputInterval": 1, "filename": "out/"+name, "binary": "false", "fixedFormat": False},
      {"format": "ExFile", "filename": "out/"+name, "outputInterval": 2},
      {"format": "PythonFile", "filename": "out/"+name, "outputInterval": 2, "binary":False, "onlyNodalValues":True},
    ]
  },
}
