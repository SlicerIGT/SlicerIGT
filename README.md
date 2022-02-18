# SlicerIGT
This repository contains the SlicerIGT extension for 3D Slicer, which provides components for implementing image-guided intervention systems.

## User documentation

To learn how to use SlicerIGT, refer to [SlicerIGT tutorials](http://www.slicerigt.org/wp/user-tutorial/).

Modules provided by this extension:

### Breach warning

Display distance of tracked tool from an object and give visual and audible warning if the tool gets too close. Helps users to avoid certain regions during surgical resection or other interventions.

### Collect points

Record motion trajectory of a tracked tool into markups fiducial list, markups curve, or model node.

### Create models

Module for creating simple shapes, such as sphere, cylinder, prism, needle, and coordinate system axes.

### Fiducial registration wizard

Register images, models, or any other nodes to each other using manually placed landmark points. Rigid, affine, and warping transforms are supported.

### Fiducials to model registration

Automatic registration of model (surface mesh) to markups fiducials (point list) using iterative closest points (ICP) method.

### Model registration

Automatic registration of models (surface meshes) using iterative closest points (ICP) method.

### Path explorer

Module for specifying tool trajectories by entry and target points and reslice image along and orthogonal to the trajectory line.

Hit `E` or `T` key to switch between placement of entry and target point.

### Pivot calibration

Determine transformation between tooltip and tracking sensor attached to a tracked tool.

### Texture model

Apply texture to a surface model. Useful for displaying models acquired by surface scanners.

### Transform processor

This module allows combining, inverting, stabilizing, termporal smoothing of transforms in real-time and writing results into a new output.

Processing modes:
- Quaternion Average: Compute the quaternion average of all Source transforms provided.
- Compute Rotation Only: Compute a copy of the rotation from the Source to the Reference.
- Compute Translation Only: Compute a copy of the translation from the Source to the Reference.
- Compute Full Transform: Compute a copy of the full transform from the Source to the Reference.
- Compute Inverse: Compute the inverse of transform to parent, and store it in another node.
- Compute Shaft Pivot: Compute a constrained version of an Source transform, the translation and z direction are preserved but the other axes resemble the Target coordinate system.
- Stabilize: Compute a stabilized transform by low-pass filtering. Computation based on the former [TrackerStabilizer extension](https://github.com/lchauvin/TrackerStabilizer).

#### Stabilize mode

Position tracking systems are useful for tracking surgical tools, robots, patient motion, etc. However, some tracking systems provide noisy (shaky) measurements and it can be useful to reduce random noise by filtering. Filtering can be also useful for converting abrupt position/orientation changes to smooth transitions, for example for moving a virtual camera.

TrackerStabilizer module provides temporal low-pass filtering for any situations requiring stable position/orientation where time delays are tolerable. User can adjust cut-off frequency (i.e., smoothing parameter): the lower the value, the smoother the motion is but it is important to note that the delay is also increased.

The GUI of the TrackerStabilizer module contains 4 elements:
- Input Transform: This is the raw data, usually raw tracker data, represented by a vtkMRMLLinearTransformNode
- Output Transform: This is the filtered transform represented also by a vtkMRMLLinearTransformNode. This node can be created directly from the list
- Filter checkbox: This checkbox turn on/off the filtering
- Smoothing slider: This slider control the smoothing parameter. In the left, smoother, with relatively important delay. In the right, faster (almost no delay), but noisier.

### Ultrasound snapshots

Module for taking a snapshot of a position tracked ultrasound image and display it as a model node.

### Viewpoint

Control 3D view camera using tracked tools and objects. It can either simulate a camera mounted on a tool or can continuously update camera position to keep a selected node in the center of the view.

### Volume reslice driver

Move slice view (e.g., to show slice of an image) driven by position and orientation of a tracked tool (or any linear transform), or ROI or line markup.

### Watchdog

Displays warning if selected transforms are not continuously updated. It is useful for detecting tracking errors, such as occluded marker or network connection error.

## Developer documentation

Basic information for developers is available [here](http://www.slicerigt.org/wp/developer-tutorial/).

### Software release process

SlicerIGT packages are generated automatically every night from latest code version on master branch and a branch matching the name of the latest Slicer stable branch (e.g., master-48). Exact content can be determined from git hash included in the relase package name and content of [SlicerIGT repository](https://github.com/SlicerIGT/SlicerIGT).

Nightly test results are available on Slicer dashboard: [stable](http://slicer.cdash.org/index.php?project=Slicer4), [nightly](http://slicer.cdash.org/index.php?project=SlicerPreview)

# Acknowledgements

This work was partially funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO), and by National Institute of Health (grants 5P01CA067165, 5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377, 5R42CA137886, 8P41EB015898).
