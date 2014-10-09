import os.path, datetime
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
#
# PlusRemote
#

class PlusRemote(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Plus Remote"
    self.parent.categories = ["IGT"]
    self.parent.contributors = ["Amelie Meyer (PerkLab, Queen's University), Franklin King (PerkLab, Queen's University), Tamas Ungi (PerkLab, Queen's University), Andras Lasso (PerkLab, Queen's University)"]
    self.parent.helpText = """
This is a convenience module for sending commands a <a href="www.plustoolkit.org">PLUS server</a> for recording data and reconstruction of 3D volumes from tracked 2D image slices.
"""
    self.parent.acknowledgementText = """
This work was funded by Cancer Care Ontario Applied Cancer Research Unit (ACRU) and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) grants.
"""

#
# qPlusRemoteWidget
#
class PlusRemoteWidget(ScriptedLoadableModuleWidget):

  def __init__(self, parent = None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self.logic = PlusRemoteLogic()
    self.commandToMethodHashtable = {}
    self.parameterNode = None
    self.parameterNodeObserver = None
    self.connectorNode = None
    self.connectorNodeObserverTagList = []
    self.connectorNodeConnected = False
    self.roiNode = None
    self.outputSpacingValue = 0
    self.outputOriginValue = []
    self.outputExtentValue = []
    self.scoutVolumeFilename = "ScoutScan.mha" # constant
    self.scoutVolumeNodeName = "ScoutScan" # constant
    self.applyHoleFillingForSnapshot = False # constant
    self.defaultParameterNode = None

  def setup(self):
    # Instantiate and connect widgets
    ScriptedLoadableModuleWidget.setup(self)

    # Module requires openigtlinkremote
    try:
      slicer.modules.openigtlinkremote
    except:
      self.errorLabel = qt.QLabel("Could not find OpenIGTLink Remote module")
      self.layout.addWidget(self.errorLabel)
      return

    self.plusRemoteModuleDirectoryPath = slicer.modules.plusremote.path.replace("PlusRemote.py","")
    # Used Icons
    self.recordIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.stopIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
    self.waitIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Wait.png')
    self.visibleOffIcon = qt.QIcon(":Icons\VisibleOff.png")
    self.visibleOnIcon = qt.QIcon(":Icons\VisibleOn.png")

    # Parameter sets
    defaultParametersCollapsibleButton = ctk.ctkCollapsibleButton()
    defaultParametersCollapsibleButton.text = "Parameter set"
    defaultParametersCollapsibleButton.collapsed = True
    self.layout.addWidget(defaultParametersCollapsibleButton)
    defaultParametersLayout = qt.QFormLayout(defaultParametersCollapsibleButton)

    self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
    self.parameterNodeSelector.nodeTypes = ( ("vtkMRMLScriptedModuleNode"), "" )
    self.parameterNodeSelector.addAttribute( "vtkMRMLScriptedModuleNode", "ModuleName", "PlusRemote" )
    self.parameterNodeSelector.selectNodeUponCreation = True
    self.parameterNodeSelector.addEnabled = True
    self.parameterNodeSelector.renameEnabled = True
    self.parameterNodeSelector.removeEnabled = True
    self.parameterNodeSelector.noneEnabled = False
    self.parameterNodeSelector.showHidden = True
    self.parameterNodeSelector.showChildNodeTypes = False
    self.parameterNodeSelector.baseName = "PlusRemote"
    self.parameterNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.parameterNodeSelector.setToolTip( "Pick parameter set" )
    defaultParametersLayout.addRow("Parameter set: ", self.parameterNodeSelector)

    # Parameters
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)
    parametersLayout = qt.QFormLayout(parametersCollapsibleButton)

    self.linkInputSelector = slicer.qMRMLNodeComboBox()
    self.linkInputSelector.nodeTypes = ( ("vtkMRMLIGTLConnectorNode"), "" )
    self.linkInputSelector.selectNodeUponCreation = True
    self.linkInputSelector.addEnabled = False
    self.linkInputSelector.removeEnabled = True
    self.linkInputSelector.noneEnabled = False
    self.linkInputSelector.showHidden = False
    self.linkInputSelector.showChildNodeTypes = False
    self.linkInputSelector.setMRMLScene( slicer.mrmlScene )
    self.linkInputSelector.setToolTip( "Pick connector node" )
    parametersLayout.addRow("OpenIGTLinkConnector: ", self.linkInputSelector)

    parametersControlsLayout = qt.QHBoxLayout()
    parametersLayout.addRow(parametersControlsLayout)

    self.captureIDLabel = qt.QLabel()
    self.captureIDLabel.setText("Capture Device ID:")
    parametersControlsLayout.addWidget(self.captureIDLabel)

    self.captureIDSelector = qt.QComboBox()
    self.captureIDSelector.setToolTip("Pick capture device ID")
    self.captureIDSelector.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    parametersControlsLayout.addWidget(self.captureIDSelector)

    self.volumeReconstructorIDLabel = qt.QLabel()
    self.volumeReconstructorIDLabel.setText("  Reconstructor Device ID:")
    parametersControlsLayout.addWidget(self.volumeReconstructorIDLabel)

    self.volumeReconstructorIDSelector = qt.QComboBox()
    self.volumeReconstructorIDSelector.setToolTip( "Pick volume reconstructor device ID" )
    self.volumeReconstructorIDSelector.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    parametersControlsLayout.addWidget(self.volumeReconstructorIDSelector)

    # Recording
    recordingCollapsibleButton = ctk.ctkCollapsibleButton()
    recordingCollapsibleButton.text = "Recording"
    self.layout.addWidget(recordingCollapsibleButton)
    recordingLayout = qt.QFormLayout(recordingCollapsibleButton)

    #Move to the same row, use grid and box layouts
    recordingControlsLayout = qt.QHBoxLayout()
    recordingLayout.addRow(recordingControlsLayout)

    self.startStopRecordingButton = qt.QPushButton("  Start Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setIcon(self.recordIcon)
    self.startStopRecordingButton.setEnabled(False)
    self.startStopRecordingButton.setToolTip("If clicked, start recording")
    self.startStopRecordingButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    recordingControlsLayout.addWidget(self.startStopRecordingButton)

    self.recordSettingsButton = ctk.ctkExpandButton()
    recordingControlsLayout.addWidget(self.recordSettingsButton)

    recordParametersControlsLayout = qt.QGridLayout()
    recordingControlsLayout.addLayout(recordParametersControlsLayout)

#     self.displayDefaultLayoutButton = qt.QPushButton("Show Live Image")
#     self.displayDefaultLayoutButton.visible = False
#     self.displayDefaultLayoutButton.setCheckable(True)
#     self.displayDefaultLayoutButton.setToolTip("Show live image")
#     recordParametersControlsLayout.addWidget(self.displayDefaultLayoutButton, 0, 1)

    self.filenameLabel = qt.QLabel()
    self.filenameLabel.setText("Filename:")
    self.filenameLabel.visible = False
    recordParametersControlsLayout.addWidget(self.filenameLabel, 1, 0)

    self.filenameBox = qt.QLineEdit()
    self.filenameBox.setToolTip("Set filename (optional)")
    self.filenameBox.visible = False
    self.filenameBox.setText("Recording.mha")
    recordParametersControlsLayout.addWidget(self.filenameBox, 1, 1)

    self.filenameCompletionLabel = qt.QLabel()
    self.filenameCompletionLabel.setText("Add timestamp to filename:")
    self.filenameCompletionLabel.visible = False
    recordParametersControlsLayout.addWidget(self.filenameCompletionLabel, 2, 0)

    self.filenameCompletionBox = qt.QCheckBox()
    self.filenameCompletionBox.visible = False
    recordParametersControlsLayout.addWidget(self.filenameCompletionBox, 2, 1)

    self.recordingStatus = qt.QMessageBox()
    self.recordingStatus.setIcon(qt.QMessageBox.Information)
    self.recordingStatus.setToolTip("Recording status")
    self.recordingStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.recordingStatus.setEnabled(False)
    recordingControlsLayout.addWidget(self.recordingStatus)

     # Offline Reconstruction
    offlineReconstructCollapsibleButton = ctk.ctkCollapsibleButton()
    offlineReconstructCollapsibleButton.text = "Offline reconstruction of recorded volume"
    self.layout.addWidget(offlineReconstructCollapsibleButton)
    offlineReconstructLayout = qt.QFormLayout(offlineReconstructCollapsibleButton)

    offlineReconstructControlsLayout = qt.QHBoxLayout()
    offlineReconstructLayout.addRow(offlineReconstructControlsLayout)

    self.offlineReconstructButton = qt.QPushButton("  Offline Reconstruction")
    self.offlineReconstructButton.setCheckable(True)
    self.offlineReconstructButton.setIcon(self.recordIcon)
    self.offlineReconstructButton.setEnabled(False)
    self.offlineReconstructButton.setToolTip("If clicked, reconstruct recorded volume")
    self.offlineReconstructButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    offlineReconstructControlsLayout.addWidget(self.offlineReconstructButton)

    self.offlineReconstructSettingsButton = ctk.ctkExpandButton()
    offlineReconstructControlsLayout.addWidget(self.offlineReconstructSettingsButton)

    offlineReconstructParametersLayout = qt.QGridLayout()
    offlineReconstructControlsLayout.addLayout(offlineReconstructParametersLayout)

    self.offlineVolumeSpacingLabel = qt.QLabel()
    self.offlineVolumeSpacingLabel.setText("Spacing:  ")
    self.offlineVolumeSpacingLabel.visible = False
    offlineReconstructParametersLayout.addWidget(self.offlineVolumeSpacingLabel, 0, 0)

    self.offlineVolumeSpacingBox = qt.QDoubleSpinBox()
    self.offlineVolumeSpacingBox.setToolTip("Output volume spacing for offline reconstruction")
    self.offlineVolumeSpacingBox.value = 3
    self.offlineVolumeSpacingBox.visible = False
    offlineReconstructParametersLayout.addWidget(self.offlineVolumeSpacingBox, 0, 1)

    self.outpuVolumeDeviceLabel = qt.QLabel()
    self.outpuVolumeDeviceLabel.setText("Output Volume Device:")
    self.outpuVolumeDeviceLabel.visible = False
    offlineReconstructParametersLayout.addWidget(self.outpuVolumeDeviceLabel, 1, 0)

    self.outpuVolumeDeviceBox = qt.QLineEdit()
    self.outpuVolumeDeviceBox.setText("RecVol_Reference")
    self.outpuVolumeDeviceBox.setToolTip( "Set output volume device (optional)" )
    self.outpuVolumeDeviceBox.visible = False
    offlineReconstructParametersLayout.addWidget(self.outpuVolumeDeviceBox, 1, 1)

    self.offlineVolumeToReconstructLabel = qt.QLabel()
    self.offlineVolumeToReconstructLabel.setText("Volume to Reconstruct:")
    self.offlineVolumeToReconstructLabel.visible = False
    offlineReconstructParametersLayout.addWidget(self.offlineVolumeToReconstructLabel, 2, 0)

    self.offlineVolumeToReconstructSelector = qt.QComboBox()
    self.offlineVolumeToReconstructSelector.setEditable(True)
    self.offlineVolumeToReconstructSelector.setToolTip( "Pick/set volume to reconstruct" )
    self.offlineVolumeToReconstructSelector.visible = False
    offlineReconstructParametersLayout.addWidget(self.offlineVolumeToReconstructSelector, 2, 1)

    self.showOfflineReconstructionResultOnCompletionLabel = qt.QLabel()
    self.showOfflineReconstructionResultOnCompletionLabel.setText("Show results on completion:")
    self.showOfflineReconstructionResultOnCompletionLabel.visible = False
    offlineReconstructParametersLayout.addWidget(self.showOfflineReconstructionResultOnCompletionLabel)

    self.showOfflineReconstructionResultOnCompletionCheckBox = qt.QCheckBox()
    self.showOfflineReconstructionResultOnCompletionCheckBox.visible = False
    self.showOfflineReconstructionResultOnCompletionCheckBox.setChecked(True)
    offlineReconstructParametersLayout.addWidget(self.showOfflineReconstructionResultOnCompletionCheckBox)

    self.offlineReconstructStatus = qt.QMessageBox()
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction status")
    self.offlineReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.offlineReconstructStatus.setEnabled(False)
    offlineReconstructControlsLayout.addWidget(self.offlineReconstructStatus)

    # Scout scan (record and low resolution reconstruction) and live reconstruction
    # Scout scan part
    scoutScanCollapsibleButton = ctk.ctkCollapsibleButton()
    scoutScanCollapsibleButton.text = "Live reconstruction"
    self.layout.addWidget(scoutScanCollapsibleButton)
    scoutScanLayout = qt.QFormLayout(scoutScanCollapsibleButton)

    scoutScanControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(scoutScanControlsLayout)

    self.startStopScoutScanButton = qt.QPushButton("  Scout scan\n  Start recording")
    self.startStopScoutScanButton.setCheckable(True)
    self.startStopScoutScanButton.setIcon(self.recordIcon)
    self.startStopScoutScanButton.setToolTip("If clicked, start recording")
    self.startStopScoutScanButton.setEnabled(False)
    self.startStopScoutScanButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    scoutScanControlsLayout.addWidget(self.startStopScoutScanButton)

    self.scoutSettingsButton = ctk.ctkExpandButton()
    scoutScanControlsLayout.addWidget(self.scoutSettingsButton)

    scoutScanParametersControlsLayout = qt.QGridLayout()
    scoutScanControlsLayout.addLayout(scoutScanParametersControlsLayout)

    self.outputVolumeSpacingLabel = qt.QLabel()
    self.outputVolumeSpacingLabel.setText("Spacing:")
    self.outputVolumeSpacingLabel.visible = False
    scoutScanParametersControlsLayout.addWidget(self.outputVolumeSpacingLabel, 0, 0)

    self.outputVolumeSpacingBox = qt.QDoubleSpinBox()
    self.outputVolumeSpacingBox.setToolTip( "Default output volume spacing for low resolution scout scan" )
    self.outputVolumeSpacingBox.value = 3.0
    self.outputVolumeSpacingBox.visible = False
    scoutScanParametersControlsLayout.addWidget(self.outputVolumeSpacingBox, 0, 1)

    self.scoutScanFilenameLabel = qt.QLabel()
    self.scoutScanFilenameLabel.setText("Recording filename:")
    self.scoutScanFilenameLabel.visible = False
    self.scoutScanFilenameLabel.setSizePolicy(qt.QSizePolicy.Minimum, qt.QSizePolicy.Minimum)
    scoutScanParametersControlsLayout.addWidget(self.scoutScanFilenameLabel)

    self.scoutScanRecordingLineEdit = qt.QLineEdit()
    self.scoutScanRecordingLineEdit.visible = False
    self.scoutScanRecordingLineEdit.setToolTip( "Scout scan recording filename" )
    self.scoutScanRecordingLineEdit.setText("ScoutScanRecording.mha")
    self.scoutScanRecordingLineEdit.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    scoutScanParametersControlsLayout.addWidget(self.scoutScanRecordingLineEdit)

    self.scoutFilenameCompletionLabel = qt.QLabel()
    self.scoutFilenameCompletionLabel.setText("Add timestamp to filename:")
    self.scoutFilenameCompletionLabel.visible = False
    scoutScanParametersControlsLayout.addWidget(self.scoutFilenameCompletionLabel)

    self.scoutFilenameCompletionBox = qt.QCheckBox()
    self.scoutFilenameCompletionBox.visible = False
    scoutScanParametersControlsLayout.addWidget(self.scoutFilenameCompletionBox)

    self.scoutViewsLabel = qt.QLabel()
    self.scoutViewsLabel.setText("Show results on completion:")
    self.scoutViewsLabel.visible = False
    scoutScanParametersControlsLayout.addWidget(self.scoutViewsLabel)

    self.showScoutReconstructionResultOnCompletionCheckBox = qt.QCheckBox()
    self.showScoutReconstructionResultOnCompletionCheckBox.visible = False
    self.showScoutReconstructionResultOnCompletionCheckBox.setChecked(True)
    scoutScanParametersControlsLayout.addWidget(self.showScoutReconstructionResultOnCompletionCheckBox)

    self.recordAndReconstructStatus = qt.QMessageBox()
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan status")
    self.recordAndReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.recordAndReconstructStatus.setEnabled(False)
    scoutScanControlsLayout.addWidget(self.recordAndReconstructStatus)

    # Live Reconstruction Part
    liveReconstructionControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(liveReconstructionControlsLayout)

    self.startStopLiveReconstructionButton = qt.QPushButton("  Start live reconstruction")
    self.startStopLiveReconstructionButton.setCheckable(True)
    self.startStopLiveReconstructionButton.setIcon(self.recordIcon)
    self.startStopLiveReconstructionButton.setToolTip("If clicked, start live reconstruction")
    self.startStopLiveReconstructionButton.setEnabled(False)
    self.startStopLiveReconstructionButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    liveReconstructionControlsLayout.addWidget(self.startStopLiveReconstructionButton, 0, 0)

    liveReconstructionParametersControlsLayout = qt.QGridLayout()
    liveReconstructionControlsLayout.addLayout(liveReconstructionParametersControlsLayout)

    self.displayRoiLabel = qt.QLabel()
    self.displayRoiLabel.setText("ROI:")
    liveReconstructionParametersControlsLayout.addWidget(self.displayRoiLabel, 0, 0)

    self.displayRoiButton = qt.QToolButton()
    self.displayRoiButton.setCheckable(True)
    self.displayRoiButton.setIcon(self.visibleOffIcon)
    self.displayRoiButton.setToolTip("If clicked, display ROI")
    liveReconstructionParametersControlsLayout.addWidget(self.displayRoiButton, 0, 1)

    self.outputSpacingLiveReconstructionLabel = qt.QLabel()
    self.outputSpacingLiveReconstructionLabel.setText("Spacing:")
    liveReconstructionParametersControlsLayout.addWidget(self.outputSpacingLiveReconstructionLabel, 1, 0)

    self.outputSpacingLiveReconstructionBox = qt.QDoubleSpinBox()
    self.outputSpacingLiveReconstructionBox.setToolTip( "Set output volume spacing for live reconstruction" )
    self.outputSpacingLiveReconstructionBox.value = 1.0
    self.outputSpacingLiveReconstructionBox.setSingleStep(0.1)
    liveReconstructionParametersControlsLayout.addWidget(self.outputSpacingLiveReconstructionBox, 1, 1)

    self.liveReconstructionSettingsButton = ctk.ctkExpandButton()
    liveReconstructionControlsLayout.addWidget(self.liveReconstructionSettingsButton, 0, 2)

    self.liveReconstructStatus = qt.QMessageBox()
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Live reconstruction status")
    self.liveReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.liveReconstructStatus.setEnabled(False)
    liveReconstructionControlsLayout.addWidget(self.liveReconstructStatus)

    liveReconstructionAdvancedParametersLayout = qt.QGridLayout()
    scoutScanLayout.addRow(liveReconstructionAdvancedParametersLayout)

    self.liveOutpuVolumeDeviceLabel = qt.QLabel()
    self.liveOutpuVolumeDeviceLabel.setText("Output volume device: ")
    self.liveOutpuVolumeDeviceLabel.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveOutpuVolumeDeviceLabel, 0, 0)

    self.liveOutputVolumeDeviceBox = qt.QLineEdit()
    self.liveOutputVolumeDeviceBox.setToolTip( "Set output volume device (optional)" )
    self.liveOutputVolumeDeviceBox.visible = False
    self.liveOutputVolumeDeviceBox.readOnly = True
    self.liveOutputVolumeDeviceBox.setText("liveReconstruction")
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveOutputVolumeDeviceBox, 0, 1)

    self.liveVolumeToReconstructLabel = qt.QLabel()
    self.liveVolumeToReconstructLabel.setText("Output filename:")
    self.liveVolumeToReconstructLabel.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveVolumeToReconstructLabel, 1, 0)

    self.liveVolumeToReconstructFilename = qt.QLineEdit()
    self.liveVolumeToReconstructFilename.setText("LiveReconstructedVolume.mha")
    self.liveVolumeToReconstructFilename.visible = False
    self.liveVolumeToReconstructFilename.setToolTip( "Volume for live reconstruction" )
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveVolumeToReconstructFilename, 1, 1)

    self.liveFilenameCompletionLabel = qt.QLabel()
    self.liveFilenameCompletionLabel.setText("Add timestamp to filename:")
    self.liveFilenameCompletionLabel.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveFilenameCompletionLabel, 2, 0)

    self.liveFilenameCompletionBox = qt.QCheckBox()
    self.liveFilenameCompletionBox.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveFilenameCompletionBox, 2, 1)

    liveReconstructionAdvancedParametersControlsLayout = qt.QGridLayout()
    scoutScanLayout.addRow(liveReconstructionAdvancedParametersControlsLayout)

    self.outputExtentLiveReconstructionLabel = qt.QLabel()
    self.outputExtentLiveReconstructionLabel.setText("Extent:")
    self.outputExtentLiveReconstructionLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.outputExtentLiveReconstructionLabel, 0, 0)

    self.outputExtentROIBoxDirection1 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection1.setToolTip( "Display output extent (use ROI to modify it)" )
    self.outputExtentROIBoxDirection1.setReadOnly(True)
    self.outputExtentROIBoxDirection1.setRange(0,800)
    self.outputExtentROIBoxDirection1.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.outputExtentROIBoxDirection1, 0, 1)

    self.outputExtentROIBoxDirection2 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection2.setToolTip( "Display output extent (use ROI to modify it)" )
    self.outputExtentROIBoxDirection2.setReadOnly(True)
    self.outputExtentROIBoxDirection2.setRange(0,800)
    self.outputExtentROIBoxDirection2.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.outputExtentROIBoxDirection2, 0, 2)

    self.outputExtentROIBoxDirection3 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection3.setToolTip( "Display output extent (use ROI to modify it)" )
    self.outputExtentROIBoxDirection3.setReadOnly(True)
    self.outputExtentROIBoxDirection3.setRange(0,800)
    self.outputExtentROIBoxDirection3.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.outputExtentROIBoxDirection3, 0, 3)

    self.snapshotIntervalSecLabel = qt.QLabel()
    self.snapshotIntervalSecLabel.setText("Snapshots:           every ")
    self.snapshotIntervalSecLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotIntervalSecLabel, 1, 0)

    self.snapshotIntervalSecSpinBox = qt.QSpinBox()
    self.snapshotIntervalSecSpinBox.setToolTip( "Takes snapshots every ... seconds (if 0, no snapshots)" )
    self.snapshotIntervalSecSpinBox.value = 3
    self.snapshotIntervalSecSpinBox.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotIntervalSecSpinBox, 1, 1)

    self.snapshotsTimeLabel = qt.QLabel()
    self.snapshotsTimeLabel.setText("  seconds")
    self.snapshotsTimeLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsTimeLabel, 1, 2)

