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
    self.roiInitializationFlag = 0
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

    # Recording
    recordingCollapsibleButton = ctk.ctkCollapsibleButton()
    recordingCollapsibleButton.text = "Recording"
    self.layout.addWidget(recordingCollapsibleButton)
    recordingLayout = qt.QFormLayout(recordingCollapsibleButton)

    self.captureIDSelector = qt.QComboBox()
    self.captureIDSelector.setToolTip( "Pick capture device ID" )
    recordingLayout.addRow("Capture Device ID: ", self.captureIDSelector)

    self.fileNameBox = qt.QLineEdit()
    recordingLayout.addRow("Filename: ", self.fileNameBox)

    # Move to the same row, use grid layout

#     recordingControlsLayout = qt.QGridLayout()
#
#     self.startRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
#     self.stopRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
#     self.startStopRecordingButton = qt.QPushButton(" Scout Scan\n Start Recording")
#     self.startStopRecordingButton.setCheckable(True)
#     self.startStopRecordingButton.setIcon(self.startRecordingIcon)
#     self.startStopRecordingButton.setToolTip("If checked, start recording")
#     self.startStopRecordingButton.setEnabled(False)
#     recordingControlsLayout.addWidget(self.startStopRecordingButton, 0, 0)
#
#     self.scoutSettingsButton = ctk.ctkExpandButton()
#     recordingControlsLayout.addWidget(self.scoutSettingsButton, 0, 1)
#
#     self.testButton = qt.QPushButton("testButton")
#     recordingControlsLayout.addWidget(self.testButton, 0, 2)
#     self.testButton.visible = False
#
#     recordingLayout.addRow(recordingControlsLayout)

    # Scout scan (record and low resolution reconstruction)
    scoutScanCollapsibleButton = ctk.ctkCollapsibleButton()
    scoutScanCollapsibleButton.text = "Scout Scan (Record and Reconstruct)"
    self.layout.addWidget(scoutScanCollapsibleButton)
    scoutScanLayout = qt.QFormLayout(scoutScanCollapsibleButton)

    # Move to the same row, use grid layout
    scoutScanControlsLayout = qt.QGridLayout()
    scoutScanLayout.addRow(scoutScanControlsLayout)

    self.startRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.stopRecordingIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
    self.recordAndReconstructButton = qt.QPushButton(" Scout Scan\n Start Recording")
    self.recordAndReconstructButton.setCheckable(True)
    self.recordAndReconstructButton.setIcon(self.startRecordingIcon)
    self.recordAndReconstructButton.setToolTip("If checked, start recording")
    self.recordAndReconstructButton.setEnabled(False)
    scoutScanControlsLayout.addWidget(self.recordAndReconstructButton, 0, 0)

    self.scoutSettingsButton = ctk.ctkExpandButton()
    scoutScanControlsLayout.addWidget(self.scoutSettingsButton, 0, 1)

    self.outputVolumeSpacingBox = qt.QDoubleSpinBox()
    self.outputVolumeSpacingBox.setToolTip( "Set output volume spacing" )
    self.outputVolumeSpacingBox.value = 0.3
    self.outputVolumeSpacingBox.setSingleStep(0.1)
    #scoutScanLayout.addRow("Output Volume Spacing: ", self.outputVolumeSpacingBox)
    scoutScanControlsLayout.addWidget(self.outputVolumeSpacingBox, 0, 2)
    self.outputVolumeSpacingBox.visible = False

    self.otherBox = qt.QDoubleSpinBox()
    scoutScanControlsLayout.addWidget(self.otherBox, 1, 2)
    self.otherBox.visible = False


    self.volumeToReconstructSelector = qt.QComboBox()
    self.volumeToReconstructSelector.setEditable(True)
    self.volumeToReconstructSelector.setToolTip( "Pick/set volume to reconstruct" )
    scoutScanLayout.addRow("Volume to reconstruct:", self.volumeToReconstructSelector)

    self.reconstructVolumeButton = qt.QPushButton("Reconstruct Recorded Volume")
    scoutScanLayout.addRow(self.reconstructVolumeButton)

    # Reconstruction
    reconstructionCollapsibleButton = ctk.ctkCollapsibleButton()
    reconstructionCollapsibleButton.text = "Live Reconstruction"
    self.layout.addWidget(reconstructionCollapsibleButton)
    reconstructionLayout = qt.QFormLayout(reconstructionCollapsibleButton)

    self.volumeReconstructorIDSelector = qt.QComboBox()
    self.volumeReconstructorIDSelector.setToolTip( "Pick volume reconstructor device ID" )
    reconstructionLayout.addRow("Volume Reconstructor Device ID: ", self.volumeReconstructorIDSelector)

    self.outputSpacingROIBox = qt.QDoubleSpinBox()
    self.outputSpacingROIBox.setToolTip( "Set output volume spacing" )
    self.outputSpacingROIBox.value = 0.3
    self.outputSpacingROIBox.setSingleStep(0.1)
    reconstructionLayout.addRow("Output Volume Spacing: ", self.outputSpacingROIBox)

    liveReconstructionControlsLayout = qt.QGridLayout()
    reconstructionLayout.addRow(liveReconstructionControlsLayout)

    self.outputExtentROIBoxDirection1 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection1.setToolTip( "Set output extent or use displayed ROI" )
    self.outputExtentROIBoxDirection1.value = 0.3
    #reconstructionLayout.addRow("Output Volume Spacing: ", self.outputExtentROIBoxDirection1)
    liveReconstructionControlsLayout.addWidget(self.outputExtentROIBoxDirection1, 0, 0)

    self.outputExtentROIBoxDirection2 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection2.setToolTip( "Set output extent or use displayed ROI" )
    self.outputExtentROIBoxDirection2.value = 0.3
    liveReconstructionControlsLayout.addWidget(self.outputExtentROIBoxDirection2, 0, 1)

    self.outputExtentROIBoxDirection3 = qt.QDoubleSpinBox()
    self.outputExtentROIBoxDirection3.setToolTip( "Set output extent or use displayed ROI" )
    self.outputExtentROIBoxDirection3.value = 0.3
    liveReconstructionControlsLayout.addWidget(self.outputExtentROIBoxDirection3, 0, 2)

