import os
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time


def setButtonStyle(button, textScale = 1.0): #TODO obsolete
    style = """
    {0}         {{border-style: outset; border-width: 2px; border-radius: 10px; background-color: #C7DDF5; border-color: #9ACEFF; font-size: {1}pt; height: {2}px}}
    {0}:pressed  {{border-style: outset; border-width: 2px; border-radius: 10px; background-color: #9ACEFF; border-color: #9ACEFF; font-size: {1}pt; height: {2}px}}
    {0}:checked {{border-style: outset; border-width: 2px; border-radius: 10px; background-color: #669ACC; border-color: #9ACEFF; font-size: {1}pt; height: {2}px}}
    """.format(button.className(), 12*textScale, qt.QDesktopWidget().screenGeometry().height()*0.05)
    
    button.setStyleSheet(style)

#
# GuideletLoadable
#

class GuideletLoadable(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Guidelet"
    self.parent.categories = ["Guidelet"] 
    self.parent.dependencies = []
    self.parent.contributors = [""]


#
# GuideletWidget
#

class GuideletWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)
    self.guideletInstance = None
    self.guideletLogic = self.createGuideletLogic()

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    
    # Launcher panel
    launcherCollapsibleButton = ctk.ctkCollapsibleButton()
    launcherCollapsibleButton.text = "Guidelet launcher"
    self.layout.addWidget(launcherCollapsibleButton)
    self.launcherFormLayout = qt.QFormLayout(launcherCollapsibleButton)

    self.addLauncherWidgets()  

    # Show guidelet button
    self.launchGuideletButton = qt.QPushButton("Start "+self.moduleName)
    self.launchGuideletButton.toolTip = "Launch the " + self.moduleName + " application in full screen"
    self.launcherFormLayout.addWidget(self.launchGuideletButton)
    self.launchGuideletButton.connect('clicked()', self.onLaunchGuideletButtonClicked)

    # Add vertical spacer
    self.layout.addStretch(1)

  def addLauncherWidgets(self):
    lnNode = slicer.util.getNode(self.moduleName)

    # PlusServer
    try:
      slicer.modules.plusremote
    except:
      self.errorLabel = qt.QLabel("Error: Could not find Plus Remote module. Please install the SlicerIGT extension.")
      self.layout.addWidget(self.errorLabel)
      return

    self.lineEdit = qt.QLineEdit()
    leLabel = qt.QLabel()
    leLabel.setText("Set the Plus Server Host and Name Port:")
    hbox = qt.QHBoxLayout()
    hbox.addWidget(leLabel)
    hbox.addWidget(self.lineEdit)
    self.launcherFormLayout.addRow(hbox)

    if lnNode is not None and lnNode.GetParameter('PlusServerHostNamePort'):
        #logging.debug("There is already a connector PlusServerHostNamePort parameter " + lnNode.GetParameter('PlusServerHostNamePort'))
        self.lineEdit.setDisabled(True)
        self.lineEdit.setText(lnNode.GetParameter('PlusServerHostNamePort'))
    else:
        #self.lineEdit.setDisabled(False)
        settings = slicer.app.userSettings()
        plusServerHostNamePort = settings.value(self.moduleName+'/PlusServerHostNamePort', 'localhost:18944')
        self.lineEdit.setText(plusServerHostNamePort)

  def cleanup(self):
    self.launchGuideletButton.disconnect('clicked()', self.onLaunchGuideletButtonClicked)
    if self.guideletLogic:
      self.guideletLogic.cleanup()
    if self.guideletInstance:
      self.guideletInstance.cleanup()
   
  def onLaunchGuideletButtonClicked(self):
    logging.debug('onLaunchGuideletButtonClicked')
    
    parameterList = self.collectParameterList()

    if not self.guideletInstance:
      self.guideletInstance = self.createGuideletInstance(parameterList)
    self.guideletInstance.showFullScreen()

  def collectParameterList(self):
    parameterList = None
    settings = slicer.app.userSettings()
    if self.lineEdit.isEnabled() and self.lineEdit.text != '':
        settings.setValue(self.moduleName + '/PlusServerHostNamePort', self.lineEdit.text)        
        parameterList = {'PlusServerHostNamePort':self.lineEdit.text}
    return parameterList

  def createGuideletInstance(self, parameterList = None):
    raise NotImplementedError("Abstract method must be overridden!")

  def createGuideletLogic(self):
    raise NotImplementedError("Abstract method must be overridden!")


#
# GuideletLogic
#

class GuideletLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent = None):
    ScriptedLoadableModuleLogic.__init__(self, parent)

  def createParameterNode(self):
    node = ScriptedLoadableModuleLogic.createParameterNode(self)
    parameterList = {'LiveUltrasoundNodeName': 'Image_Reference',
                     'LiveUltrasoundNodeName_Needle': 'Image_Needle',
                     'PlusServerHostNamePort':'localhost:18944'
                     }

    for parameter in parameterList:
      if not node.GetParameter(parameter):
        node.SetParameter(parameter, str(parameterList[parameter]))

    return node

  def cleanup(self):
    pass

 
#	
#	GuideletTest
#	

class GuideletTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  #TODO add common test utility methods here
  
  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    # self.test_SliceletBase1() # Here the tests should be added that are common for all full screen applets e.g.  # TODO defines tests


class Guidelet(object):

  @staticmethod
  def showToolbars(show):
    for toolbar in slicer.util.mainWindow().findChildren('QToolBar'):
      toolbar.setVisible(show)

  @staticmethod
  def showModulePanel(show):
    slicer.util.mainWindow().findChildren('QDockWidget','PanelDockWidget')[0].setVisible(show)

  @staticmethod
  def showMenuBar(show):
    for menubar in slicer.util.mainWindow().findChildren('QMenuBar'):
      menubar.setVisible(show)

  @staticmethod
  def onGenericCommandResponseReceived(commandId, responseNode):
    if responseNode:
      logging.debug("Response from PLUS: {0}".format(responseNode.GetText(0)))
    else:
      logging.debug("Timeout. Command Id: {0}".format(commandId))

  @staticmethod
  def showUltrasoundIn3dView(show):
    redNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
    if show:
      redNode.SetSliceVisible(1)
    else:
      redNode.SetSliceVisible(0)

  VIEW_ULTRASOUND = unicode("Ultrasound")
  VIEW_ULTRASOUND_3D = unicode("Ultrasound + 3D")
  VIEW_ULTRASOUND_DUAL_3D = unicode("Ultrasound + Dual 3D")
  VIEW_3D = unicode("3D")
  VIEW_DUAL_3D = unicode("Dual 3D")
  
  def __init__(self, parent, logic, parameterList=None, configurationName='Default'):
    logging.debug('Guidelet.__init__')
    self.parent = parent
    self.logic = logic
    self.configurationName = configurationName
    self.parameterNodeObserver = None
    self.parameterNode = self.logic.getParameterNode()
    self.layoutManager = slicer.app.layoutManager()

    if parameterList is not None:
      for parameter in parameterList:
        self.parameterNode.SetParameter(parameter, str(parameterList[parameter]))
        logging.info(parameter + ' ' + self.parameterNode.GetParameter(parameter))

    self.setAndObserveParameterNode(self.parameterNode)

    self.ultrasound = self.getUltrasoundClass()

    self.setupConnectorNode()

    self.sliceletDockWidget = qt.QDockWidget(self.parent)
    style = "QDockWidget:title {background-color: #9ACEFF;}"    
    self.sliceletDockWidget.setStyleSheet(style)

    self.mainWindow=slicer.util.mainWindow()
    self.sliceletDockWidget.setParent(self.mainWindow)
    self.mainWindow.addDockWidget(qt.Qt.LeftDockWidgetArea, self.sliceletDockWidget)
        
    self.sliceletPanel = qt.QFrame(self.sliceletDockWidget)
    self.sliceletPanelLayout = qt.QVBoxLayout(self.sliceletPanel)    
    self.sliceletDockWidget.setWidget(self.sliceletPanel)
    
    # Color scheme: #C7DDF5, #9ACEFF, #669ACC, #336799
    style = "QFrame {background-color: #336799; border-color: #9ACEFF;}"
    self.sliceletPanel.setStyleSheet(style)

    self.setupFeaturePanelList()
    self.setupAdvancedPanel()
    self.setupAdditionalPanel()

    self.addConnectorObservers()

    # Setting up callback functions for widgets.
    self.setupConnections()

  def setupFeaturePanelList(self):
    featurePanelList = self.createFeaturePanels()

    self.collapsibleButtonGroup = qt.QButtonGroup()
    for panel in featurePanelList:
      self.collapsibleButtonGroup.addButton(panel)

  def getUltrasoundClass(self):
    return UltraSound(self)
    
  def cleanup(self):
    self.ultrasound.cleanup()
    self.disconnect()

  def createFeaturePanels(self):
    self.ultrasoundCollapsibleButton, self.ultrasoundLayout = self.ultrasound.setupPanel(self.sliceletPanelLayout)
    self.advancedCollapsibleButton = ctk.ctkCollapsibleButton()

    featurePanelList = [self.ultrasoundCollapsibleButton, self.advancedCollapsibleButton]

    return featurePanelList
  
  def setupAdvancedPanel(self):
    logging.debug('setupAdvancedPanel')

    self.advancedCollapsibleButton.setProperty('collapsedHeight', 20)
    #self.setButtonStyle(self.advancedCollapsibleButton, 2.0)
    self.advancedCollapsibleButton.text = "Settings"
    self.sliceletPanelLayout.addWidget(self.advancedCollapsibleButton)

    self.advancedLayout = qt.QFormLayout(self.advancedCollapsibleButton)
    self.advancedLayout.setContentsMargins(12, 4, 4, 4)
    self.advancedLayout.setSpacing(4)

    # Layout selection combo box
    self.viewSelectorComboBox = qt.QComboBox(self.advancedCollapsibleButton)
    self.setupViewerLayouts()
    self.advancedLayout.addRow("Layout: ", self.viewSelectorComboBox)

    self.registerCustomLayouts()

    self.selectView(self.VIEW_ULTRASOUND_3D)

    # OpenIGTLink connector node selection
    self.linkInputSelector = slicer.qMRMLNodeComboBox()
    self.linkInputSelector.nodeTypes = ("vtkMRMLIGTLConnectorNode", "")
    self.linkInputSelector.selectNodeUponCreation = True
    self.linkInputSelector.addEnabled = False
    self.linkInputSelector.removeEnabled = True
    self.linkInputSelector.noneEnabled = False
    self.linkInputSelector.showHidden = False
    self.linkInputSelector.showChildNodeTypes = False
    self.linkInputSelector.setMRMLScene( slicer.mrmlScene )
    self.linkInputSelector.setToolTip( "Select connector node" )
    self.advancedLayout.addRow("OpenIGTLink connector: ", self.linkInputSelector)

    self.showFullSlicerInterfaceButton = qt.QPushButton()
    self.showFullSlicerInterfaceButton.setText("Show full user interface")
    setButtonStyle(self.showFullSlicerInterfaceButton)
    #self.showFullSlicerInterfaceButton.setSizePolicy(self.sizePolicy)
    self.advancedLayout.addRow(self.showFullSlicerInterfaceButton)

    self.saveSceneButton = qt.QPushButton()
    self.saveSceneButton.setText("Save slicelet scene")
    setButtonStyle(self.saveSceneButton)
    self.advancedLayout.addRow(self.saveSceneButton)

    self.saveDirectoryLineEdit = qt.QLineEdit()
    self.saveDirectoryLineEdit.setText(self.getSavedScenesDirectory())
    saveLabel = qt.QLabel()
    saveLabel.setText("Save scene directory:")
    hbox = qt.QHBoxLayout()
    hbox.addWidget(saveLabel)
    hbox.addWidget(self.saveDirectoryLineEdit)
    self.advancedLayout.addRow(hbox)

  def setupViewerLayouts(self):
    self.viewSelectorComboBox.addItem(self.VIEW_ULTRASOUND)
    self.viewSelectorComboBox.addItem(self.VIEW_ULTRASOUND_3D)
    self.viewSelectorComboBox.addItem(self.VIEW_ULTRASOUND_DUAL_3D)
    self.viewSelectorComboBox.addItem(self.VIEW_3D)
    self.viewSelectorComboBox.addItem(self.VIEW_DUAL_3D)

  def setupAdditionalPanel(self):
    pass

  def registerCustomLayouts(self):#common
    layoutLogic = self.layoutManager.layoutLogic()
    customLayout = ("<layout type=\"horizontal\" split=\"false\" >"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "    <property name=\"viewlabel\" action=\"default\">1</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"2\" type=\"secondary\">"
      "   <property name=\"viewlabel\" action=\"default\">2</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.dual3dCustomLayoutId=503
    layoutLogic.GetLayoutNode().AddLayoutDescription(self.dual3dCustomLayoutId, customLayout)

    customLayout = ("<layout type=\"horizontal\" split=\"false\" >"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "    <property name=\"viewlabel\" action=\"default\">1</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
      "   <property name=\"orientation\" action=\"default\">Axial</property>"
      "   <property name=\"viewlabel\" action=\"default\">R</property>"
      "   <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.red3dCustomLayoutId=504
    layoutLogic.GetLayoutNode().AddLayoutDescription(self.red3dCustomLayoutId, customLayout)
    
    customLayout = ("<layout type=\"horizontal\" split=\"false\" >"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "    <property name=\"viewlabel\" action=\"default\">1</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"2\" type=\"secondary\">"
      "   <property name=\"viewlabel\" action=\"default\">2</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
      "   <property name=\"orientation\" action=\"default\">Axial</property>"
      "   <property name=\"viewlabel\" action=\"default\">R</property>"
      "   <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.redDual3dCustomLayoutId=505
    layoutLogic.GetLayoutNode().AddLayoutDescription(self.redDual3dCustomLayoutId, customLayout)

  def setupScene(self):

    #create transforms
    self.ReferenceToRas = slicer.util.getNode('ReferenceToRas')
    if not self.ReferenceToRas:
      self.ReferenceToRas=slicer.vtkMRMLLinearTransformNode()
      self.ReferenceToRas.SetName("ReferenceToRas")
      m = vtk.vtkMatrix4x4()
      m.SetElement( 0, 0, 0 )
      m.SetElement( 0, 2, -1 )
      m.SetElement( 1, 1, 0 )
      m.SetElement( 1, 1, -1 )
      m.SetElement( 2, 2, 0 )
      m.SetElement( 2, 0, -1 )
      self.ReferenceToRas.SetMatrixTransformToParent(m)
      slicer.mrmlScene.AddNode(self.ReferenceToRas)

    # setup feature scene
    self.ultrasound.setupScene()

  def onSaveSceneClicked(self):#common
    #
    # save the mrml scene to a temp directory, then zip it
    #
    applicationLogic = slicer.app.applicationLogic()
    sceneSaveDirectory = self.saveDirectoryLineEdit.text

    # Save the last used directory
    self.setSavedScenesDirectory(sceneSaveDirectory)
    
    sceneSaveDirectory = sceneSaveDirectory + "/" + self.logic.moduleName + "-" + time.strftime("%Y%m%d-%H%M%S")
    logging.info("Saving scene to: {0}".format(sceneSaveDirectory))
    if not os.access(sceneSaveDirectory, os.F_OK):
      os.makedirs(sceneSaveDirectory)
    if applicationLogic.SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None):
      logging.info("Scene saved to: {0}".format(sceneSaveDirectory)) 
    else:
      logging.error("Scene saving failed")

  def setupConnections(self):
    logging.debug('Guidelet.setupConnections()')
    self.ultrasoundCollapsibleButton.connect('toggled(bool)', self.onUltrasoundPanelToggled)
    self.ultrasound.setupConnections()
    #advanced settings panel
    self.showFullSlicerInterfaceButton.connect('clicked()', self.onShowFullSlicerInterfaceClicked)
    self.saveSceneButton.connect('clicked()', self.onSaveSceneClicked)
    self.linkInputSelector.connect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeActivated)
    self.viewSelectorComboBox.connect('activated(int)', self.onViewSelect)

  def disconnect(self):
    self.removeConnectorObservers()
    # Remove observer to old parameter node
    self.removeParameterNodeObserver()

    self.ultrasoundCollapsibleButton.disconnect('toggled(bool)', self.onUltrasoundPanelToggled)
    #advanced settings panel
    self.showFullSlicerInterfaceButton.disconnect('clicked()', self.onShowFullSlicerInterfaceClicked)
    self.saveSceneButton.disconnect('clicked()', self.onSaveSceneClicked)
    self.linkInputSelector.disconnect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeActivated)
    self.viewSelectorComboBox.disconnect('activated(int)', self.onViewSelect)

  def showFullScreen(self):
  
    # We hide all toolbars, etc. which is inconvenient as a default startup setting,
    # therefore disable saving of window setup.
    settings = qt.QSettings()
    settings.setValue('MainWindow/RestoreGeometry', 'false')
    
    self.showToolbars(False)
    self.showModulePanel(False)
    self.showMenuBar(False)
        
    self.sliceletDockWidget.show()
    mainWindow=slicer.util.mainWindow()
    mainWindow.showFullScreen()

  def onShowFullSlicerInterfaceClicked(self):
    self.showToolbars(True)
    self.showModulePanel(True)
    self.showMenuBar(True)
    slicer.util.mainWindow().showMaximized()
    
    # Save current state
    settings = qt.QSettings()
    settings.setValue('MainWindow/RestoreGeometry', 'true')

  def executeCommand(self, command, commandResponseCallback):
    command.RemoveObservers(slicer.modulelogic.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    command.AddObserver(slicer.modulelogic.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent, commandResponseCallback)
    slicer.modules.openigtlinkremote.logic().SendCommand(command, self.connectorNode.GetID())
    
  def setAndObserveParameterNode(self, parameterNode):
    if parameterNode == self.parameterNode and self.parameterNodeObserver:
      # no change and node is already observed
      return
    # Remove observer to old parameter node
    self.removeParameterNodeObserver()
    # Set and observe new parameter node
    self.parameterNode = parameterNode
    if self.parameterNode:
      self.parameterNodeObserver = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent,
                                                                  self.onParameterNodeModified)
    # Update GUI
    self.updateGUIFromParameterNode()

  def removeParameterNodeObserver(self):
    if self.parameterNode and self.parameterNodeObserver:
      self.parameterNode.RemoveObserver(self.parameterNodeObserver)
      self.parameterNodeObserver = None

  def onParameterNodeModified(self, observer, eventid):
    logging.debug('onParameterNodeModified')
    self.updateGUIFromParameterNode()
    
  def updateGUIFromParameterNode(self):#TODO
    parameterNode = self.parameterNode
    if not parameterNode:
      return
  
  def setupConnectorNode(self):
    logging.info("setupConnectorNode")
    self.connectorNodeObserverTagList = []
    self.connectorNodeConnected = False
    self.connectorNode = self.ultrasound.createPlusConnector()
    self.connectorNode.Start()

  def onConnectorNodeConnected(self, caller, event, force=False):
    logging.info("onConnectorNodeConnected")
    # Multiple notifications may be sent when connecting/disconnecting,
    # so we just if we know about the state change already
    if self.connectorNodeConnected and not force:
        return
    self.connectorNodeConnected = True
    self.ultrasound.onConnectorNodeConnected()
    self.delayedFitUltrasoundImageToView(5000)

  def onConnectorNodeDisconnected(self, caller, event, force=False):
    logging.info("onConnectorNodeDisconnected")
    # Multiple notifications may be sent when connecting/disconnecting,
    # so we just if we know about the state change already
    if not self.connectorNodeConnected and not force:
        return
    self.connectorNodeConnected = False
    self.ultrasound.onConnectorNodeDisconnected()

  def onConnectorNodeActivated(self):
    logging.debug('onConnectorNodeActivated')
  
    self.removeConnectorObservers()

    # Start using new connector.
    self.connectorNode = self.linkInputSelector.currentNode()

    if not self.connectorNode:
      logging.warning('No connector node found!')
      return
      
    self.addConnectorObservers()

  def removeConnectorObservers(self):
    # Clean up observers to old connector.
    if self.connectorNode and self.connectorNodeObserverTagList:
      for tag in self.connectorNodeObserverTagList:
        self.connectorNode.RemoveObserver(tag)
      self.connectorNodeObserverTagList = []

  def addConnectorObservers(self):

    # Force initial update
    if self.connectorNode.GetState() == slicer.vtkMRMLIGTLConnectorNode.STATE_CONNECTED:
      self.onConnectorNodeConnected(None, None, True)
    else:
      self.onConnectorNodeDisconnected(None, None, True)

    # Add observers for connect/disconnect events
    events = [[slicer.vtkMRMLIGTLConnectorNode.ConnectedEvent, self.onConnectorNodeConnected],
              [slicer.vtkMRMLIGTLConnectorNode.DisconnectedEvent, self.onConnectorNodeDisconnected]]
    for tagEventHandler in events:
      connectorNodeObserverTag = self.connectorNode.AddObserver(tagEventHandler[0], tagEventHandler[1])
      self.connectorNodeObserverTagList.append(connectorNodeObserverTag)

  def fitUltrasoundImageToView(self):
    redWidget = self.layoutManager.sliceWidget('Red')
    redWidget.sliceController().fitSliceToBackground()

  def delayedFitUltrasoundImageToView(self, delayMsec=500):
    qt.QTimer.singleShot(delayMsec, self.fitUltrasoundImageToView)

  def selectView(self, viewName):
    index = self.viewSelectorComboBox.findText(viewName)
    if index == -1:
      index = 0
    self.viewSelectorComboBox.setCurrentIndex(index)
    self.onViewSelect(index)

  def onViewSelect(self, layoutIndex):
    text = self.viewSelectorComboBox.currentText
    logging.debug('onViewSelect: {0}'.format(text))
    if text == self.VIEW_ULTRASOUND:
      self.layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)
      self.delayedFitUltrasoundImageToView()
      self.showUltrasoundIn3dView(False)
    elif text == self.VIEW_ULTRASOUND_3D:
      self.layoutManager.setLayout(self.red3dCustomLayoutId)
      self.delayedFitUltrasoundImageToView()
      self.showUltrasoundIn3dView(True)
    elif text == self.VIEW_ULTRASOUND_DUAL_3D:
      self.layoutManager.setLayout(self.redDual3dCustomLayoutId)
      self.delayedFitUltrasoundImageToView()
      self.showUltrasoundIn3dView(True)
    elif text == self.VIEW_3D:
      self.layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
      self.showUltrasoundIn3dView(True)
    elif text == self.VIEW_DUAL_3D:
      self.layoutManager.setLayout(self.dual3dCustomLayoutId)
      self.showUltrasoundIn3dView(False)

  def writeTransformToSettings(self, transformName, transformMatrix):
    transformMatrixArray = []
    for r in xrange(4):
      for c in xrange(4):
        transformMatrixArray.append(transformMatrix.GetElement(r,c))
    transformMatrixString = ' '.join(map(str, transformMatrixArray)) # string, numbers are separated by spaces
    settings = slicer.app.userSettings()
    settingString = self.logic.moduleName + '/Configurations/' + self.configurationName + '/{0}' # Write to selected configuration
    settings.setValue(settingString.format(transformName), transformMatrixString)
    
  def readTransformFromSettings(self, transformName):
    transformMatrix = vtk.vtkMatrix4x4()
    settings = slicer.app.userSettings()
    settingString = self.logic.moduleName + '/Configurations/' + self.configurationName + '/{0}' # Read from selected configuration
    transformMatrixString = settings.value(settingString.format(transformName))
    if not transformMatrixString: 
      settingString = self.logic.moduleName + '/Configurations/Default/{0}' # Read from default configuration
      transformMatrixString = settings.value(settingString.format(transformName))
      if not transformMatrixString: 
        return None
    transformMatrixArray = map(float, transformMatrixString.split(' '))
    for r in xrange(4):
      for c in xrange(4):
        transformMatrix.SetElement(r,c, transformMatrixArray[r*4+c])
    return transformMatrix

  def getSavedScenesDirectory(self):
    settings = slicer.app.userSettings()
    settingString = self.logic.moduleName + "/SavedScenesDirectory"
    sceneSaveDirectory = settings.value(settingString)
    if not sceneSaveDirectory:
      sceneSaveDirectory = self.parameterNode.GetParameter('DefaultSavedScenesPath')
    return sceneSaveDirectory

  def setSavedScenesDirectory(self, sceneSaveDirectory):
    settings = slicer.app.userSettings()
    settingString = self.logic.moduleName + "/SavedScenesDirectory"
    settings.setValue(settingString, sceneSaveDirectory)

  def onUltrasoundPanelToggled(self, toggled):
    if not toggled:
      # deactivate placement mode
      interactionNode = slicer.app.applicationLogic().GetInteractionNode()
      interactionNode.SetCurrentInteractionMode(interactionNode.ViewTransform)   
      return

    logging.debug('onTumorContouringPanelToggled: {0}'.format(toggled))

    self.showDefaultView()

  def showDefaultView(self):
    self.selectView(self.VIEW_ULTRASOUND) # Red only layout