#     self.drawModelLabel = qt.QLabel()
#     self.drawModelLabel.setText("Live surface model:  ")
#     self.drawModelLabel.visible = False
#     liveReconstructionAdvancedParametersControlsLayout.addWidget(self.drawModelLabel, 2, 0)
#
#     self.drawModelBox = qt.QCheckBox()
#     self.drawModelBox.visible = False
#     liveReconstructionAdvancedParametersControlsLayout.addWidget(self.drawModelBox, 2, 1)

    self.viewsLabel = qt.QLabel()
    self.viewsLabel.setText("Show results on completion:")
    self.viewsLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.viewsLabel, 3, 0)

    self.showLiveReconstructionResultOnCompletionCheckBox = qt.QCheckBox()
    self.showLiveReconstructionResultOnCompletionCheckBox.visible = False
    self.showLiveReconstructionResultOnCompletionCheckBox.setChecked(True)
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.showLiveReconstructionResultOnCompletionCheckBox, 3, 1)

    # Transform Update
    transformUpdateCollapsibleButton = ctk.ctkCollapsibleButton()
    transformUpdateCollapsibleButton.text = "Transform update"
    transformUpdateCollapsibleButton.collapsed = True
    self.layout.addWidget(transformUpdateCollapsibleButton)
    transformUpdateLayout = qt.QFormLayout(transformUpdateCollapsibleButton)

    self.transformUpdateInputSelector = slicer.qMRMLNodeComboBox()
    self.transformUpdateInputSelector.nodeTypes = ( ("vtkMRMLLinearTransformNode"), "" )
    self.transformUpdateInputSelector.selectNodeUponCreation = True
    self.transformUpdateInputSelector.addEnabled = False
    self.transformUpdateInputSelector.removeEnabled = True
    self.transformUpdateInputSelector.renameEnabled = True
    self.transformUpdateInputSelector.noneEnabled = False
    self.transformUpdateInputSelector.showHidden = False
    self.transformUpdateInputSelector.showChildNodeTypes = False
    self.transformUpdateInputSelector.setMRMLScene( slicer.mrmlScene )
    self.transformUpdateInputSelector.setToolTip( "Pick transform node" )
    transformUpdateLayout.addRow("Transform node: ", self.transformUpdateInputSelector)

    self.updateTransformButton = qt.QPushButton("Update")
    transformUpdateLayout.addRow(self.updateTransformButton)

    self.configFileNameBox = qt.QLineEdit()
    transformUpdateLayout.addRow("Filename: ", self.configFileNameBox)

    self.saveTransformButton = qt.QPushButton("Save configuration")
    transformUpdateLayout.addRow(self.saveTransformButton)

    replyUpdateCollapsibleButton = ctk.ctkCollapsibleButton()
    replyUpdateCollapsibleButton.text = "Reply"
    replyUpdateCollapsibleButton.collapsed = True
    self.layout.addWidget(replyUpdateCollapsibleButton)
    replyLayout = qt.QFormLayout(replyUpdateCollapsibleButton)

    self.replyBox = qt.QPlainTextEdit()
    self.replyBox.setReadOnly(True)
    replyLayout.addRow(self.replyBox)

    # connections

    self.linkInputSelector.connect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeSelected)

    self.startStopRecordingButton.connect('clicked(bool)', self.onStartStopRecordingButtonClicked)
    self.offlineReconstructButton.connect('clicked(bool)', self.onReconstVolume)
    self.startStopScoutScanButton.connect('clicked(bool)', self.onStartStopScoutScanButtonClicked)
    self.startStopLiveReconstructionButton.connect('clicked(bool)', self.onStartStopLiveReconstructionButtonClicked)

    self.displayRoiButton.connect('clicked(bool)', self.onDisplayRoiButtonClicked)
    #self.displayDefaultLayoutButton.connect('clicked(bool)', self.onDisplayDefaultLayoutButtonClicked)
    self.recordSettingsButton.connect('clicked(bool)', self.onRecordSettingsButtonClicked)
    self.offlineReconstructSettingsButton.connect('clicked(bool)', self.onOfflineReconstructSettingsButtonClicked)
    self.scoutSettingsButton.connect('clicked(bool)', self.onScoutSettingsButtonClicked)
    self.liveReconstructionSettingsButton.connect('clicked(bool)', self.onLiveReconstructionSettingsButtonClicked)
    #self.drawModelBox.connect('stateChanged(int)', self.drawLiveSurfaceModel)

    self.updateTransformButton.connect('clicked(bool)', self.onUpdateTransform)
    self.saveTransformButton.connect('clicked(bool)', self.onSaveTransform)

    self.parameterNodeSelector.connect('currentNodeIDChanged(QString)', self.onParameterSetSelected)
    self.linkInputSelector.connect('currentNodeIDChanged(QString)', self.updateParameterNodeFromGui)
    self.captureIDSelector.connect('currentIndexChanged(QString)', self.updateParameterNodeFromGui)
    self.volumeReconstructorIDSelector.connect('currentIndexChanged(QString)', self.updateParameterNodeFromGui)
    #self.displayDefaultLayoutButton.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.filenameBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.filenameCompletionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.offlineVolumeSpacingBox.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.outpuVolumeDeviceBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.offlineVolumeToReconstructSelector.connect('currentIndexChanged(int)', self.updateParameterNodeFromGui)
    self.showOfflineReconstructionResultOnCompletionCheckBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.outputVolumeSpacingBox.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.scoutScanRecordingLineEdit.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.scoutFilenameCompletionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.showScoutReconstructionResultOnCompletionCheckBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.displayRoiButton.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.outputSpacingLiveReconstructionBox.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.liveOutputVolumeDeviceBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection1.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection2.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection3.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.snapshotIntervalSecSpinBox.connect('valueChanged(int)', self.updateParameterNodeFromGui)
    self.showLiveReconstructionResultOnCompletionCheckBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.liveFilenameCompletionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)

    self.snapshotTimer = qt.QTimer()
    self.snapshotTimer.setSingleShot(True)
    self.snapshotTimer.timeout.connect(self.onRequestVolumeReconstructionSnapshot)
    
    self.layout.addStretch(1)

    self.onConnectorNodeSelected()
    self.createDefaultParameterSet()
    self.onParameterSetSelected()

