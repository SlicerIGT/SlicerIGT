import os.path, datetime
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# PlusRemote
#

class PlusRemote(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Plus Remote"
    self.parent.categories = ["IGT"]
    self.parent.dependencies = ["OpenIGTLinkRemote"]
    self.parent.contributors = ["Amelie Meyer (PerkLab, Queen's University), Franklin King (PerkLab, Queen's University), Tamas Ungi (PerkLab, Queen's University), Andras Lasso (PerkLab, Queen's University)"]
    self.parent.helpText = """This is a convenience module for sending commands a <a href="www.plustoolkit.org">PLUS server</a> for recording data and reconstruction of 3D volumes from tracked 2D image slices."""
    self.parent.acknowledgementText = """This work was funded by Cancer Care Ontario Applied Cancer Research Unit (ACRU) and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) grants."""

#
# qPlusRemoteWidget
#
class PlusRemoteWidget(ScriptedLoadableModuleWidget):

  def __init__(self, parent = None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self.logic = PlusRemoteLogic()
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
    recordParametersControlsLayout.addWidget(self.filenameLabel, 0, 0)

    self.filenameBox = qt.QLineEdit()
    self.filenameBox.setToolTip("Set filename (optional)")
    self.filenameBox.visible = False
    self.filenameBox.setText("Recording.mha")
    recordParametersControlsLayout.addWidget(self.filenameBox, 0, 1, 1, 3)

    self.filenameCompletionLabel = qt.QLabel()
    self.filenameCompletionLabel.setText("Add timestamp to filename:")
    self.filenameCompletionLabel.visible = False
    recordParametersControlsLayout.addWidget(self.filenameCompletionLabel, 1, 0)

    self.filenameCompletionBox = qt.QCheckBox()
    self.filenameCompletionBox.visible = False
    recordParametersControlsLayout.addWidget(self.filenameCompletionBox, 1, 1)

    self.enableCompressionLabel = qt.QLabel()
    self.enableCompressionLabel.setText("Compression:")
    self.enableCompressionLabel.visible = False
    recordParametersControlsLayout.addWidget(self.enableCompressionLabel, 1, 2)

    self.enableCompressionBox = qt.QCheckBox()
    self.enableCompressionBox.visible = False
    recordParametersControlsLayout.addWidget(self.enableCompressionBox, 1, 3)

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

    self.saveConfigButton = qt.QPushButton("Save configuration")
    transformUpdateLayout.addRow(self.saveConfigButton)

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
    self.saveConfigButton.connect('clicked(bool)', self.onSaveConfig)

    self.parameterNodeSelector.connect('currentNodeIDChanged(QString)', self.onParameterSetSelected)
    self.linkInputSelector.connect('currentNodeIDChanged(QString)', self.updateParameterNodeFromGui)
    self.captureIDSelector.connect('currentIndexChanged(QString)', self.updateParameterNodeFromGui)
    self.volumeReconstructorIDSelector.connect('currentIndexChanged(QString)', self.updateParameterNodeFromGui)
    #self.displayDefaultLayoutButton.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.filenameBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.filenameCompletionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.enableCompressionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
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
      self.parameterNodeSelector.addNode()

  def onParameterSetSelected(self):
    if self.parameterNode and self.parameterNodeObserver:
      self.parameterNode.RemoveObserver(self.parameterNodeObserver)
    self.parameterNode = self.parameterNodeSelector.currentNode()
    if self.parameterNode:
      # Set up default values for new nodes
      self.logic.setDefaultParameters(self.parameterNode)
      self.parameterNodeObserver = self.parameterNode.AddObserver('currentNodeChanged(vtkMRMLNode*)', self.updateGuiFromParameterNode)
    self.updateGuiFromParameterNode()

  def updateGuiFromParameterNode(self):
  
    if not self.parameterNode:
      self.onConnectorNodeDisconnected(None, None, True)
      return

    #Update interface using parameter node saved values
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

    self.parameterCheckBoxList = {'RoiDisplay': self.displayRoiButton, 'RecordingFilenameCompletion': self.filenameCompletionBox, 'EnableCompression':self.enableCompressionBox, 'OfflineDefaultLayout': self.showOfflineReconstructionResultOnCompletionCheckBox, 'ScoutFilenameCompletion': self.scoutFilenameCompletionBox, 'ScoutDefaultLayout': self.showScoutReconstructionResultOnCompletionCheckBox, 'LiveFilenameCompletion': self.liveFilenameCompletionBox, 'LiveDefaultLayout': self.showLiveReconstructionResultOnCompletionCheckBox}
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

      # Enable/disable buttons based on connector node status (when we updated the GUI the update signal was blocked)
      self.onConnectorNodeSelected()

  def updateParameterNodeFromGui(self):
    # Update parameter node value to save when user change value in the interface

    if not self.parameterNode:
      # Create a parameter node that can store values set in the GUI

      if not self.linkInputSelector.currentNode():
        # There is no valid connector node and no valid parameter node:
        # don't create a new parameter node, as probably the scene has just been closed
        return

      # Block events so that onParameterSetSelected is not called,
      # becuase it would update the GUI from the node.
      self.parameterNodeSelector.blockSignals(True)
      self.parameterNodeSelector.addNode()
      self.parameterNodeSelector.blockSignals(False)
      self.parameterNode = self.parameterNodeSelector.currentNode()
      if not self.parameterNode:
        logging.warning('Failed to create PlusRemote parameter node')
        return
      # Set up default values for new nodes
      self.logic.setDefaultParameters(self.parameterNode)
      self.parameterNodeObserver = self.parameterNode.AddObserver('currentNodeChanged(vtkMRMLNode*)', self.updateGuiFromParameterNode)

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

      # Connector node is selected
      if self.connectorNode.GetState() == slicer.vtkMRMLIGTLConnectorNode.STATE_CONNECTED:
        self.onConnectorNodeConnected(None, None, True)
      else:
        self.onConnectorNodeDisconnected(None, None, True)

      # Add observers for connect/disconnect events
      events = [[slicer.vtkMRMLIGTLConnectorNode.ConnectedEvent, self.onConnectorNodeConnected], [slicer.vtkMRMLIGTLConnectorNode.DisconnectedEvent, self.onConnectorNodeDisconnected]]
      for tagEventHandler in events:
        connectorNodeObserverTag = self.connectorNode.AddObserver(tagEventHandler[0], tagEventHandler[1])
        self.connectorNodeObserverTagList.append(connectorNodeObserverTag)

    else:

      # No connector node is selected
      self.onConnectorNodeDisconnected(None, None, True)

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
    self.enableCompressionLabel.visible = self.recordSettingsButton.checked
    self.enableCompressionBox.visible = self.recordSettingsButton.checked
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
    else:
      self.displayRoiButton.setToolTip("If clicked, show live image")
      redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('None'))

#
# Filenames completion
#
  def generateRecordingOutputFilename(self):
    if self.filenameCompletionBox.isChecked():
      return self.logic.addTimestampToFilename(self.filenameBox.text)
    else:
      return str(self.filenameBox.text)

  def generateScoutRecordingOutputFilename(self):
    if self.scoutFilenameCompletionBox.isChecked():
      return self.logic.addTimestampToFilename(self.scoutScanRecordingLineEdit.text)
    else:
      return str(self.scoutScanRecordingLineEdit.text)

  def getLiveReconstructionOutputFilename(self):
    if self.liveFilenameCompletionBox.isChecked():
      return self.logic.addTimestampToFilename(self.liveVolumeToReconstructFilename.text)
    else:
      return str(self.liveVolumeToReconstructFilename.text)

#
# Commands
#
  def onStartRecording(self):
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.generateRecordingOutputFilename(), self.enableCompressionBox.checked, self.printCommandResponse)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStartScoutRecording(self):
    self.lastScoutRecordingOutputFilename = self.generateScoutRecordingOutputFilename()
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.lastScoutRecordingOutputFilename, False, self.printCommandResponse)

  def onStopScoutRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onScoutVolumeRecorded)

  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputSpacingValue, self.outputOriginValue, self.outputExtentValue, self.printCommandResponse, self.getLiveReconstructionOutputFilename(), self.liveOutputVolumeDeviceBox.text)
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
    self.logic.getVolumeReconstructionSnapshot(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.liveVolumeToReconstructFilename.text, self.liveOutputVolumeDeviceBox.text, self.applyHoleFillingForSnapshot, self.onSnapshotAcquired)
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Snapshot")

  def onUpdateTransform(self):
    self.logic.updateTransform(self.linkInputSelector.currentNode().GetID(), self.transformUpdateInputSelector.currentNode(), self.printCommandResponse)

  def onSaveConfig(self):
    self.logic.saveConfig(self.linkInputSelector.currentNode().GetID(), self.configFileNameBox.text, self.printCommandResponse)