class UltraSound(object):

  DEFAULT_IMAGE_SIZE = [800, 600, 1]

  def __init__(self, guideletParent):
    self.guideletParent = guideletParent
    self.captureDeviceName='CaptureDevice'

    from PlusRemote import PlusRemoteLogic
    self.plusRemoteLogic = PlusRemoteLogic()

    moduleDir = os.path.dirname(__file__)
    iconPathRecord = os.path.join(moduleDir, 'Resources', 'Icons', 'icon_Record.png')
    iconPathStop = os.path.join(moduleDir, 'Resources', 'Icons', 'icon_Stop.png')

    if os.path.isfile(iconPathRecord):
      self.recordIcon = qt.QIcon(iconPathRecord)
    if os.path.isfile(iconPathStop):
      self.stopIcon = qt.QIcon(iconPathStop)

  def onConnectorNodeConnected(self):
    self.freezeUltrasoundButton.setText('Freeze')
    self.startStopRecordingButton.setEnabled(True)

  def onConnectorNodeDisconnected(self):
    self.freezeUltrasoundButton.setText('Un-freeze')
    self.startStopRecordingButton.setEnabled(False)

  def setupPanel(self, parentWidget):
    logging.debug('UltraSound.setupPanel')
    collapsibleButton = ctk.ctkCollapsibleButton()

    collapsibleButton.setProperty('collapsedHeight', 20)
    setButtonStyle(collapsibleButton, 2.0)
    collapsibleButton.text = "Ultrasound"
    #self.sliceletPanelLayout.addWidget(collapsibleButton)
    parentWidget.addWidget(collapsibleButton)

    ultrasoundLayout = qt.QFormLayout(collapsibleButton)
    ultrasoundLayout.setContentsMargins(12,4,4,4)
    ultrasoundLayout.setSpacing(4)

    self.startStopRecordingButton = qt.QPushButton("  Start Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setIcon(self.recordIcon)
    setButtonStyle(self.startStopRecordingButton)
    self.startStopRecordingButton.setToolTip("If clicked, start recording")

    self.freezeUltrasoundButton = qt.QPushButton('Freeze')
    setButtonStyle(self.freezeUltrasoundButton)

    hbox = qt.QHBoxLayout()
    hbox.addWidget(self.startStopRecordingButton)
    hbox.addWidget(self.freezeUltrasoundButton)
    ultrasoundLayout.addRow(hbox)

    self.usFrozen=False

    self.brigthnessContrastButtonNormal = qt.QPushButton()
    self.brigthnessContrastButtonNormal.text = "Normal"
    setButtonStyle(self.brigthnessContrastButtonNormal)
    self.brigthnessContrastButtonNormal.setEnabled(True)

    self.brigthnessContrastButtonBright = qt.QPushButton()
    self.brigthnessContrastButtonBright.text = "Bright"
    setButtonStyle(self.brigthnessContrastButtonBright)
    self.brigthnessContrastButtonBright.setEnabled(True)

    self.brigthnessContrastButtonBrighter = qt.QPushButton()
    self.brigthnessContrastButtonBrighter.text = "Brighter"
    setButtonStyle(self.brigthnessContrastButtonBrighter)
    self.brigthnessContrastButtonBrighter.setEnabled(True)

    brightnessContrastBox = qt.QHBoxLayout()
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonNormal)
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonBright)
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonBrighter)
    ultrasoundLayout.addRow(brightnessContrastBox)

    return collapsibleButton, ultrasoundLayout

  def setupScene(self):
    logging.info("UltraSound.setupScene")

    # live ultrasound
    liveUltrasoundNodeName = self.guideletParent.parameterNode.GetParameter('LiveUltrasoundNodeName')
    self.liveUltrasoundNode_Reference = slicer.util.getNode(liveUltrasoundNodeName)
    if not self.liveUltrasoundNode_Reference:
      imageSpacing=[0.2, 0.2, 0.2]
      # Create an empty image volume
      imageData=vtk.vtkImageData()
      imageData.SetDimensions(self.DEFAULT_IMAGE_SIZE)
      imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
      thresholder=vtk.vtkImageThreshold()
      thresholder.SetInputData(imageData)
      thresholder.SetInValue(0)
      thresholder.SetOutValue(0)
      # Create volume node
      self.liveUltrasoundNode_Reference=slicer.vtkMRMLScalarVolumeNode()
      self.liveUltrasoundNode_Reference.SetName(liveUltrasoundNodeName)
      self.liveUltrasoundNode_Reference.SetSpacing(imageSpacing)
      self.liveUltrasoundNode_Reference.SetImageDataConnection(thresholder.GetOutputPort())
      # Add volume to scene
      slicer.mrmlScene.AddNode(self.liveUltrasoundNode_Reference)
      displayNode=slicer.vtkMRMLScalarVolumeDisplayNode()
      slicer.mrmlScene.AddNode(displayNode)
      colorNode = slicer.util.getNode('Grey')
      displayNode.SetAndObserveColorNodeID(colorNode.GetID())
      self.liveUltrasoundNode_Reference.SetAndObserveDisplayNodeID(displayNode.GetID())
      #self.liveUltrasoundNode_Reference.CreateDefaultStorageNode()

    self.setupResliceDriver()

  def setupResliceDriver(self):
    layoutManager = slicer.app.layoutManager()
    # Show ultrasound in red view.
    redSlice = layoutManager.sliceWidget('Red')
    redSliceLogic = redSlice.sliceLogic()
    redSliceLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.liveUltrasoundNode_Reference.GetID())
    # Set up volume reslice driver.
    resliceLogic = slicer.modules.volumereslicedriver.logic()
    if resliceLogic:
      redNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
      # Typically the image is zoomed in, therefore it is faster if the original resolution is used
      # on the 3D slice (and also we can show the full image and not the shape and size of the 2D view)
      redNode.SetSliceResolutionMode(slicer.vtkMRMLSliceNode.SliceResolutionMatchVolumes)
      resliceLogic.SetDriverForSlice(self.liveUltrasoundNode_Reference.GetID(), redNode)
      resliceLogic.SetModeForSlice(6, redNode) # Transverse mode, default for PLUS ultrasound.
      resliceLogic.SetFlipForSlice(False, redNode)
      resliceLogic.SetRotationForSlice(180, redNode)
      redSliceLogic.FitSliceToAll()
    else:
      logging.warning('Logic not found for Volume Reslice Driver')

    self.liveUltrasoundNode_Reference.SetAndObserveTransformNodeID(self.guideletParent.ReferenceToRas.GetID())

  def setupConnections(self):
    self.startStopRecordingButton.connect('clicked(bool)', self.onStartStopRecordingClicked)
    self.freezeUltrasoundButton.connect('clicked()', self.onFreezeUltrasoundClicked)
    self.brigthnessContrastButtonNormal.connect('clicked()', self.onBrightnessContrastNormalClicked)
    self.brigthnessContrastButtonBright.connect('clicked()', self.onBrightnessContrastBrightClicked)
    self.brigthnessContrastButtonBrighter.connect('clicked()', self.onBrightnessContrastBrighterClicked)

  def disconnect(self):
    self.startStopRecordingButton.disconnect('clicked(bool)', self.onStartStopRecordingClicked)
    self.freezeUltrasoundButton.disconnect('clicked()', self.onFreezeUltrasoundClicked)
    self.brigthnessContrastButtonNormal.disconnect('clicked()', self.onBrightnessContrastNormalClicked)
    self.brigthnessContrastButtonBright.disconnect('clicked()', self.onBrightnessContrastBrightClicked)
    self.brigthnessContrastButtonBrighter.disconnect('clicked()', self.onBrightnessContrastBrighterClicked)

  def createPlusConnector(self):
    connectorNode = slicer.util.getNode('PlusConnector')
    if not connectorNode:
      connectorNode = slicer.vtkMRMLIGTLConnectorNode()
      slicer.mrmlScene.AddNode(connectorNode)
      connectorNode.SetName('PlusConnector')
      hostNamePort = self.guideletParent.parameterNode.GetParameter('PlusServerHostNamePort') # example: "localhost:18944"
      [hostName, port] = hostNamePort.split(':')
      connectorNode.SetTypeClient(hostName, int(port))
      logging.debug("PlusConnector created")
    return connectorNode

  def recordingCommandCompleted(self, command, q):
    statusText = "Command {0} [{1}]: {2}\n".format(command.GetCommandName(), command.GetID(), command.StatusToString(command.GetStatus()))
    statusTextUser = "{0} {1}\n".format(command.GetCommandName(), command.StatusToString(command.GetStatus()))
    if command.GetResponseMessage():
      statusText = statusText + command.GetResponseMessage()
      statusTextUser = command.GetResponseMessage()
    elif command.GetResponseText():
      statusText = statusText + command.GetResponseText()
      statusTextUser = command.GetResponseText()
    logging.info(statusText)
    self.startStopRecordingButton.setToolTip(statusTextUser)

  def cleanup(self):
    self.disconnect()

  def onStartStopRecordingClicked(self):

    if self.startStopRecordingButton.isChecked():
      self.startStopRecordingButton.setText("  Stop Recording")
      self.startStopRecordingButton.setIcon(self.stopIcon)
      self.startStopRecordingButton.setToolTip("Recording is being started...")

      # Important to save as .mhd because that does not require lengthy finalization (merging into a single file)
      recordPrefix = self.guideletParent.parameterNode.GetParameter('RecordingFilenamePrefix')
      recordExt = self.guideletParent.parameterNode.GetParameter('RecordingFilenameExtension')
      self.recordingFileName =  recordPrefix + time.strftime("%Y%m%d-%H%M%S") + recordExt

      logging.info("Starting recording to: {0}".format(self.recordingFileName))

      self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('CaptureDeviceId', self.captureDeviceName)
      self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('OutputFilename', self.recordingFileName)
      self.guideletParent.executeCommand(self.plusRemoteLogic.cmdStartRecording, self.recordingCommandCompleted)

    else:
      logging.info("Stopping recording")
      self.startStopRecordingButton.setText("  Start Recording")
      self.startStopRecordingButton.setIcon(self.recordIcon)
      self.startStopRecordingButton.setToolTip( "Recording is being stopped..." )
      self.plusRemoteLogic.cmdStopRecording.SetCommandAttribute('CaptureDeviceId', self.captureDeviceName)
      self.guideletParent.executeCommand(self.plusRemoteLogic.cmdStopRecording, self.recordingCommandCompleted)

  def onFreezeUltrasoundClicked(self):
    logging.debug('onFreezeUltrasoundClicked')
    self.usFrozen = not self.usFrozen
    if self.usFrozen:
      self.guideletParent.connectorNode.Stop()
    else:
      self.guideletParent.connectorNode.Start()

  def setImageMinMaxLevel(self, minLevel, maxLevel):
    self.liveUltrasoundNode_Reference.GetDisplayNode().SetAutoWindowLevel(0)
    self.liveUltrasoundNode_Reference.GetDisplayNode().SetWindowLevelMinMax(minLevel,maxLevel)

  def onBrightnessContrastNormalClicked(self):
    logging.debug('onBrightnessContrastNormalClicked')
    self.setImageMinMaxLevel(0,200)

  def onBrightnessContrastBrightClicked(self):
    logging.debug('onBrightnessContrastBrightClicked')
    self.setImageMinMaxLevel(0,120)

  def onBrightnessContrastBrighterClicked(self):
    logging.debug('onBrightnessContrastBrighterClicked')
    self.setImageMinMaxLevel(0,60)