#
# Parameter set saving and reloading
#
  def createDefaultParameterSet(self):
    #Default set created only if no parameterNode exists, to avoid creation of a new default set each time the module is reloaded
    existingParameterNodes = self.logic.getAllParameterNodes()
    if not existingParameterNodes:
      self.defaultParameterNode = self.parameterNodeSelector.addNode()

  def onParameterSetSelected(self):
    self.parameterNode = self.parameterNodeSelector.currentNode()
    if self.parameterNode and self.parameterNodeObserver:
      self.parameterNode.RemoveObserver(self.parameterNodeObserver)
      self.parameterNodeObserver = self.parameterNode.AddObserver('currentNodeChanged(vtkMRMLNode*)', self.updateGUIFromParameterNode)
    # Set up default values for new nodes
    if self.parameterNode:
      self.logic.setDefaultParameters(self.parameterNode)
    self.updateGuiFromParameterNode()

  def updateGuiFromParameterNode(self):
    #Update interface using parameter node saved values
    self.parameterNode = self.parameterNodeSelector.currentNode()

    self.parameterValueList = {'OfflineReconstructionSpacing': self.offlineVolumeSpacingBox, 'ScoutScanSpacing': self.outputVolumeSpacingBox, 'LiveReconstructionSpacing': self.outputSpacingLiveReconstructionBox, 'SnapshotsNumber': self.snapshotIntervalSecSpinBox, 'RoiExtent1': self.outputExtentROIBoxDirection1, 'RoiExtent2': self.outputExtentROIBoxDirection2, 'RoiExtent3': self.outputExtentROIBoxDirection3}
    for parameter in self.parameterValueList:
      if self.parameterNode.GetParameter(parameter):
        #Block Signal to avoid call of updateParameterNodeFromGui when parameter is set programmatically - we only want to catch when user change values
        self.parameterValueList[parameter].blockSignals(True)
        self.parameterValueList[parameter].setValue(float(self.parameterNode.GetParameter(parameter)))
      self.parameterValueList[parameter].blockSignals(False)

    self.parameterBoxList = {'RecordingFilename': self.filenameBox, 'OfflineOutputVolumeDevice': self.outpuVolumeDeviceBox, 'ScoutScanFilename': self.scoutScanRecordingLineEdit, 'LiveRecOutputVolumeDevice': self.liveOutputVolumeDeviceBox}
    for parameter in self.parameterBoxList:
      if self.parameterNode.GetParameter(parameter):
        self.parameterBoxList[parameter].blockSignals(True)
        self.parameterBoxList[parameter].setText(self.parameterNode.GetParameter(parameter))
      self.parameterBoxList[parameter].blockSignals(False)

    self.parameterCheckBoxList = {'RoiDisplay': self.displayRoiButton, 'RecordingFilenameCompletion': self.filenameCompletionBox, 'OfflineDefaultLayout': self.showOfflineReconstructionResultOnCompletionCheckBox, 'ScoutFilenameCompletion': self.scoutFilenameCompletionBox, 'ScoutDefaultLayout': self.showScoutReconstructionResultOnCompletionCheckBox, 'LiveFilenameCompletion': self.liveFilenameCompletionBox, 'LiveDefaultLayout': self.showLiveReconstructionResultOnCompletionCheckBox}
    for parameter in self.parameterCheckBoxList:
      if self.parameterNode.GetParameter(parameter):
        self.parameterCheckBoxList[parameter].blockSignals(True)
        if self.parameterNode.GetParameter(parameter) == "True":
          self.parameterCheckBoxList[parameter].setChecked(True)
        else:
          self.parameterCheckBoxList[parameter].setChecked(False)
      self.parameterCheckBoxList[parameter].blockSignals(False)
      self.onDisplayRoiButtonClicked()

    self.parameterVolumeList = {'OfflineVolumeToReconstruct': self.offlineVolumeToReconstructSelector}
    for parameter in self.parameterVolumeList:
      if self.parameterNode.GetParameter(parameter):
        self.parameterVolumeList[parameter].blockSignals(True)
        self.parameterVolumeList[parameter].setCurrentIndex(int(self.parameterNode.GetParameter(parameter)))
      self.parameterVolumeList[parameter].blockSignals(False)

    self.parameterNodesList = {'OpenIGTLinkConnector': self.linkInputSelector}
    for parameter in self.parameterNodesList:
      if self.parameterNode.GetParameter(parameter):
        self.parameterNodesList[parameter].blockSignals(True)
        self.parameterNodesList[parameter].setCurrentNodeID(self.parameterNode.GetParameter(parameter))
      self.parameterNodesList[parameter].blockSignals(False)

    if self.parameterNode.GetParameter('CaptureID'):
      self.captureIDSelector.blockSignals(True)
      for i in range(0, self.captureIDSelector.count):
        if self.parameterNode.GetParameter('CaptureID') == self.captureIDSelector.itemText(i):
          self.captureIDSelector.setCurrentIndex(int(self.parameterNode.GetParameter('CaptureIdIndex')))
      self.captureIDSelector.blockSignals(False)

    if self.parameterNode.GetParameter('VolumeReconstructor'):
      self.volumeReconstructorIDSelector.blockSignals(True)
      for i in range(0, self.volumeReconstructorIDSelector.count):
        if self.parameterNode.GetParameter('VolumeReconstructor') == self.volumeReconstructorIDSelector.itemText(i):
          self.volumeReconstructorIDSelector.setCurrentIndex(int(self.parameterNode.GetParameter('VolumeReconstructorIndex')))
      self.volumeReconstructorIDSelector.blockSignals(False)

      self.roiNode = self.parameterNode.GetNthNodeReference('ROI', 0)

  def updateParameterNodeFromGui(self):
    #Update parameter node value to save when user change value in the interface
    if not self.parameterNode:
      return
    self.parametersList = {'OpenIGTLinkConnector': self.linkInputSelector.currentNodeID, 'CaptureID': self.captureIDSelector.currentText, 'CaptureIdIndex': self.captureIDSelector.currentIndex, 'VolumeReconstructor': self.volumeReconstructorIDSelector.currentText, 'VolumeReconstructorIndex': self.volumeReconstructorIDSelector.currentIndex, 'RoiDisplay': self.displayRoiButton.isChecked(), 'RoiExtent1': self.outputExtentROIBoxDirection1.value, 'RoiExtent2': self.outputExtentROIBoxDirection2.value, 'RoiExtent3': self.outputExtentROIBoxDirection3.value, 'OfflineReconstructionSpacing': self.offlineVolumeSpacingBox.value, 'OfflineVolumeToReconstruct': self.offlineVolumeToReconstructSelector.currentIndex, 'ScoutScanSpacing': self.outputVolumeSpacingBox.value, 'LiveReconstructionSpacing': self.outputSpacingLiveReconstructionBox.value, 'RecordingFilename': self.filenameBox.text, 'OfflineOutputVolumeDevice': self.outpuVolumeDeviceBox.text, 'ScoutScanFilename': self.scoutScanRecordingLineEdit.text, 'LiveRecOutputVolumeDevice': self.liveOutputVolumeDeviceBox.text, 'RecordingFilenameCompletion': self.filenameCompletionBox.isChecked(), 'ScoutFilenameCompletion': self.scoutFilenameCompletionBox.isChecked(), 'LiveFilenameCompletion': self.liveFilenameCompletionBox.isChecked(), 'OfflineDefaultLayout': self.showOfflineReconstructionResultOnCompletionCheckBox.isChecked(), 'ScoutDefaultLayout': self.showScoutReconstructionResultOnCompletionCheckBox.isChecked(), 'SnapshotsNumber': self.snapshotIntervalSecSpinBox.value, 'LiveDefaultLayout': self.showLiveReconstructionResultOnCompletionCheckBox.isChecked()}
    for parameter in self.parametersList:
      self.parameterNode.SetParameter(parameter, str(self.parametersList[parameter]))
    if self.roiNode:
      roiNodeID = self.roiNode.GetID()
      self.parameterNode.SetNthNodeReferenceID('ROI', 0, roiNodeID)

