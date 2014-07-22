from __main__ import vtk, qt, ctk, slicer
#
# PlusRemote
#

class PlusRemote:
  def __init__(self, parent):
    parent.title = "Plus Remote"
    parent.categories = ["IGT"]
    parent.contributors = ["Franklin King (Queen's University), Tamas Ungi (Queen's University)"]
    parent.helpText = """
    This is a convenience module for sending commands through OpenIGTLink Remote to PLUS. See <a>https://www.assembla.com/spaces/plus/wiki/PlusServer_commands</a> for more information about Plus Server commands.
    """
    parent.acknowledgementText = """
    This work was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)
"""
    #parent.icon = qt.QIcon(":Icons/PlusRemote.png")
    self.parent = parent


#
# qPlusRemoteWidget
#
class PlusRemoteWidget:

  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    self.logic = PlusRemoteLogic()
    self.commandToMethodHashtable = {}
    self.connectorNode = None
    self.connectorNodeObserverTagList = []
    self.connectorNodeConnected = False
    self.roiNode = None
    self.outputSpacingValue = 0
    self.outputOriginValue = []
    self.outputExtentValue = []

    if not parent:
      self.setup()
      self.parent.show()

    self.plusRemoteModuleDirectoryPath = slicer.modules.plusremote.path.replace("PlusRemote.py","")

  def setup(self):
    # Instantiate and connect widgets

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "PlusRemote Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    #reloadFormLayout.addWidget(self.reloadAndTestButton)
    #self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)


    # Module requires openigtlinkremote
    try:
      slicer.modules.openigtlinkremote
    except:
      self.errorLabel = qt.QLabel("Could not find OpenIGTLink Remote module")
      self.layout.addWidget(self.errorLabel)
      return

    # IGTLink Connector
    connectorCollapsibleButton = ctk.ctkCollapsibleButton()
    connectorCollapsibleButton.text = "OpenIGTLink Connector"
    self.layout.addWidget(connectorCollapsibleButton)
    connectorLayout = qt.QFormLayout(connectorCollapsibleButton)

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
    connectorLayout.addRow("OpenIGTLinkConnector: ", self.linkInputSelector)

    # Record and Reconstruct Parameters
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)
    parametersLayout = qt.QFormLayout(parametersCollapsibleButton)

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

    self.fileNameBox = qt.QLineEdit()
    self.fileNameBox.setToolTip("Set Filename (optional)")
    recordingLayout.addRow("Filename: ", self.fileNameBox)

    recordingControlsLayout = qt.QHBoxLayout()
    recordingLayout.addRow(recordingControlsLayout)

    self.startRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.stopRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
    self.startStopRecordingButton = qt.QPushButton("Start Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setIcon(self.startRecordingIcon)
    self.startStopRecordingButton.setEnabled(False)
    self.startStopRecordingButton.setToolTip("If clicked, start recording")
    recordingControlsLayout.addWidget(self.startStopRecordingButton)

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

    #Move to the same row, use grid and box layouts
    offlineReconstructControlsLayout = qt.QHBoxLayout()
    offlineReconstructLayout.addRow(offlineReconstructControlsLayout)

    self.offlineVolumeSpacingLabel = qt.QLabel()
    self.offlineVolumeSpacingLabel.setText("Spacing:  ")
    offlineReconstructControlsLayout.addWidget(self.offlineVolumeSpacingLabel)

    self.offlineVolumeSpacingBox = qt.QDoubleSpinBox()
    self.offlineVolumeSpacingBox.setToolTip("Output volume spacing for offline reconstruction")
    self.offlineVolumeSpacingBox.value = 3
    offlineReconstructControlsLayout.addWidget(self.offlineVolumeSpacingBox)

    self.outpuVolumeDeviceLabel = qt.QLabel()
    self.outpuVolumeDeviceLabel.setText("   Output Volume Device:")
    offlineReconstructControlsLayout.addWidget(self.outpuVolumeDeviceLabel)

    self.outpuVolumeDeviceBox = qt.QLineEdit()
    self.outpuVolumeDeviceBox.setToolTip( "Set output volume device (optional)" )
    offlineReconstructControlsLayout.addWidget(self.outpuVolumeDeviceBox)

    self.offlineVolumeToReconstructSelector = qt.QComboBox()
    self.offlineVolumeToReconstructSelector.setEditable(True)
    self.offlineVolumeToReconstructSelector.setToolTip( "Pick/set volume to reconstruct" )
    self.offlineVolumeToReconstructSelector.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    offlineReconstructLayout.addRow("Filename:", self.offlineVolumeToReconstructSelector)

    offlineReconstructControls2Layout = qt.QHBoxLayout()
    offlineReconstructLayout.addRow(offlineReconstructControls2Layout)

#     self.offlineReconstructButton = qt.QPushButton("Offline Reconstruction")
#     self.offlineReconstructButton.setToolTip("If clicked, reconstruct recorded volume")
#     self.offlineReconstructButton.setEnabled(False)
#     #self.offlineReconstructButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
#     offlineReconstructControls2Layout.addWidget(self.offlineReconstructButton)

    self.offlineReconstructStartIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.offlineReconstructWaitIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Wait.png')
    self.offlineReconstructButton = qt.QPushButton("Offline Reconstruction")
    self.offlineReconstructButton.setCheckable(True)
    self.offlineReconstructButton.setIcon(self.offlineReconstructStartIcon)
    self.offlineReconstructButton.setEnabled(False)
    self.offlineReconstructButton.setToolTip("If clicked, reconstruct recorded volume")
    offlineReconstructControls2Layout.addWidget(self.offlineReconstructButton)

    self.offlineReconstructStatus = qt.QMessageBox()
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction status")
    self.offlineReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.offlineReconstructStatus.setEnabled(False)
    offlineReconstructControls2Layout.addWidget(self.offlineReconstructStatus)

    # Scout scan (record and low resolution reconstruction) and live reconstruction
    # Scout scan part
    scoutScanCollapsibleButton = ctk.ctkCollapsibleButton()
    scoutScanCollapsibleButton.text = "Scout Scan and Live Reconstruction"
    self.layout.addWidget(scoutScanCollapsibleButton)
    scoutScanLayout = qt.QFormLayout(scoutScanCollapsibleButton)

    scoutScanControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(scoutScanControlsLayout)

    self.recordAndReconstructButton = qt.QPushButton(" Scout Scan\n Start Recording")
    self.recordAndReconstructButton.setCheckable(True)
    self.recordAndReconstructButton.setIcon(self.startRecordingIcon)
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
    #self.outputVolumeSpacingBox.setReadOnly(True)
    self.outputVolumeSpacingBox.value = 3.0
    self.outputVolumeSpacingBox.visible = False
    scoutScanParametersControlsLayout.addWidget(self.outputVolumeSpacingBox, 0, 1)

    self.volumeToReconstructLabel = qt.QLabel()
    self.volumeToReconstructLabel.setText("Filename:")
    self.volumeToReconstructLabel.visible = False
    self.volumeToReconstructLabel.setSizePolicy(qt.QSizePolicy.Minimum, qt.QSizePolicy.Minimum)
    scoutScanParametersControlsLayout.addWidget(self.volumeToReconstructLabel)

    self.volumeToReconstructSelector = qt.QLineEdit()
    self.volumeToReconstructSelector.readOnly = True
    self.volumeToReconstructSelector.visible = False
    self.volumeToReconstructSelector.setToolTip( "Volume to reconstruct" )
    self.volumeToReconstructSelector.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    scoutScanParametersControlsLayout.addWidget(self.volumeToReconstructSelector)

    self.drawModelLabel = qt.QLabel()
    self.drawModelLabel.setText("Draw model:  ")
    self.drawModelLabel.visible = False
    scoutScanParametersControlsLayout.addWidget(self.drawModelLabel)

    self.drawModelBox = qt.QCheckBox()
    scoutScanParametersControlsLayout.addWidget(self.drawModelBox)
    self.drawModelBox.visible = False

    self.recordAndReconstructStatus = qt.QMessageBox()
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan status")
    self.recordAndReconstructStatus.setStandardButtons(qt.QMessageBox.NoButton)
    self.recordAndReconstructStatus.setEnabled(False)
    scoutScanControlsLayout.addWidget(self.recordAndReconstructStatus)

    # Live Reconstruction Part
    liveReconstructionControlsLayout = qt.QHBoxLayout()
    scoutScanLayout.addRow(liveReconstructionControlsLayout)

    self.startLiveReconstructionIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.stopLiveReconstructionIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
    self.liveReconstructionButton = qt.QPushButton("Start Live Reconstruction")
    self.liveReconstructionButton.setCheckable(True)
    self.liveReconstructionButton.setIcon(self.startLiveReconstructionIcon)
    self.liveReconstructionButton.setToolTip("If clicked, start live reconstruction")
    self.liveReconstructionButton.setEnabled(False)
    self.liveReconstructionButton.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    liveReconstructionControlsLayout.addWidget(self.liveReconstructionButton, 0, 0)

    liveReconstructionParametersControlsLayout = qt.QGridLayout()
    liveReconstructionControlsLayout.addLayout(liveReconstructionParametersControlsLayout)

    self.displayRoiLabel = qt.QLabel()
    self.displayRoiLabel.setText("ROI:")
    liveReconstructionParametersControlsLayout.addWidget(self.displayRoiLabel, 0, 0)

    self.notVisibleRoiIcon = qt.QIcon(":Icons\VisibleOff.png")
    self.visibleRoiIcon = qt.QIcon(":Icons\VisibleOn.png")
    self.displayRoiButton = qt.QToolButton()
    self.displayRoiButton.setCheckable(True)
    self.displayRoiButton.setIcon(self.notVisibleRoiIcon)
    self.displayRoiButton.setToolTip("If clicked, display ROI")
    liveReconstructionParametersControlsLayout.addWidget(self.displayRoiButton, 0, 1)

    self.outputSpacingLiveReconstructionLabel = qt.QLabel()
    self.outputSpacingLiveReconstructionLabel.setText("Spacing:")
    liveReconstructionParametersControlsLayout.addWidget(self.outputSpacingLiveReconstructionLabel, 1, 0)

    self.outputSpacingLiveReconstructionBox = qt.QDoubleSpinBox()
    self.outputSpacingLiveReconstructionBox.setToolTip( "Set output volume spacing for live reconstruction" )
    self.outputSpacingLiveReconstructionBox.value = self.outputVolumeSpacingBox.value
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

    self.snapshotsLabel = qt.QLabel()
    self.snapshotsLabel.setText("Snapshots:                           every ")
    self.snapshotsLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsLabel, 1, 0)

    self.snapshotsBox = qt.QSpinBox()
    self.snapshotsBox.setToolTip( "Takes snapshots every ... seconds" )
    self.snapshotsBox.value = 3
    self.snapshotsBox.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsBox, 1, 1)

    self.snapshotsTimeLabel = qt.QLabel()
    self.snapshotsTimeLabel.setText("seconds")
    self.snapshotsTimeLabel.visible = False
    liveReconstructionAdvancedParametersControlsLayout.addWidget(self.snapshotsTimeLabel, 1, 2)

    # Transform Update
    transformUpdateCollapsibleButton = ctk.ctkCollapsibleButton()
    transformUpdateCollapsibleButton.text = "Transform Update"
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

    self.currentDeviceConfigFileName = qt.QLineEdit()
    self.currentDeviceConfigFileName.setReadOnly(True)
    transformUpdateLayout.addRow("Current Device Set Config Filename: ", self.currentDeviceConfigFileName)

    self.updateTransformButton = qt.QPushButton("Update")
    transformUpdateLayout.addRow(self.updateTransformButton)

    self.configFileNameBox = qt.QLineEdit()
    transformUpdateLayout.addRow("Filename: ", self.configFileNameBox)

    self.saveTransformButton = qt.QPushButton("Save Config")
    transformUpdateLayout.addRow(self.saveTransformButton)

    replyUpdateCollapsibleButton = ctk.ctkCollapsibleButton()
    replyUpdateCollapsibleButton.text = "Reply"
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
    self.scoutSettingsButton.connect('clicked(bool)', self.onScoutSettingsButtonClicked)
    self.liveReconstructionSettingsButton.connect('clicked(bool)', self.onLiveReconstructionSettingsButtonClicked)

    self.drawModelBox.connect('stateChanged(int)', self.onDrawModelChecked)
    #self.drawModelBox.connect('stateChanged(int)', self.onRecordAndReconstructButtonClicked)

    self.updateTransformButton.connect('clicked(bool)', self.onUpdateTransform)
    self.saveTransformButton.connect('clicked(bool)', self.onSaveTransform)

    self.layout.addStretch(1)

    self.onConnectorNodeSelected()

  def cleanup(self):
    print "Cleanup is called"
    pass

  def onConnectorNodeSelected(self):
    if self.connectorNode and self.connectorNodeObserverTagList:
      for tag in self.connectorNodeObserverTagList:
        self.connectorNode.RemoveObserver(tag)
      self.connectorNodeObserverTagList = []

    self.connectorNode = self.linkInputSelector.currentNode()

    # Force initial update
    if self.connectorNode:
      self.onConfigFileQueried()
      if self.connectorNode.GetState() == slicer.vtkMRMLIGTLConnectorNode.STATE_CONNECTED:
        self.onConnectorNodeConnected(None, None, True)
      else:
        self.onConnectorNodeDisconnected(None, None, True)

    # Add observers for connect/disconnect events
      events = [ [slicer.vtkMRMLIGTLConnectorNode.ConnectedEvent, self.onConnectorNodeConnected], [slicer.vtkMRMLIGTLConnectorNode.DisconnectedEvent, self.onConnectorNodeDisconnected]]
      for tagEventHandler in events:
        connectorNodeObserverTag = self.connectorNode.AddObserver(tagEventHandler[0], tagEventHandler[1])
        self.connectorNodeObserverTagList.append(connectorNodeObserverTag)

  def onConnectorNodeConnected(self, caller, event, force=False):
    # Multiple notifications may be sent when connecting/disconnecting,
    # so we just if we know about the state change already
    if self.connectorNodeConnected and not force:
        return
    self.connectorNodeConnected = True
    print("Server connected!")
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
    print("Server disconnected!")
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
      self.startStopRecordingButton.setText("Stop Recording")
      self.startStopRecordingButton.setIcon(self.stopRecordingIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, stop recording" )
      self.onStartRecording()
      self.onDrawModelChecked()
    else:
      self.startStopRecordingButton.setText("Start Recording")
      self.startStopRecordingButton.setIcon(self.startRecordingIcon)
      self.startStopRecordingButton.setToolTip( "If clicked, start recording" )
      self.onStopRecording()

  def onScoutSettingsButtonClicked(self, status):
    self.outputVolumeSpacingBox.visible = self.scoutSettingsButton.checked
    self.volumeToReconstructSelector.visible = self.scoutSettingsButton.checked
    self.outputVolumeSpacingLabel.visible = self.scoutSettingsButton.checked
    self.volumeToReconstructLabel.visible = self.scoutSettingsButton.checked
    self.drawModelLabel.visible = self.scoutSettingsButton.checked
    self.drawModelBox.visible = self.scoutSettingsButton.checked

  def onRecordAndReconstructButtonClicked(self):
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Scout scan in progress")
    if self.recordAndReconstructButton.isChecked():
      self.recordAndReconstructButton.setText(" Scout Scan\n Stop Recording and Recontruct Recorded Volume")
      self.recordAndReconstructButton.setIcon(self.stopRecordingIcon)
      self.recordAndReconstructButton.setToolTip( "If clicked, stop recording and reconstruct recorded volume" )
      self.outputSpacingLiveReconstructionBox.value = self.outputVolumeSpacingBox.value
      self.onStartRecording()
    else:
      self.recordAndReconstructButton.setText(" Scout Scan\n Start Recording")
      self.recordAndReconstructButton.setIcon(self.startRecordingIcon)
      self.recordAndReconstructButton.setToolTip( "If clicked, start recording" )
      self.onStopScoutRecording()

  def onDrawModelChecked(self):
    if self.recordAndReconstructButton.isChecked():
      if self.drawModelBox.isChecked():
        print("draw model")
        imageSliceNode = slicer.mrmlScene.GetNodesByName('RecVol_Reference')
       #imageSliceNode = slicer.vtkMRMLScalarVolumeDisplayNode.SafeDownCast(imageSliceNode.GetItemAsObject(0))
       #imageSliceNode = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(imageSliceNode.GetItemAsObject(0))
        print imageSliceNode
#         imageSliceVolumeNode = slicer.vtkMRMLDisplayableNode.SetAndObserveDisplayNodeID(imageSliceNode.GetID())

#         imageSliceVolumeNode = slicer.vtkMRMLStorableNode.SafeDownCast(imageSliceNode.GetItemAsObject(0))
#         imageSliceVolumeNode.updateScene(slicer.mrmlScene)
      else:
        print("hide model")

  def onLiveReconstructionSettingsButtonClicked(self, status):
    self.outputExtentLiveReconstructionLabel.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection1.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection2.visible = self.liveReconstructionSettingsButton.checked
    self.outputExtentROIBoxDirection3.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsLabel.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsBox.visible = self.liveReconstructionSettingsButton.checked
    self.snapshotsTimeLabel.visible = self.liveReconstructionSettingsButton.checked

  def onLiveReconstructionButtonClicked(self):
    snapshotIntervalSec = (self.snapshotsBox.value)
    self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.liveReconstructStatus.setToolTip("Live Reconstruction in progress")

    if self.snapshotsBox.value != 0:
      self.snapshotTimer = qt.QTimer()
      self.snapshotTimer.timeout.connect(self.onSnapshotQuiered)
      self.snapshotTimer.start(snapshotIntervalSec*1000)

    if self.liveReconstructionButton.isChecked():
      if self.roiNode:
        self.updateVolumeExtentFromROI()
      self.liveReconstructionButton.setText("Stop Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.stopLiveReconstructionIcon)
      self.liveReconstructionButton.setToolTip( "If clicked, stop live reconstruction" )
      self.onStartReconstruction()
    else:
      self.liveReconstructionButton.setText("Start Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.startLiveReconstructionIcon)
      self.liveReconstructionButton.setToolTip( "If clicked, start live reconstruction" )
      self.onStopReconstruction()

  def onDisplayRoiButtonClicked(self):
    if self.displayRoiButton.isChecked():
      self.displayRoiButton.setIcon(self.visibleRoiIcon)
      self.displayRoiButton.setToolTip("If clicked, hide ROI")
      self.roiNode.SetDisplayVisibility(1)
    else:
      self.displayRoiButton.setIcon(self.notVisibleRoiIcon)
      self.displayRoiButton.setToolTip("If clicked, display ROI")
      self.roiNode.SetDisplayVisibility(0)

  def onConfigFileQueried(self):
    import os.path
    self.currentDeviceConfigFileName.clear()

    plusConfigFilePath = os.path.expanduser('~/PlusApp-2.1.2.3392-Win64/bin/PlusConfig.xml')
    plusConfigFile = open(plusConfigFilePath)
    linesArray = plusConfigFile.readlines()
    for i in range(0, len(linesArray)):
      if 'LastDeviceSetConfigurationFileName' in linesArray[i]:
        linesArray[i] = linesArray[i].replace('" />','')
        linesArray[i] = os.path.basename(linesArray[i])
        self.currentDeviceConfigFileName.setText(linesArray[i])
    plusConfigFile.close()

  def onStartRecording(self):
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.fileNameBox.text, self.onGenericCommandResponseReceived)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStopScoutRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onScoutVolumeRecorded)

  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputSpacingValue, self.outputOriginValue, self.outputExtentValue, self.onGenericCommandResponseReceived)

  def onStopReconstruction(self):
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.onVolumeLiveReconstructed)

  def onReconstVolume(self):
    self.offlineReconstructButton.setIcon(self.offlineReconstructWaitIcon)
    self.offlineReconstructButton.setText("Offline Reconstruction in progress ...")
    self.offlineReconstructButton.setEnabled(False)
    self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.offlineReconstructStatus.setToolTip("Offline reconstruction in progress")
    outputSpacing = [self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value, self.offlineVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.offlineVolumeToReconstructSelector.currentText, outputSpacing, self.onVolumeReconstructed, self.fileNameBox.text, self.outpuVolumeDeviceBox.text)

  def onScoutScanReconstVolume(self):
    self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
    self.recordAndReconstructStatus.setToolTip("Recording completed. Reconstruction in progress")
    outputSpacing = [self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value, self.outputVolumeSpacingBox.value]
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.volumeToReconstructSelector.text, outputSpacing, self.onScoutVolumeReconstructed, "scoutFile.mha", "ScoutScan")

  def onSnapshotQuiered(self, stopFlag = ""):
    if self.liveReconstructionButton.isChecked():
      print("Snapshot")
      self.liveReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.liveReconstructStatus.setToolTip("Snapshot")
      self.logic.getVolumeReconstructionSnapshot(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.fileNameBox.text, self.onGenericCommandResponseReceived)
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
    import os.path
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
    import os.path
    self.onGenericCommandResponseReceived(commandId,textNode)
    self.offlineReconstructButton.setEnabled(True)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)

    self.volumeToReconstructSelector.clear()
    self.volumeToReconstructSelector.insert(volumeToReconstructFileName)
    if commandResponseElement.GetAttribute("Status") == "SUCCESS":
      self.onScoutScanReconstVolume()

  def onVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    offlineReconstMessage = commandResponseElement.GetAttribute("Message")

    self.offlineReconstructButton.setIcon(self.offlineReconstructStartIcon)
    self.offlineReconstructButton.setText("Offline Reconstruction")
    self.offlineReconstructButton.setEnabled(True)
    self.offlineReconstructButton.setChecked(False)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.offlineReconstructStatus.setToolTip(commandResponseElement.GetAttribute("Message"))
    else:
      self.offlineReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.offlineReconstructStatus.setToolTip(offlineReconstMessage)

  def onScoutVolumeReconstructed(self, commandId, textNode):
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    scoutScanMessage = commandResponseElement.GetAttribute("Message")
    scoutScanReconstructFileName = os.path.basename(scoutScanMessage)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS"):
      self.onRoiInitialization()
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Information)
      self.recordAndReconstructStatus.setToolTip("Scout scan completed, file saved as "+ scoutScanReconstructFileName)
    else:
      self.recordAndReconstructStatus.setIcon(qt.QMessageBox.Critical)
      self.recordAndReconstructStatus.setToolTip(scoutScanMessage)

    scoutScanVolumeNode = slicer.util.getNode('ScoutScan')
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    #selectionNode.SetReferenceSecondaryVolumeID(writtenNode.GetID())
    selectionNode.SetReferenceActiveVolumeID(scoutScanVolumeNode.GetID())
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.PropagateVolumeSelection(0)
    applicationLogic.FitSliceToAll()

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

