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
    self.parent.contributors = ["Franklin King (Queen's University), Tamas Ungi (Queen's University)"]
    self.parent.helpText = """
    This is a convenience module for sending commands through OpenIGTLink Remote to PLUS. See <a>https://www.assembla.com/spaces/plus/wiki/PlusServer_commands</a> for more information about Plus Server commands.
    """
    self.parent.acknowledgementText = """
    This work was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)
"""
    #parent.icon = qt.QIcon(":Icons/PlusRemote.png

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
    self.snapshotCounter = 0
    self.filename = "Recording.mha"
    self.scoutFilename = "ScoutScanRecording.mha"
    self.displayNode = None
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

    # Parameters sets
    defaultParametersCollapsibleButton = ctk.ctkCollapsibleButton()
    defaultParametersCollapsibleButton.text = "Parameters Sets"
    self.layout.addWidget(defaultParametersCollapsibleButton)
    defaultParametersLayout = qt.QFormLayout(defaultParametersCollapsibleButton)

    self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
    self.parameterNodeSelector.nodeTypes = ( ("vtkMRMLScriptedModuleNode"), "" )
    self.parameterNodeSelector.selectNodeUponCreation = True
    self.parameterNodeSelector.addEnabled = True
    self.parameterNodeSelector.renameEnabled = True
    self.parameterNodeSelector.removeEnabled = True
    self.parameterNodeSelector.noneEnabled = False
    self.parameterNodeSelector.showHidden = True
    self.parameterNodeSelector.showChildNodeTypes = False
    self.parameterNodeSelector.baseName = "Default Parameter Set"
    self.parameterNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.parameterNodeSelector.setToolTip( "Pick parameters set" )
    defaultParametersLayout.addRow("Parameters Set: ", self.parameterNodeSelector)

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

    self.displayDefaultLayoutButton = qt.QPushButton("Show Live Image")
    self.displayDefaultLayoutButton.visible = False
    self.displayDefaultLayoutButton.setCheckable(True)
    self.displayDefaultLayoutButton.setToolTip("Show live image")
    recordParametersControlsLayout.addWidget(self.displayDefaultLayoutButton, 0, 1)

    self.filenameLabel = qt.QLabel()
    self.filenameLabel.setText("Filename:")
    self.filenameLabel.visible = False
    recordParametersControlsLayout.addWidget(self.filenameLabel, 1, 0)

    self.filenameBox = qt.QLineEdit()
    self.filenameBox.setToolTip("Set Filename (optional)")
    self.filenameBox.visible = False
    self.filenameBox.setText(self.filename)
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
    offlineReconstructCollapsibleButton.text = "Offline Reconstruction of recorded volume"
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

    self.offlineRecDefaultLayoutLabel = qt.QLabel()
    self.offlineRecDefaultLayoutLabel.setText("Show results on completion:")
    self.offlineRecDefaultLayoutLabel.visible = False
    offlineReconstructParametersLayout.addWidget(self.offlineRecDefaultLayoutLabel)

    self.offlineRecDefaultLayoutBox = qt.QCheckBox()
    self.offlineRecDefaultLayoutBox.visible = False
    self.offlineRecDefaultLayoutBox.setChecked(True)
    offlineReconstructParametersLayout.addWidget(self.offlineRecDefaultLayoutBox)

    self.offlineReconstructStatus = qt.QMessageBox()
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction status")
    self.offlineReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.offlineReconstructStatus.setEnabled(False)
    offlineReconstructControlsLayout.addWidget(self.offlineReconstructStatus)

    # Scout scan (record and low resolution reconstruction) and live reconstruction
    # Scout scan part
    scoutScanCollapsibleButton = ctk.ctkCollapsibleButton()
    scoutScanCollapsibleButton.text = "Scout Scan and Live Reconstruction"
    self.layout.addWidget(scoutScanCollapsibleButton)
    scoutScanLayout = qt.QFormLayout(scoutScanCollapsibleButton)

    scoutScanControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(scoutScanControlsLayout)

    self.recordAndReconstructButton = qt.QPushButton("  Scout Scan\n  Start Recording")
    self.recordAndReconstructButton.setCheckable(True)
    self.recordAndReconstructButton.setIcon(self.recordIcon)
    self.recordAndReconstructButton.setToolTip("If clicked, start recording")
    self.recordAndReconstructButton.setEnabled(False)
    self.recordAndReconstructButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    scoutScanControlsLayout.addWidget(self.recordAndReconstructButton)

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
    self.scoutScanFilenameLabel.setText("Recording Filename:")
    self.scoutScanFilenameLabel.visible = False
    self.scoutScanFilenameLabel.setSizePolicy(qt.QSizePolicy.Minimum, qt.QSizePolicy.Minimum)
    scoutScanParametersControlsLayout.addWidget(self.scoutScanFilenameLabel)

    self.scoutScanFilenameBox = qt.QLineEdit()
    self.scoutScanFilenameBox.visible = False
    self.scoutScanFilenameBox.setToolTip( "Scout scan recording filename" )
    self.scoutScanFilenameBox.setText(self.scoutFilename)
    self.scoutScanFilenameBox.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    scoutScanParametersControlsLayout.addWidget(self.scoutScanFilenameBox)

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

    self.scoutViewsBox = qt.QCheckBox()
    self.scoutViewsBox.visible = False
    self.scoutViewsBox.setChecked(True)
    scoutScanParametersControlsLayout.addWidget(self.scoutViewsBox)

    self.recordAndReconstructStatus = qt.QMessageBox()
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan status")
    self.recordAndReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.recordAndReconstructStatus.setEnabled(False)
    scoutScanControlsLayout.addWidget(self.recordAndReconstructStatus)

    # Live Reconstruction Part
    liveReconstructionControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(liveReconstructionControlsLayout)

    self.liveReconstructionButton = qt.QPushButton("  Start Live Reconstruction")
    self.liveReconstructionButton.setCheckable(True)
    self.liveReconstructionButton.setIcon(self.recordIcon)
    self.liveReconstructionButton.setToolTip("If clicked, start live reconstruction")
    self.liveReconstructionButton.setEnabled(False)
    self.liveReconstructionButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    liveReconstructionControlsLayout.addWidget(self.liveReconstructionButton, 0, 0)

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
    self.liveOutpuVolumeDeviceLabel.setText("Output Volume Device:   ")
    self.liveOutpuVolumeDeviceLabel.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveOutpuVolumeDeviceLabel, 0, 0)

    self.liveOutpuVolumeDeviceBox = qt.QLineEdit()
    self.liveOutpuVolumeDeviceBox.setToolTip( "Set output volume device (optional)" )
    self.liveOutpuVolumeDeviceBox.visible = False
    self.liveOutpuVolumeDeviceBox.readOnly = True
    self.liveOutpuVolumeDeviceBox.setText("liveReconstruction")
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveOutpuVolumeDeviceBox, 0, 1)

    liveReconstructionAdvancedParametersControlsLayout = qt.QGridLayout()
    scoutScanLayout.addRow(liveReconstructionAdvancedParametersControlsLayout)

    self.liveVolumeToReconstructLabel = qt.QLabel()
    self.liveVolumeToReconstructLabel.setText("Output Filename:")
    self.liveVolumeToReconstructLabel.visible = False
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveVolumeToReconstructLabel, 1, 0)

    self.liveVolumeToReconstructFilename = qt.QLineEdit()
    self.liveVolumeToReconstructFilename.readOnly = True
    self.liveVolumeToReconstructFilename.visible = False
    self.liveVolumeToReconstructFilename.setToolTip( "Volume for live reconstruction" )
    liveReconstructionAdvancedParametersLayout.addWidget(self.liveVolumeToReconstructFilename, 1, 1)

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

    self.snapshotsLabel = qt.QLabel()
    self.snapshotsLabel.setText("Snapshots:           every ")
    self.snapshotsLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsLabel, 1, 0)

    self.snapshotsBox = qt.QSpinBox()
    self.snapshotsBox.setToolTip( "Takes snapshots every ... seconds (if 0, no snapshots)" )
    self.snapshotsBox.value = 3
    self.snapshotsBox.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsBox, 1, 1)

    self.snapshotsTimeLabel = qt.QLabel()
    self.snapshotsTimeLabel.setText("seconds")
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

    self.viewsBox = qt.QCheckBox()
    self.viewsBox.visible = False
    self.viewsBox.setChecked(True)
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.viewsBox, 3, 1)

    # Transform Update
    transformUpdateCollapsibleButton = ctk.ctkCollapsibleButton()
    transformUpdateCollapsibleButton.text = "Transform Update"
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