#
# Connector observation and actions
#
  def onConnectorNodeSelected(self):
    if self.connectorNode and self.connectorNodeObserverTagList:
      for tag in self.connectorNodeObserverTagList:
        self.connectorNode.RemoveObserver(tag)
      self.connectorNodeObserverTagList = []

    self.connectorNode = self.linkInputSelector.currentNode()

    # Force initial update
    if self.connectorNode:
      if self.connectorNode.GetState() == slicer.vtkMRMLIGTLConnectorNode.STATE_CONNECTED:
        self.onConnectorNodeConnected(None, None, True)
      else:
        self.onConnectorNodeDisconnected(None, None, True)

      # Add observers for connect/disconnect events
      events = [[slicer.vtkMRMLIGTLConnectorNode.ConnectedEvent, self.onConnectorNodeConnected], [slicer.vtkMRMLIGTLConnectorNode.DisconnectedEvent, self.onConnectorNodeDisconnected]]
      for tagEventHandler in events:
        connectorNodeObserverTag = self.connectorNode.AddObserver(tagEventHandler[0], tagEventHandler[1])
        self.connectorNodeObserverTagList.append(connectorNodeObserverTag)

  def onConnectorNodeConnected(self, caller, event, force=False):
    # Multiple notifications may be sent when connecting/disconnecting,
    # so we just if we know about the state change already
    if self.connectorNodeConnected and not force:
        return
    self.connectorNodeConnected = True
    self.replyBox.setPlainText("IGTLConnector connected")
    self.captureIDSelector.setDisabled(False)
    self.volumeReconstructorIDSelector.setDisabled(False)
    self.logic.getCaptureDeviceIds(self.linkInputSelector.currentNode().GetID(), self.onGetCaptureDeviceCommandResponseReceived)
    self.logic.getVolumeReconstructorDeviceIds(self.linkInputSelector.currentNode().GetID(), self.onGetVolumeReconstructorDeviceCommandResponseReceived)

  def onConnectorNodeDisconnected(self, caller, event, force=False):
    # Multiple notifications may be sent when connecting/disconnecting,
    # so we just if we know about the state change already
    if not self.connectorNodeConnected  and not force:
        return
    self.connectorNodeConnected = False
    self.replyBox.setPlainText("IGTLConnector not connected or missing")
    self.startStopRecordingButton.setEnabled(False)
    self.startStopScoutScanButton.setEnabled(False)
    self.startStopLiveReconstructionButton.setEnabled(False)
    self.offlineReconstructButton.setEnabled(False)
    self.recordingStatus.setEnabled(False)
    self.offlineReconstructStatus.setEnabled(False)
    self.recordAndReconstructStatus.setEnabled(False)
    self.liveReconstructStatus.setEnabled(False)
    self.captureIDSelector.setDisabled(True)
    self.volumeReconstructorIDSelector.setDisabled(True)

  def getLiveVolumeRecNode(self):
    liveVolumeRecName = self.liveOutputVolumeDeviceBox.text
    liveVolumeRecNode = slicer.util.getNode(liveVolumeRecName)
    return liveVolumeRecNode
    
  def getOfflineVolumeRecNode(self):
    offlineVolumeRecName = self.outpuVolumeDeviceBox.text
    offlineVolumeRecNode = slicer.util.getNode(offlineVolumeRecName)
    return offlineVolumeRecNode

  def getScoutVolumeNode(self):
    scoutScanVolumeNode = slicer.util.getNode(self.scoutVolumeNodeName)
    return scoutScanVolumeNode

