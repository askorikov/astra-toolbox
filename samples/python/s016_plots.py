# -----------------------------------------------------------------------
# Copyright: 2010-2022, imec Vision Lab, University of Antwerp
#            2013-2022, CWI, Amsterdam
#
# Contact: astra@astra-toolbox.com
# Website: http://www.astra-toolbox.com/
#
# This file is part of the ASTRA Toolbox.
#
#
# The ASTRA Toolbox is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The ASTRA Toolbox is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the ASTRA Toolbox. If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------

import astra
import numpy as np
import matplotlib.pyplot as plt
plt.gray()


vol_geom = astra.create_vol_geom(256, 256)
proj_geom = astra.create_proj_geom(
    'parallel', 1.0, 384, np.linspace(0, np.pi, 180, False)
)

# As before, create a sinogram from a phantom
phantom_id, P = astra.data2d.shepp_logan(vol_geom)
proj_id = astra.create_projector('cuda', proj_geom, vol_geom)
sinogram_id, sinogram = astra.create_sino(P, proj_id)

plt.imshow(P)
plt.figure()
plt.imshow(sinogram)

# Create a data object for the reconstruction
rec_id = astra.data2d.create('-vol', vol_geom)

# Set up the parameters for a reconstruction algorithm using the GPU
cfg = astra.astra_dict('SIRT_CUDA')
cfg['ReconstructionDataId'] = rec_id
cfg['ProjectionDataId'] = sinogram_id

# Create the algorithm object from the configuration structure
alg_id = astra.algorithm.create(cfg)

# Run 1500 iterations of the algorithm one at a time, keeping track of errors
n_iters = 1500
phantom_error = np.zeros(n_iters)
residual_error = np.zeros(n_iters)
for i in range(n_iters):
    # Run a single iteration
    astra.algorithm.run(alg_id, 1)
    residual_error[i] = astra.algorithm.get_res_norm(alg_id)
    rec = astra.data2d.get(rec_id)
    phantom_error[i] = np.sqrt(((rec - P)**2).sum())

# Get the result
rec = astra.data2d.get(rec_id)
plt.figure()
plt.imshow(rec)

plt.figure()
plt.plot(residual_error)
plt.figure()
plt.plot(phantom_error)
plt.show()

# Clean up
astra.algorithm.delete(alg_id)
astra.data2d.delete(rec_id)
astra.data2d.delete(sinogram_id)
astra.data2d.delete(phantom_id)
astra.projector.delete(proj_id)