#     self.currentDeviceConfigFileName = qt.QLineEdit()
#     self.currentDeviceConfigFileName.setReadOnly(True)
#     transformUpdateLayout.addRow("Current Device Set Config Filename: ", self.currentDeviceConfigFileName)

    self.updateTransformButton = qt.QPushButton("Update")
    transformUpdateLayout.addRow(self.updateTransformButton)

    self.configFileNameBox = qt.QLineEdit()
    transformUpdateLayout.addRow("Filename: ", self.configFileNameBox)

    self.saveTransformButton = qt.QPushButton("Save Config")
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
    self.recordAndReconstructButton.connect('clicked(bool)', self.onRecordAndReconstructButtonClicked)
    self.liveReconstructionButton.connect('clicked(bool)', self.onLiveReconstructionButtonClicked)

    self.displayRoiButton.connect('clicked(bool)', self.onDisplayRoiButtonClicked)
    self.displayDefaultLayoutButton.connect('clicked(bool)', self.onDisplayDefaultLayoutButtonClicked)
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
    self.offlineRecDefaultLayoutBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.outputVolumeSpacingBox.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.scoutScanFilenameBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.scoutFilenameCompletionBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.scoutViewsBox.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.displayRoiButton.connect('clicked(bool)', self.updateParameterNodeFromGui)
    self.outputSpacingLiveReconstructionBox.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.liveOutpuVolumeDeviceBox.connect('textEdited(QString)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection1.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection2.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.outputExtentROIBoxDirection3.connect('valueChanged(double)', self.updateParameterNodeFromGui)
    self.snapshotsBox.connect('valueChanged(int)', self.updateParameterNodeFromGui)
    self.viewsBox.connect('clicked(bool)', self.updateParameterNodeFromGui)

    self.layout.addStretch(1)

    self.onConnectorNodeSelected()
    self.createDefaultParameterSet()

  def createDefaultParameterSet(self):
    nodesCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScriptedModuleNode')
    if nodesCount == 0:
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
    self.parameterNode = self.parameterNodeSelector.currentNode()

    self.parameterValueList = {'OfflineReconstructionSpacing': self.offlineVolumeSpacingBox, 'ScoutScanSpacing': self.outputVolumeSpacingBox, 'LiveReconstructionSpacing': self.outputSpacingLiveReconstructionBox, 'SnapshotsNumber': self.snapshotsBox, 'RoiExtent1': self.outputExtentROIBoxDirection1, 'RoiExtent2': self.outputExtentROIBoxDirection2, 'RoiExtent3': self.outputExtentROIBoxDirection3}
    for parameter in self.parameterValueList:
      if self.parameterNode.GetParameter(parameter):
        #Block Signal to avoid call of updateParameterNodeFromGui when parameter is set programmatically
        self.parameterValueList[parameter].blockSignals(True)
        self.parameterValueList[parameter].setValue(float(self.parameterNode.GetParameter(parameter)))
      self.parameterValueList[parameter].blockSignals(False)

    self.parameterBoxList = {'RecordingFilename': self.filenameBox, 'OfflineOutputVolumeDevice': self.outpuVolumeDeviceBox, 'ScoutScanFilename': self.scoutScanFilenameBox, 'LiveRecOutputVolumeDevice': self.liveOutpuVolumeDeviceBox}
    for parameter in self.parameterBoxList:
      if self.parameterNode.GetParameter(parameter):
        self.parameterBoxList[parameter].blockSignals(True)
        self.parameterBoxList[parameter].setText(self.parameterNode.GetParameter(parameter))
      self.parameterBoxList[parameter].blockSignals(False)

    self.parameterCheckBoxList = {'RoiDisplay': self.displayRoiButton, 'RecordingFilenameCompletion': self.filenameCompletionBox, 'OfflineDefaultLayout': self.offlineRecDefaultLayoutBox, 'ScoutFilenameCompletion': self.scoutFilenameCompletionBox, 'ScoutDefaultLayout': self.scoutViewsBox, 'LiveDefaultLayout': self.viewsBox}
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

    if self.roiNode:
      self.updateROIFromVolumeExtent()

  def updateParameterNodeFromGui(self):
    if not self.parameterNode:
      return
    self.parametersList = {'OpenIGTLinkConnector': self.linkInputSelector.currentNodeID, 'CaptureID': self.captureIDSelector.currentText, 'CaptureIdIndex': self.captureIDSelector.currentIndex, 'VolumeReconstructor': self.volumeReconstructorIDSelector.currentText, 'VolumeReconstructorIndex': self.volumeReconstructorIDSelector.currentIndex, 'RoiDisplay': self.displayRoiButton.isChecked(), 'RoiExtent1': self.outputExtentROIBoxDirection1.value, 'RoiExtent2': self.outputExtentROIBoxDirection2.value, 'RoiExtent3': self.outputExtentROIBoxDirection3.value, 'OfflineReconstructionSpacing': self.offlineVolumeSpacingBox.value, 'OfflineVolumeToReconstruct': self.offlineVolumeToReconstructSelector.currentIndex, 'ScoutScanSpacing': self.outputVolumeSpacingBox.value, 'LiveReconstructionSpacing': self.outputSpacingLiveReconstructionBox.value, 'RecordingFilename': self.filenameBox.text, 'OfflineOutputVolumeDevice': self.outpuVolumeDeviceBox.text, 'ScoutScanFilename': self.scoutScanFilenameBox.text, 'LiveRecOutputVolumeDevice': self.liveOutpuVolumeDeviceBox.text, 'RecordingFilenameCompletion': self.filenameCompletionBox.isChecked(), 'ScoutFilenameCompletion': self.scoutFilenameCompletionBox.isChecked(), 'OfflineDefaultLayout': self.offlineRecDefaultLayoutBox.isChecked(), 'ScoutDefaultLayout': self.scoutViewsBox.isChecked(), 'SnapshotsNumber': self.snapshotsBox.value, 'LiveDefaultLayout': self.viewsBox.isChecked()}
    for parameter in self.parametersList:
      self.parameterNode.SetParameter(parameter, str(self.parametersList[parameter]))

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
    self.recordAndReconstructButton.setEnabled(False)
    self.liveReconstructionButton.setEnabled(False)
    self.offlineReconstructButton.setEnabled(False)
    self.recordingStatus.setEnabled(False)
    self.offlineReconstructStatus.setEnabled(False)
    self.recordAndReconstructStatus.setEnabled(False)
    self.liveReconstructStatus.setEnabled(False)
    self.captureIDSelector.setDisabled(True)
    self.volumeReconstructorIDSelector.setDisabled(True)

  def onStartStopRecordingButtonClicked(self):
    self.recordingStatus.setIcon(qt.QMessageBox.Information)
    self.recordingStatus.setToolTip("Recording in progress")
    if self.startStopRecordingButton.isChecked():
      self.startStopRecordingButton.setText("  Stop Recording")
      self.startStopRecordingButton.setIcon(self.stopIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, stop recording" )
      self.onStartRecording()
      #self.onDrawModelChecked()
    else:
      self.startStopRecordingButton.setText("  Start Recording")
      self.startStopRecordingButton.setIcon(self.recordIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, start recording" )
      self.onStopRecording()

  def onRecordSettingsButtonClicked(self, status):
    self.filenameLabel.visible = self.recordSettingsButton.checked
    self.filenameBox.visible = self.recordSettingsButton.checked
    self.filenameCompletionLabel.visible = self.recordSettingsButton.checked
    self.filenameCompletionBox.visible = self.recordSettingsButton.checked
    self.displayDefaultLayoutButton.visible = self.recordSettingsButton.checked

  def completeRecordingFilename(self):
    if self.filenameCompletionBox.isChecked():
      self.filename = self.logic.filenameCompletion(self.filenameBox.text)
    else:
      self.filename = str(self.filenameBox.text)

  def completeScoutRecordingFilename(self):
    if self.scoutFilenameCompletionBox.isChecked():
      self.filename = self.logic.filenameCompletion(self.scoutScanFilenameBox.text)
    else:
      self.filename = str(self.scoutScanFilenameBox.text)

  def onOfflineReconstructSettingsButtonClicked(self):
    self.offlineVolumeSpacingLabel.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeSpacingBox.visible = self.offlineReconstructSettingsButton.checked
    self.outpuVolumeDeviceLabel.visible = self.offlineReconstructSettingsButton.checked
    self.outpuVolumeDeviceBox.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeToReconstructLabel.visible = self.offlineReconstructSettingsButton.checked
    self.offlineVolumeToReconstructSelector.visible = self.offlineReconstructSettingsButton.checked
    self.offlineRecDefaultLayoutLabel.visible = self.offlineReconstructSettingsButton.checked
    self.offlineRecDefaultLayoutBox.visible = self.offlineReconstructSettingsButton.checked

  def onScoutSettingsButtonClicked(self, status):
    self.outputVolumeSpacingBox.visible = self.scoutSettingsButton.checked
    self.scoutScanFilenameBox.visible = self.scoutSettingsButton.checked
    self.outputVolumeSpacingLabel.visible = self.scoutSettingsButton.checked
    self.scoutScanFilenameLabel.visible = self.scoutSettingsButton.checked
    self.scoutFilenameCompletionLabel.visible = self.scoutSettingsButton.checked
    self.scoutFilenameCompletionBox.visible = self.scoutSettingsButton.checked
    self.scoutViewsLabel.visible = self.scoutSettingsButton.checked
    self.scoutViewsBox.visible = self.scoutSettingsButton.checked

  def onRecordAndReconstructButtonClicked(self):
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan in progress")
    if self.recordAndReconstructButton.isChecked():
      self.recordAndReconstructButton.setText("  Scout Scan\n  Stop Recording and Reconstruct Recorded Volume")
      self.recordAndReconstructButton.setIcon(self.stopIcon)
      self.recordAndReconstructButton.setToolTip( "If clicked, stop recording and reconstruct recorded volume" )
      self.onStartScoutRecording()
    else:
      self.onStopScoutRecording()

  def onLiveReconstructionSettingsButtonClicked(self, status):
    self.outputExtentLiveReconstructionLabel.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection1.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection2.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection3.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsLabel.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsBox.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsTimeLabel.visible = self.liveReconstructionSettingsButton.checked
    #self.drawModelLabel.visible = self.liveReconstructionSettingsButton.checked
    #self.drawModelBox.visible = self.liveReconstructionSettingsButton.checked
    self.viewsLabel.visible = self.liveReconstructionSettingsButton.checked
    self.viewsBox.visible = self.liveReconstructionSettingsButton.checked
    self.liveVolumeToReconstructFilename.visible = self.liveReconstructionSettingsButton.checked
    self.liveVolumeToReconstructLabel.visible = self.liveReconstructionSettingsButton.checked
    self.liveOutpuVolumeDeviceLabel.visible = self.liveReconstructionSettingsButton.checked
    self.liveOutpuVolumeDeviceBox.visible = self.liveReconstructionSettingsButton.checked

  def onLiveReconstructionButtonClicked(self):
    snapshotIntervalSec = (self.snapshotsBox.value)
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Live Reconstruction in progress")

    if self.snapshotsBox.value != 0:
      self.snapshotTimer = qt.QTimer()
      self.snapshotTimer.timeout.connect(self.onSnapshotQuiered)
      self.snapshotTimer.start(snapshotIntervalSec*1000)

    #Option to create surface model
      #if self.drawModelBox.isChecked():
#       self.drawLiveSurfaceModel()
#     else:
#       print("hide model")

    if self.liveReconstructionButton.isChecked():
      if self.roiNode:
        self.updateVolumeExtentFromROI()
      self.liveReconstructionButton.setText("  Stop Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.stopIcon)
      self.liveReconstructionButton.setToolTip( "If clicked, stop live reconstruction" )
      self.onStartReconstruction()
    else:
      self.liveReconstructionButton.setText("  Start Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.recordIcon)
      self.liveReconstructionButton.setToolTip( "If clicked, start live reconstruction" )
      self.onStopReconstruction()

  def onDisplayRoiButtonClicked(self):
    if self.displayRoiButton.isChecked() and self.roiNode:
      self.displayRoiButton.setIcon(self.visibleOnIcon)
      self.displayRoiButton.setToolTip("If clicked, hide ROI")
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

#   def onConfigFileQueried(self):
#     import os.path
#     self.currentDeviceConfigFileName.clear()
#
#     plusConfigFilePath = os.path.expanduser('~/PlusApp-2.1.2.3392-Win64/bin/PlusConfig.xml')
#     plusConfigFile = open(plusConfigFilePath)
#     linesArray = plusConfigFile.readlines()
#     for i in range(0, len(linesArray)):
#       if 'LastDeviceSetConfigurationFileName' in linesArray[i]:
#         linesArray[i] = linesArray[i].replace('" />','')
#         linesArray[i] = os.path.basename(linesArray[i])
#         self.currentDeviceConfigFileName.setText(linesArray[i])
#     plusConfigFile.close()

  def onStartRecording(self):
    self.completeRecordingFilename()
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.filename, self.onGenericCommandResponseReceived)

  def onStartScoutRecording(self):
    self.completeScoutRecordingFilename()
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.scoutFilename, self.onGenericCommandResponseReceived)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStopScoutRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onScoutVolumeRecorded)

  def onStartReconstruction(self):
    liveOutputVolumeFilename = "LiveReconstructedVolume.mha"
    self.liveVolumeToReconstructFilename.setText(liveOutputVolumeFilename)
    if self.liveOutpuVolumeDeviceBox.text == "":
      liveOutputDeviceVolumeName = "liveReconstruction"
    else:
      liveOutputDeviceVolumeName = self.liveOutpuVolumeDeviceBox.text
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputSpacingValue, self.outputOriginValue, self.outputExtentValue, self.onGenericCommandResponseReceived, liveOutputVolumeFilename, liveOutputDeviceVolumeName)

  def onStopReconstruction(self):
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.onVolumeLiveReconstructed, self.liveVolumeToReconstructFilename.text, self.liveOutpuVolumeDeviceBox.text)

  def onReconstVolume(self):
    self.offlineReconstructButton.setIcon(self.waitIcon)
    self.offlineReconstructButton.setText("  Offline Reconstruction in progress ...")
    self.offlineReconstructButton.setEnabled(False)
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction in progress")
    outputSpacing = [self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.offlineVolumeToReconstructSelector.currentText, outputSpacing, self.onVolumeReconstructed, "RecVol_Reference.mha", self.outpuVolumeDeviceBox.text)

  def onScoutScanReconstVolume(self):
    self.recordAndReconstructButton.setIcon(self.waitIcon)
    self.recordAndReconstructButton.setText("  Scout Scan\n  Reconstruction in progress ...")
    self.recordAndReconstructButton.setEnabled(False)
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Recording completed. Reconstruction in progress")
    outputSpacing = [self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.scoutFilename, outputSpacing, self.onScoutVolumeReconstructed, "scoutFile.mha", "ScoutScan")

  def onSnapshotQuiered(self, stopFlag = ""):
    if self.liveReconstructionButton.isChecked():
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.liveReconstructStatus.setToolTip("Snapshot")
      self.logic.getVolumeReconstructionSnapshot(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.scoutFilename, self.onSnapshotAcquiered)
      self.snapshotCounter+=1
    else:
      self.snapshotTimer.stop()

  def onUpdateTransform(self):
    self.logic.updateTransform(self.linkInputSelector.currentNode().GetID(), self.transformUpdateInputSelector.currentNode(), self.onGenericCommandResponseReceived)

  def onSaveTransform(self):
    self.logic.saveTransform(self.linkInputSelector.currentNode().GetID(), self.configFileNameBox.text, self.onGenericCommandResponseReceived)

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

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    logic.discardCommand(commandId, self.linkInputSelector.currentNode().GetID())

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    captureDeviceIdsListString = commandResponseElement.GetAttribute("Message")
    captureDevicesIdsList = captureDeviceIdsListString.split(",")

    for i in range(0,len(captureDevicesIdsList)):
      if self.captureIDSelector.findText(captureDevicesIdsList[i]) == -1:
        self.captureIDSelector.addItem(captureDevicesIdsList[i])
    self.startStopRecordingButton.setEnabled(True)

  def onGetVolumeReconstructorDeviceCommandResponseReceived(self, commandId, textNode):
    if not textNode:
        self.replyBox.setPlainText("GetVolumeReconstructorDeviceCommand timeout: {0}".format(commandId))
        return

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    logic.discardCommand(commandId, self.linkInputSelector.currentNode().GetID())

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    volumeReconstructorDeviceIdsListString = commandResponseElement.GetAttribute("Message")
    volumeReconstructorDeviceIdsList = volumeReconstructorDeviceIdsListString.split(",")
    self.volumeReconstructorIDSelector.clear()
    self.volumeReconstructorIDSelector.addItems(volumeReconstructorDeviceIdsList)
    self.offlineReconstructButton.setEnabled(True)
    self.recordAndReconstructButton.setEnabled(True)
    self.liveReconstructionButton.setEnabled(True)
    self.recordingStatus.setEnabled(True)
    self.offlineReconstructStatus.setEnabled(True)
    self.recordAndReconstructStatus.setEnabled(True)
    self.liveReconstructStatus.setEnabled(True)

  def onVolumeRecorded(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)
    self.offlineReconstructButton.setEnabled(True)

    logic = PlusRemoteLogic()
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

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)

    if commandResponseElement.GetAttribute("Status") == "SUCCESS":
      self.onScoutScanReconstVolume()

  def onVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    offlineReconstMessage = commandResponseElement.GetAttribute("Message")

    self.offlineReconstructButton.setIcon(self.recordIcon)
    self.offlineReconstructButton.setText("Offline Reconstruction")
    self.offlineReconstructButton.setEnabled(True)
    self.offlineReconstructButton.setChecked(False)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.offlineReconstructStatus.setToolTip(commandResponseElement.GetAttribute("Message"))
    else:
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip(offlineReconstMessage)

    if self.offlineRecDefaultLayoutBox.isChecked():
      offlineVolumeRecName = self.outpuVolumeDeviceBox.text
      self.offlineVolumeRecNode = slicer.util.getNode(offlineVolumeRecName)

      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceSecondaryVolumeID(self.offlineVolumeRecNode.GetID())
      selectionNode.SetReferenceActiveVolumeID(self.offlineVolumeRecNode.GetID())
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.PropagateVolumeSelection(0)
      applicationLogic.FitSliceToAll()

      redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
      redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('Image_Reference').GetID())
      yellowLogic = slicer.app.layoutManager().sliceWidget('Yellow').sliceLogic()
      yellowLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.offlineVolumeRecNode.GetID())
      greenLogic = slicer.app.layoutManager().sliceWidget('Green').sliceLogic()
      greenLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.offlineVolumeRecNode.GetID())

      self.volumeRenderingDisplay(self.offlineVolumeRecNode)

  def onScoutVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    scoutScanMessage = commandResponseElement.GetAttribute("Message")
    scoutScanReconstructFileName = os.path.basename(scoutScanMessage)

    self.recordAndReconstructButton.setIcon(self.recordIcon)
    self.recordAndReconstructButton.setText("  Scout Scan\n  Start Recording")
    self.recordAndReconstructButton.setEnabled(True)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.onRoiInitialization()
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.recordAndReconstructStatus.setToolTip("Scout scan completed, file saved as "+ scoutScanReconstructFileName)
    else:
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.recordAndReconstructStatus.setToolTip(scoutScanMessage)

    if self.scoutViewsBox.isChecked():
      self.scoutScanVolumeNode = slicer.util.getNode('ScoutScan')
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceActiveVolumeID(self.scoutScanVolumeNode.GetID())
      applicationLogic = slicer.app.applicationLogic()
      applicationLogic.PropagateVolumeSelection(0)
      applicationLogic.FitSliceToAll()
      #Red view: Image_Reference
      redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
      redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('Image_Reference').GetID())
      #3D view
      self.volumeRenderingDisplay(self.scoutScanVolumeNode)

  def onSnapshotAcquiered(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    if self.viewsBox.isChecked():
      #liveVolumeRecName = self.liveOutpuVolumeDeviceBox.text
      #self.liveVolumeRecNode = slicer.util.getNode(liveVolumeRecName)
      self.liveVolumeRecNode = slicer.util.getNode('liveReconstruction')
      self.liveVolumeReconstructionDisplay()
      self.volumeRenderingDisplay(self.liveVolumeRecNode)

  def onVolumeLiveReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    liveReconstructMessage = commandResponseElement.GetAttribute("Message")
    liveReconstructVolumeFileName = os.path.basename(liveReconstructMessage)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.liveReconstructStatus.setToolTip("Live reconstruction completed, file saved as "+ liveReconstructVolumeFileName)
    else:
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.liveReconstructStatus.setToolTip(liveReconstructMessage)

    if self.viewsBox.isChecked():
      #liveVolumeRecName = self.liveOutpuVolumeDeviceBox.text
      #self.liveVolumeRecNode = slicer.util.getNode(liveVolumeRecName)
      self.liveVolumeRecNode = slicer.util.getNode('liveReconstruction')
      self.liveVolumeReconstructionDisplay()
      self.volumeRenderingDisplay(self.liveVolumeRecNode)

  def liveVolumeReconstructionDisplay(self):
    self.scoutScanVolumeNode = slicer.util.getNode('ScoutScan')
    liveVolumeRecName = self.liveOutpuVolumeDeviceBox.text
    self.liveVolumeRecNode = slicer.util.getNode(liveVolumeRecName)

    selectionliveRecNode = slicer.app.applicationLogic().GetSelectionNode()
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.PropagateVolumeSelection(0)

    redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    redLogic.GetSliceCompositeNode().SetBackgroundVolumeID(slicer.util.getNode('Image_Reference').GetID())
    yellowLogic = slicer.app.layoutManager().sliceWidget('Yellow').sliceLogic()
    yellowLogic.GetSliceCompositeNode().SetForegroundVolumeID(self.liveVolumeRecNode.GetID())
    yellowLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.scoutScanVolumeNode.GetID())
    yellowLogic.GetSliceCompositeNode().SetForegroundOpacity(50)
    greenLogic = slicer.app.layoutManager().sliceWidget('Green').sliceLogic()
    greenLogic.GetSliceCompositeNode().SetForegroundVolumeID(self.liveVolumeRecNode.GetID())
    greenLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.scoutScanVolumeNode.GetID())
    greenLogic.GetSliceCompositeNode().SetForegroundOpacity(50)

  def volumeRenderingDisplay(self, volumeNode):
    volRenderingLogic = slicer.modules.volumerendering.logic()
    if not self.displayNode:
      self.displayNode = volRenderingLogic.CreateVolumeRenderingDisplayNode()
      slicer.mrmlScene.AddNode(self.displayNode)
      self.displayNode.UnRegister(volRenderingLogic)
    volRenderingLogic.UpdateDisplayNodeFromVolumeNode(self.displayNode,volumeNode)
    volumeNode.AddAndObserveDisplayNodeID(self.displayNode.GetID())

  def onRoiInitialization(self):
    if self.roiNode:
      slicer.mrmlScene.RemoveNode(self.roiNode)

    reconstructedNode = slicer.mrmlScene.GetNodesByName('ScoutScan')
    reconstructedVolumeNode = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(reconstructedNode.GetItemAsObject(0))

    roiCenterInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    roiRadiusInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    bounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

    if reconstructedVolumeNode:
      reconstructedVolumeNode.GetRASBounds(bounds)
      for i in range(0,5,2):
        roiCenterInit[i] = (bounds[i+1] + bounds[i])/2
        roiRadiusInit[i] = (bounds[i+1] - bounds[i])/2
      self.roiNode = slicer.vtkMRMLAnnotationROINode()
      self.roiNode.SetXYZ(roiCenterInit[0], roiCenterInit[2], roiCenterInit[4])
      self.roiNode.SetRadiusXYZ(roiRadiusInit[0], roiRadiusInit[2], roiRadiusInit[4])
      self.roiNode.Initialize(slicer.mrmlScene)
      self.roiNode.SetDisplayVisibility(0)
      self.roiNode.SetInteractiveMode(1)
    self.updateVolumeExtentFromROI()

  def updateVolumeExtentFromROI(self):
    roiCenter = [0.0, 0.0, 0.0]
    roiRadius = [0.0, 0.0, 0.0]
    roiOrigin = [0.0, 0.0, 0.0]
    if self.roiNode:
      self.roiNode.GetXYZ(roiCenter)
      self.roiNode.GetRadiusXYZ(roiRadius)

    for i in range(0,len(roiCenter)):
        roiOrigin[i] = roiCenter[i] - roiRadius[i]
    self.outputOriginValue = roiOrigin
    self.outputExtentValue = [0, int((2*roiRadius[0])/self.outputSpacingLiveReconstructionBox.value), 0, int((2*roiRadius[1])/self.outputSpacingLiveReconstructionBox.value), 0, int((2*roiRadius[2])/self.outputSpacingLiveReconstructionBox.value)]
    self.outputSpacingValue = [self.outputSpacingLiveReconstructionBox.value, self.outputSpacingLiveReconstructionBox.value, self.outputSpacingLiveReconstructionBox.value]

    self.outputExtentROIBoxDirection1.value = self.outputExtentValue[1]
    self.outputExtentROIBoxDirection2.value = self.outputExtentValue[3]
    self.outputExtentROIBoxDirection3.value = self.outputExtentValue[5]

  def updateROIFromVolumeExtent(self):
    if self.roiNode:
      slicer.mrmlScene.RemoveNode(self.roiNode)

    roiCenter = [0.0, 0.0, 0.0]
    roiRadius = [0.0, 0.0, 0.0]

    roiRadius[0] = (self.outputSpacingLiveReconstructionBox.value * self.outputExtentROIBoxDirection1.value)/2
    roiRadius[1] = (self.outputSpacingLiveReconstructionBox.value * self.outputExtentROIBoxDirection2.value)/2
    roiRadius[2] = (self.outputSpacingLiveReconstructionBox.value * self.outputExtentROIBoxDirection3.value)/2
    roiOrigin = self.outputOriginValue
    for i in range(0,len(roiCenter)):
      roiCenter[i] = roiOrigin[i] + roiRadius[i]

    self.roiNode = slicer.vtkMRMLAnnotationROINode()
    self.roiNode.SetXYZ(roiCenter[0], roiCenter[1], roiCenter[2])
    self.roiNode.SetRadiusXYZ(roiRadius[0], roiRadius[1], roiRadius[2])
    self.roiNode.Initialize(slicer.mrmlScene)
    self.roiNode.SetDisplayVisibility(0)
    self.roiNode.SetInteractiveMode(1)

