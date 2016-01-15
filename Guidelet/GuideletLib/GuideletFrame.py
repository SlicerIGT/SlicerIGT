import os
from __main__ import vtk, qt, ctk, slicer
import logging
import time

from UltraSound import UltraSound

class Guidelet(object):

  @staticmethod
  def showToolbars(show):
    for toolbar in slicer.util.mainWindow().findChildren('QToolBar'):
      toolbar.setVisible(show)

  def showModulePanel(self, show):
    modulePanelDockWidget = slicer.util.mainWindow().findChildren('QDockWidget','PanelDockWidget')[0]
    modulePanelDockWidget.setVisible(show)

    if show:
      mainWindow=slicer.util.mainWindow()
      mainWindow.tabifyDockWidget(self.sliceletDockWidget, modulePanelDockWidget)

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
  
  def __init__(self, parent, logic, configurationName='Default'):
    logging.debug('Guidelet.__init__')
    self.parent = parent
    self.logic = logic
    self.configurationName = configurationName
    self.parameterNodeObserver = None
    self.parameterNode = self.logic.getParameterNode()
    self.layoutManager = slicer.app.layoutManager()

    self.logic.updateParameterNodeFromSettings(self.parameterNode, self.configurationName)

    self.setAndObserveParameterNode(self.parameterNode)

    self.ultrasound = self.getUltrasoundClass()

    self.setupConnectorNode()

    self.sliceletDockWidget = qt.QDockWidget(self.parent)

    self.mainWindow=slicer.util.mainWindow()
    self.sliceletDockWidget.setParent(self.mainWindow)
    self.mainWindow.addDockWidget(qt.Qt.LeftDockWidgetArea, self.sliceletDockWidget)
        
    self.sliceletPanel = qt.QFrame(self.sliceletDockWidget)
    self.sliceletPanelLayout = qt.QVBoxLayout(self.sliceletPanel)    
    self.sliceletDockWidget.setWidget(self.sliceletPanel)
    
    self.setupFeaturePanelList()
    self.setupAdvancedPanel()
    self.setupAdditionalPanel()

    self.addConnectorObservers()

    # Setting up callback functions for widgets.
    self.setupConnections()

    self.sliceletDockWidget.setStyleSheet(self.loadStyleSheet())

  def loadStyleSheet(self):
    moduleDir = os.path.dirname(__file__)
    style = self.parameterNode.GetParameter('StyleSheet')
    styleFile = os.path.join(moduleDir, 'Resources', 'StyleSheets', style)
    f = qt.QFile(styleFile)
    if not f.exists():
      logging.debug("Unable to load stylesheet, file not found")
      return ""
    else:
      f.open(qt.QFile.ReadOnly | qt.QFile.Text)
      ts = qt.QTextStream(f)
      stylesheet = ts.readAll()
      return stylesheet
  
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
    self.showFullSlicerInterfaceButton.setText("Show 3D Slicer user interface")
    self.advancedLayout.addRow(self.showFullSlicerInterfaceButton)

    self.showGuideletFullscreenButton = qt.QPushButton()
    self.showGuideletFullscreenButton.setText("Show guidelet in full screen")
    self.advancedLayout.addRow(self.showGuideletFullscreenButton)

    self.saveSceneButton = qt.QPushButton()
    self.saveSceneButton.setText("Save slicelet scene")
    self.advancedLayout.addRow(self.saveSceneButton)

    self.saveDirectoryLineEdit = qt.QLineEdit()
    node = self.logic.getParameterNode()
    sceneSaveDirectory = node.GetParameter('SavedScenesDirectory')
    self.saveDirectoryLineEdit.setText(sceneSaveDirectory)
    saveLabel = qt.QLabel()
    saveLabel.setText("Save scene directory:")
    hbox = qt.QHBoxLayout()
    hbox.addWidget(saveLabel)
    hbox.addWidget(self.saveDirectoryLineEdit)
    self.advancedLayout.addRow(hbox)

    self.exitButton = qt.QPushButton()
    self.exitButton.setText("Exit")
    self.advancedLayout.addRow(self.exitButton)

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

  def onSaveDirectoryPreferencesChanged(self):
    sceneSaveDirectory = self.saveDirectoryLineEdit.text
    self.logic.updateSettings({'SavedScenesDirectory' : sceneSaveDirectory}, self.configurationName)
    node = self.logic.getParameterNode()
    self.logic.updateParameterNodeFromUserPreferences(node, {'SavedScenesDirectory' : sceneSaveDirectory})
    
  def onSaveSceneClicked(self):#common
    #
    # save the mrml scene to a temp directory, then zip it
    #
    node = self.logic.getParameterNode()
    sceneSaveDirectory = node.GetParameter('SavedScenesDirectory')
    sceneSaveDirectory = sceneSaveDirectory + "/" + self.logic.moduleName + "-" + time.strftime("%Y%m%d-%H%M%S")
    logging.info("Saving scene to: {0}".format(sceneSaveDirectory))
    if not os.access(sceneSaveDirectory, os.F_OK):
      os.makedirs(sceneSaveDirectory)
      
    applicationLogic = slicer.app.applicationLogic()
    if applicationLogic.SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None):
      logging.info("Scene saved to: {0}".format(sceneSaveDirectory)) 
    else:
      logging.error("Scene saving failed")

  def onExitButtonClicked(self):
    mainwindow = slicer.util.mainWindow()
    mainwindow.close()

  def setupConnections(self):
    logging.debug('Guidelet.setupConnections()')
    self.ultrasoundCollapsibleButton.connect('toggled(bool)', self.onUltrasoundPanelToggled)
    self.ultrasound.setupConnections()
    #advanced settings panel
    self.showFullSlicerInterfaceButton.connect('clicked()', self.onShowFullSlicerInterfaceClicked)
    self.showGuideletFullscreenButton.connect('clicked()', self.onShowGuideletFullscreenButton)
    self.saveSceneButton.connect('clicked()', self.onSaveSceneClicked)
    self.linkInputSelector.connect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeActivated)
    self.viewSelectorComboBox.connect('activated(int)', self.onViewSelect)
    self.exitButton.connect('clicked()', self.onExitButtonClicked)
    self.saveDirectoryLineEdit.connect('editingFinished()', self.onSaveDirectoryPreferencesChanged)

  def disconnect(self):
    self.removeConnectorObservers()
    # Remove observer to old parameter node
    self.removeParameterNodeObserver()

    self.ultrasoundCollapsibleButton.disconnect('toggled(bool)', self.onUltrasoundPanelToggled)
    #advanced settings panel
    self.showFullSlicerInterfaceButton.disconnect('clicked()', self.onShowFullSlicerInterfaceClicked)
    self.showGuideletFullscreenButton.disconnect('clicked()', self.onShowGuideletFullscreenButton)
    self.saveSceneButton.disconnect('clicked()', self.onSaveSceneClicked)
    self.linkInputSelector.disconnect("nodeActivated(vtkMRMLNode*)", self.onConnectorNodeActivated)
    self.viewSelectorComboBox.disconnect('activated(int)', self.onViewSelect)
    self.exitButton.disconnect('clicked()', self.onExitButtonClicked)
    self.saveDirectoryLineEdit.disconnect('editingFinished()', self.onSaveDirectoryPreferencesChanged)

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

  def onShowGuideletFullscreenButton(self):
    self.showFullScreen()

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