#       IJKToRASMatrix = vtk.vtkMatrix4x4()
#       IJKToRASMatrix = reconstructedVolumeNode.GetIJKToRASMatrix(IJKToRASMatrix)
#       print IJKToRASMatrix

#       volumeVoxelsNumberList =
#       for line in range(0,2):
#           for row in range(0,2):
#           roiOriginRAS[line] = IJKToRASMatrix[line][row]*(volumeVoxelsNumberList[line]/2)

#       volumeVoxelsNumberList =
#       roiOriginRAS = reconstructedVolumeNode.GetIJKToRASMatrix().MultiplyPoint(volumeVoxelsNumberList + (1,))
#       print roiOriginRAS[:3]

  def updateVolumeExtentFromROI(self):
    roiCenter = [0.0, 0.0, 0.0]
    roiRadius = [0.0, 0.0, 0.0]
    roiOrigin = [0.0, 0.0, 0.0]
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

  def onReload(self,moduleName="PlusRemote"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer
    import vtk, qt, ctk
    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent().parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)

    # delete the old widget instance
    if hasattr(globals()['slicer'].modules, widgetName):
      getattr(globals()['slicer'].modules, widgetName).cleanup()

    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()
    setattr(globals()['slicer'].modules, widgetName, globals()[widgetName.lower()])

