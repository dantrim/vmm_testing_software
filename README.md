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

#### VTS Server backend
Follow these instructions:

```bash
git clone https://github.com/dantrim/vmm_testing_software.git
```
Edit [vts.pro](vts/vts.pro), specifically the lines with `boostinclude` and `boostlib` to point to your locations of boosts `include/` and `lib/` directories. Nothing else in [vts.pro](vts/vts.pro) needs to be changed.

Next, we compile the VTS source code:

```bash
cd vmm_testing_software/
cd vts/
mkdir build
cd build/
qmake ../vts.pro
make 2>&1 |tee compile.log
```
If this compilation succeeds, a binary named `vts_server` should be located in `vmm_testing_software/vts/build/`.

#### VTS user interface and python virtual env
Next, we obtain the python dependencies for running the user interface and top-level VTS interface, which is all written in python. The python implementation of Qt, *PySide2* is used for this. We grab all of the dependencies by setting up a virtual environment.

```bash
$ cd vmm_testing_software/
$ cd vts/
$ python3 -m venv vtsenv
(vtsenv) $ source vtsenv/bin/activate
(vtsenv) $ pip3 install -r requirements.txt
(vtsenv) $ deactivate
$
```
Once this succeeds you should be able to, for example, run the following command without fail, in a new shell:
```bash
$ cd vmm_testing_software/
$ cd vts/
$ source vtsenv/bin/activate
(vtsenv) $ python
(vtsenv) $ >>> from PySide2 import QtCore
(vtsenv) $ >>> [Ctrl]+d
(vtsenv) $ deactivate
$
```

#### VTS Configuration for your machine
If you have followed the last two steps, then all of the software required for VTS is installed successfully. Now we need to edit the configuration for VTS to be able to *run*.

1) Edit [vts/config/vts_default.json](vts/config/vts_default.json) so that the paths up to the "vmm_testing_software/" are correct for your machine

That's it!

## Running VTS

Each time you want to run VTS, you must return to the virtual environment:

```bash
$ cd vmm_testing_software/vts/
$ source vtsenv/bin/activate
(vtsenv) $ cd python/
(vtsenv) $ python vts.py --gui
```
At which point the VTS user interface will appear on your screen.

Once done with VTS, to exit the virtual environment simply do:
```bash
(vtsenv) $ deactivate
$
```


