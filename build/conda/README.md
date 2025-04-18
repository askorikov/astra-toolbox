Building conda packages
---

# Linux

Requirements: [podman](https://github.com/containers/podman) and [buildah](https://github.com/containers/buildah).

1. Change to `astra-toolbox/build/conda` directory
1. Build container images by running the `containers/setup*.sh` scripts
2. Edit `astra-toolbox/linux_deb*_build_config.yaml` and `libastra/linux_deb*_build_config.yaml` files to select the desired versions of Python and CUDA, respectively. 
3. Run `./release.sh`

# Windows

Requirements: conda-build, git, [Visual Studio 2017](https://community.chocolatey.org/packages/visualstudio2017community) with [Build Tools](https://community.chocolatey.org/packages/visualstudio2017buildtools) and [Native Desktop workload](https://community.chocolatey.org/packages/visualstudio2017-workload-nativedesktop), [Windows SDK version 10.0.22621.2](https://community.chocolatey.org/packages/windows-sdk-11-version-22H2-all), [boost 1.74](https://community.chocolatey.org/packages/boost-msvc-14.1), CUDA toolkit of desired version(s).

1. Activate conda: `C:\tools\miniconda3\condabin\activate.bat`
2. Activate VS Build Tools: `"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 10.0.22621.0 -vcvars_ver=14.1`
3. Change to `astra-toolbox\build\conda` directory
4. Edit `astra-toolbox\win64_config.yaml` and `libastra\win64_build_config.yaml` files to select the desired versions of Python and CUDA, respectively. 
5. Build libastra packages, skipping the testing phase:
`conda build -m libastra\win64_build_config.yaml -c nvidia --no-test libastra`
6. Build and test astra-toolbox packages:
`conda build -m astra-toolbox\win64_build_config.yaml -c nvidia astra-toolbox`
7. Test the previously built libastra packages:
`conda build -c nvidia --test C:\tools\miniconda3\conda-bld\win-64\libastra*.tar.bz2`

# Local installation

The built packages can be installed locally using `conda install astra-toolbox -c nvidia -c local`.