#
# Functions called when commands/setting buttons are clicked
#
  # 1 - Commands buttons

  def onStartStopRecordingButtonClicked(self):
    if self.startStopRecordingButton.isChecked():
      self.startStopRecordingButton.setText("  Stop Recording")
      self.startStopRecordingButton.setIcon(self.stopIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, stop recording" )
      self.recordingStatus.setIcon(qt.QMessageBox.Information)
      self.recordingStatus.setToolTip("Recording in progress")
      self.onStartRecording()
      #self.onDrawModelChecked()
    else:
      self.startStopRecordingButton.setText("  Start Recording")
      self.startStopRecordingButton.setIcon(self.recordIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, start recording" )
      self.recordingStatus.setIcon(qt.QMessageBox.Information)
      self.recordingStatus.setToolTip("Recording is being stopped")
      self.onStopRecording()

  def onStartStopScoutScanButtonClicked(self):
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan in progress")
    if self.startStopScoutScanButton.isChecked():
      self.startStopScoutScanButton.setText("  Scout Scan\n  Stop Recording and Reconstruct Recorded Volume")
      self.startStopScoutScanButton.setIcon(self.stopIcon)
      self.startStopScoutScanButton.setToolTip( "If clicked, stop recording and reconstruct recorded volume" )
      self.onStartScoutRecording()
    else:
      self.onStopScoutRecording()

  def onStartStopLiveReconstructionButtonClicked(self):
 
    #Option to create surface model
      #if self.drawModelBox.isChecked():
#       self.drawLiveSurfaceModel()
#     else:
#       print("hide model")

    if self.startStopLiveReconstructionButton.isChecked():
      if self.roiNode:
        self.updateVolumeExtentFromROI()
      self.startStopLiveReconstructionButton.setText("  Stop Live Reconstruction")
      self.startStopLiveReconstructionButton.setIcon(self.stopIcon)
      self.startStopLiveReconstructionButton.setToolTip( "If clicked, stop live reconstruction" )
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.liveReconstructStatus.setToolTip("Live reconstruction in progress")
      self.onStartReconstruction()
    else:
      self.startStopLiveReconstructionButton.setText("  Start Live Reconstruction")
      self.startStopLiveReconstructionButton.setIcon(self.recordIcon)
      self.startStopLiveReconstructionButton.setToolTip( "If clicked, start live reconstruction" )
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.liveReconstructStatus.setToolTip("Live reconstruction is being stopped")
      self.onStopReconstruction()

  # 2 - Advanced settings buttons

  def onRecordSettingsButtonClicked(self, status):
    self.filenameLabel.visible = self.recordSettingsButton.checked
    self.filenameBox.visible = self.recordSettingsButton.checked
    self.filenameCompletionLabel.visible = self.recordSettingsButton.checked
    self.filenameCompletionBox.visible = self.recordSettingsButton.checked
    #self.displayDefaultLayoutButton.visible = self.recordSettingsButton.checked

  def onOfflineReconstructSettingsButtonClicked(self):
    self.offlineVolumeSpacingLabel.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeSpacingBox.visible = self.offlineReconstructSettingsButton.checked
    self.outpuVolumeDeviceLabel.visible = self.offlineReconstructSettingsButton.checked
    self.outpuVolumeDeviceBox.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeToReconstructLabel.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeToReconstructSelector.visible = self.offlineReconstructSettingsButton.checked
    self.showOfflineReconstructionResultOnCompletionLabel.visible = self.offlineReconstructSettingsButton.checked
    self.showOfflineReconstructionResultOnCompletionCheckBox.visible = self.offlineReconstructSettingsButton.checked

  def onScoutSettingsButtonClicked(self, status):
    self.outputVolumeSpacingBox.visible = self.scoutSettingsButton.checked
    self.scoutScanRecordingLineEdit.visible = self.scoutSettingsButton.checked
    self.outputVolumeSpacingLabel.visible = self.scoutSettingsButton.checked
    self.scoutScanFilenameLabel.visible = self.scoutSettingsButton.checked
    self.scoutFilenameCompletionLabel.visible = self.scoutSettingsButton.checked
    self.scoutFilenameCompletionBox.visible = self.scoutSettingsButton.checked
    self.scoutViewsLabel.visible = self.scoutSettingsButton.checked
    self.showScoutReconstructionResultOnCompletionCheckBox.visible = self.scoutSettingsButton.checked

  def onLiveReconstructionSettingsButtonClicked(self, status):
    self.outputExtentLiveReconstructionLabel.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection1.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection2.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection3.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotIntervalSecLabel.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotIntervalSecSpinBox.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsTimeLabel.visible = self.liveReconstructionSettingsButton.checked
    #self.drawModelLabel.visible = self.liveReconstructionSettingsButton.checked
    #self.drawModelBox.visible = self.liveReconstructionSettingsButton.checked
    self.viewsLabel.visible = self.liveReconstructionSettingsButton.checked
    self.showLiveReconstructionResultOnCompletionCheckBox.visible = self.liveReconstructionSettingsButton.checked
    self.liveVolumeToReconstructFilename.visible = self.liveReconstructionSettingsButton.checked
    self.liveVolumeToReconstructLabel.visible = self.liveReconstructionSettingsButton.checked
    self.liveOutpuVolumeDeviceLabel.visible = self.liveReconstructionSettingsButton.checked
    self.liveOutputVolumeDeviceBox.visible = self.liveReconstructionSettingsButton.checked
    self.liveFilenameCompletionLabel.visible = self.liveReconstructionSettingsButton.checked
    self.liveFilenameCompletionBox.visible = self.liveReconstructionSettingsButton.checked

#
# Display functions
#
  def onDisplayRoiButtonClicked(self):
    if self.displayRoiButton.isChecked():
      self.displayRoiButton.setIcon(self.visibleOnIcon)
      self.displayRoiButton.setToolTip("If clicked, hide ROI")
      if self.roiNode:
        self.roiNode.SetDisplayVisibility(1)
    else:
      self.displayRoiButton.setIcon(self.visibleOffIcon)
      self.displayRoiButton.setToolTip("If clicked, display ROI")
      if self.roiNode:
        self.roiNode.SetDisplayVisibility(0)

  def onDisplayDefaultLayoutButtonClicked(self):
    #Display live image in red view if button checked
    redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    if self.displayDefaultLayoutButton.isChecked():
      self.displayRoiButton.setToolTip("If clicked, hide live image")
      redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('Image_Reference').GetID())
    #Reslice it
      #imageReferenceNode = slicer.util.getNode('Image_Reference')
      #imageReferenceNode = slicer.mrmlScene.GetNodesByName('Image_Reference')
#       imageReferenceMRMLNode = slicer.vtkMRMLNode.SafeDownCast(imageReferenceNode.GetItemAsObject(0))
#       print imageReferenceMRMLNode
      #redSliceNode = slicer.vtkMRMLSliceNode.SafeDownCast(imageReferenceNode.GetItemAsObject(0))
      #print redSliceNode
      #redSliceNode.SetOrientationToReformat()

      #vtkSlicerVolumeResliceDriverLogic SetModeForSlice( int mode, vtkMRMLSliceNode* sliceNode )
      #vtkMRMLSliceNode SetOrientation("Reformat") ou SetOrientationToReformat()
      #qMRMLSliceControllerWidget setSliceOrientation("Reformat")

      #redWidget = slicer.app.layoutManager().sliceWidget('Red')
      #redWidget.setSliceOrientation("Reformat")
      #controller = redWidget.sliceController()
      #controller.showReformatWidget(True)

#       driverToRASMatrix = vtk.vtkMatrix4x4()
#       driverToRASMatrix.Identity()
#       driverToRASTransform = vtk.vtkTransform()
#       driverToRASTransform.SetMatrix(driverToRASMatrix)
#       driverToRASTransform.Update()
#       sliceToDriverTransform = vtk.vtkTransform()
#       sliceToDriverTransform.Identity()
#       sliceToDriverTransform.RotateZ(180)
#       sliceToRASTransform = vtk.vtkTransform()
#       sliceToRASTransform.Identity()
#       sliceToRASTransform.Concatenate(driverToRASTransform )
#       sliceToRASTransform.Concatenate(sliceToDriverTransform)
#       sliceToRASTransform.Update()
#       redNode = slicer.util.getNode('Image_Reference')
#       redNode.SetSliceToRAS(sliceToRASTransform.GetMatrix())
#       redNode.UpdateMatrices()

    else:
      self.displayRoiButton.setToolTip("If clicked, show live image")
      redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('None'))