#   def drawLiveSurfaceModel(self):
#     if self.drawModelBox.isChecked():
#       if self.liveReconstructionButton.isChecked():
#         appender = vtk.vtkAppendPolyData()
#
#         if slicer.util.getNode('Image_Reference') is None:
#           self.liveReconstructStatus.setIcon(qt.QMessageBox.Critical)
#           self.liveReconstructStatus.setToolTip("Error: no ImageToReferenceTransform. Unable to draw surface model")
#         else:
#           liveOutputDeviceVolumeNode = slicer.mrmlScene.GetNodesByName('liveReconstruction') #ou self.liveOutpuVolumeDeviceBox.text
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
  def __init__(self):
    self.commandToMethodHashtable = {}
    self.defaultCommandTimeoutSec = 30
    self.timerIntervalSec = 0.1
    self.timer = qt.QTimer()
    self.timer.timeout.connect(self.getCommandReply)
    pass

  def setDefaultParameters(self, parameterNode):
    parameterList = {'RoiDisplay': False, 'RecordingFilename': "Recording.mha", 'RecordingFilenameCompletion': False, 'OfflineReconstructionSpacing': 3.0, 'OfflineVolumeToReconstruct': 0, 'OfflineOutputVolumeDevice': "RecVol_Reference" , 'OfflineDefaultLayout': True, 'ScoutScanSpacing': 3.0, 'ScoutScanFilename': "ScoutScanRecording.mha", 'ScoutFilenameCompletion': False, 'ScoutDefaultLayout': True, 'LiveReconstructionSpacing': 1.0, 'LiveRecOutputVolumeDevice': "liveReconstruction", 'RoiExtent1': 0.0, 'RoiExtent2': 0.0, 'RoiExtent3': 0.0, 'SnapshotsNumber': 3, 'LiveDefaultLayout': True}
    for parameter in parameterList:
      if not parameterNode.GetParameter(parameter):
        parameterNode.SetParameter(parameter, str(parameterList[parameter]))

  def filenameCompletion(self, filename):
    basename = filename.replace(".mha","")
    self.dateTimeList = {'Month': str(datetime.datetime.now().month), 'Day': str(datetime.datetime.now().day), 'Hour': str(datetime.datetime.now().hour), 'Minute': str(datetime.datetime.now().minute), 'Second': str(datetime.datetime.now().second)}
    for element in self.dateTimeList:
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

