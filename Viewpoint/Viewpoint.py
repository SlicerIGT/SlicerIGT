from __main__ import vtk, qt, ctk, slicer
import logging
import time

#
# Viewpoint
#
class Viewpoint:

  def __init__(self, parent):
    parent.title = "Viewpoint"
    parent.categories = ["IGT"]
    parent.dependencies = []
    parent.contributors = ["Thomas Vaughan (Queen's)",
                           "Andras Lasso (Queen's)",
                           "Tamas Ungi (Queen's)",
                           "Gabor Fichtinger (Queen's)"]
    parent.helpText = """This module adjusts camera viewpoint of a 3D viewer.
Bullseye View: position/orient the camera using a tracked tool. 
"""
    parent.acknowledgementText = """
    This work is part of the Breast NaviKnife project within the Laboratory for Percutaneous Surgery, Queen's University, Kingston, Ontario. Thomas Vaughan is funded by an NSERC Postgraduate award. Gabor Fichtinger is funded as a Cancer Care Ontario (CCO) Chair.
    """ # replace with organization, grant and thanks.
    self.parent = parent

#
# ViewpointWidget
#

class ViewpointWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
      self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

    self.logic = ViewpointLogic()

    # BULLSEYE
    self.sliderTranslationDefaultMm    = 0
    self.sliderTranslationMinMm        = -200
    self.sliderTranslationMaxMm        = 200
    self.sliderViewAngleDefaultDeg     = 30
    self.cameraViewAngleMinDeg         = 5.0  # maximum magnification
    self.cameraViewAngleMaxDeg         = 150.0 # minimum magnification
    self.sliderParallelScaleDefaultDeg = 1
    self.cameraParallelScaleMinDeg     = 0.001  # maximum magnification
    self.cameraParallelScaleMaxDeg     = 1000.0 # minimum magnification

    self.sliderSingleStepValue = 1
    self.sliderPageStepValue   = 10

    self.checkStateUNCHECKED = 0
    self.checkStateCHECKED = 2

    self.toggleBullseyeButtonTextState0 = "Enable Bullseye View Mode"
    self.toggleBullseyeButtonTextState1 = "Disable Bullseye View Mode"

    # AUTO-CENTER
    self.sliderMultiplier = 100.0
    self.rangeSliderMaximum = self.sliderMultiplier
    self.rangeSliderMinimum = -self.sliderMultiplier
    self.rangeSliderMaximumValueDefault = self.sliderMultiplier
    self.rangeSliderMinimumValueDefault = -self.sliderMultiplier

    self.sliderSingleStepValue = 0.01
    self.sliderPageStepValue   = 0.1

    self.updateRateMinSeconds = 0
    self.updateRateMaxSeconds = 1
    self.updateRateDefaultSeconds = 0.1

    self.timeUnsafeToAdjustMinSeconds = 0
    self.timeUnsafeToAdjustMaxSeconds = 5
    self.timeUnsafeToAdjustDefaultSeconds = 1

    self.timeAdjustToRestMinSeconds = 0
    self.timeAdjustToRestMaxSeconds = 5
    self.timeAdjustToRestDefaultSeconds = 1

    self.timeRestToSafeMinSeconds = 0
    self.timeRestToSafeMaxSeconds = 5
    self.timeRestToSafeDefaultSeconds = 1

    self.toggleAutoCenterButtonTextState0 = "Enable Auto-Center Mode"
    self.toggleAutoCenterButtonTextState1 = "Disable Auto-Center Mode"

  def setup(self):
    # TODO: The following line is strictly for debug purposes, should be removed when this module is done
    slicer.tvwidget = self

    # Collapsible buttons
    self.viewCollapsibleButton = ctk.ctkCollapsibleButton()
    self.viewCollapsibleButton.text = "View Selection"
    self.layout.addWidget(self.viewCollapsibleButton)

    # Layout within the collapsible button
    self.viewFormLayout = qt.QFormLayout(self.viewCollapsibleButton)

    self.viewLabel = qt.QLabel()
    self.viewLabel.setText("Scene Camera: ")
    self.viewSelector = slicer.qMRMLNodeComboBox()
    self.viewSelector.nodeTypes = ( ("vtkMRMLViewNode"), "" )
    self.viewSelector.noneEnabled = True
    self.viewSelector.addEnabled = False
    self.viewSelector.removeEnabled = False
    self.viewSelector.setMRMLScene( slicer.mrmlScene )
    self.viewSelector.setToolTip("Pick the view which should be adjusted, e.g. 'View1'")
    self.viewFormLayout.addRow(self.viewLabel, self.viewSelector)

    # Collapsible buttons
    self.bullseyeParametersCollapsibleButton = ctk.ctkCollapsibleButton()
    self.bullseyeParametersCollapsibleButton.text = "Parameters for Bullseye View"
    self.layout.addWidget(self.bullseyeParametersCollapsibleButton)

    # Layout within the collapsible button
    self.bullseyeParametersFormLayout = qt.QFormLayout(self.bullseyeParametersCollapsibleButton)

    # Transform combobox
    self.transformLabel = qt.QLabel()
    self.transformLabel.setText("Camera positioning transform: ")
    self.transformSelector = slicer.qMRMLNodeComboBox()
    self.transformSelector.nodeTypes = [ "vtkMRMLLinearTransformNode" ]
    self.transformSelector.noneEnabled = False
    self.transformSelector.addEnabled = False
    self.transformSelector.removeEnabled = False
    self.transformSelector.setMRMLScene( slicer.mrmlScene )
    self.transformSelector.setToolTip("Pick the transform that the camera should follow, e.g. 'cauteryCameraToCauteryTransform'")
    self.bullseyeParametersFormLayout.addRow(self.transformLabel, self.transformSelector)

    # "Camera Control" Collapsible
    self.cameraControlCollapsibleButton = ctk.ctkCollapsibleButton()
    self.cameraControlCollapsibleButton.text = "Camera Control"
    self.bullseyeParametersFormLayout.addRow(self.cameraControlCollapsibleButton)

    # Layout within the collapsible button
    self.cameraControlFormLayout = qt.QFormLayout(self.cameraControlCollapsibleButton)

    # "Degrees of Freedom" Collapsible button
    self.degreesOfFreedomCollapsibleButton = ctk.ctkCollapsibleGroupBox()
    self.degreesOfFreedomCollapsibleButton.title = "Degrees of Freedom"
    self.cameraControlFormLayout.addRow(self.degreesOfFreedomCollapsibleButton)

    # Layout within the collapsible button
    self.degreesOfFreedomFormLayout = qt.QFormLayout(self.degreesOfFreedomCollapsibleButton)

    # A series of radio buttons for changing the degrees of freedom
    self.degreesOfFreedom3Label = qt.QLabel(qt.Qt.Horizontal,None)
    self.degreesOfFreedom3Label.setText("3DOF: ")
    self.degreesOfFreedom3RadioButton = qt.QRadioButton()
    self.degreesOfFreedom3RadioButton.setToolTip("The camera will always look at the target model (or if unselected will act like 5DOF)")
    self.degreesOfFreedomFormLayout.addRow(self.degreesOfFreedom3Label,self.degreesOfFreedom3RadioButton)

    self.degreesOfFreedom5Label = qt.QLabel(qt.Qt.Horizontal,None)
    self.degreesOfFreedom5Label.setText("5DOF: ")
    self.degreesOfFreedom5RadioButton = qt.QRadioButton()
    self.degreesOfFreedom5RadioButton.setToolTip("The camera will always be oriented with the selected 'up direction'")
    self.degreesOfFreedomFormLayout.addRow(self.degreesOfFreedom5Label,self.degreesOfFreedom5RadioButton)

    self.degreesOfFreedom6Label = qt.QLabel(qt.Qt.Horizontal,None)
    self.degreesOfFreedom6Label.setText("6DOF: ")
    self.degreesOfFreedom6RadioButton = qt.QRadioButton()
    self.degreesOfFreedom6RadioButton.setToolTip("The camera will be virtually attached to the tool, and rotate together with it")
    self.degreesOfFreedom6RadioButton.setChecked(self.checkStateCHECKED)
    self.degreesOfFreedomFormLayout.addRow(self.degreesOfFreedom6Label,self.degreesOfFreedom6RadioButton)

    # "Up Direction" Collapsible button
    self.upDirectionCollapsibleButton = ctk.ctkCollapsibleGroupBox()
    self.upDirectionCollapsibleButton.title = "Up Direction"
    self.upDirectionCollapsibleButton.setVisible(False)
    self.cameraControlFormLayout.addRow(self.upDirectionCollapsibleButton)

    # Layout within the collapsible button
    self.upDirectionFormLayout = qt.QFormLayout(self.upDirectionCollapsibleButton)

    # Radio buttons for each of the anatomical directions
    self.upDirectionAnteriorLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionAnteriorLabel.setText("Anterior: ")
    self.upDirectionAnteriorRadioButton = qt.QRadioButton()
    self.upDirectionAnteriorRadioButton.setChecked(self.checkStateCHECKED)
    self.upDirectionFormLayout.addRow(self.upDirectionAnteriorLabel,self.upDirectionAnteriorRadioButton)

    self.upDirectionPosteriorLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionPosteriorLabel.setText("Posterior: ")
    self.upDirectionPosteriorRadioButton = qt.QRadioButton()
    self.upDirectionFormLayout.addRow(self.upDirectionPosteriorLabel,self.upDirectionPosteriorRadioButton)

    self.upDirectionRightLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionRightLabel.setText("Right: ")
    self.upDirectionRightRadioButton = qt.QRadioButton()
    self.upDirectionFormLayout.addRow(self.upDirectionRightLabel,self.upDirectionRightRadioButton)

    self.upDirectionLeftLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionLeftLabel.setText("Left: ")
    self.upDirectionLeftRadioButton = qt.QRadioButton()
    self.upDirectionFormLayout.addRow(self.upDirectionLeftLabel,self.upDirectionLeftRadioButton)

    self.upDirectionSuperiorLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionSuperiorLabel.setText("Superior: ")
    self.upDirectionSuperiorRadioButton = qt.QRadioButton()
    self.upDirectionFormLayout.addRow(self.upDirectionSuperiorLabel,self.upDirectionSuperiorRadioButton)

    self.upDirectionInferiorLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.upDirectionInferiorLabel.setText("Inferior: ")
    self.upDirectionInferiorRadioButton = qt.QRadioButton()
    self.upDirectionFormLayout.addRow(self.upDirectionInferiorLabel,self.upDirectionInferiorRadioButton)

    # "Target Model" Collapsible button
    self.targetModelCollapsibleButton = ctk.ctkCollapsibleGroupBox()
    self.targetModelCollapsibleButton.title = "Target Model"
    self.targetModelCollapsibleButton.setVisible(False)
    self.cameraControlFormLayout.addRow(self.targetModelCollapsibleButton)

    # Layout within the collapsible button
    self.targetModelFormLayout = qt.QFormLayout(self.targetModelCollapsibleButton)

    # Selection of the target model
    self.targetModelLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.targetModelLabel.text = "Target model: "
    self.targetModelSelector = slicer.qMRMLNodeComboBox()
    self.targetModelSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.targetModelSelector.noneEnabled = False
    self.targetModelSelector.addEnabled = False
    self.targetModelSelector.removeEnabled = False
    self.targetModelSelector.setMRMLScene( slicer.mrmlScene )
    self.targetModelSelector.setToolTip("This model be the center of rotation using 3DOF Viewpoint (e.g. tumour)")
    self.targetModelFormLayout.addRow(self.targetModelLabel,self.targetModelSelector)

    # "Zoom" Collapsible button
    self.zoomCollapsibleButton = ctk.ctkCollapsibleGroupBox()
    self.zoomCollapsibleButton.title = "Zoom"
    self.cameraControlFormLayout.addRow(self.zoomCollapsibleButton)

    # Layout within the collapsible button
    self.zoomFormLayout = qt.QFormLayout(self.zoomCollapsibleButton)

    # Camera viewing angle (perspective projection only)
    self.cameraViewAngleLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.cameraViewAngleLabel.setText("View angle (degrees): ")
    self.cameraViewAngleSlider = slicer.qMRMLSliderWidget()
    self.cameraViewAngleSlider.minimum = self.cameraViewAngleMinDeg
    self.cameraViewAngleSlider.maximum = self.cameraViewAngleMaxDeg
    self.cameraViewAngleSlider.value = self.sliderViewAngleDefaultDeg
    self.cameraViewAngleSlider.singleStep = self.sliderSingleStepValue
    self.cameraViewAngleSlider.pageStep = self.sliderPageStepValue
    self.cameraViewAngleSlider.setToolTip("Make the current viewing target look larger/smaller.")
    self.zoomFormLayout.addRow(self.cameraViewAngleLabel,self.cameraViewAngleSlider)

    # Camera parallel scale (parallel projection only)
    self.cameraParallelScaleLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.cameraParallelScaleLabel.setText("View scale: ")
    self.cameraParallelScaleLabel.setVisible(False)
    self.cameraParallelScaleSlider = slicer.qMRMLSliderWidget()
    self.cameraParallelScaleSlider.minimum = self.cameraParallelScaleMinDeg
    self.cameraParallelScaleSlider.maximum = self.cameraParallelScaleMaxDeg
    self.cameraParallelScaleSlider.value = self.sliderParallelScaleDefaultDeg
    self.cameraParallelScaleSlider.singleStep = self.sliderSingleStepValue
    self.cameraParallelScaleSlider.pageStep = self.sliderPageStepValue
    self.cameraParallelScaleSlider.setToolTip("Make the current viewing target look larger/smaller.")
    self.cameraParallelScaleSlider.setVisible(False)
    self.zoomFormLayout.addRow(self.cameraParallelScaleLabel,self.cameraParallelScaleSlider)

    # "Translation" Collapsible
    self.translationCollapsibleButton = ctk.ctkCollapsibleGroupBox()
    self.translationCollapsibleButton.title = "Translation"
    self.cameraControlFormLayout.addRow(self.translationCollapsibleButton)

    # Layout within the collapsible button
    self.translationFormLayout = qt.QFormLayout(self.translationCollapsibleButton)

    self.cameraXPosLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.cameraXPosLabel.text = "Left/Right (mm): "
    self.cameraXPosSlider = slicer.qMRMLSliderWidget()
    self.cameraXPosSlider.minimum = self.sliderTranslationMinMm
    self.cameraXPosSlider.maximum = self.sliderTranslationMaxMm
    self.cameraXPosSlider.value = self.sliderTranslationDefaultMm
    self.cameraXPosSlider.singleStep = self.sliderSingleStepValue
    self.cameraXPosSlider.pageStep = self.sliderPageStepValue
    self.translationFormLayout.addRow(self.cameraXPosLabel,self.cameraXPosSlider)

    self.cameraYPosLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.cameraYPosLabel.setText("Down/Up (mm): ")
    self.cameraYPosSlider = slicer.qMRMLSliderWidget()
    self.cameraYPosSlider.minimum = self.sliderTranslationMinMm
    self.cameraYPosSlider.maximum = self.sliderTranslationMaxMm
    self.cameraYPosSlider.value = self.sliderTranslationDefaultMm
    self.cameraYPosSlider.singleStep = self.sliderSingleStepValue
    self.cameraYPosSlider.pageStep = self.sliderPageStepValue
    self.translationFormLayout.addRow(self.cameraYPosLabel,self.cameraYPosSlider)

    self.cameraZPosLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.cameraZPosLabel.setText("Front/Back (mm): ")
    self.cameraZPosSlider = slicer.qMRMLSliderWidget()
    self.cameraZPosSlider.minimum = self.sliderTranslationMinMm
    self.cameraZPosSlider.maximum = self.sliderTranslationMaxMm
    self.cameraZPosSlider.value = self.sliderTranslationDefaultMm
    self.cameraZPosSlider.singleStep = self.sliderSingleStepValue
    self.cameraZPosSlider.pageStep = self.sliderPageStepValue
    self.translationFormLayout.addRow(self.cameraZPosLabel,self.cameraZPosSlider)

    # Camera parallel projection checkbox
    self.cameraParallelProjectionLabel = qt.QLabel()
    self.cameraParallelProjectionLabel.setText("Parallel Projection")
    self.cameraParallelProjectionCheckbox = qt.QCheckBox()
    self.cameraParallelProjectionCheckbox.setCheckState(self.checkStateUNCHECKED)
    self.cameraParallelProjectionCheckbox.setToolTip("If checked, render with parallel projection (box-shaped view). Otherwise render with perspective projection (cone-shaped view).")
    self.cameraControlFormLayout.addRow(self.cameraParallelProjectionLabel,self.cameraParallelProjectionCheckbox)

    # "Toggle Tool Point of View" button
    self.toggleBullseyeButton = qt.QPushButton()
    self.toggleBullseyeButton.setToolTip("The camera will continuously update its position so that it follows the tool.")
    self.toggleBullseyeButton.setText(self.toggleBullseyeButtonTextState0)
    self.layout.addWidget(self.toggleBullseyeButton)

    # AUTO-CENTER

    # Collapsible buttons
    self.autoCenterParametersCollapsibleButton = ctk.ctkCollapsibleButton()
    self.autoCenterParametersCollapsibleButton.text = "Parameters for Auto-Center"
    self.layout.addWidget(self.autoCenterParametersCollapsibleButton)

    # Layout within the collapsible button
    self.autoCenterParametersFormLayout = qt.QFormLayout(self.autoCenterParametersCollapsibleButton)

    # Transform combobox
    self.modelLabel = qt.QLabel()
    self.modelLabel.setText("Followed model: ")
    self.modelSelector = slicer.qMRMLNodeComboBox()
    self.modelSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.modelSelector.noneEnabled = False
    self.modelSelector.addEnabled = False
    self.modelSelector.removeEnabled = False
    self.modelSelector.setMRMLScene( slicer.mrmlScene )
    self.modelSelector.setToolTip("Pick the model that the camera should follow, e.g. 'tumorModel'")
    self.autoCenterParametersFormLayout.addRow(self.modelLabel, self.modelSelector)

    self.safeZoneXRangeLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.safeZoneXRangeLabel.text = "Safe Zone (Viewport X percentage): "
    self.safeZoneXRangeSlider = slicer.qMRMLRangeWidget()
    self.safeZoneXRangeSlider.maximum = self.rangeSliderMaximum
    self.safeZoneXRangeSlider.minimum = self.rangeSliderMinimum
    self.safeZoneXRangeSlider.maximumValue = self.rangeSliderMaximumValueDefault
    self.safeZoneXRangeSlider.minimumValue = self.rangeSliderMinimumValueDefault
    self.autoCenterParametersFormLayout.addRow(self.safeZoneXRangeLabel,self.safeZoneXRangeSlider)

    self.safeZoneYRangeLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.safeZoneYRangeLabel.setText("Safe Zone (Viewport Y percentage): ")
    self.safeZoneYRangeSlider = slicer.qMRMLRangeWidget()
    self.safeZoneYRangeSlider.maximum = self.rangeSliderMaximum
    self.safeZoneYRangeSlider.minimum = self.rangeSliderMinimum
    self.safeZoneYRangeSlider.maximumValue = self.rangeSliderMaximumValueDefault
    self.safeZoneYRangeSlider.minimumValue = self.rangeSliderMinimumValueDefault
    self.autoCenterParametersFormLayout.addRow(self.safeZoneYRangeLabel,self.safeZoneYRangeSlider)

    self.safeZoneZRangeLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.safeZoneZRangeLabel.setText("Safe Zone (Viewport Z percentage): ")
    self.safeZoneZRangeSlider = slicer.qMRMLRangeWidget()
    self.safeZoneZRangeSlider.maximum = self.rangeSliderMaximum
    self.safeZoneZRangeSlider.minimum = self.rangeSliderMinimum
    self.safeZoneZRangeSlider.maximumValue = self.rangeSliderMaximumValueDefault
    self.safeZoneZRangeSlider.minimumValue = self.rangeSliderMinimumValueDefault
    self.autoCenterParametersFormLayout.addRow(self.safeZoneZRangeLabel,self.safeZoneZRangeSlider)

    self.adjustXLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.adjustXLabel.setText("Adjust Along Camera X")
    self.adjustXCheckbox = qt.QCheckBox()
    self.adjustXCheckbox.setCheckState(self.checkStateCHECKED)
    self.adjustXCheckbox.setToolTip("If checked, adjust the camera so that it aligns with the target model along the x axis.")
    self.autoCenterParametersFormLayout.addRow(self.adjustXLabel,self.adjustXCheckbox)

    self.adjustYLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.adjustYLabel.setText("Adjust Along Camera Y")
    self.adjustYCheckbox = qt.QCheckBox()
    self.adjustYCheckbox.setCheckState(self.checkStateCHECKED)
    self.adjustXCheckbox.setToolTip("If checked, adjust the camera so that it aligns with the target model along the y axis.")
    self.autoCenterParametersFormLayout.addRow(self.adjustYLabel,self.adjustYCheckbox)

    self.adjustZLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.adjustZLabel.setText("Adjust Along Camera Z")
    self.adjustZCheckbox = qt.QCheckBox()
    self.adjustZCheckbox.setCheckState(self.checkStateUNCHECKED)
    self.adjustXCheckbox.setToolTip("If checked, adjust the camera so that it aligns with the target model along the z axis.")
    self.autoCenterParametersFormLayout.addRow(self.adjustZLabel,self.adjustZCheckbox)

    self.updateRateLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.updateRateLabel.setText("Update rate (seconds): ")
    self.updateRateSlider = slicer.qMRMLSliderWidget()
    self.updateRateSlider.minimum = self.updateRateMinSeconds
    self.updateRateSlider.maximum = self.updateRateMaxSeconds
    self.updateRateSlider.value = self.updateRateDefaultSeconds
    self.updateRateSlider.singleStep = self.sliderSingleStepValue
    self.updateRateSlider.pageStep = self.sliderPageStepValue
    self.updateRateSlider.setToolTip("The rate at which the view will be checked and updated.")
    self.autoCenterParametersFormLayout.addRow(self.updateRateLabel,self.updateRateSlider)

    self.timeUnsafeToAdjustLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.timeUnsafeToAdjustLabel.setText("Time Unsafe to Adjust (seconds): ")
    self.timeUnsafeToAdjustSlider = slicer.qMRMLSliderWidget()
    self.timeUnsafeToAdjustSlider.minimum = self.timeUnsafeToAdjustMinSeconds
    self.timeUnsafeToAdjustSlider.maximum = self.timeUnsafeToAdjustMaxSeconds
    self.timeUnsafeToAdjustSlider.value = self.timeUnsafeToAdjustDefaultSeconds
    self.timeUnsafeToAdjustSlider.singleStep = self.sliderSingleStepValue
    self.timeUnsafeToAdjustSlider.pageStep = self.sliderPageStepValue
    self.timeUnsafeToAdjustSlider.setToolTip("The length of time in which the model must be in the unsafe zone before the camera is adjusted.")
    self.autoCenterParametersFormLayout.addRow(self.timeUnsafeToAdjustLabel,self.timeUnsafeToAdjustSlider)

    self.timeAdjustToRestLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.timeAdjustToRestLabel.setText("Time Adjust to Rest (seconds): ")
    self.timeAdjustToRestSlider = slicer.qMRMLSliderWidget()
    self.timeAdjustToRestSlider.minimum = self.timeAdjustToRestMinSeconds
    self.timeAdjustToRestSlider.maximum = self.timeAdjustToRestMaxSeconds
    self.timeAdjustToRestSlider.value = self.timeAdjustToRestDefaultSeconds
    self.timeAdjustToRestSlider.singleStep = self.sliderSingleStepValue
    self.timeAdjustToRestSlider.pageStep = self.sliderPageStepValue
    self.timeAdjustToRestSlider.setToolTip("The length of time an adjustment takes.")
    self.autoCenterParametersFormLayout.addRow(self.timeAdjustToRestLabel,self.timeAdjustToRestSlider)

    self.timeRestToSafeLabel = qt.QLabel(qt.Qt.Horizontal,None)
    self.timeRestToSafeLabel.setText("Time Rest to Safe (seconds): ")
    self.timeRestToSafeSlider = slicer.qMRMLSliderWidget()
    self.timeRestToSafeSlider.minimum = self.timeRestToSafeMinSeconds
    self.timeRestToSafeSlider.maximum = self.timeRestToSafeMaxSeconds
    self.timeRestToSafeSlider.value = self.timeRestToSafeDefaultSeconds
    self.timeRestToSafeSlider.singleStep = self.sliderSingleStepValue
    self.timeRestToSafeSlider.pageStep = self.sliderPageStepValue
    self.timeRestToSafeSlider.setToolTip("The length of time after an adjustment that the camera remains motionless.")
    self.autoCenterParametersFormLayout.addRow(self.timeRestToSafeLabel,self.timeRestToSafeSlider)

    self.toggleAutoCenterButton = qt.QPushButton()
    self.toggleAutoCenterButton.setToolTip("The camera will continuously update its position so that it follows the model.")
    self.toggleAutoCenterButton.setText(self.toggleAutoCenterButtonTextState0)
    self.layout.addWidget(self.toggleAutoCenterButton)

    #Connections
    self.toggleBullseyeButton.connect('clicked()', self.toggleBullseyeButtonPressed)
    self.cameraParallelProjectionCheckbox.connect('stateChanged(int)', self.toggleCameraParallelProjectionCheckboxPressed)
    self.cameraViewAngleSlider.connect('valueChanged(double)', self.changeCameraViewAngleDeg)
    self.cameraParallelScaleSlider.connect('valueChanged(double)', self.changeCameraParallelScale)
    self.cameraXPosSlider.connect('valueChanged(double)', self.changeCameraXPosMm)
    self.cameraYPosSlider.connect('valueChanged(double)', self.changeCameraYPosMm)
    self.cameraZPosSlider.connect('valueChanged(double)', self.changeCameraZPosMm)
    self.upDirectionAnteriorRadioButton.connect('clicked()', self.changeUpToAnterior)
    self.upDirectionPosteriorRadioButton.connect('clicked()', self.changeUpToPosterior)
    self.upDirectionLeftRadioButton.connect('clicked()', self.changeUpToLeft)
    self.upDirectionRightRadioButton.connect('clicked()', self.changeUpToRight)
    self.upDirectionSuperiorRadioButton.connect('clicked()', self.changeUpToSuperior)
    self.upDirectionInferiorRadioButton.connect('clicked()', self.changeUpToInferior)
    self.degreesOfFreedom3RadioButton.connect('clicked()', self.changeInterfaceTo3DOFMode)
    self.degreesOfFreedom5RadioButton.connect('clicked()', self.changeInterfaceTo5DOFMode)
    self.degreesOfFreedom6RadioButton.connect('clicked()', self.changeInterfaceTo6DOFMode)
    self.toggleAutoCenterButton.connect('clicked()', self.toggleAutoCenterButtonPressed)
    self.viewSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.updateWidgets)

    # disable all parameter widgets initially, because view selector will be "none"
    self.disableAutoCenterAllWidgets()
    self.disableBullseyeAllWidgets()

    # Add vertical spacer
    self.layout.addStretch(1)

  def getViewpointForCurrentViewNode(self):
    return self.logic.getViewpointForViewNode(self.viewSelector.currentNode())

  def updateWidgets(self):
    if (not self.viewSelector.currentNode()):
      self.disableAutoCenterAllWidgets()
      self.disableBullseyeAllWidgets()
      return;
    # assume all widgets are to be enabled, disable as necessary
    self.enableAutoCenterAllWidgets()
    self.enableBullseyeAllWidgets()
    self.toggleBullseyeButton.setText(self.toggleBullseyeButtonTextState0)
    self.toggleAutoCenterButton.setText(self.toggleAutoCenterButtonTextState0)

    currentViewpoint = self.getViewpointForCurrentViewNode()
    if (currentViewpoint.currentMode == currentViewpoint.currentModeAUTOCENTER):
      self.disableAutoCenterParameterWidgets()
      self.disableBullseyeAllWidgets()
      self.toggleAutoCenterButton.setText(self.toggleAutoCenterButtonTextState1)
      self.toggleBullseyeButton.setText(self.toggleBullseyeButtonTextState0)
    elif (currentViewpoint.currentMode == currentViewpoint.currentModeBULLSEYE):
      self.disableAutoCenterAllWidgets()
      self.toggleAutoCenterButton.setText(self.toggleAutoCenterButtonTextState0)
      self.toggleBullseyeButton.setText(self.toggleBullseyeButtonTextState1)

    # Bullseye parameters
    self.transformSelector.setCurrentNode(currentViewpoint.bullseyeTransformNode)
    self.degreesOfFreedom6RadioButton.setChecked(self.checkStateUNCHECKED)
    self.degreesOfFreedom5RadioButton.setChecked(self.checkStateUNCHECKED)
    self.degreesOfFreedom3RadioButton.setChecked(self.checkStateUNCHECKED)
    if (currentViewpoint.bullseyeForcedUpDirection and currentViewpoint.bullseyeForcedTarget):
      self.degreesOfFreedom3RadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeForcedUpDirection):
      self.degreesOfFreedom5RadioButton.setChecked(self.checkStateCHECKED)
    else:
      self.degreesOfFreedom6RadioButton.setChecked(self.checkStateCHECKED)
    self.upDirectionAnteriorRadioButton.setChecked(self.checkStateUNCHECKED)
    self.upDirectionPosteriorRadioButton.setChecked(self.checkStateUNCHECKED)
    self.upDirectionRightRadioButton.setChecked(self.checkStateUNCHECKED)
    self.upDirectionLeftRadioButton.setChecked(self.checkStateUNCHECKED)
    self.upDirectionSuperiorRadioButton.setChecked(self.checkStateUNCHECKED)
    self.upDirectionInferiorRadioButton.setChecked(self.checkStateUNCHECKED)
    if (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASAnterior)):
      self.upDirectionRightRadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASLeft)):
      self.upDirectionLeftRadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASAnterior)):
      self.upDirectionAnteriorRadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASPosterior)):
      self.upDirectionPosteriorRadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASSuperior)):
      self.upDirectionSuperiorRadioButton.setChecked(self.checkStateCHECKED)
    elif (currentViewpoint.bullseyeIsUpDirectionEqualTo(currentViewpoint.bullseyeUpDirectionRASInferior)):
      self.upDirectionInferiorRadioButton.setChecked(self.checkStateCHECKED)
    self.targetModelSelector.setCurrentNode(currentViewpoint.bullseyeTargetModelNode)
    self.cameraViewAngleSlider.value = currentViewpoint.bullseyeCameraViewAngleDeg
    self.cameraParallelScaleSlider.value = currentViewpoint.bullseyeCameraParallelScale
    self.cameraXPosSlider.value = currentViewpoint.bullseyeCameraXPosMm
    self.cameraYPosSlider.value = currentViewpoint.bullseyeCameraYPosMm
    self.cameraZPosSlider.value = currentViewpoint.bullseyeCameraZPosMm
    if (currentViewpoint.bullseyeCameraParallelProjection):
      self.cameraParallelProjectionCheckbox.setCheckState(self.checkStateCHECKED)
    else:
      self.cameraParallelProjectionCheckbox.setCheckState(self.checkStateUNCHECKED)
    # Auto-center parameters
    self.modelSelector.setCurrentNode(currentViewpoint.autoCenterModelNode)
    self.safeZoneXRangeSlider.maximumValue = currentViewpoint.autoCenterSafeXMaximumNormalizedViewport*self.sliderMultiplier
    self.safeZoneXRangeSlider.minimumValue = currentViewpoint.autoCenterSafeXMinimumNormalizedViewport*self.sliderMultiplier
    self.safeZoneYRangeSlider.maximumValue = currentViewpoint.autoCenterSafeYMaximumNormalizedViewport*self.sliderMultiplier
    self.safeZoneYRangeSlider.minimumValue = currentViewpoint.autoCenterSafeYMinimumNormalizedViewport*self.sliderMultiplier
    self.safeZoneZRangeSlider.maximumValue = currentViewpoint.autoCenterSafeZMaximumNormalizedViewport*self.sliderMultiplier
    self.safeZoneZRangeSlider.minimumValue = currentViewpoint.autoCenterSafeZMinimumNormalizedViewport*self.sliderMultiplier
    self.updateRateSlider.value = currentViewpoint.autoCenterUpdateRateSeconds
    self.timeUnsafeToAdjustSlider.value = currentViewpoint.autoCenterTimeUnsafeToAdjustMaximumSeconds
    self.timeAdjustToRestSlider.value = currentViewpoint.autoCenterTimeAdjustToRestMaximumSeconds
    self.timeRestToSafeSlider.value = currentViewpoint.autoCenterTimeRestToSafeMaximumSeconds
    if (currentViewpoint.autoCenterAdjustX):
      self.adjustXCheckbox.setCheckState(self.checkStateCHECKED)
    else:
      self.adjustXCheckbox.setCheckState(self.checkStateUNCHECKED)
    if (currentViewpoint.autoCenterAdjustY):
      self.adjustYCheckbox.setCheckState(self.checkStateCHECKED)
    else:
      self.adjustYCheckbox.setCheckState(self.checkStateUNCHECKED)
    if (currentViewpoint.autoCenterAdjustZ):
      self.adjustZCheckbox.setCheckState(self.checkStateCHECKED)
    else:
      self.adjustZCheckbox.setCheckState(self.checkStateUNCHECKED)

  def toggleBullseyeButtonPressed(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    if currentViewpoint.currentMode == currentViewpoint.currentModeOFF:
      self.updateBullseyeParameters();
      currentViewpoint.bullseyeStart()
    elif currentViewpoint.currentMode == currentViewpoint.currentModeBULLSEYE:
      currentViewpoint.bullseyeStop()
    else:
      logging.error("Error: Unhandled case in toggleBullseyeButtonPressed. Current state is neither off nor bullseye view.")
    self.updateWidgets()

  def toggleAutoCenterButtonPressed(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    if currentViewpoint.currentMode == currentViewpoint.currentModeOFF:
      self.updateAutoCenterLogicParameters()
      currentViewpoint.autoCenterStart()
    elif currentViewpoint.currentMode == currentViewpoint.currentModeAUTOCENTER:
      currentViewpoint.autoCenterStop()
    else:
      logging.error("Error: Unhandled case in toggleAutoCenterButtonPressed. Current state is neither off nor autocenter.")
    self.updateWidgets()

  # SPECIFIC TO BULLSEYE

  def updateBullseyeParameters(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    if (self.viewSelector.currentNode()):
      currentViewpoint.setViewNode(self.viewSelector.currentNode())
    if (self.transformSelector.currentNode()):
      currentViewpoint.bullseyeSetTransformNode(self.transformSelector.currentNode())
    if (self.targetModelSelector.currentNode()):
      currentViewpoint.bullseyeSetTargetModelNode(self.targetModelSelector.currentNode())

  def enableBullseyeSelectors(self):
    self.transformSelector.enabled = True
    self.targetModelSelector.enabled = True

  def disableBullseyeSelectors(self):
    self.transformSelector.enabled = False
    self.targetModelSelector.enabled = False

  def enableBullseyeParameterWidgets(self):
    self.enableBullseyeSelectors()
    self.degreesOfFreedom3RadioButton.enabled = True
    self.degreesOfFreedom5RadioButton.enabled = True
    self.degreesOfFreedom6RadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.upDirectionAnteriorRadioButton.enabled = True
    self.cameraViewAngleSlider.enabled = True
    self.cameraParallelScaleSlider.enabled = True
    self.cameraXPosSlider.enabled = True
    self.cameraYPosSlider.enabled = True
    self.cameraZPosSlider.enabled = True
    self.cameraParallelProjectionCheckbox.enabled = True

  def disableBullseyeParameterWidgets(self):
    self.disableBullseyeSelectors()
    self.degreesOfFreedom3RadioButton.enabled = False
    self.degreesOfFreedom5RadioButton.enabled = False
    self.degreesOfFreedom6RadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.upDirectionAnteriorRadioButton.enabled = False
    self.cameraViewAngleSlider.enabled = False
    self.cameraParallelScaleSlider.enabled = False
    self.cameraXPosSlider.enabled = False
    self.cameraYPosSlider.enabled = False
    self.cameraZPosSlider.enabled = False
    self.cameraParallelProjectionCheckbox.enabled = False

  def enableBullseyeAllWidgets(self):
    self.enableBullseyeParameterWidgets()
    self.toggleBullseyeButton.enabled = True

  def disableBullseyeAllWidgets(self):
    self.disableBullseyeParameterWidgets()
    self.toggleBullseyeButton.enabled = False

  def toggleCameraParallelProjectionCheckboxPressed(self, dummyState): # dummyState is a tristate variable, we just want True/False
    currentViewpoint = self.getViewpointForCurrentViewNode()
    state = self.cameraParallelProjectionCheckbox.isChecked()
    currentViewpoint.bullseyeSetCameraParallelProjection(state)
    if (state == False): # unchecked
      self.cameraParallelScaleLabel.setVisible(False)
      self.cameraParallelScaleSlider.setVisible(False)
      self.cameraViewAngleLabel.setVisible(True)
      self.cameraViewAngleSlider.setVisible(True)
    else: # checked
      self.cameraParallelScaleLabel.setVisible(True)
      self.cameraParallelScaleSlider.setVisible(True)
      self.cameraViewAngleLabel.setVisible(False)
      self.cameraViewAngleSlider.setVisible(False)

  def changeCameraViewAngleDeg(self, val):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetCameraViewAngleDeg(val)

  def changeCameraParallelScale(self, val):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetCameraParallelScale(val)

  def changeCameraXPosMm(self, val):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetCameraXPosMm(val)

  def changeCameraYPosMm(self, val):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetCameraYPosMm(val)

  def changeCameraZPosMm(self, val):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetCameraZPosMm(val)

  def changeInterfaceTo3DOFMode(self):
    self.upDirectionCollapsibleButton.setVisible(True)
    self.targetModelCollapsibleButton.setVisible(True)
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeChangeTo3DOFMode()

  def changeInterfaceTo5DOFMode(self):
    self.upDirectionCollapsibleButton.setVisible(True)
    self.targetModelCollapsibleButton.setVisible(False)
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeChangeTo5DOFMode()

  def changeInterfaceTo6DOFMode(self):
    self.upDirectionCollapsibleButton.setVisible(False)
    self.targetModelCollapsibleButton.setVisible(False)
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeChangeTo6DOFMode()

  def changeUpToAnterior(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASAnterior)

  def changeUpToPosterior(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASPosterior)

  def changeUpToRight(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASRight)

  def changeUpToLeft(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASLeft)

  def changeUpToSuperior(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASSuperior)

  def changeUpToInferior(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.bullseyeSetUpDirectionRAS(currentViewpoint.bullseyeUpDirectionRASInferior)

  # SPECIFIC TO AUTO-CENTER

  def updateAutoCenterLogicParameters(self):
    currentViewpoint = self.getViewpointForCurrentViewNode()
    currentViewpoint.setViewNode(self.viewSelector.currentNode())
    currentViewpoint.autoCenterSetModelNode(self.modelSelector.currentNode())
    currentViewpoint.autoCenterSetSafeXMaximum(self.safeZoneXRangeSlider.maximumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetSafeXMinimum(self.safeZoneXRangeSlider.minimumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetSafeYMaximum(self.safeZoneYRangeSlider.maximumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetSafeYMinimum(self.safeZoneYRangeSlider.minimumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetSafeZMaximum(self.safeZoneZRangeSlider.maximumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetSafeZMinimum(self.safeZoneZRangeSlider.minimumValue/self.sliderMultiplier)
    currentViewpoint.autoCenterSetAdjustX(self.adjustXCheckbox.isChecked())
    currentViewpoint.autoCenterSetAdjustY(self.adjustYCheckbox.isChecked())
    currentViewpoint.autoCenterSetAdjustZ(self.adjustZCheckbox.isChecked())
    currentViewpoint.autoCenterSetUpdateRateSeconds(self.updateRateSlider.value)
    currentViewpoint.autoCenterSetTimeUnsafeToAdjustMaximumSeconds(self.timeUnsafeToAdjustSlider.value)
    currentViewpoint.autoCenterSetTimeAdjustToRestMaximumSeconds(self.timeAdjustToRestSlider.value)
    currentViewpoint.autoCenterSetTimeRestToSafeMaximumSeconds(self.timeRestToSafeSlider.value)

  def enableAutoCenterParameterWidgets(self):
    self.modelSelector.enabled = True
    self.safeZoneXRangeSlider.enabled = True
    self.safeZoneYRangeSlider.enabled = True
    self.safeZoneZRangeSlider.enabled = True
    self.adjustXCheckbox.enabled = True
    self.adjustYCheckbox.enabled = True
    self.adjustZCheckbox.enabled = True
    self.updateRateSlider.enabled = True
    self.timeUnsafeToAdjustSlider.enabled = True
    self.timeAdjustToRestSlider.enabled = True
    self.timeRestToSafeSlider.enabled = True

  def disableAutoCenterParameterWidgets(self):
    self.modelSelector.enabled = False
    self.safeZoneXRangeSlider.enabled = False
    self.safeZoneYRangeSlider.enabled = False
    self.safeZoneZRangeSlider.enabled = False
    self.adjustXCheckbox.enabled = False
    self.adjustYCheckbox.enabled = False
    self.adjustZCheckbox.enabled = False
    self.updateRateSlider.enabled = False
    self.timeUnsafeToAdjustSlider.enabled = False
    self.timeAdjustToRestSlider.enabled = False
    self.timeRestToSafeSlider.enabled = False

  def enableAutoCenterAllWidgets(self):
    self.enableAutoCenterParameterWidgets()
    self.toggleAutoCenterButton.enabled = True

  def disableAutoCenterAllWidgets(self):
    self.disableAutoCenterParameterWidgets()
    self.toggleAutoCenterButton.enabled = False

#
# ViewpointLogic
#

class ViewpointLogic:

  def __init__(self):
    self.nodeInstanceDictionary = {}

  def getViewpointForViewNode(self, viewNode):
    if (viewNode == None):
      logging.error("viewNode given to Viewpoint logic is None. Aborting operation.")
      return
    if (not viewNode in self.nodeInstanceDictionary):
      self.nodeInstanceDictionary[viewNode] = ViewpointInstance()
    return self.nodeInstanceDictionary[viewNode]

#
# Viewpoint Instance
# Each view is associated with its own viewpoint instance,
# this allows support of multiple views with their own
# viewpoint parameters and settings.
#

class ViewpointInstance:
  def __init__(self):
    # global
    self.viewNode = None

    self.currentMode = 0
    self.currentModeOFF = 0
    self.currentModeBULLSEYE = 1
    self.currentModeAUTOCENTER = 2

    # BULLSEYE
    self.bullseyeTransformNode = None
    self.bullseyeTransformNodeObserverTags = []
    self.bullseyeCameraXPosMm =  0.0
    self.bullseyeCameraYPosMm =  0.0
    self.bullseyeCameraZPosMm =  0.0

    self.bullseyeCameraParallelProjection = False # False = perspective, True = parallel. This is consistent with the
                                          # representation in the vtkCamera class and documentation

    self.bullseyeForcedUpDirection = False # False = if the user rotates the tool, then the camera rotates with it
                                   # True = the up direction is fixed according to this next variable:
    self.bullseyeUpDirectionRAS = [0,1,0] # Anterior by default
    self.bullseyeUpDirectionRASRight = [1,0,0]
    self.bullseyeUpDirectionRASLeft = [-1,0,0]
    self.bullseyeUpDirectionRASAnterior = [0,1,0]
    self.bullseyeUpDirectionRASPosterior = [0,-1,0]
    self.bullseyeUpDirectionRASSuperior = [0,0,1]
    self.bullseyeUpDirectionRASInferior = [0,0,-1]

    self.bullseyeForcedTarget = False # False = camera points the direction the user is pointing it
                              # True = camera always points to the target model
    self.bullseyeTargetModelNode = None
    self.bullseyeTargetModelMiddleInRASMm = [0,0,0]

    self.bullseyeCameraViewAngleDeg  =  30.0
    self.bullseyeCameraParallelScale = 1.0

    # AUTO-CENTER
    #inputs
    self.autoCenterSafeXMinimumNormalizedViewport = -1.0
    self.autoCenterSafeXMaximumNormalizedViewport = 1.0
    self.autoCenterSafeYMinimumNormalizedViewport = -1.0
    self.autoCenterSafeYMaximumNormalizedViewport = 1.0
    self.autoCenterSafeZMinimumNormalizedViewport = -1.0
    self.autoCenterSafeZMaximumNormalizedViewport = 1.0

    self.autoCenterAdjustX = True
    self.autoCenterAdjustY = True
    self.autoCenterAdjustZ = False

    self.autoCenterModelNode = None

    self.autoCenterTimeUnsafeToAdjustMaximumSeconds = 1
    self.autoCenterTimeAdjustToRestMaximumSeconds = 0.2
    self.autoCenterTimeRestToSafeMaximumSeconds = 1

    self.autoCenterUpdateRateSeconds = 0.02

    # current state
    self.autoCenterSystemTimeAtLastUpdateSeconds = 0
    self.autoCenterTimeInStateSeconds = 0
    self.autoCenterState = 0 # 0 = in safe zone (initial state), 1 = in unsafe zone, 2 = adjusting, 3 = resting
    self.autoCenterStateSAFE = 0
    self.autoCenterStateUNSAFE = 1
    self.autoCenterStateADJUST = 2
    self.autoCenterStateREST = 3
    self.autoCenterBaseCameraTranslationRas = [0,0,0]
    self.autoCenterBaseCameraPositionRas = [0,0,0]
    self.autoCenterBaseCameraFocalPointRas = [0,0,0]
    self.autoCenterModelInSafeZone = True

    self.autoCenterModelTargetPositionViewport = [0,0,0]

  def setViewNode(self, node):
    self.viewNode = node

  def getCurrentMode(self):
    return self.currentMode

  def isCurrentModeOFF(self):
    return (self.currentMode == self.currentModeOFF)

  def isCurrentModeBullseye(self):
    return (self.currentMode == self.currentModeBULLSEYE)

  def isCurrentModeAutoCenter(self):
    return (self.currentMode == self.currentModeAUTOCENTER)

  def getCameraNode(self, viewName):
    """
    Get camera for the selected 3D view
    """
    camerasLogic = slicer.modules.cameras.logic()
    camera = camerasLogic.GetViewActiveCameraNode(slicer.util.getNode(viewName))
    return camera

  def convertRasToViewport(self, positionRas):
    """Computes normalized view coordinates from RAS coordinates
    Normalized view coordinates origin is in bottom-left corner, range is [-1,+1]
    """
    x = vtk.mutable(positionRas[0])
    y = vtk.mutable(positionRas[1])
    z = vtk.mutable(positionRas[2])
    view = slicer.app.layoutManager().threeDWidget(self.getThreeDWidgetIndex()).threeDView()
    renderer = view.renderWindow().GetRenderers().GetItemAsObject(0)
    renderer.WorldToView(x,y,z)
    return [x.get(), y.get(), z.get()]

  def convertViewportToRas(self, positionViewport):
    x = vtk.mutable(positionViewport[0])
    y = vtk.mutable(positionViewport[1])
    z = vtk.mutable(positionViewport[2])
    view = slicer.app.layoutManager().threeDWidget(self.getThreeDWidgetIndex()).threeDView()
    renderer = view.renderWindow().GetRenderers().GetItemAsObject(0)
    renderer.ViewToWorld(x,y,z)
    return [x.get(), y.get(), z.get()]

  def convertPointRasToCamera(self, positionRas):
    viewName = self.viewNode.GetName()
    cameraNode = self.getCameraNode(viewName)
    cameraObj = cameraNode.GetCamera()
    modelViewTransform = cameraObj.GetModelViewTransformObject()
    positionRasHomog = [positionRas[0], positionRas[1], positionRas[2], 1] # convert to homogeneous
    positionCamHomog = [0,0,0,1] # to be filled in
    modelViewTransform.MultiplyPoint(positionRasHomog, positionCamHomog)
    positionCam = [positionCamHomog[0], positionCamHomog[1], positionCamHomog[2]] # convert from homogeneous
    return positionCam

  def convertVectorCameraToRas(self, positionCam):
    viewName = self.viewNode.GetName()
    cameraNode = self.getCameraNode(viewName)
    cameraObj = cameraNode.GetCamera()
    modelViewTransform = cameraObj.GetModelViewTransformObject()
    modelViewMatrix = modelViewTransform.GetMatrix()
    modelViewInverseMatrix = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Invert(modelViewMatrix, modelViewInverseMatrix)
    modelViewInverseTransform = vtk.vtkTransform()
    modelViewInverseTransform.DeepCopy(modelViewTransform)
    modelViewInverseTransform.SetMatrix(modelViewInverseMatrix)
    positionCamHomog = [positionCam[0], positionCam[1], positionCam[2], 0] # convert to homogeneous
    positionRasHomog = [0,0,0,0] # to be filled in
    modelViewInverseTransform.MultiplyPoint(positionCamHomog, positionRasHomog)
    positionRas = [positionRasHomog[0], positionRasHomog[1], positionRasHomog[2]] # convert from homogeneous
    return positionRas

  def resetCameraClippingRange(self):
    view = slicer.app.layoutManager().threeDWidget(self.getThreeDWidgetIndex()).threeDView()
    renderer = view.renderWindow().GetRenderers().GetItemAsObject(0)
    renderer.ResetCameraClippingRange()

  def getThreeDWidgetIndex(self):
    if (not self.viewNode):
      logging.error("Error in getThreeDWidgetIndex: No View node selected. Returning 0.");
      return 0
    layoutManager = slicer.app.layoutManager()
    for threeDViewIndex in xrange(layoutManager.threeDViewCount):
      threeDViewNode = layoutManager.threeDWidget(threeDViewIndex).threeDView().mrmlViewNode()
      if (threeDViewNode == self.viewNode):
        return threeDViewIndex
    logging.error("Error in getThreeDWidgetIndex: Can't find the index. Selected View does not exist? Returning 0.");
    return 0

  # TRACK VIEW

  def bullseyeStart(self):
    logging.debug("Start Bullseye Mode")
    if (self.currentMode != self.currentModeOFF):
      logging.error("Cannot activate viewpoint until the current mode is set to off!")
      return

    if (not self.viewNode):
      logging.warning("A node is missing. Nothing will happen until the comboboxes have items selected.")
      return

    if (not self.bullseyeTransformNode):
      logging.warning("Transform node is missing. Nothing will happen until a transform node is provided as input.")
      return

    if (self.bullseyeForcedTarget and not self.bullseyeTargetModelNode):
      logging.error("Error in bullseyeSetTargetModelNode: No targetModelNode provided as input when forced target is set. Check input parameters.")
      return

    self.currentMode = self.currentModeBULLSEYE
    self.bullseyeAddObservers()
    self.bullseyeUpdate()

  def bullseyeStop(self):
    logging.debug("Stop Viewpoint Mode")
    if (self.currentMode != self.currentModeBULLSEYE):
      logging.error("bullseyeStop was called, but viewpoint mode is not BULLSEYE. No action performed.")
      return
    self.currentMode = self.currentModeOFF
    self.bullseyeRemoveObservers();

  def bullseyeUpdate(self):
    # no logging - it slows Slicer down a *lot*

    # Need to set camera attributes according to the concatenated transform
    toolCameraToRASTransform = vtk.vtkGeneralTransform()
    self.bullseyeTransformNode.GetTransformToWorld(toolCameraToRASTransform)

    cameraOriginInRASMm = self.bullseyeComputeCameraOriginInRASMm(toolCameraToRASTransform)
    focalPointInRASMm = self.bullseyeComputeCameraFocalPointInRASMm(toolCameraToRASTransform)
    upDirectionInRAS = self.bullseyeComputeCameraUpDirectionInRAS(toolCameraToRASTransform,cameraOriginInRASMm,focalPointInRASMm)

    self.bullseyeSetCameraParameters(cameraOriginInRASMm,focalPointInRASMm,upDirectionInRAS)

  def bullseyeAddObservers(self): # mostly copied from PositionErrorMapping.py in PLUS
    logging.debug("Adding observers...")
    transformModifiedEvent = 15000
    transformNode = self.bullseyeTransformNode
    while transformNode:
      logging.debug("Add observer to {0}".format(transformNode.GetName()))
      self.bullseyeTransformNodeObserverTags.append([transformNode, transformNode.AddObserver(transformModifiedEvent, self.bullseyeOnTransformModified)])
      transformNode = transformNode.GetParentTransformNode()
    logging.debug("Done adding observers")

  def bullseyeRemoveObservers(self):
    logging.debug("Removing observers...")
    for nodeTagPair in self.bullseyeTransformNodeObserverTags:
      nodeTagPair[0].RemoveObserver(nodeTagPair[1])
    logging.debug("Done removing observers")

  def bullseyeOnTransformModified(self, observer, eventid):
    # no logging - it slows Slicer down a *lot*
    self.bullseyeUpdate()

  def bullseyeSetTransformNode(self, transformNode):
    self.bullseyeTransformNode = transformNode

  def bullseyeSetTargetModelNode(self, targetModelNode):
    if (self.bullseyeForcedTarget and not targetModelNode):
      logging.error("Error in bullseyeSetTargetModelNode: No targetModelNode provided as input. Check input parameters.")
      return
    self.bullseyeTargetModelNode = targetModelNode
    targetModel = targetModelNode.GetPolyData()
    targetModelBoundingBox = targetModel.GetBounds()
    # find the middle of the target model
    middleXInTumorMm = ( targetModelBoundingBox[0] + targetModelBoundingBox[1]) / 2
    middleYInTumorMm = ( targetModelBoundingBox[2] + targetModelBoundingBox[3]) / 2
    middleZInTumorMm = ( targetModelBoundingBox[4] + targetModelBoundingBox[5]) / 2
    middlePInTumorMm = 1 # represent as a homogeneous point
    middlePointInTumorMm4 = [middleXInTumorMm,middleYInTumorMm,middleZInTumorMm,middlePInTumorMm]
    middlePointInRASMm4 = [0,0,0,1]; # placeholder values
    targetModelNode.TransformPointToWorld(middlePointInTumorMm4,middlePointInRASMm4)
    # reduce dimensionality back to 3
    middlePointInRASMm3 = [middlePointInRASMm4[0], middlePointInRASMm4[1], middlePointInRASMm4[2]]
    self.bullseyeTargetModelMiddleInRASMm = middlePointInRASMm3

  def bullseyeChangeTo3DOFMode(self):
    self.bullseyeForcedUpDirection = True
    self.bullseyeForcedTarget = True

  def bullseyeChangeTo5DOFMode(self):
    self.bullseyeForcedUpDirection = True
    self.bullseyeForcedTarget = False

  def bullseyeChangeTo6DOFMode(self):
    self.bullseyeForcedUpDirection = False
    self.bullseyeForcedTarget = False

  def bullseyeIsUpDirectionEqualTo(self, compareDirection):
    if (compareDirection[0]*self.bullseyeUpDirectionRAS[0]+
        compareDirection[1]*self.bullseyeUpDirectionRAS[1]+
        compareDirection[2]*self.bullseyeUpDirectionRAS[2] > 0.9999): # dot product close to 1
      return True;
    return False;

  def bullseyeSetCameraParallelProjection(self,newParallelProjectionState):
    logging.debug("bullseyeSetCameraParallelProjection")
    self.bullseyeCameraParallelProjection = newParallelProjectionState

  def bullseyeSetCameraViewAngleDeg(self,valueDeg):
    logging.debug("bullseyeSetCameraViewAngleDeg")
    self.bullseyeCameraViewAngleDeg = valueDeg
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeSetCameraParallelScale(self,newScale):
    logging.debug("bullseyeSetCameraParallelScale")
    self.bullseyeCameraParallelScale = newScale
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeSetCameraXPosMm(self,valueMm):
    logging.debug("bullseyeSetCameraXPosMm")
    self.bullseyeCameraXPosMm = valueMm
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeSetCameraYPosMm(self,valueMm):
    logging.debug("bullseyeSetCameraYPosMm")
    self.bullseyeCameraYPosMm = valueMm
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeSetCameraZPosMm(self,valueMm):
    logging.debug("bullseyeSetCameraZPosMm")
    self.bullseyeCameraZPosMm = valueMm
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeSetUpDirectionRAS(self,vectorInRAS):
    logging.debug("bullseyeSetUpDirectionRAS")
    self.bullseyeUpDirectionRAS = vectorInRAS
    if (self.currentMode == self.currentModeBULLSEYE):
      self.bullseyeUpdate()

  def bullseyeComputeCameraOriginInRASMm(self, toolCameraToRASTransform):
    # Need to get camera origin and axes from camera coordinates into Slicer RAS coordinates
    cameraOriginInToolCameraMm = [self.bullseyeCameraXPosMm,self.bullseyeCameraYPosMm,self.bullseyeCameraZPosMm]
    cameraOriginInRASMm = [0,0,0] # placeholder values
    toolCameraToRASTransform.TransformPoint(cameraOriginInToolCameraMm,cameraOriginInRASMm)
    return cameraOriginInRASMm

  def bullseyeComputeCameraFocalPointInRASMm(self, toolCameraToRASTransform):
    focalPointInRASMm = [0,0,0]; # placeholder values
    if (self.bullseyeForcedTarget == True):
      focalPointInRASMm = self.bullseyeTargetModelMiddleInRASMm
    else:
      # camera distance depends on slider, but lies in -z (which is the direction that the camera is facing)
      focalPointInToolCameraMm = [self.bullseyeCameraXPosMm,self.bullseyeCameraYPosMm,self.bullseyeCameraZPosMm-200] # The number 200 mm is arbitrary. TODO: Change so that this is the camera-tumor distance
      focalPointInRASMm = [0,0,0] # placeholder values
      toolCameraToRASTransform.TransformPoint(focalPointInToolCameraMm,focalPointInRASMm)
    return focalPointInRASMm

  def bullseyeComputeCameraProjectionDirectionInRAS(self, cameraOriginInRASMm, focalPointInRASMm):
    math = vtk.vtkMath()
    directionFromOriginToFocalPointRAS = [0,0,0] # placeholder values
    math.Subtract(focalPointInRASMm,cameraOriginInRASMm,directionFromOriginToFocalPointRAS)
    math.Normalize(directionFromOriginToFocalPointRAS)
    numberDimensions = 3;
    lengthMm = math.Norm(directionFromOriginToFocalPointRAS,numberDimensions)
    epsilon = 0.0001
    if (lengthMm < epsilon):
      logging.warning("Warning: bullseyeComputeCameraProjectionDirectionInRAS() is computing a zero vector. Check target model? Using [0,0,-1] as target direction.")
      directionFromOriginToFocalPointRAS = [0,0,-1];
    return directionFromOriginToFocalPointRAS

  def bullseyeComputeCameraUpDirectionInRAS(self, toolCameraToRASTransform, cameraOriginInRASMm, focalPointInRASMm):
    upDirectionInRAS = [0,0,0] # placeholder values
    if (self.bullseyeForcedUpDirection == True):
      math = vtk.vtkMath()
      # cross product of forwardDirectionInRAS vector with upInRAS vector is the rightDirectionInRAS vector
      upInRAS = self.bullseyeUpDirectionRAS
      forwardDirectionInRAS = self.bullseyeComputeCameraProjectionDirectionInRAS(cameraOriginInRASMm, focalPointInRASMm)
      rightDirectionInRAS = [0,0,0] # placeholder values
      math.Cross(forwardDirectionInRAS,upInRAS,rightDirectionInRAS)
      numberDimensions = 3;
      lengthMm = math.Norm(rightDirectionInRAS,numberDimensions)
      epsilon = 0.0001
      if (lengthMm < epsilon): # must check for this case
        logging.warning("Warning: length of cross product in bullseyeComputeCameraUpDirectionInRAS is zero. Workaround used")
        backupUpDirectionInRAS = [1,1,1] # if the previous cross product was zero, then this shouldn't be
        math.Normalize(backupUpDirectionInRAS)
        upInRAS = backupUpDirectionInRAS
        math.Cross(forwardDirectionInRAS,upInRAS,rightDirectionInRAS)
      math.Normalize(rightDirectionInRAS)
      # now compute the cross product between the rightDirectionInRAS and forwardDirectionInRAS directions to get a corrected up vector
      upDirectionInRAS = [0,0,0] # placeholder values
      math.Cross(rightDirectionInRAS,forwardDirectionInRAS,upDirectionInRAS)
      math.Normalize(upDirectionInRAS)
    else:
      upDirectionInToolCamera = [0,1,0] # standard up direction in OpenGL
      dummyPoint = [0,0,0] # Needed by the TransformVectorAtPoint function
      toolCameraToRASTransform.TransformVectorAtPoint(dummyPoint,upDirectionInToolCamera,upDirectionInRAS)
    return upDirectionInRAS

  def bullseyeSetCameraParameters(self,cameraOriginInRASMm,focalPointInRASMm,upDirectionInRAS):
    viewName = self.viewNode.GetName()
    cameraNode = self.getCameraNode(viewName)
    camera = cameraNode.GetCamera()
    if (self.bullseyeCameraParallelProjection == False):
      camera.SetViewAngle(self.bullseyeCameraViewAngleDeg)
    elif (self.bullseyeCameraParallelProjection == True):
      camera.SetParallelScale(self.bullseyeCameraParallelScale)
    else:
      logging.error("Error in Viewpoint: cameraParallelProjection is not 0 or 1. No projection mode has been set! No updates are being performed.")
      return
    # Parallel (a.k.a. orthographic) / perspective projection mode is stored in the view node.
    # Change it in the view node instead of directly in the camera VTK object
    # (if we changed the projection mode in the camera VTK object then the next time the camera is updated from the view node
    # the rendering mode is reset to the value stored in the view node).
    viewNode = slicer.mrmlScene.GetNodeByID(cameraNode.GetActiveTag())
    viewNodeParallelProjection = (viewNode.GetRenderMode() == slicer.vtkMRMLViewNode.Orthographic)
    if viewNodeParallelProjection != self.bullseyeCameraParallelProjection:
      viewNode.SetRenderMode(slicer.vtkMRMLViewNode.Orthographic if self.bullseyeCameraParallelProjection else slicer.vtkMRMLViewNode.Perspective)

    camera.SetRoll(180) # appears to be the default value for a camera in Slicer
    camera.SetPosition(cameraOriginInRASMm)
    camera.SetFocalPoint(focalPointInRASMm)
    camera.SetViewUp(upDirectionInRAS)

    self.resetCameraClippingRange() # without this line, some objects do not appear in the 3D view

  # AUTO-CENTER

  def autoCenterStart(self):
    if (self.currentMode != self.currentModeOFF):
      logging.error("Viewpoints is already active! Can't activate auto-center mode until the current mode is off!")
      return
    if not self.viewNode:
      logging.warning("View node not set. Will not proceed until view node is selected.")
      return
    if not self.autoCenterModelNode:
      logging.warning("Model node not set. Will not proceed until model node is selected.")
      return
    self.autoCenterSetModelTargetPositionViewport()
    self.autoCenterSystemTimeAtLastUpdateSeconds = time.time()
    nextUpdateTimerMilliseconds = self.autoCenterUpdateRateSeconds * 1000
    qt.QTimer.singleShot(nextUpdateTimerMilliseconds ,self.autoCenterUpdate)

    self.currentMode = self.currentModeAUTOCENTER

  def autoCenterStop(self):
    logging.debug("autoCenterStop")
    if (self.currentMode != self.currentModeAUTOCENTER):
      logging.error("autoCenterStop was called, but viewpoint mode is not AUTOCENTER. No action performed.")
      return
    self.currentMode = self.currentModeOFF

  def autoCenterUpdate(self):
    if (self.currentMode != self.currentModeAUTOCENTER):
      return

    deltaTimeSeconds = time.time() - self.autoCenterSystemTimeAtLastUpdateSeconds
    self.autoCenterSystemTimeAtLastUpdateSeconds = time.time()

    self.autoCenterTimeInStateSeconds = self.autoCenterTimeInStateSeconds + deltaTimeSeconds

    self.autoCenterUpdateModelInSafeZone()
    self.autoCenterApplyStateMachine()

    nextUpdateTimerMilliseconds = self.autoCenterUpdateRateSeconds * 1000
    qt.QTimer.singleShot(nextUpdateTimerMilliseconds ,self.autoCenterUpdate)

  def autoCenterApplyStateMachine(self):
    if (self.autoCenterState == self.autoCenterStateUNSAFE and self.autoCenterModelInSafeZone):
      self.autoCenterState = self.autoCenterStateSAFE
      self.autoCenterTimeInStateSeconds = 0
    if (self.autoCenterState == self.autoCenterStateSAFE and not self.autoCenterModelInSafeZone):
      self.autoCenterState = self.autoCenterStateUNSAFE
      self.autoCenterTimeInStateSeconds = 0
    if (self.autoCenterState == self.autoCenterStateUNSAFE and self.autoCenterTimeInStateSeconds >= self.autoCenterTimeUnsafeToAdjustMaximumSeconds):
      self.autoCenterSetCameraTranslationParameters()
      self.autoCenterState = self.autoCenterStateADJUST
      self.autoCenterTimeInStateSeconds = 0
    if (self.autoCenterState == self.autoCenterStateADJUST):
      self.autoCenterTranslateCamera()
      if (self.autoCenterTimeInStateSeconds >= self.autoCenterTimeAdjustToRestMaximumSeconds):
        self.autoCenterState = self.autoCenterStateREST
        self.autoCenterTimeInStateSeconds = 0
    if (self.autoCenterState == self.autoCenterStateREST and self.autoCenterTimeInStateSeconds >= self.autoCenterTimeRestToSafeMaximumSeconds):
      self.autoCenterState = self.autoCenterStateSAFE
      self.autoCenterTimeInStateSeconds = 0

  def autoCenterUpdateModelInSafeZone(self):
    if (self.autoCenterState == self.autoCenterStateADJUST or
        self.autoCenterState == self.autoCenterStateREST):
      return
    pointsRas = self.autoCenterGetModelCurrentBoundingBoxPointsRas()
    # Assume we are safe, until shown otherwise
    foundSafe = True
    for pointRas in pointsRas:
      coordsNormalizedViewport = self.convertRasToViewport(pointRas)
      XNormalizedViewport = coordsNormalizedViewport[0]
      YNormalizedViewport = coordsNormalizedViewport[1]
      ZNormalizedViewport = coordsNormalizedViewport[2]
      if ( XNormalizedViewport > self.autoCenterSafeXMaximumNormalizedViewport or
           XNormalizedViewport < self.autoCenterSafeXMinimumNormalizedViewport or
           YNormalizedViewport > self.autoCenterSafeYMaximumNormalizedViewport or
           YNormalizedViewport < self.autoCenterSafeYMinimumNormalizedViewport or
           ZNormalizedViewport > self.autoCenterSafeZMaximumNormalizedViewport or
           ZNormalizedViewport < self.autoCenterSafeZMinimumNormalizedViewport ):
        foundSafe = False
        break
    self.autoCenterModelInSafeZone = foundSafe

  def autoCenterSetModelTargetPositionViewport(self):
    self.autoCenterModelTargetPositionViewport = [(self.autoCenterSafeXMinimumNormalizedViewport + self.autoCenterSafeXMaximumNormalizedViewport)/2.0,
                                        (self.autoCenterSafeYMinimumNormalizedViewport + self.autoCenterSafeYMaximumNormalizedViewport)/2.0,
                                        (self.autoCenterSafeZMinimumNormalizedViewport + self.autoCenterSafeZMaximumNormalizedViewport)/2.0]

  def autoCenterSetCameraTranslationParameters(self):
    viewName = self.viewNode.GetName()
    cameraNode = self.getCameraNode(viewName)
    cameraPosRas = [0,0,0]
    cameraNode.GetPosition(cameraPosRas)
    self.autoCenterBaseCameraPositionRas = cameraPosRas
    cameraFocRas = [0,0,0]
    cameraNode.GetFocalPoint(cameraFocRas)
    self.autoCenterBaseCameraFocalPointRas = cameraFocRas

    # find the translation in RAS
    modelCurrentPositionCamera = self.autoCenterGetModelCurrentCenterCamera()
    modelTargetPositionCamera = self.autoCenterGetModelTargetPositionCamera()
    cameraTranslationCamera = [0,0,0]
    if self.autoCenterAdjustX:
      cameraTranslationCamera[0] = modelCurrentPositionCamera[0] - modelTargetPositionCamera[0]
    if self.autoCenterAdjustY:
      cameraTranslationCamera[1] = modelCurrentPositionCamera[1] - modelTargetPositionCamera[1]
    if self.autoCenterAdjustZ:
      cameraTranslationCamera[2] = modelCurrentPositionCamera[2] - modelTargetPositionCamera[2]
    self.autoCenterBaseCameraTranslationRas = self.convertVectorCameraToRas(cameraTranslationCamera)

  def autoCenterTranslateCamera(self):
    # linear interpolation between base and target positions, based on the timer
    weightTarget = 1 # default value
    if (self.autoCenterTimeAdjustToRestMaximumSeconds != 0):
      weightTarget = self.autoCenterTimeInStateSeconds / self.autoCenterTimeAdjustToRestMaximumSeconds
    if (weightTarget > 1):
      weightTarget = 1
    cameraNewPositionRas = [0,0,0]
    cameraNewFocalPointRas = [0,0,0]
    for i in xrange(0,3):
      translation = weightTarget * self.autoCenterBaseCameraTranslationRas[i]
      cameraNewPositionRas[i] = translation + self.autoCenterBaseCameraPositionRas[i]
      cameraNewFocalPointRas[i] = translation + self.autoCenterBaseCameraFocalPointRas[i]
    viewName = self.viewNode.GetName()
    cameraNode = self.getCameraNode(viewName)
    cameraNode.SetPosition(cameraNewPositionRas)
    cameraNode.SetFocalPoint(cameraNewFocalPointRas)
    self.resetCameraClippingRange()

  def autoCenterGetModelCurrentCenterRas(self):
    modelBoundsRas = [0,0,0,0,0,0]
    self.autoCenterModelNode.GetRASBounds(modelBoundsRas)
    modelCenterX = (modelBoundsRas[0] + modelBoundsRas[1]) / 2
    modelCenterY = (modelBoundsRas[2] + modelBoundsRas[3]) / 2
    modelCenterZ = (modelBoundsRas[4] + modelBoundsRas[5]) / 2
    modelPosRas = [modelCenterX, modelCenterY, modelCenterZ]
    return modelPosRas

  def autoCenterGetModelCurrentCenterCamera(self):
    modelCenterRas = self.autoCenterGetModelCurrentCenterRas()
    modelCenterCamera = self.convertPointRasToCamera(modelCenterRas)
    return modelCenterCamera

  def autoCenterGetModelCurrentBoundingBoxPointsRas(self):
    pointsRas = []
    boundsRas = [0,0,0,0,0,0]
    self.autoCenterModelNode.GetRASBounds(boundsRas)
    # permute through the different combinations of x,y,z; min,max
    for x in [0,1]:
      for y in [0,1]:
        for z in [0,1]:
          pointRas = []
          pointRas.append(boundsRas[0+x])
          pointRas.append(boundsRas[2+y])
          pointRas.append(boundsRas[4+z])
          pointsRas.append(pointRas)
    return pointsRas

  def autoCenterGetModelTargetPositionRas(self):
    return self.convertViewportToRas(self.autoCenterModelTargetPositionViewport)

  def autoCenterGetModelTargetPositionCamera(self):
    modelTargetPositionRas = self.autoCenterGetModelTargetPositionRas()
    modelTargetPositionCamera = self.convertPointRasToCamera(modelTargetPositionRas)
    return modelTargetPositionCamera

  def autoCenterSetSafeXMinimum(self, val):
    self.autoCenterSafeXMinimumNormalizedViewport = val

  def autoCenterSetSafeXMaximum(self, val):
    self.autoCenterSafeXMaximumNormalizedViewport = val

  def autoCenterSetSafeYMinimum(self, val):
    self.autoCenterSafeYMinimumNormalizedViewport = val

  def autoCenterSetSafeYMaximum(self, val):
    self.autoCenterSafeYMaximumNormalizedViewport = val

  def autoCenterSetSafeZMinimum(self, val):
    self.autoCenterSafeZMinimumNormalizedViewport = val

  def autoCenterSetSafeZMaximum(self, val):
    self.autoCenterSafeZMaximumNormalizedViewport = val

  def autoCenterSetAdjustX(self, val):
    self.autoCenterAdjustX = val

  def autoCenterSetAdjustY(self, val):
    self.autoCenterAdjustY = val

  def autoCenterSetAdjustZ(self, val):
    self.autoCenterAdjustZ = val

  def autoCenterSetAdjustXTrue(self):
    self.autoCenterAdjustX = True

  def autoCenterSetAdjustXFalse(self):
    self.autoCenterAdjustX = False

  def autoCenterSetAdjustYTrue(self):
    self.autoCenterAdjustY = True

  def autoCenterSetAdjustYFalse(self):
    self.autoCenterAdjustY = False

  def autoCenterSetAdjustZTrue(self):
    self.autoCenterAdjustZ = True

  def autoCenterSetAdjustZFalse(self):
    self.autoCenterAdjustZ = False

  def autoCenterSetTimeUnsafeToAdjustMaximumSeconds(self, val):
    self.autoCenterTimeUnsafeToAdjustMaximumSeconds = val

  def autoCenterSetTimeAdjustToRestMaximumSeconds(self, val):
    self.autoCenterTimeAdjustToRestMaximumSeconds = val

  def autoCenterSetTimeRestToSafeMaximumSeconds(self, val):
    self.autoCenterTimeRestToSafeMaximumSeconds = val

  def autoCenterSetUpdateRateSeconds(self, val):
    self.autoCenterUpdateRateSeconds = val

  def autoCenterSetModelNode(self, node):
    self.autoCenterModelNode = node