#
# Filenames completion
#
  def generateRecordingOutputFilename(self):
    if self.filenameCompletionBox.isChecked():
      return self.logic.filenameCompletion(self.filenameBox.text)
    else:
      return str(self.filenameBox.text)

  def generateScoutRecordingOutputFilename(self):
    if self.scoutFilenameCompletionBox.isChecked():
      return self.logic.filenameCompletion(self.scoutScanRecordingLineEdit.text)
    else:
      return str(self.scoutScanRecordingLineEdit.text)

  def getLiveReconstructionOutputFilename(self):
    if self.liveFilenameCompletionBox.isChecked():
      return self.logic.filenameCompletion(self.liveVolumeToReconstructFilename.text)
    else:
      return str(self.liveVolumeToReconstructFilename.text)

#
# Commands
#
  def onStartRecording(self):
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.generateRecordingOutputFilename(), self.onGenericCommandResponseReceived)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStartScoutRecording(self):
    self.lastScoutRecordingOutputFilename = self.generateScoutRecordingOutputFilename()
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.lastScoutRecordingOutputFilename, self.onGenericCommandResponseReceived)

  def onStopScoutRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onScoutVolumeRecorded)

  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputSpacingValue, self.outputOriginValue, self.outputExtentValue, self.onGenericCommandResponseReceived, self.getLiveReconstructionOutputFilename(), self.liveOutputVolumeDeviceBox.text)
    # Set up timer for requesting snapshot
    snapshotIntervalSec = (self.snapshotIntervalSecSpinBox.value)
    if snapshotIntervalSec > 0:
      self.snapshotTimer.start(snapshotIntervalSec*1000)

  def onStopReconstruction(self):
    self.snapshotTimer.stop()
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.onVolumeLiveReconstructed, self.getLiveReconstructionOutputFilename(), self.liveOutputVolumeDeviceBox.text)

  def onReconstVolume(self):
    self.offlineReconstructButton.setIcon(self.waitIcon)
    self.offlineReconstructButton.setText("  Offline Reconstruction in progress ...")
    self.offlineReconstructButton.setEnabled(False)
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction in progress")
    outputSpacing = [self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.offlineVolumeToReconstructSelector.currentText, outputSpacing, self.onVolumeReconstructed, "RecVol_Reference.mha", self.outpuVolumeDeviceBox.text)

  def onScoutScanReconstVolume(self):
    self.startStopScoutScanButton.setIcon(self.waitIcon)
    self.startStopScoutScanButton.setText("  Scout Scan\n  Reconstruction in progress ...")
    self.startStopScoutScanButton.setEnabled(False)
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Recording completed. Reconstruction in progress")
    outputSpacing = [self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.lastScoutRecordingOutputFilename, outputSpacing, self.onScoutVolumeReconstructed, self.scoutVolumeFilename, self.scoutVolumeNodeName)

  def onRequestVolumeReconstructionSnapshot(self, stopFlag = ""):
    self.logic.getVolumeReconstructionSnapshot(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.scoutScanRecordingLineEdit.text, self.liveOutputVolumeDeviceBox.text, self.applyHoleFillingForSnapshot, self.onSnapshotAcquired)
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Snapshot")

  def onUpdateTransform(self):
    self.logic.updateTransform(self.linkInputSelector.currentNode().GetID(), self.transformUpdateInputSelector.currentNode(), self.onGenericCommandResponseReceived)

  def onSaveTransform(self):
    self.logic.saveTransform(self.linkInputSelector.currentNode().GetID(), self.configFileNameBox.text, self.onGenericCommandResponseReceived)

#
# Functions associated to commands
#
  def onGenericCommandResponseReceived(self, commandId, responseNode):
    if responseNode:
      self.replyBox.clear()
      self.replyBox.setPlainText(responseNode.GetText(0))
    else:
      self.replyBox.setPlainText("Command timeout: {0}".format(commandId))

  def onGetCaptureDeviceCommandResponseReceived(self, commandId, textNode):
    if not textNode:
        self.replyBox.setPlainText("GetCaptureDeviceCommand timeout: {0}".format(commandId))
        return

    commandResponse=textNode.GetText(0)
    #self.logic.discardCommand(commandId, self.linkInputSelector.currentNode().GetID())

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    captureDeviceIdsListString = commandResponseElement.GetAttribute("Message")
    if captureDeviceIdsListString:
      captureDevicesIdsList = captureDeviceIdsListString.split(",")
    else:
      captureDevicesIdsList = []

    for i in range(0,len(captureDevicesIdsList)):
      if self.captureIDSelector.findText(captureDevicesIdsList[i]) == -1:
        self.captureIDSelector.addItem(captureDevicesIdsList[i])

  def onGetVolumeReconstructorDeviceCommandResponseReceived(self, commandId, textNode):
    if not textNode:
        self.replyBox.setPlainText("GetVolumeReconstructorDeviceCommand timeout: {0}".format(commandId))
        return

    commandResponse=textNode.GetText(0)
    #self.logic.discardCommand(commandId, self.linkInputSelector.currentNode().GetID())

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    volumeReconstructorDeviceIdsListString = commandResponseElement.GetAttribute("Message")
    if volumeReconstructorDeviceIdsListString:
      volumeReconstructorDeviceIdsList = volumeReconstructorDeviceIdsListString.split(",")
    else:
      volumeReconstructorDeviceIdsList = []

    self.volumeReconstructorIDSelector.clear()
    self.volumeReconstructorIDSelector.addItems(volumeReconstructorDeviceIdsList)
    self.startStopRecordingButton.setEnabled(True)
    self.offlineReconstructButton.setEnabled(True)
    self.startStopScoutScanButton.setEnabled(True)
    self.startStopLiveReconstructionButton.setEnabled(True)
    self.recordingStatus.setEnabled(True)
    self.offlineReconstructStatus.setEnabled(True)
    self.recordAndReconstructStatus.setEnabled(True)
    self.liveReconstructStatus.setEnabled(True)

  def onVolumeRecorded(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)
    self.offlineReconstructButton.setEnabled(True)

    commandResponse=textNode.GetText(0)

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)
    self.offlineVolumeToReconstructSelector.insertItem(0,volumeToReconstructFileName)
    self.offlineVolumeToReconstructSelector.setCurrentIndex(0)

    if commandResponseElement.GetAttribute("Status") == "SUCCESS":
      self.recordingStatus.setIcon(qt.QMessageBox.Information)
      self.recordingStatus.setToolTip("Recording completed, saved as " + volumeToReconstructFileName)
    else:
      self.recordingStatus.setIcon(qt.QMessageBox.Critical)
      self.recordingStatus.setToolTip(stopRecordingMessage)

  def onScoutVolumeRecorded(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)
    self.offlineReconstructButton.setEnabled(True)

    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)

    if commandResponseElement.GetAttribute("Status") == "SUCCESS":
      self.onScoutScanReconstVolume()

  def onVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    self.offlineReconstructButton.setIcon(self.recordIcon)
    self.offlineReconstructButton.setText("Offline Reconstruction")
    self.offlineReconstructButton.setEnabled(True)
    self.offlineReconstructButton.setChecked(False)

    if not textNode:
      # volume reconstruction command timed out
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip("Timeout while waiting for volume reconstruction result")
      return

    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    offlineReconstMessage = commandResponseElement.GetAttribute("Message")

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.offlineReconstructStatus.setToolTip(commandResponseElement.GetAttribute("Message"))
    else:
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip(offlineReconstMessage)

    if self.showOfflineReconstructionResultOnCompletionCheckBox.isChecked():
      self.showInSliceViewers(self.getOfflineVolumeRecNode(), ["Yellow", "Green"])
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.FitSliceToAll()

      self.showVolumeRendering(self.getOfflineVolumeRecNode())

  def onScoutVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    scoutScanMessage = commandResponseElement.GetAttribute("Message")
    scoutScanReconstructFileName = os.path.basename(scoutScanMessage)

    self.startStopScoutScanButton.setIcon(self.recordIcon)
    self.startStopScoutScanButton.setText("  Scout Scan\n  Start Recording")
    self.startStopScoutScanButton.setEnabled(True)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      #Create and initialize ROI after scout scan because low resolution scout scan is used to set a smaller ROI for the live high resolution reconstruction
      self.onRoiInitialization()
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.recordAndReconstructStatus.setToolTip("Scout scan completed, file saved as "+ scoutScanReconstructFileName)
    else:
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.recordAndReconstructStatus.setToolTip(scoutScanMessage)

    if self.showScoutReconstructionResultOnCompletionCheckBox.isChecked():
      scoutScanVolumeNode = self.getScoutVolumeNode()