#     self.annotationTextNode = slicer.util.getNode('vtkMRMLAnnotationTextNode1')
#     slicer.mrmlScene.RemoveNode(self.annotationTextNode)
#     self.annotationTextNode = slicer.util.getNode('vtkMRMLAnnotationTextNode2')
#     slicer.mrmlScene.RemoveNode(self.annotationTextNode)
#     self.annotationTextNode = slicer.util.getNode('vtkMRMLAnnotationTextNode3')
#     slicer.mrmlScene.RemoveNode(self.annotationTextNode)
#     self.annotationTextNode = slicer.util.getNode('vtkMRMLAnnotationTextNode4')
#     slicer.mrmlScene.RemoveNode(self.annotationTextNode)

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

  def reconstructRecorded(self, connectorNodeId, volumeReconstructorDeviceId, volumeToReconstructId, outputSpacing, method, outputVolumeFilename, outputVolumeDevice):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " InputSeqFilename=" + "\"" + volumeToReconstructId + "\"" + " OutputSpacing=" + "\"" + "%f %f %f" % tuple(outputSpacing) + "\"" + " OutputVolFilename=" + "\"" + outputVolumeFilename +"\"" + " OutputVolDeviceName=" + "\"" + outputVolumeDevice +"\""
    self.executeCommand(connectorNodeId, "ReconstructVolume", parameters, method)

  def startRecording(self, connectorNodeId, captureName, fileName, method):
    parameters = "CaptureDeviceId=" + "\"" + captureName + "\"" + " OutputFilename=" + "\"" + fileName + "\""
    self.executeCommand(connectorNodeId, "StartRecording", parameters, method)

  def stopRecording(self, connectorNodeId, captureName, method):
    self.executeCommand(connectorNodeId, "StopRecording", "CaptureDeviceId=" + "\"" + captureName + "\"", method)

  def getVolumeReconstructionSnapshot(self, connectorNodeId, volumeReconstructorDeviceId, fileName, method):
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputFilename=" + "\"" + fileName + "\""  + " OutputVolDeviceName=" + "\"" + "liveReconstruction" +"\""
    self.executeCommand(connectorNodeId, "GetVolumeReconstructionSnapshot", parameters, method)

  def updateTransform(self, connectorNodeId, transformNode, method):
    transformMatrix = transformNode.GetMatrixTransformToParent()
    transformValue = ""
    for i in range(0,4):
      for j in range(0,4):
        transformValue = transformValue + str(transformMatrix.GetElement(i,j)) + " "

    transformDate = str(datetime.datetime.now())

    parameters = "TransformName=" + "\"" + transformNode.GetName() + "\"" + " TransformValue=" + "\"" + transformValue + "\"" + "TransformDate=" + "\"" + transformDate + "\""
    self.executeCommand(connectorNodeId, "UpdateTransform", parameters, method)

  def saveTransform(self, connectorNodeId, filename, method):
    parameters = "Filename=" + "\"" + filename + "\""
    self.executeCommand(connectorNodeId, "SaveConfig", parameters, method)

  def discardCommand(self, commandId, connectorNodeId):
    slicer.modules.openigtlinkremote.logic().DiscardCommand(commandId, connectorNodeId)
