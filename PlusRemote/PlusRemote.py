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
    self.eventToMethodHashtable = {}
    self.connectorNode = None
    self.connectorNodeObserverTagList = []
    self.connectorNodeConnected = False
    if not parent:
      self.setup()
      self.parent.show()

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

    recordingControlsLayout = qt.QGridLayout()
    self.startRecordingButton = qt.QPushButton("Start Recording")
    self.startRecordingButton.setEnabled(False)
    recordingControlsLayout.addWidget(self.startRecordingButton,0,0)
    self.stopRecordingButton = qt.QPushButton("Stop Recording")
    self.stopRecordingButton.setEnabled(False)
    recordingControlsLayout.addWidget(self.stopRecordingButton,0,1)
    recordingLayout.addRow(recordingControlsLayout)

    self.startStopRecordingButton = qt.QPushButton("Start/Stop Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setEnabled(False)
    self.startStopRecordingButton.setToolTip( "Start/Stop Recording" )
    recordingLayout.addRow(self.startStopRecordingButton)

    # Reconstruction
    reconstructionCollapsibleButton = ctk.ctkCollapsibleButton()
    reconstructionCollapsibleButton.text = "Reconstruction"
    self.layout.addWidget(reconstructionCollapsibleButton)
    reconstructionLayout = qt.QFormLayout(reconstructionCollapsibleButton)

    self.volumeReconstructorIDSelector = qt.QComboBox()
    self.volumeReconstructorIDSelector.setToolTip( "Pick volume reconstructor device ID" )
    reconstructionLayout.addRow("Volume Reconstructor Device ID: ", self.volumeReconstructorIDSelector)

    self.outputVolumeSpacingBox = qt.QDoubleSpinBox()
    self.outputVolumeSpacingBox.setToolTip( "Set output volume spacing" )
    self.outputVolumeSpacingBox.value = 1.0
    reconstructionLayout.addRow("Output Volume Spacing: ", self.outputVolumeSpacingBox)

    liveReconstructionLayout = qt.QGridLayout()
    self.startReconstructionButton = qt.QPushButton("Start Live Reconstruction")
    self.startReconstructionButton.setEnabled(False)
    liveReconstructionLayout.addWidget(self.startReconstructionButton,0,0)
    self.stopReconstructionButton = qt.QPushButton("Stop Live Reconstruction")
    self.stopReconstructionButton.setEnabled(False)
    liveReconstructionLayout.addWidget(self.stopReconstructionButton,0,1)
    reconstructionLayout.addRow(liveReconstructionLayout)

    self.volumeToReconstructSelector = qt.QComboBox()
    self.volumeToReconstructSelector.setEditable(True)
    self.volumeToReconstructSelector.setToolTip( "Pick/set volume to reconstruct" )
    reconstructionLayout.addRow("Volume to Reconstruct: ", self.volumeToReconstructSelector)

    self.displayRoiButton = qt.QToolButton()
    notVisibleRoiIcon = qt.QIcon("C:\Users\meyer\Documents\GitHub\Slicer-3.6-ITKv4\Libs\qMRMLWidgets\Resources\Icons\VisibleOff.png")
    self.displayRoiButton.setIcon(notVisibleRoiIcon)
    self.displayRoiButton.setToolTip("If checked, display ROI")
    reconstructionLayout.addRow("Display ROI: ", self.displayRoiButton)

    self.reconstructVolumeButton = qt.QPushButton("Reconstruct Recorded Volume")
    reconstructionLayout.addRow(self.reconstructVolumeButton)

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

    self.startRecordingButton.connect('clicked(bool)', self.onStartRecording)
    self.stopRecordingButton.connect('clicked(bool)', self.onStopRecording)

    self.startStopRecordingButton.connect('toggled(bool)', self.onStartStopButtonClicked)

    self.startReconstructionButton.connect('clicked(bool)', self.onStartReconstruction)
    self.stopReconstructionButton.connect('clicked(bool)', self.onStopReconstruction)
    self.reconstructVolumeButton.connect('clicked(bool)', self.onReconstVolume)

    self.updateTransformButton.connect('clicked(bool)', self.onUpdateTransform)
    self.saveTransformButton.connect('clicked(bool)', self.onSaveTransform)

    self.layout.addStretch(1)

    self.onConnectorNodeSelected()

  def cleanup(self):
    print "Cleanup is called"
    pass

  def onStartStopButtonClicked(self):
    notVisibleRoiIcon = qt.QIcon("C:\Users\meyer\Documents\GitHub\Slicer-3.6-ITKv4\Libs\qMRMLWidgets\Resources\Icons\VisibleOff.png")
    visibleRoiIcon = qt.QIcon("C:\Users\meyer\Documents\GitHub\Slicer-3.6-ITKv4\Libs\qMRMLWidgets\Resources\Icons\VisibleOn.png")
    #if stateChanged == 3:
    if self.startStopRecordingButton.isChecked() is True:
      self.displayRoiButton.setIcon(visibleRoiIcon)
      self.displayRoiButton.setToolTip("If checked, hide ROI")
      self.onStartRecording()
    else:
      self.displayRoiButton.setIcon(notVisibleRoiIcon)
      self.displayRoiButton.setToolTip("If checked, display ROI")
      self.onStopRecording()

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

    #self.onConfigFileQueried()

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
    self.startRecordingButton.setEnabled(False)
    self.stopRecordingButton.setEnabled(False)
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
#     self.volumeToReconstructSelector.insertItem(0,self.fileNameBox.text)
#     self.volumeToReconstructSelector.setCurrentIndex(0)

  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onVolumeRecorded)

  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputVolumeSpacingBox.value, self.onGenericCommandResponseReceived)

  def onStopReconstruction(self):
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.onGenericCommandResponseReceived)

  def onReconstVolume(self):
    #if self.linkInputSelector.currentNode() is None:
    #  self.logic.reconstructRecorded(None, self.volumeToReconstructSelector.currentText, self.volumeReconstructorIDSelector.currentText, self.onGenericCommandResponseReceived)
    #else:
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.volumeToReconstructSelector.currentText, self.onGenericCommandResponseReceived)
    #self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeToReconstructSelector.currentText, self.onGenericCommandResponseReceived)

  def onUpdateTransform(self):
    self.logic.updateTransform(self.linkInputSelector.currentNode().GetID(), self.transformUpdateInputSelector.currentNode(), self.onGenericCommandResponseReceived)

  def onSaveTransform(self):
    self.logic.saveTransform(self.linkInputSelector.currentNode().GetID(), self.onGenericCommandResponseReceived)

  def onGenericCommandResponseReceived(self, commandId, responseNode):
    if responseNode:
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
    self.startRecordingButton.setEnabled(True)
    self.stopRecordingButton.setEnabled(True)
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
    self.startReconstructionButton.setEnabled(True)
    self.stopReconstructionButton.setEnabled(True)


  def onVolumeRecorded(self, commandId, textNode):
    import os.path
    self.onGenericCommandResponseReceived(commandId,textNode)

    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)

    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    stopRecordingMessage = commandResponseElement.GetAttribute("Message")
    volumeToReconstructFileName = os.path.basename(stopRecordingMessage)
    self.volumeToReconstructSelector.insertItem(0,volumeToReconstructFileName)
    self.volumeToReconstructSelector.setCurrentIndex(0)


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
      #print self.commandToMethodHashtable[commandId]
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
    #parameters = ""
    self.executeCommand(connectorNodeId,"RequestDeviceIds", parameters, method)

  def getVolumeReconstructorDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualVolumeReconstructor" +"\""
    #parameters = ""
    self.executeCommand(connectorNodeId, "RequestDeviceIds", parameters, method)

  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacingValue, method):
    parameters =  "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + "OutputSpacing=" + "\"" + str(outputSpacingValue) +"\""
    #parameters = ""
    self.executeCommand(connectorNodeId, "StartVolumeReconstruction", parameters, method)

  #def stopVolumeReconstruction(self, volumeReconstructorDeviceId, ouputVolDeviceName, ouputVolFileName):
  def stopVolumeReconstruction(self, connectorNodeId, method):
    #parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "OutputVolumeDeviceName" + "\"" + "OutputVolFileName" + "\"" + outputVolFileName +"\""
    parameters = ""
    self.executeCommand(connectorNodeId, "StopVolumeReconstruction", parameters, method)

  def reconstructRecorded(self, connectorNodeId, volumeReconstructorDeviceId, volumeToReconstructId, method):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + " InputSeqFilename=" + "\"" + volumeToReconstructId + "\"" + " OutputVolFilename=" + "\"" + "scoutFile.mha" +"\""
    self.executeCommand(connectorNodeId, "ReconstructVolume", parameters, method)

  def startRecording(self, connectorNodeId, captureName, fileName, method):
    parameters = "CaptureDeviceId=" + "\"" + captureName + "\"" + " OutputFilename=" + "\"" + fileName + "\""
    self.executeCommand(connectorNodeId, "StartRecording", parameters, method)

  def stopRecording(self, connectorNodeId, captureName, method):
    self.executeCommand(connectorNodeId, "StopRecording", "CaptureDeviceId=" + "\"" + captureName + "\"", method)

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