#
# Functions associated to commands
#
  def printCommandResponse(self, command, q):
    statusText = "Command {0} [{1}]: {2}\n".format(command.GetCommandName(), command.GetID(), command.StatusToString(command.GetStatus()))
    if command.GetResponseMessage():
      statusText = statusText + command.GetResponseMessage()
    elif command.GetResponseText():
      statusText = statusText + command.GetResponseText()
    self.replyBox.setPlainText(statusText)

  def onGetCaptureDeviceCommandResponseReceived(self, command, q):
    self.printCommandResponse(command, q)
    if command.GetStatus() != command.CommandSuccess:
      return

    captureDeviceIdsListString = command.GetResponseMessage()
    if captureDeviceIdsListString:
      captureDevicesIdsList = captureDeviceIdsListString.split(",")
    else:
      captureDevicesIdsList = []

    for i in range(0,len(captureDevicesIdsList)):
      if self.captureIDSelector.findText(captureDevicesIdsList[i]) == -1:
        self.captureIDSelector.addItem(captureDevicesIdsList[i])

  def onGetVolumeReconstructorDeviceCommandResponseReceived(self, command, q):
    self.printCommandResponse(command, q)
    if command.GetStatus() != command.CommandSuccess:
      return

    volumeReconstructorDeviceIdsListString = command.GetResponseMessage()
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

  def onVolumeRecorded(self, command, q):
    self.printCommandResponse(command, q)
    self.offlineReconstructButton.setEnabled(True)

    if command.GetStatus() == command.CommandExpired:
      self.recordingStatus.setIcon(qt.QMessageBox.Critical)
      self.recordingStatus.setToolTip("Timeout while waiting for volume reconstruction result")
      return

    if command.GetStatus() != command.CommandSuccess:
      self.recordingStatus.setIcon(qt.QMessageBox.Critical)
      self.recordingStatus.setToolTip(command.GetResponseMessage())
      return

    volumeToReconstructFileName = os.path.basename(command.GetResponseMessage())
    self.offlineVolumeToReconstructSelector.insertItem(0,volumeToReconstructFileName)
    self.offlineVolumeToReconstructSelector.setCurrentIndex(0)
    self.recordingStatus.setIcon(qt.QMessageBox.Information)
    self.recordingStatus.setToolTip("Recording completed, saved as " + volumeToReconstructFileName)
    
  def onScoutVolumeRecorded(self, command, q):
    self.printCommandResponse(command,q)
    self.offlineReconstructButton.setEnabled(True)

    if command.GetStatus() == command.CommandExpired:
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip("Timeout while waiting for volume reconstruction result")
      return

    if command.GetStatus() == command.CommandSuccess:
      self.lastScoutRecordingOutputFilename = os.path.basename(command.GetResponseMessage())
      self.onScoutScanReconstVolume()

  def onVolumeReconstructed(self, command, q):
    self.printCommandResponse(command,q)

    self.offlineReconstructButton.setIcon(self.recordIcon)
    self.offlineReconstructButton.setText("Offline Reconstruction")
    self.offlineReconstructButton.setEnabled(True)
    self.offlineReconstructButton.setChecked(False)

    if command.GetStatus() == command.CommandExpired:
      # volume reconstruction command timed out
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip("Timeout while waiting for volume reconstruction result")
      return

    if command.GetStatus() != command.CommandSuccess:
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip(command.GetResponseMessage())
      return

    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip(command.GetResponseMessage())
      
    # Order of OpenIGTLink message receiving and processing is not guaranteed to be the same
    # therefore we wait a bit to make sure the image message is processed as well
    qt.QTimer.singleShot(100, self.onVolumeReconstructedFinalize)

  def onVolumeReconstructedFinalize(self):
    
    if self.showOfflineReconstructionResultOnCompletionCheckBox.isChecked():
      self.showInSliceViewers(self.getOfflineVolumeRecNode(), ["Yellow", "Green"])
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.FitSliceToAll()

      self.showVolumeRendering(self.getOfflineVolumeRecNode())

  def onScoutVolumeReconstructed(self, command, q):
    self.printCommandResponse(command,q)

    if command.GetStatus() == command.CommandExpired:
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.recordAndReconstructStatus.setToolTip("Timeout while waiting for scout volume reconstruction result")
      return

    self.startStopScoutScanButton.setIcon(self.recordIcon)
    self.startStopScoutScanButton.setText("  Scout Scan\n  Start Recording")
    self.startStopScoutScanButton.setEnabled(True)

    if command.GetStatus() != command.CommandSuccess:
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.recordAndReconstructStatus.setToolTip(command.GetResponseMessage())
      return

    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    scoutScanReconstructFileName = os.path.basename(command.GetResponseMessage())
    self.recordAndReconstructStatus.setToolTip("Scout scan completed, file saved as "+ scoutScanReconstructFileName)

    # Order of OpenIGTLink message receiving and processing is not guaranteed to be the same
    # therefore we wait a bit to make sure the image message is processed as well
    qt.QTimer.singleShot(100, self.onScoutVolumeReconstructedFinalize)

  def onScoutVolumeReconstructedFinalize(self):
    #Create and initialize ROI after scout scan because low resolution scout scan is used to set a smaller ROI for the live high resolution reconstruction
    self.onRoiInitialization()

    if self.showScoutReconstructionResultOnCompletionCheckBox.isChecked():
      scoutScanVolumeNode = self.getScoutVolumeNode()

      scoutVolumeDisplay = scoutScanVolumeNode.GetDisplayNode()
      self.scoutWindow = scoutVolumeDisplay.GetWindow()
      self.scoutLevel = scoutVolumeDisplay.GetLevel()

      self.showInSliceViewers(scoutScanVolumeNode, ["Yellow", "Green"])
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.FitSliceToAll()

      #3D view
      self.showVolumeRendering(scoutScanVolumeNode)

  def onSnapshotAcquired(self, command, q):
    self.printCommandResponse(command,q)
    
    if not self.startStopLiveReconstructionButton.isChecked():
      # live volume reconstruction is not active
      return
    

    # Order of OpenIGTLink message receiving and processing is not guaranteed to be the same
    # therefore we wait a bit to make sure the image message is processed as well
    qt.QTimer.singleShot(100, self.onSnapshotAcquiredFinalize)

  def onSnapshotAcquiredFinalize(self):
    if self.showLiveReconstructionResultOnCompletionCheckBox.isChecked():
      self.showInSliceViewers(self.getLiveVolumeRecNode(), ["Yellow", "Green"])
      self.showVolumeRendering(self.getLiveVolumeRecNode())

    # Request the next snapshot
    snapshotIntervalSec = (self.snapshotIntervalSecSpinBox.value)
    if snapshotIntervalSec > 0:
      self.snapshotTimer.start(snapshotIntervalSec*1000)

  def onVolumeLiveReconstructed(self, command, q):
    self.printCommandResponse(command,q)

    if command.GetStatus() == command.CommandExpired:
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.liveReconstructStatus.setToolTip("Failed to stop volume reconstruction")
      return

    if command.GetStatus() != command.CommandSuccess:
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.liveReconstructStatus.setToolTip(command.GetResponseMessage())
      return

    # We successfully received a volume
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    liveReconstructVolumeFileName = os.path.basename(command.GetResponseMessage())
    self.liveReconstructStatus.setToolTip("Live reconstruction completed, file saved as "+ liveReconstructVolumeFileName)

    # Order of OpenIGTLink message receiving and processing is not guaranteed to be the same
    # therefore we wait a bit to make sure the image message is processed as well
    qt.QTimer.singleShot(100, self.onVolumeLiveReconstructedFinalize)

  def onVolumeLiveReconstructedFinalize(self):
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

    self.defaultCommandTimeoutSec = 30

    # Create commands
    self.cmdGetCaptureDeviceIds = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdGetCaptureDeviceIds.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdGetCaptureDeviceIds.SetCommandName('RequestDeviceIds')
    self.cmdGetCaptureDeviceIds.SetCommandAttribute('DeviceType','VirtualDiscCapture')        
    self.cmdGetReconstructorDeviceIds = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdGetReconstructorDeviceIds.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdGetReconstructorDeviceIds.SetCommandName('RequestDeviceIds')
    self.cmdGetReconstructorDeviceIds.SetCommandAttribute('DeviceType','VirtualVolumeReconstructor')
    self.cmdStartVolumeReconstruction = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdStartVolumeReconstruction.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdStartVolumeReconstruction.SetCommandName('StartVolumeReconstruction')    
    self.cmdStopVolumeReconstruction = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdStopVolumeReconstruction.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdStopVolumeReconstruction.SetCommandName('StopVolumeReconstruction')    
    self.cmdReconstructRecorded = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdReconstructRecorded.SetCommandTimeoutSec(2*self.defaultCommandTimeoutSec);
    self.cmdReconstructRecorded.SetCommandName('ReconstructVolume')
    self.cmdStartRecording = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdStartRecording.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdStartRecording.SetCommandName('StartRecording')    
    self.cmdStopRecording = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdStopRecording.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdStopRecording.SetCommandName('StopRecording')    
    self.cmdGetVolumeReconstructionSnapshot = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdGetVolumeReconstructionSnapshot.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdGetVolumeReconstructionSnapshot.SetCommandName('GetVolumeReconstructionSnapshot')    
    self.cmdUpdateTransform = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdUpdateTransform.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdUpdateTransform.SetCommandName('UpdateTransform')    
    self.cmdSaveConfig = slicer.vtkSlicerOpenIGTLinkCommand()
    self.cmdSaveConfig.SetCommandTimeoutSec(self.defaultCommandTimeoutSec);
    self.cmdSaveConfig.SetCommandName('SaveConfig')    
        
    pass

  def __del__(self):
    # Clean up commands
    self.cmdGetCaptureDeviceIds.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdGetReconstructorDeviceIds.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdStartVolumeReconstruction.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdStopVolumeReconstruction.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdReconstructRecorded.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdStartRecording.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdStopRecording.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdGetVolumeReconstructionSnapshot.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdUpdateTransform.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    self.cmdSaveConfig.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    
  def setDefaultParameters(self, parameterNode):
    parameterList = {'RoiDisplay': False, 'RecordingFilename': "Recording.mha", 'RecordingFilenameCompletion': False, 'OfflineReconstructionSpacing': 3.0, 'OfflineVolumeToReconstruct': 0, 'OfflineOutputVolumeDevice': "RecVol_Reference" , 'OfflineDefaultLayout': True, 'ScoutScanSpacing': 3.0, 'ScoutScanFilename': "ScoutScanRecording.mha", 'ScoutFilenameCompletion': False, 'ScoutDefaultLayout': True, 'LiveReconstructionSpacing': 1.0, 'LiveRecOutputVolumeDevice': "liveReconstruction", 'RoiExtent1': 0.0, 'RoiExtent2': 0.0, 'RoiExtent3': 0.0, 'SnapshotsNumber': 3, 'LiveFilenameCompletion': False, 'LiveDefaultLayout': True}
    for parameter in parameterList:
      if not parameterNode.GetParameter(parameter):
        parameterNode.SetParameter(parameter, str(parameterList[parameter]))

  def addTimestampToFilename(self, filename):
    import re
    import time
    # Get timestamp, for example: 20140528_133802
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    # Add timestamp to filename:
    # - if .mha, .mhd, or nrrd extension specified: find the last occurrence ($) of .mhd, .mha or nrrd, and insert timestamp before that
    # - if no extension found then append timestamp to the end
    return re.sub('(\.nrrd$)|(\.mh[ad]$)|$','{0}\g<0>'.format(timestamp),filename, 1, re.IGNORECASE)

    
  def executeCommand(self, command, connectorNodeId, responseCallbackMethod):
    command.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    command.AddObserver(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent, responseCallbackMethod)
    slicer.modules.openigtlinkremote.logic().SendCommand(command, connectorNodeId)

  def getCaptureDeviceIds(self, connectorNodeId, responseCallbackMethod):
    self.executeCommand(self.cmdGetCaptureDeviceIds, connectorNodeId, responseCallbackMethod)

  def getVolumeReconstructorDeviceIds(self, connectorNodeId, responseCallbackMethod):
    self.executeCommand(self.cmdGetReconstructorDeviceIds, connectorNodeId, responseCallbackMethod)

  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacing, outputOrigin, outputExtent, responseCallbackMethod, liveOutputVolumeFilename, liveOutputVolumeDevice):
    self.cmdStartVolumeReconstruction.SetCommandAttribute('VolumeReconstructorDeviceId', volumeReconstructorDeviceId)
    self.cmdStartVolumeReconstruction.SetCommandAttribute('OutputSpacing', '%f %f %f' % tuple(outputSpacing))
    self.cmdStartVolumeReconstruction.SetCommandAttribute('OutputOrigin', '%f %f %f' % tuple(outputOrigin))
    self.cmdStartVolumeReconstruction.SetCommandAttribute('OutputExtent', '%i %i %i %i %i %i' % tuple(outputExtent))
    self.cmdStartVolumeReconstruction.SetCommandAttribute('OutputVolFilename', liveOutputVolumeFilename)
    self.cmdStartVolumeReconstruction.SetCommandAttribute('OutputVolDeviceName', liveOutputVolumeDevice)
    self.executeCommand(self.cmdStartVolumeReconstruction, connectorNodeId, responseCallbackMethod)

  def stopVolumeReconstruction(self, connectorNodeId, volumeReconstructorDeviceId, responseCallbackMethod, liveOutputVolumeFilename, liveOutputVolumeDevice):
    self.cmdStopVolumeReconstruction.SetCommandAttribute('VolumeReconstructorDeviceId', volumeReconstructorDeviceId)
    self.cmdStopVolumeReconstruction.SetCommandAttribute('OutputVolFilename', liveOutputVolumeFilename)
    self.cmdStopVolumeReconstruction.SetCommandAttribute('OutputVolDeviceName', liveOutputVolumeDevice)
    self.executeCommand(self.cmdStopVolumeReconstruction, connectorNodeId, responseCallbackMethod)

  def reconstructRecorded(self, connectorNodeId, volumeReconstructorDeviceId, inputSequenceFilename, outputSpacing, responseCallbackMethod, outputVolumeFilename, outputVolumeDevice):
    self.cmdReconstructRecorded.SetCommandAttribute('VolumeReconstructorDeviceId', volumeReconstructorDeviceId)
    self.cmdReconstructRecorded.SetCommandAttribute('InputSeqFilename', inputSequenceFilename)
    self.cmdReconstructRecorded.SetCommandAttribute('OutputSpacing', '%f %f %f' % tuple(outputSpacing))
    self.cmdReconstructRecorded.SetCommandAttribute('OutputVolFilename', outputVolumeFilename)
    self.cmdReconstructRecorded.SetCommandAttribute('OutputVolDeviceName', outputVolumeDevice)
    self.executeCommand(self.cmdReconstructRecorded, connectorNodeId, responseCallbackMethod)

  def startRecording(self, connectorNodeId, captureName, fileName, compression, responseCallbackMethod):
    self.cmdStartRecording.SetCommandAttribute('CaptureDeviceId', captureName)
    self.cmdStartRecording.SetCommandAttribute('OutputFilename', fileName)
    self.cmdStartRecording.SetCommandAttribute('EnableCompression', str(compression))
    self.executeCommand(self.cmdStartRecording, connectorNodeId, responseCallbackMethod)

  def stopRecording(self, connectorNodeId, captureName, responseCallbackMethod):
    self.cmdStopRecording.SetCommandAttribute('CaptureDeviceId', captureName)
    self.executeCommand(self.cmdStopRecording, connectorNodeId, responseCallbackMethod)

  def getVolumeReconstructionSnapshot(self, connectorNodeId, volumeReconstructorDeviceId, fileName, deviceName, applyHoleFilling, responseCallbackMethod):
    self.cmdGetVolumeReconstructionSnapshot.SetCommandAttribute('VolumeReconstructorDeviceId', volumeReconstructorDeviceId)
    self.cmdGetVolumeReconstructionSnapshot.SetCommandAttribute('OutputVolFilename', fileName)
    self.cmdGetVolumeReconstructionSnapshot.SetCommandAttribute('OutputVolDeviceName', deviceName)
    self.cmdGetVolumeReconstructionSnapshot.SetCommandAttribute('ApplyHoleFilling', 'TRUE' if applyHoleFilling else 'FALSE')
    self.executeCommand(self.cmdGetVolumeReconstructionSnapshot, connectorNodeId, responseCallbackMethod)
  
  def updateTransform(self, connectorNodeId, transformNode, responseCallbackMethod):
    # Get transform matrix as string
    transformMatrix = transformNode.GetMatrixTransformToParent()
    transformValue = ""
    for i in range(0,4):
      for j in range(0,4):
        transformValue = transformValue + str(transformMatrix.GetElement(i,j)) + " "
    transformValue = transformValue[:-1] # remove last character (extra space at the end)
    # Get transform date as string
    transformDate = str(datetime.datetime.now())

    self.cmdUpdateTransform.SetCommandAttribute('TransformName', transformNode.GetName())
    self.cmdUpdateTransform.SetCommandAttribute('TransformValue', transformValue)
    self.cmdUpdateTransform.SetCommandAttribute('TransformDate', transformDate)
    self.executeCommand(self.cmdUpdateTransform, connectorNodeId, responseCallbackMethod)

  def saveConfig(self, connectorNodeId, filename, responseCallbackMethod):
    self.cmdSaveConfig.SetCommandAttribute('Filename', filename)
    self.executeCommand(self.cmdSaveConfig, connectorNodeId, responseCallbackMethod)

