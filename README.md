# vmm_testing_software

This repository contains the software implementing the testing functionality for the
VMM3a ASIC, referred to as *VTS* (VMM Testing Software).


## Base Requirements

* Python >= 3.6
* A recent version of Qt 5 (preferably Qt 5.13)
* ROOT6
* Boost >=1.60

## Installation

### Installing pre-requisites

 **Python3**
 * macOSx:  use [Homebrew](https://brew.sh/): `brew install python3`
 * linux: use aptitude or yum, see some examples [here](https://docs.python-guide.org/starting/install3/linux/)

 **Qt**
 * Qt can be obtained for free, using OpenSource privileges [here](https://www.qt.io/download-open-source)
  
 **ROOT**
  * See detailed instructions from [VERSO](https://gitlab.cern.ch/NSWelectronics/vmm_readout_software#installing-root)

 **Boost**
  * See detailed instructions from [VERSO](https://gitlab.cern.ch/NSWelectronics/vmm_readout_software#installing-boost)
  
### Installing VTS

Follow these instructions:

```bash
git clone https://github.com/dantrim/vmm_testing_software.git
```
Edit [vts.pro](vts/vts.pro), specifically the lines with `boostinclude` and `boostlib` to point to your locations of boosts `include/` and `lib/` directories. Nothing else in [vts.pro](vts/vts.pro) needs to be changed.

Next, we compile the VTS source code:

```bash
cd vmm_testing_software
mkdir build
cd build/
qmake ../vts.pro
make 2>&1 |tee compile.log
```
If this compilation succeeds, a binary named `vts_server` should be located in `vmm_testing_software/build/`.
