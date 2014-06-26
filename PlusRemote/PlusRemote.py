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
    self.connectorNodeObserverTag = None
    self.onModuleEnteredTimer()
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
    self.volumeToReconstructSelector.setToolTip( "Pick volume to reconstruct" )
    reconstructionLayout.addRow("Volume to Reconstruct: ", self.volumeToReconstructSelector)
    
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
    
    self.testBox = qt.QPlainTextEdit()
    self.testBox.setReadOnly(True)
    replyLayout.addRow(self.testBox)
    
    #self.refreshButton = qt.QPushButton("Refresh")
    #self.refreshButton.name = "PlusRemote Refresh"
    #reloadFormLayout.addWidget(self.refreshButton)
    #self.refreshButton.connect('clicked()', self.connectorRefresh)
    #self.refreshButton.connect('clicked()', self.connectorRefreshingTimer)
    
    # connections
    
    #self.linkInputSelector.connect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeSelected)
    
    self.startRecordingButton.connect('clicked(bool)', self.onStartRecording)
    self.stopRecordingButton.connect('clicked(bool)', self.onStopRecording)
    
    self.startReconstructionButton.connect('clicked(bool)', self.onStartReconstruction)
    self.stopReconstructionButton.connect('clicked(bool)', self.onStopReconstruction)
    self.reconstructVolumeButton.connect('clicked(bool)', self.onReconstVolume)
    
    self.updateTransformButton.connect('clicked(bool)', self.onUpdateTransform)
    self.saveTransformButton.connect('clicked(bool)', self.onSaveTransform)
    
    self.layout.addStretch(1)
  
  
  def cleanup(self):
    pass

  def onModuleEnteredTimer(self):
    #Single shot timer for module initialization
    #self.initTimer = qt.QTimer()
    #self.initTimer.timeout.connect(self.onModuleEntered)
    ##self.initTimer.setSingleShot(true)
    #self.initTimer.start(1)
    qt.QTimer.singleShot(1, self.onConnectorNodeSelected)

  #def onModuleEntered(self):
  #  print("Use initTimer")
  #  self.initTimer.stop()
  #  self.onConnectorNodeSelected()

  def onConnectorNodeSelected(self):
    self.connectorNode = self.linkInputSelector.currentNode()
    if self.connectorNode and self.connectorNodeObserverTag:
      self.connectorNode.RemoveObserver(self.connectorNodeObserverTag)
    if self.connectorNode:
      #self.connectorNodeObserverTag = self.connectorNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.onConnectorChanged)
      self.connectorNodeObserverTag = self.connectorNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.fonctionTest)
      receiveEvent = 118948 #(see vtkMRMLIGTLConnectorNode.h)
      #self.connectorNodeObserverTag = self.connectorNode.AddObserver(receiveEvent, self.onConnectorChanged)
      print self.connectorNodeObserverTag
      self.connectorNodeObserverCommand = self.connectorNode.GetCommand(self.connectorNodeObserverTag)
      self.connectorNodeObserverCommand.AbortFlagOn = True
    #self.onConnectorChanged(self.connectorNode, vtk.vtkCommand.ModifiedEvent)
    self.volumeToReconstructListUpdate()
  
  #def onConnectorNodeSelected(self):
  #  self.connectorNode = self.linkInputSelector.currentNode()
  #  self.connectorNodeObserverTag = self.connectorNode.AddObserver(118948 , self.ProcessMRMLEvents)
  #  self.connectorNodeObserverTag = slicer.vtkObserverManager.SetAndObserveObjectEvents(self.linkInputSelector.currentNode(),ReceiveEvent)

  #def ProcessMRMLEvents(self,callerID,event,callDataID = None):
  #  ''' gets called, when an observed MRML event was fired '''
  #  ReceiveEvent = 118948
  #  #if event == ReceiveEvent:
  #  self.fonctionTest(self.connectorNodeObserverTag, ReceiveEvent)

  def fonctionTest(self, caller, event):
  #def fonctionTest(self):
    #self.connectorNode.RemoveObserver(self.connectorNodeObserverTag)
    print("Coucou!")
    #self.onConnectorNodeSelected()

  def onConnectorChanged(self, caller, event):
  #def onConnectorChanged(self):
    #print locals().keys()
    if (self.linkInputSelector.currentNode() is None) or (self.linkInputSelector.currentNode().GetState() != slicer.vtkMRMLIGTLConnectorNode.STATE_CONNECTED):
      self.startRecordingButton.setEnabled(False)
      self.stopRecordingButton.setEnabled(False)
      self.startReconstructionButton.setEnabled(False)
      self.stopReconstructionButton.setEnabled(False)
      self.captureIDSelector.clear()
      self.volumeReconstructorIDSelector.clear()
      self.replyBox.setPlainText("IGTLConnector not connected or missing")
    else:
      self.replyBox.setPlainText("IGTLConnector connected")
      self.logic.getCaptureDeviceIds(self.linkInputSelector.currentNode().GetID(), self.onGetCaptureDeviceCommandResponseReceived)
      self.logic.getVolumeReconstructorDeviceIds(self.linkInputSelector.currentNode().GetID(), self.onGetVolumeReconstructorDeviceCommandResponseReceived)

  def volumeToReconstructListUpdate(self):
    import glob
    filepath = glob.glob('C:/Users/meyer/PlusApp-2.1.2.3392-Win64/data/*.mha')
    i=0
    while i<len(filepath)-1:
      i+=1
      volumeToReconstructFile = filepath[i].lstrip('C:/Users/meyer/PlusApp-2.1.2.3392-Win64/data\\')
      self.volumeToReconstructSelector.addItem(volumeToReconstructFile)

  def onStartRecording(self):
    self.logic.startRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.fileNameBox.text, self.onGenericCommandResponseReceived)
    self.volumeToReconstructSelector.addItem(self.fileNameBox.text)
  
  def onStopRecording(self):
    self.logic.stopRecording(self.linkInputSelector.currentNode().GetID(), self.captureIDSelector.currentText, self.onGenericCommandResponseReceived)
  
  def onStartReconstruction(self):
    self.logic.startVolumeReconstuction(self.linkInputSelector.currentNode().GetID(), self.volumeReconstructorIDSelector.currentText, self.outputVolumeSpacingBox.value, self.onGenericCommandResponseReceived)
  
  def onStopReconstruction(self):
    self.logic.stopVolumeReconstruction(self.linkInputSelector.currentNode().GetID(), self.onGenericCommandResponseReceived)
  
  def onReconstVolume(self):
    self.logic.reconstructRecorded(self.linkInputSelector.currentNode().GetID(), self.volumeToReconstructSelector.currentText, self.volumeReconstructorIDSelector.currentText, self.onGenericCommandResponseReceived)

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
    logic = PlusRemoteLogic()
    commandResponse=textNode.GetText(0)
    logic.discardCommand(commandId, self.linkInputSelector.currentNode().GetID())
    
    commandResponseElement = vtk.vtkXMLUtilities.ReadElementFromString(commandResponse)
    captureDeviceIdsListString = commandResponseElement.GetAttribute("Message")
    
    captureDevicesIdsList = captureDeviceIdsListString.split(",")
    self.captureIDSelector.clear()
    self.captureIDSelector.addItems(captureDevicesIdsList)
    self.startRecordingButton.setEnabled(True)
    self.stopRecordingButton.setEnabled(True)
  
  def onGetVolumeReconstructorDeviceCommandResponseReceived(self, commandId, textNode):
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
      #self.replyBox.setPlainText("Waiting for reply...")
      #if not self.timer:
      self.timer = qt.QTimer()
      self.timer.timeout.connect(self.getCommandReply)
      self.timer.start(100)
  
  def getCommandReply(self):
    print self.commandToMethodHashtable.values()
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
        #self.replyBox.setPlainText("No reply: Timeout")
        connectorNodeId = commandToMethodItem['connectorNodeId']
        self.discardCommand(commandId, connectorNodeId)
        self.timer.stop() 
      else:
        self.commandToMethodHashtable[commandId]['remainingTime'] = remainingTime
        #self.timer.stop()    
    if not self.commandToMethodHashtable:
      self.timer.stop()
      self.timer = None
  
  def getCaptureDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualDiscCapture" +"\""
    #parameters = ""
    self.executeCommand(connectorNodeId,"RequestDeviceIds", parameters, method)
    #return commandCounter
  
  def getVolumeReconstructorDeviceIds(self, connectorNodeId, method):
    parameters = "DeviceType=" + "\"" + "VirtualVolumeReconstructor" +"\""
    #parameters = ""
    self.executeCommand(connectorNodeId, "RequestDeviceIds", parameters, method)
    #return commandCounter
  
  def startVolumeReconstuction(self, connectorNodeId, volumeReconstructorDeviceId, outputSpacingValue, method):
    parameters =  "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + "OutputSpacing=" + "\"" + str(outputSpacingValue) +"\""
    #parameters = ""
    self.executeCommand(connectorNodeId, "StartVolumeReconstruction", parameters, method)
  
  #def stopVolumeReconstruction(self, volumeReconstructorDeviceId, ouputVolDeviceName, ouputVolFileName): 
  def stopVolumeReconstruction(self, connectorNodeId, method):
    #parameters = "VolumeReconstructorDeviceID=" + "\"" + volumeReconstructorDeviceId + "OutputVolumeDeviceName" + "\"" + "OutputVolFileName" + "\"" + outputVolFileName +"\""
    parameters = ""
    self.executeCommand(connectorNodeId, "StopVolumeReconstruction", parameters, method)
  
  def reconstructRecorded(self, connectorNodeId, fileName, volumeReconstructorDeviceId, method):
    parameters = "VolumeReconstructorDeviceId=" + "\"" + volumeReconstructorDeviceId + "\"" + "InputSeqFilename=" + "\"" + fileName + "\"" + "OutputVolFilename=" + "\"" + "scoutFile.mha" +"\""
    self.executeCommand(connectorNodeId, "ReconstructVolume", parameters, method)
  
  def startRecording(self, connectorNodeId, captureName, fileName, method):
    parameters = "CaptureDeviceId=" + "\"" + captureName + "\"" + " OutputFilename=" + "\"" + fileName + "\""
    #print parameters
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