#     self.liveReconstructionExtentLabel = qt.QLabel()
#     self.liveReconstructionExtentLabel.setText("Output Volume Extent:")

    self.snapshotsBox = qt.QLineEdit()
    self.snapshotsBox.setToolTip( "Takes snapshots every ... seconds" )
    reconstructionLayout.addRow("Snapshots: ", self.snapshotsBox)

    self.startLiveReconstructionIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Record.png')
    self.stopLiveReconstructionIcon = qt.QIcon(self.plusRemoteModuleDirectoryPath+'/Resources/Icons/icon_Stop.png')
    self.liveReconstructionButton = qt.QPushButton("Start Live Reconstruction")
    self.liveReconstructionButton.setCheckable(True)
    self.liveReconstructionButton.setIcon(self.startLiveReconstructionIcon)
    self.liveReconstructionButton.setToolTip("If checked, start live reconstruction")
    self.liveReconstructionButton.setEnabled(False)
    reconstructionLayout.addRow(self.liveReconstructionButton)

    self.notVisibleRoiIcon = qt.QIcon(":Icons\VisibleOff.png")
    self.visibleRoiIcon = qt.QIcon(":Icons\VisibleOn.png")
    self.displayRoiButton = qt.QToolButton()
    self.displayRoiButton.setCheckable(True)
    self.displayRoiButton.setIcon(self.notVisibleRoiIcon)
    self.displayRoiButton.setToolTip("If checked, display ROI")
    reconstructionLayout.addRow("Display ROI: ", self.displayRoiButton)

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
    transformUpdateLayout.addRow("Current Device Set Config File Name: ", self.currentDeviceConfigFileName)

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

    self.recordAndReconstructButton.connect('clicked(bool)', self.onRecordAndReconstructButtonClicked)
    self.liveReconstructionButton.connect('clicked(bool)', self.onLiveReconstructionButtonClicked)

    self.displayRoiButton.connect('clicked(bool)', self.onDisplayRoiButtonClicked)
    self.scoutSettingsButton.connect('clicked(bool)', self.onScoutSettingsButtonClicked)

    self.reconstructVolumeButton.connect('clicked(bool)', self.onReconstVolume)

    self.updateTransformButton.connect('clicked(bool)', self.onUpdateTransform)
    self.saveTransformButton.connect('clicked(bool)', self.onSaveTransform)

    self.snapshotsBox.connect('textEdited(text)', self.onSnapshotQuiered)

    self.layout.addStretch(1)

    self.onConnectorNodeSelected()

  def cleanup(self):
    print "Cleanup is called"
    pass

  def onScoutSettingsButtonClicked(self, status):
    self.outputVolumeSpacingBox.visible = self.scoutSettingsButton.checked
    self.otherBox.visible = self.scoutSettingsButton.checked

  def onRecordAndReconstructButtonClicked(self):
    if self.recordAndReconstructButton.isChecked():
      self.recordAndReconstructButton.setText(" Scout Scan\n Stop Recording and Recontruct Recorded Volume")
      self.recordAndReconstructButton.setIcon(self.stopRecordingIcon)
      self.recordAndReconstructButton.setToolTip( "If checked, stop recording and reconstruct recorded volume" )
      self.onStartRecording()
    else:
      self.recordAndReconstructButton.setText(" Scout Scan\n Start Recording")
      self.recordAndReconstructButton.setIcon(self.startRecordingIcon)
      self.recordAndReconstructButton.setToolTip( "If checked, start recording" )
      self.onStopRecording()

  def onLiveReconstructionButtonClicked(self):
    if self.liveReconstructionButton.isChecked():
      if self.roiNode:
        self.updateVolumeExtentFromROI()
      self.liveReconstructionButton.setText("Stop Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.stopLiveReconstructionIcon)
      self.liveReconstructionButton.setToolTip( "If checked, stop live reconstruction" )
      self.onStartReconstruction()
    else:
      self.liveReconstructionButton.setText("Start Live Reconstruction")
      self.liveReconstructionButton.setIcon(self.startLiveReconstructionIcon)
      self.liveReconstructionButton.setToolTip( "If checked, start live reconstruction" )
      self.onStopReconstruction()

  def onDisplayRoiButtonClicked(self):
    if self.displayRoiButton.isChecked():
      self.displayRoiButton.setIcon(self.visibleRoiIcon)
      self.displayRoiButton.setToolTip("If checked, hide ROI")
      self.roiNode.SetDisplayVisibility(1)
    else:
      self.displayRoiButton.setIcon(self.notVisibleRoiIcon)
      self.displayRoiButton.setToolTip("If checked, display ROI")
      self.roiNode.SetDisplayVisibility(0)

  def onConnectorNodeSelected(self):
    if self.connectorNode and self.connectorNodeObserverTagList:
      for tag in self.connectorNodeObserverTagList:
        self.connectorNode.RemoveObserver(tag)
      self.connectorNodeObserverTagList = []

    self.connectorNode = self.linkInputSelector.currentNode()

    # Force initial update
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
    self.startReconstructionButton.setEnabled(False)
    self.stopReconstructionButton.setEnabled(False)
    self.startStopRecordingButton.setEnabled(False)
    self.captureIDSelector.setDisabled(True)
    self.volumeReconstructorIDSelector.setDisabled(True)

#   def onConfigFileQueried(self):
#     #GetNewDeviceSetConfigurationFileName () vtk PlusConfig
#     import glob, os.path
#     plusConfigFilePath = os.path.expanduser('~/PlusApp-2.1.2.3392-Win64/bin/PlusConfig.xml')
#     plusConfigFile = open(plusConfigFilePath)
#     linesArray = plusConfigFile.readlines()
#     i=0
#     while i < len(linesArray):
#       if 'LastDeviceSetConfigurationFileName' in linesArray[i]:
#         linesArray[i] = linesArray[i].replace('LastDeviceSetConfigurationFileName=','')
#         linesArray[i] = linesArray[i].replace('"','')
#         linesArray[i] = linesArray[i].replace('/>','')
#         linesArray[i] = os.path.basename(linesArray[i])
#         self.currentDeviceConfigFileName.setText(linesArray[i])
#       i+=1
#     plusConfigFile.close()

  def onStartRecording(self):
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.fileNameBox.text, self.onGenericCommandResponseReceived)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputSpacingValue, self.outputOriginValue, self.outputExtentValue, self.onGenericCommandResponseReceived)

  def onStopReconstruction(self):
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.onGenericCommandResponseReceived)

  def onReconstVolume(self):
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.volumeToReconstructSelector.currentText, self.onVolumeReconstructed)

  def onSnapshotQuiered(self):
    self.logic.getVolumeReconstructionSnapshot(self.linkInputSelector.currentNode().GetID(), self.volumeToReconstructSelector.currentText, self.onGenericCommandResponseReceived)

  def onUpdateTransform(self):
    self.logic.updateTransform(self.linkInputSelector.currentNode().GetID(), self.transformUpdateInputSelector.currentNode(), self.onGenericCommandResponseReceived)

  def onSaveTransform(self):
    self.logic.saveTransform(self.linkInputSelector.currentNode().GetID(), self.onGenericCommandResponseReceived)

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

    i=0
    for i in range(0,len(captureDevicesIdsList)):
      if self.captureIDSelector.findText(captureDevicesIdsList[i]) == -1:
        self.captureIDSelector.addItem(captureDevicesIdsList[i])
    self.recordAndReconstructButton.setEnabled(True)

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
    self.liveReconstructionButton.setEnabled(True)

  def onVolumeRecorded(self, commandId, textNode):
    import os.path, time
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)
    self.volumeToReconstructSelector.insertItem(0,volumeToReconstructFileName)
    self.volumeToReconstructSelector.setCurrentIndex(0)

    if commandResponseElement.GetAttribute("Status") == "SUCCESS":
      #time.sleep(5) #let Plus Server time to save the file
      self.onReconstVolume()

  def onVolumeReconstructed(self, commandId, textNode):
    print("onVolumeReconstructed")
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)

    if (commandResponseElement.GetAttribute("Status") == "SUCCESS") and not self.roiNode:
      print("success reconstruction")
      self.onRoiInitialization()


  def onRoiInitialization(self):
    print("onRoiInitialization")
    reconstructedNode = slicer.mrmlScene.GetNodesByName('RecVol_Reference')
    reconstructedVolumeNode = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(reconstructedNode.GetItemAsObject(0))

    roiCenterInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    roiRadiusInit = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    bounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

    if self.roiInitializationFlag == 0 and reconstructedVolumeNode:
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
      self.roiInitializationFlag = 1

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
    self.outputExtentValue = [0, int((2*roiRadius[0])/self.outputSpacingROIBox.value), 0, int((2*roiRadius[1])/self.outputSpacingROIBox.value), 0, int((2*roiRadius[2])/self.outputSpacingROIBox.value)]
    self.outputSpacingValue = [self.outputSpacingROIBox.value, self.outputSpacingROIBox.value, self.outputSpacingROIBox.value]
    print self.outputSpacingValue
    print self.outputOriginValue
    print self.outputExtentValue

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
    pass

  def executeCommand(self, connectorNodeId, commandName, commandParameters, responseCallback):
      commandId = slicer.modules.openigtlinkremote.logic().ExecuteCommand(connectorNodeId, commandName, commandParameters)
      self.commandToMethodHashtable[commandId]={'responseCallback': responseCallback, 'connectorNodeId': connectorNodeId, 'remainingTime': 50}
      #if not self.timer:
      self.timer = qt.QTimer()
      self.timer.timeout.connect(self.getCommandReply)
      self.timer.start(100)

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
        print responseCallback
        connectorNodeId = commandToMethodItem['connectorNodeId']
        self.discardCommand(commandId, connectorNodeId)
        self.timer.stop()
      else:
        self.commandToMethodHashtable[commandId]['remainingTime'] = remainingTime
    if not self.commandToMethodHashtable:
      self.timer.stop()
      self.timer = None

  def getCaptureDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualDiscCapture" +"\""
    self.executeCommand(connectorNodeId,"RequestDeviceIds", parameters, method)

  def getVolumeReconstructorDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualVolumeReconstructor" +"\""
    self.executeCommand(connectorNodeId, "RequestDeviceIds", parameters, method)

  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacing, outputOrigin, outputExtent, method):
    #parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputSpacing=\"%f %f %f\"" % tuple(outputSpacing) + "\"" + " OutputOrigin=\"%f %f %f\"" % tuple(outputOrigin) + "\"" + " OutputExtent=\"%i %i %i %i %i %i\"" % tuple(outputExtent) +"\""
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " OutputSpacing=" + "\"" + "%f %f %f" % tuple(outputSpacing) + "\"" + " OutputOrigin=" + "\"" + "%f %f %f" % tuple(outputOrigin) + "\"" + " OutputExtent=" + "\"" + "%i %i %i %i %i %i" % tuple(outputExtent) +"\""
    self.executeCommand(connectorNodeId, "StartVolumeReconstruction", parameters, method)

  def stopVolumeReconstruction(self, connectorNodeId, volumeReconstructorDeviceId,method):
    parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId +"\""
    self.executeCommand(connectorNodeId, "StopVolumeReconstruction", parameters, method)

  def reconstructRecorded(self, connectorNodeId, volumeReconstructorDeviceId, volumeToReconstructId, method):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " InputSeqFilename=" + "\"" + volumeToReconstructId + "\"" + " OutputVolFilename=" + "\"" + "scoutFile.mha" +"\""
    self.executeCommand(connectorNodeId, "ReconstructVolume", parameters, method)

  def startRecording(self, connectorNodeId, captureName, fileName, method):
    parameters = "CaptureDeviceId=" + "\"" + captureName + "\"" + " OutputFilename=" + "\"" + fileName + "\""
    self.executeCommand(connectorNodeId, "StartRecording", parameters, method)

  def stopRecording(self, connectorNodeId, captureName, method):
    self.executeCommand(connectorNodeId, "StopRecording", "CaptureDeviceId=" + "\"" + captureName + "\"", method)

  def getVolumeReconstructionSnapshot(self, connectorNodeId, fileName, method):
    parameters = "OutputFilename=" + "\"" + fileName + "\""
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