#
# PlusRemoteLogic
#
class PlusRemoteLogic:
  def __init__(self):
    self.commandToMethodHashtable = {}
    self.defaultCommandTimeoutSec = 30
    self.timerIntervalSec = 0.1
    self.timer = qt.QTimer()
    self.timer.timeout.connect(self.getCommandReply)
    pass

  def executeCommand(self, connectorNodeId, commandName, commandParameters, responseCallback):
      commandId = slicer.modules.openigtlinkremote.logic().ExecuteCommand(connectorNodeId, commandName, commandParameters)
      #print "Execute command: " + str(commandId) + " " + commandName
      self.commandToMethodHashtable[commandId]={'responseCallback': responseCallback, 'connectorNodeId': connectorNodeId, 'remainingTime': self.defaultCommandTimeoutSec/self.timerIntervalSec}
      if not self.timer.isActive():
        self.timer.start(self.timerIntervalSec*1000)

  def getCommandReply(self):
    for commandId in self.commandToMethodHashtable.keys():
      #print "Waiting for ACK_" + str(commandId)
      replyNodes = slicer.mrmlScene.GetNodesByName( "ACK_" + str(commandId) )
      textNode = slicer.vtkMRMLAnnotationTextNode.SafeDownCast( replyNodes.GetItemAsObject(0) )
      remainingTime = self.commandToMethodHashtable[commandId]['remainingTime']
      remainingTime = remainingTime-1
      if textNode or remainingTime<=0:
#         if textNode:
#           print "Received ACK_" + str(commandId) + " - " + textNode.GetText(0)
#         else:
#           print "Timeout waiting for ACK_" + str(commandId)
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

  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacing, outputOrigin, outputExtent, method):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputSpacing=" + "\"" + "%f %f %f" % tuple(outputSpacing) + "\"" + " OutputOrigin=" + "\"" + "%f %f %f" % tuple(outputOrigin) + "\"" + " OutputExtent=" + "\"" + "%i %i %i %i %i %i" % tuple(outputExtent) +"\""
    self.executeCommand(connectorNodeId, "StartVolumeReconstruction", parameters, method)

  def stopVolumeReconstruction(self, connectorNodeId, volumeReconstructorDeviceId,method):
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId +"\""
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
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputFilename=" + "\"" + fileName +"\""
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