#       selectionNode = slicer.app.applicationLogic().GetSelectionNode()
#       selectionNode.SetReferenceActiveVolumeID(self.scoutScanVolumeNode.GetID())
#       applicationLogic = slicer.app.applicationLogic()
#       applicationLogic.PropagateVolumeSelection(0)
#       applicationLogic.FitSliceToAll()

      scoutVolumeDisplay = scoutScanVolumeNode.GetDisplayNode()
      self.scoutWindow = scoutVolumeDisplay.GetWindow()
      self.scoutLevel = scoutVolumeDisplay.GetLevel()

      self.showInSliceViewers(scoutScanVolumeNode, ["Yellow", "Green"])
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.FitSliceToAll()

      #Red view: Image_Reference
      #redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
      #redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('Image_Reference').GetID())
      #3D view
      self.showVolumeRendering(scoutScanVolumeNode)

  def onSnapshotAcquired(self, commandId, textNode):
    
    if not self.startStopLiveReconstructionButton.isChecked():
      # live volume reconstruction is not active
      return
      
    self.onGenericCommandResponseReceived(commandId,textNode)
    if self.showLiveReconstructionResultOnCompletionCheckBox.isChecked():
      self.showInSliceViewers(self.getLiveVolumeRecNode(), ["Yellow", "Green"])
      self.showVolumeRendering(self.getLiveVolumeRecNode())

    # Request the next snapshot
    snapshotIntervalSec = (self.snapshotIntervalSecSpinBox.value)
    if snapshotIntervalSec > 0:
      self.snapshotTimer.start(snapshotIntervalSec*1000)

  def onVolumeLiveReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)
    
    if not textNode:
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.liveReconstructStatus.setToolTip("Failed to stop volume reconstruction")
      return
    
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    liveReconstructMessage = commandResponseElement.GetAttribute("Message")
    liveReconstructVolumeFileName = os.path.basename(liveReconstructMessage)

    if commandResponseElement.GetAttribute("Status") != "SUCCESS":
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.liveReconstructStatus.setToolTip(liveReconstructMessage)
      return

    # We successfully received a volume      
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Live reconstruction completed, file saved as "+ liveReconstructVolumeFileName)

    if self.showLiveReconstructionResultOnCompletionCheckBox.isChecked():
      self.showInSliceViewers(self.getLiveVolumeRecNode(), ["Yellow", "Green"])
      self.showVolumeRendering(self.getLiveVolumeRecNode())

  def showInSliceViewers(self, volumeNode, sliceWidgetNames):
    # Displays volumeNode in the selected slice viewers as background volume
    # Existing background volume is pushed to foreground, existing foreground volume will not be shown anymore
    # sliceWidgetNames is a list of slice view names, such as ["Yellow", "Green"]
    if not volumeNode:
      return
    newVolumeNodeID = volumeNode.GetID()
    for sliceWidgetName in sliceWidgetNames:
      sliceLogic = slicer.app.layoutManager().sliceWidget(sliceWidgetName).sliceLogic()
      foregroundVolumeNodeID = sliceLogic.GetSliceCompositeNode().GetForegroundVolumeID()
      backgroundVolumeNodeID = sliceLogic.GetSliceCompositeNode().GetBackgroundVolumeID()
      if foregroundVolumeNodeID or backgroundVolumeNodeID:
        # a volume is shown already, so we just need to make foreground semi-transparent to make sure the new volume will be visible
        sliceLogic.GetSliceCompositeNode().SetForegroundOpacity(0.5)
      if foregroundVolumeNodeID == newVolumeNodeID or backgroundVolumeNodeID == newVolumeNodeID:
        # new volume is already shown as foreground or background
        continue
      if backgroundVolumeNodeID:
        # there is a background volume, push it to the foreground because we will replace the background volume
        sliceLogic.GetSliceCompositeNode().SetForegroundVolumeID(backgroundVolumeNodeID)
      # show the new volume as background
      sliceLogic.GetSliceCompositeNode().SetBackgroundVolumeID(newVolumeNodeID)

  def showVolumeRendering(self, volumeNode):
    # Display reconstructed volume in Slicer 3D view
    if not volumeNode:
      return
      
    # Find existing VR display node
    volumeRenderingDisplayNode = None
    for displayNodeIndex in xrange(volumeNode.GetNumberOfDisplayNodes()):
      displayNode = volumeNode.GetNthDisplayNode(displayNodeIndex)
      if displayNode.IsA('vtkMRMLVolumeRenderingDisplayNode'):
        # Found existing VR display node
        volumeRenderingDisplayNode = displayNode
        break
    
    # Create new VR display node if not exist yet
    if not volumeRenderingDisplayNode:
      volRenderingLogic = slicer.modules.volumerendering.logic()
      volumeRenderingDisplayNode = volRenderingLogic.CreateVolumeRenderingDisplayNode()
      slicer.mrmlScene.AddNode(volumeRenderingDisplayNode)
      volumeRenderingDisplayNode.UnRegister(volRenderingLogic)
      volRenderingLogic.UpdateDisplayNodeFromVolumeNode(volumeRenderingDisplayNode,volumeNode)
      volumeNode.AddAndObserveDisplayNodeID(volumeRenderingDisplayNode.GetID())
      
    volumeRenderingDisplayNode.SetVisibility(True)
    volumeRenderingWidgetRep = slicer.modules.volumerendering.widgetRepresentation()
    volumeRenderingWidgetRep.setMRMLVolumeNode(volumeNode)
#
# ROI initialization and update
#

  def onRoiInitialization(self):
    reconstructedNode = slicer.mrmlScene.GetNodesByName(self.scoutVolumeNodeName)
    reconstructedVolumeNode = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(reconstructedNode.GetItemAsObject(0))

    roiCenterInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    roiRadiusInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    bounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

    #ROI is initialized to fit scout scan reconstructed volume
    if reconstructedVolumeNode:
      reconstructedVolumeNode.GetRASBounds(bounds)
      for i in range(0,5,2):
        roiCenterInit[i] = (bounds[i+1] + bounds[i])/2
        roiRadiusInit[i] = (bounds[i+1] - bounds[i])/2
      if self.roiNode:
        self.roiNode.SetXYZ(roiCenterInit[0], roiCenterInit[2], roiCenterInit[4])
        self.roiNode.SetRadiusXYZ(roiRadiusInit[0], roiRadiusInit[2], roiRadiusInit[4])
      else:
        self.roiNode = slicer.vtkMRMLAnnotationROINode()
        self.roiNode.SetXYZ(roiCenterInit[0], roiCenterInit[2], roiCenterInit[4])
        self.roiNode.SetRadiusXYZ(roiRadiusInit[0], roiRadiusInit[2], roiRadiusInit[4])
        self.roiNode.Initialize(slicer.mrmlScene)
        self.roiNode.SetDisplayVisibility(0)
        self.roiNode.SetInteractiveMode(1)
    self.updateVolumeExtentFromROI()

  def updateVolumeExtentFromROI(self):
    #Update volume extent values each time user modifies the ROI, as we want volume to fit ROI for live reconstruction
    roiCenter = [0.0, 0.0, 0.0]
    roiRadius = [0.0, 0.0, 0.0]
    roiOrigin = [0.0, 0.0, 0.0]
    if self.roiNode:
      self.roiNode.GetXYZ(roiCenter)
      self.roiNode.GetRadiusXYZ(roiRadius)

    for i in range(0,len(roiCenter)):
        roiOrigin[i] = roiCenter[i] - roiRadius[i]
    self.outputOriginValue = roiOrigin
    #Radius in mm, extent in pixel
    self.outputExtentValue = [0, int((2*roiRadius[0])/self.outputSpacingLiveReconstructionBox.value), 0, int((2*roiRadius[1])/self.outputSpacingLiveReconstructionBox.value), 0, int((2*roiRadius[2])/self.outputSpacingLiveReconstructionBox.value)]
    self.outputSpacingValue = [self.outputSpacingLiveReconstructionBox.value, self.outputSpacingLiveReconstructionBox.value, self.outputSpacingLiveReconstructionBox.value]

    self.outputExtentROIBoxDirection1.value = self.outputExtentValue[1]
    self.outputExtentROIBoxDirection2.value = self.outputExtentValue[3]
    self.outputExtentROIBoxDirection3.value = self.outputExtentValue[5]

#   def drawLiveSurfaceModel(self):
#     if self.drawModelBox.isChecked():
#       if self.startStopLiveReconstructionButton.isChecked():
#         appender = vtk.vtkAppendPolyData()
#
#         if slicer.util.getNode('Image_Reference') is None:
#           self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
#           self.liveReconstructStatus.setToolTip("Error: no ImageToReferenceTransform. Unable to draw surface model")
#         else:
#           liveOutputDeviceVolumeNode = slicer.mrmlScene.GetNodesByName('liveReconstruction') #ou self.liveOutputVolumeDeviceBox.text
#           liveOutputDeviceVolumeScalarNode = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(liveOutputDeviceVolumeNode.GetItemAsObject(0))
#           print liveOutputDeviceVolumeScalarNode
#           #frameList =
#           #for frameIndex in range(0, framesNumber):
#           #frame = ...(frameIndex)
#
#           imageToReferenceTransformMatrix = vtk.vtkMatrix4x4()
#           imageToReferenceNode = slicer.util.getNode('Image_Reference')
#           imageToReferenceNode.GetMatrixTransformToParent(imageToReferenceTransformMatrix)
#           imageToReferenceTransform = vtk.vtkTransform()
#           imageToReferenceTransform.SetMatrix(imageToReferenceTransformMatrix)
#
#           cubeToImageTransform = vtk.vtkTransform()
#           clipRectangleOrigin = [0, 0] #if fan?
#           clipRectangleSize = [820, 616]
#           #recuperer clipRectangle information
#           cubeToImageTransform.Translate(clipRectangleOrigin[0], clipRectangleOrigin[1], 1)
#           cubeToImageTransform.Scale(clipRectangleSize[0], clipRectangleSize[1], 1)
#           cubeToImageTransform.Translate(0.5, 0.5, 0.5)
#
#           cubeToTrackerTransform = vtk.vtkTransform()
#           cubeToTrackerTransform.Identity()
#           cubeToTrackerTransform.Concatenate(imageToReferenceTransform)
#           cubeToTrackerTransform.Concatenate(cubeToImageTransform)
#
#           cubeToTracker = vtk.vtkTransformPolyDataFilter()
#           cubeToTracker.SetTransform(cubeToTrackerTransform)
#           source = vtk.vtkCubeSource()
#           source.Update()
#           cubeToTracker.SetInputConnection(source.GetOutputPort())
#           cubeToTracker.Update()
#           appender.AddInputConnection(cubeToTracker.GetOutputPort())
#           #frame+=1
#
#           writer = vtk.vtkPolyDataWriter()
#           writer.SetInputConnection(appender.GetOutputPort())
#           writer.Update()
#
#     else:
#       print("hide model")

#
# PlusRemoteLogic
#
class PlusRemoteLogic(ScriptedLoadableModuleLogic):
  def __init__(self, parent = None):
    ScriptedLoadableModuleLogic.__init__(self, parent)

    # Allow having multiple parameter nodes in the scene
    self.isSingletonParameterNode = False

    self.commandToMethodHashtable = {}
    self.defaultCommandTimeoutSec = 30
    self.timerIntervalSec = 0.1
    self.timer = qt.QTimer()
    self.timer.timeout.connect(self.getCommandReply)
    pass

  def setDefaultParameters(self, parameterNode):
    parameterList = {'RoiDisplay': False, 'RecordingFilename': "Recording.mha", 'RecordingFilenameCompletion': False, 'OfflineReconstructionSpacing': 3.0, 'OfflineVolumeToReconstruct': 0, 'OfflineOutputVolumeDevice': "RecVol_Reference" , 'OfflineDefaultLayout': True, 'ScoutScanSpacing': 3.0, 'ScoutScanFilename': "ScoutScanRecording.mha", 'ScoutFilenameCompletion': False, 'ScoutDefaultLayout': True, 'LiveReconstructionSpacing': 1.0, 'LiveRecOutputVolumeDevice': "liveReconstruction", 'RoiExtent1': 0.0, 'RoiExtent2': 0.0, 'RoiExtent3': 0.0, 'SnapshotsNumber': 3, 'LiveFilenameCompletion': False, 'LiveDefaultLayout': True}
    for parameter in parameterList:
      if not parameterNode.GetParameter(parameter):
        parameterNode.SetParameter(parameter, str(parameterList[parameter]))

  def filenameCompletion(self, filename):
    basename = filename.replace(".mha","")
    self.dateTimeList = {'Month': str(datetime.datetime.now().month), 'Day': str(datetime.datetime.now().day), 'Hour': str(datetime.datetime.now().hour), 'Minute': str(datetime.datetime.now().minute), 'Second': str(datetime.datetime.now().second)}
    for element in self.dateTimeList:
      #If one item is only 1 digit, complete with a zero (we want the format filename_YYYYMMDD_HHMMSS)
      if len(self.dateTimeList[element]) == 1:
        self.dateTimeList[element] = str(0) + self.dateTimeList[element]
        filename = str(basename) + "_" + str(datetime.datetime.now().year) + self.dateTimeList['Month'] + self.dateTimeList['Day'] + "_" + self.dateTimeList['Hour'] + self.dateTimeList['Minute'] + self.dateTimeList['Second'] + ".mha"
    return filename

  def executeCommand(self, connectorNodeId, commandName, commandParameters, responseCallback):
      commandId = slicer.modules.openigtlinkremote.logic().ExecuteCommand(connectorNodeId, commandName, commandParameters)
      self.commandToMethodHashtable[commandId]={'responseCallback': responseCallback, 'connectorNodeId': connectorNodeId, 'remainingTime': self.defaultCommandTimeoutSec/self.timerIntervalSec}
      if not self.timer.isActive():
        self.timer.start(self.timerIntervalSec*1000)

  def getCommandReply(self):
    for commandId in self.commandToMethodHashtable.keys():
      replyNodes = slicer.mrmlScene.GetNodesByName( "ACK_" + str(commandId) )
      textNode = slicer.vtkMRMLAnnotationTextNode.SafeDownCast( replyNodes.GetItemAsObject(0) )
      remainingTime = self.commandToMethodHashtable[commandId]['remainingTime']
      remainingTime = remainingTime-1
      if textNode or remainingTime<=0:
        # We received a response or timed out waiting for a response
        commandToMethodItem = self.commandToMethodHashtable.pop(commandId)
        responseCallback = commandToMethodItem['responseCallback']
        responseCallback(commandId, textNode)
        connectorNodeId = commandToMethodItem['connectorNodeId']
        self.discardCommand(commandId, connectorNodeId)
      else:
        self.commandToMethodHashtable[commandId]['remainingTime'] = remainingTime
    if not self.commandToMethodHashtable:
      self.timer.stop()

  def getCaptureDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualDiscCapture" +"\""
    self.executeCommand(connectorNodeId,"RequestDeviceIds", parameters, method)

  def getVolumeReconstructorDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualVolumeReconstructor" +"\""
    self.executeCommand(connectorNodeId, "RequestDeviceIds", parameters, method)

  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacing, outputOrigin, outputExtent, method, liveOutputVolumeFilename, liveOutputVolumeDevice):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputSpacing=" + "\"" + "%f %f %f" % tuple(outputSpacing) + "\"" + " OutputOrigin=" + "\"" + "%f %f %f" % tuple(outputOrigin) + "\"" + " OutputExtent=" + "\"" + "%i %i %i %i %i %i" % tuple(outputExtent) + "\"" + " OutputVolFilename=" + "\"" + liveOutputVolumeFilename +"\"" + " OutputVolDeviceName=" + "\"" + liveOutputVolumeDevice +"\""
    self.executeCommand(connectorNodeId, "StartVolumeReconstruction", parameters, method)

  def stopVolumeReconstruction(self, connectorNodeId, volumeReconstructorDeviceId, method, liveOutputVolumeFilename, liveOutputVolumeDevice):
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputVolFilename=" + "\"" + liveOutputVolumeFilename +"\"" + " OutputVolDeviceName=" + "\"" + liveOutputVolumeDevice +"\""
    self.executeCommand(connectorNodeId, "StopVolumeReconstruction", parameters, method)

  def reconstructRecorded(self, connectorNodeId, volumeReconstructorDeviceId, inputSequenceFilename, outputSpacing, method, outputVolumeFilename, outputVolumeDevice):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " InputSeqFilename=" + "\"" + inputSequenceFilename + "\"" + " OutputSpacing=" + "\"" + "%f %f %f" % tuple(outputSpacing) + "\"" + " OutputVolFilename=" + "\"" + outputVolumeFilename +"\"" + " OutputVolDeviceName=" + "\"" + outputVolumeDevice +"\""
    self.executeCommand(connectorNodeId, "ReconstructVolume", parameters, method)

  def startRecording(self, connectorNodeId, captureName, fileName, method):
    parameters = "CaptureDeviceId=" + "\"" + captureName + "\"" + " OutputFilename=" + "\"" + fileName + "\""
    self.executeCommand(connectorNodeId, "StartRecording", parameters, method)

  def stopRecording(self, connectorNodeId, captureName, method):
    self.executeCommand(connectorNodeId, "StopRecording", "CaptureDeviceId=" + "\"" + captureName + "\"", method)

  def getVolumeReconstructionSnapshot(self, connectorNodeId, volumeReconstructorDeviceId, fileName, deviceName, applyHoleFilling, method):
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputFilename=" + "\"" + fileName + "\""  + " OutputVolDeviceName=" + "\"" + deviceName +"\""
    if applyHoleFilling:
      parameters = parameters + " ApplyHoleFilling=\"TRUE\""
    else:
      parameters = parameters + " ApplyHoleFilling=\"FALSE\""
    self.executeCommand(connectorNodeId, "GetVolumeReconstructionSnapshot", parameters, method)

  def updateTransform(self, connectorNodeId, transformNode, method):
    transformMatrix = transformNode.GetMatrixTransformToParent()
    transformValue = ""
    for i in range(0,4):
      for j in range(0,4):
        transformValue = transformValue + str(transformMatrix.GetElement(i,j)) + " "
    # remove last character (extra space at the end)
    transformValue = transformValue[:-1]

    transformDate = str(datetime.datetime.now())

    parameters = "TransformName=" + "\"" + transformNode.GetName() + "\"" + " TransformValue=" + "\"" + transformValue + "\"" + " TransformDate=" + "\"" + transformDate + "\""
    self.executeCommand(connectorNodeId, "UpdateTransform", parameters, method)

  def saveTransform(self, connectorNodeId, filename, method):
    parameters = "Filename=" + "\"" + filename + "\""
    self.executeCommand(connectorNodeId, "SaveConfig", parameters, method)

  def discardCommand(self, commandId, connectorNodeId):
    slicer.modules.openigtlinkremote.logic().DiscardCommand(commandId, connectorNodeId)
