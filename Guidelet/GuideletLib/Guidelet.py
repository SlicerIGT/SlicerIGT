import os
from __main__ import vtk, qt, ctk, slicer
import logging
import time

# fix unicode error by aliasing str as unicode in Python 3
if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion >= 4 and slicer.app.minorVersion >= 11):
  unicode = str

from .UltraSound import UltraSound

class Guidelet(object):
  @staticmethod
  def showToolbars(show):

    # Show/hide all existing toolbars
    for toolbar in slicer.util.mainWindow().findChildren('QToolBar'):
      toolbar.setVisible(show)

    # Prevent sequence browser toolbar showing up automatically
    # when a sequence is loaded.
    # (put in try block because Sequence Browser module is not always installed)
    try:
      slicer.modules.sequencebrowser.autoShowToolBar = show
    except:
      pass

  @staticmethod
  def showPythonConsole(show):
    slicer.util.mainWindow().pythonConsole().parent().setVisible(show)

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

    # Guidelet layout name definitions
    VIEW_ULTRASOUND = unicode("Ultrasound")
    VIEW_ULTRASOUND_3D = unicode("Ultrasound + 3D")
    VIEW_3D_ULTRASOUND = unicode("3D + Ultrasound")
    VIEW_ULTRASOUND_CAM_3D = unicode("Ultrasound + Webcam + 3D")
    VIEW_ULTRASOUND_DUAL_3D = unicode("Ultrasound + Dual 3D")
    VIEW_3D = unicode("3D")
    VIEW_DUAL_3D = unicode("Dual 3D")
    VIEW_TRIPLE_3D = unicode("Triple 3D")
    VIEW_TRIPLE_3D_PARALLEL = unicode("Triple 3D Parallel")
    VIEW_QUAD_3D = unicode("Quad 3D")

  def __init__(self, parent, logic, configurationName='Default', sliceletDockWidgetPosition = qt.Qt.LeftDockWidgetArea):
    logging.debug('Guidelet.__init__')
    self.sliceletDockWidgetPosition = sliceletDockWidgetPosition
    self.parent = parent
    self.logic = logic
    self.configurationName = configurationName
    self.parameterNodeObserver = None
    self.parameterNode = self.logic.getParameterNode()
    self.layoutManager = slicer.app.layoutManager()
    self.layoutNameToIdMap = {}
    self.layoutNameToSelectCallbackMap = {}
    self.defaultLayoutName = self.VIEW_ULTRASOUND

    self.logic.updateParameterNodeFromSettings(self.parameterNode, self.configurationName)
    self.setAndObserveParameterNode(self.parameterNode)

    self.ultrasound = self.getUltrasoundClass()
    self.fitUltrasoundImageToViewOnConnect = True
    self.setupConnectorNode()

    self.sliceletDockWidget = qt.QDockWidget(self.parent)
    self.mainWindow=slicer.util.mainWindow()
    self.sliceletDockWidget.setParent(self.mainWindow)
    self.mainWindow.addDockWidget(self.sliceletDockWidgetPosition, self.sliceletDockWidget)
    self.sliceletPanel = qt.QFrame(self.sliceletDockWidget)
    self.sliceletPanelLayout = qt.QVBoxLayout(self.sliceletPanel)
    self.sliceletDockWidget.setWidget(self.sliceletPanel)

    self.topPanelLayout = qt.QGridLayout(self.sliceletPanel)
    self.sliceletPanelLayout.addLayout(self.topPanelLayout)
    self.setupTopPanel()

    self.setupFeaturePanelList()
    self.setupAdvancedPanel()
    self.setupAdditionalPanel()

    self.addConnectorObservers()
    self.setupConnections()

    self.sliceletDockWidget.setStyleSheet(self.loadStyleSheet())

  def showModulePanel(self, show):
    modulePanelDockWidget = slicer.util.mainWindow().findChildren('QDockWidget','PanelDockWidget')[0]
    modulePanelDockWidget.setVisible(show)

    if show:
      mainWindow=slicer.util.mainWindow()
      if self.sliceletDockWidgetPosition == qt.Qt.LeftDockWidgetArea:
        mainWindow.tabifyDockWidget(self.sliceletDockWidget, modulePanelDockWidget)
        self.sliceletDockWidget.setFeatures(qt.QDockWidget.DockWidgetClosable+qt.QDockWidget.DockWidgetMovable+qt.QDockWidget.DockWidgetFloatable)
    else:
      if self.sliceletDockWidgetPosition == qt.Qt.LeftDockWidgetArea:
        # Prevent accidental closing or undocking of the slicelet's left panel
        self.sliceletDockWidget.setFeatures(0)

  def setupTopPanel(self):
    """
    Reimplement this function and put widgets in self.topPanelLayout (QGridLayout)
    """
    pass

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

  def preCleanup(self):
    self.sliceletDockWidget.setWidget(None)
    self.sliceletPanel = None
    self.mainWindow.removeDockWidget(self.sliceletDockWidget)
    self.sliceletDockWidget = None

    self.ultrasound.preCleanup()
    self.disconnect()

  def createFeaturePanels(self):
    self.ultrasoundCollapsibleButton, self.ultrasoundLayout, self.procedureLayout = self.ultrasound.setupPanel(self.sliceletPanelLayout)
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
    self.advancedLayout.addRow("Layout: ", self.viewSelectorComboBox)

    self.registerDefaultGuideletLayouts()

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
    self.showGuideletFullscreenButton.setText("Show Guidelet in full screen")
    self.advancedLayout.addRow(self.showGuideletFullscreenButton)

    self.saveSceneButton = qt.QPushButton()
    self.saveSceneButton.setText("Save Guidelet scene")
    self.advancedLayout.addRow(self.saveSceneButton)

    self.saveDirectoryLineEdit = ctk.ctkPathLineEdit()
    node = self.logic.getParameterNode()
    sceneSaveDirectory = node.GetParameter('SavedScenesDirectory')
    self.saveDirectoryLineEdit.currentPath = sceneSaveDirectory
    self.saveDirectoryLineEdit.filters = ctk.ctkPathLineEdit.Dirs
    self.saveDirectoryLineEdit.options = ctk.ctkPathLineEdit.DontUseSheet
    self.saveDirectoryLineEdit.options = ctk.ctkPathLineEdit.ShowDirsOnly
    self.saveDirectoryLineEdit.showHistoryButton = False
    self.saveDirectoryLineEdit.setMinimumWidth(100)
    self.saveDirectoryLineEdit.setMaximumWidth(500)

    saveLabel = qt.QLabel()
    saveLabel.setText("Save scene directory:")
    hbox = qt.QHBoxLayout()
    hbox.addWidget(saveLabel)
    hbox.addWidget(self.saveDirectoryLineEdit)
    self.advancedLayout.addRow(hbox)

    self.exitButton = qt.QPushButton()
    self.exitButton.setText("Exit")
    self.advancedLayout.addRow(self.exitButton)

  def setupAdditionalPanel(self):
    pass

  def registerLayout(self, layoutName, layoutId, layoutXmlDescription, layoutSelectCallback=None):
    if (type(layoutName) != str and type(layoutName)!= unicode) or len(layoutName) < 1:
      logging.error('Failed to register layout, because layout name must be a non-empty string. Got ' + repr(layoutName))
      return False
    if not isinstance(layoutId, int):
      logging.error('Failed to register layout named ' + str(layoutName) + ', because given layout ID is not an integer. Got ' + str(layoutId))
      return False
    if layoutName in self.layoutNameToIdMap:
      logging.error('Failed to register layout, because a layout with name ' + str(layoutName) + ' is already registered')
      return False

    layoutLogic = self.layoutManager.layoutLogic()
    if not isinstance(layoutId, slicer.vtkMRMLLayoutNode.SlicerLayout) and layoutLogic.GetLayoutNode().IsLayoutDescription(layoutId):
      logging.error('Failed to register layout, because a layout with ID ' + str(layoutId) + ' is already registered. Try to choose a larger number')
      return False

    logging.info('Registering layout ' + str(layoutName) + ', ' + str(layoutId))

    # Remember layout
    self.layoutNameToIdMap[layoutName] = layoutId
    self.layoutNameToSelectCallbackMap[layoutName] = layoutSelectCallback

    # Add layout to view selector combobox
    self.viewSelectorComboBox.addItem(layoutName)

    # Register layout to layout logic
    if not layoutLogic.GetLayoutNode().IsLayoutDescription(layoutId):
      layoutLogic.GetLayoutNode().AddLayoutDescription(layoutId, layoutXmlDescription)

    return True

  def registerDefaultGuideletLayouts(self): # Common
    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
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
    self.registerLayout(self.VIEW_DUAL_3D, 503, customLayout, self.hideUltrasoundSliceIn3DView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
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
    self.registerLayout(self.VIEW_ULTRASOUND_3D, 504, customLayout, self.delayedFitAndShowUltrasoundSliceIn3dView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
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
    self.registerLayout(self.VIEW_ULTRASOUND_DUAL_3D, 505, customLayout, self.delayedFitAndShowUltrasoundSliceIn3dView)

    customLayout = (
      "<layout type=\"vertical\" split=\"true\" >"
      " <item>"
      "  <layout type=\"horizontal\" split=\"false\" >"
      "   <item>"
      "    <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "      <property name=\"viewlabel\" action=\"default\">1</property>"
      "    </view>"
      "   </item>"
      "   <item>"
      "    <view class=\"vtkMRMLViewNode\" singletontag=\"2\" type=\"secondary\">"
      "     <property name=\"viewlabel\" action=\"default\">2</property>"
      "    </view>"
      "   </item>"
      "  </layout>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"3\">"
      "    <property name=\"viewlabel\" action=\"default\">3</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.registerLayout(self.VIEW_TRIPLE_3D, 506, customLayout, self.hideUltrasoundSliceIn3DView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "   <property name=\"viewlabel\" action=\"default\">1</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <layout type=\"vertical\" split=\"false\" >"
      "   <item>"
      "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
      "     <property name=\"orientation\" action=\"default\">Axial</property>"
      "     <property name=\"viewlabel\" action=\"default\">R</property>"
      "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
      "    </view>"
      "   </item>"
      "   <item>"
      "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
      "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
      "     <property name=\"viewlabel\" action=\"default\">Y</property>"
      "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
      "    </view>"
      "   </item>"
      "  </layout>"
      " </item>"
      "</layout>")
    self.registerLayout(self.VIEW_ULTRASOUND_CAM_3D, 507, customLayout, self.delayedFitAndShowUltrasoundSliceIn3dView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
      " <item>"
      "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
      "   <property name=\"orientation\" action=\"default\">Axial</property>"
      "   <property name=\"viewlabel\" action=\"default\">R</property>"
      "   <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"1\">"
      "    <property name=\"viewlabel\" action=\"default\">1</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.registerLayout(self.VIEW_3D_ULTRASOUND, 508, customLayout, self.delayedFitAndShowUltrasoundSliceIn3dView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
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
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"3\" type=\"secondary\">"
      "   <property name=\"viewlabel\" action=\"default\">3</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.registerLayout(self.VIEW_TRIPLE_3D_PARALLEL, 509, customLayout, self.hideUltrasoundSliceIn3DView)

    customLayout = (
      "<layout type=\"horizontal\" split=\"false\" >"
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
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"3\" type=\"secondary\">"
      "   <property name=\"viewlabel\" action=\"default\">3</property>"
      "  </view>"
      " </item>"
      " <item>"
      "  <view class=\"vtkMRMLViewNode\" singletontag=\"4\" type=\"secondary\">"
      "   <property name=\"viewlabel\" action=\"default\">4</property>"
      "  </view>"
      " </item>"
      "</layout>")
    self.registerLayout(self.VIEW_QUAD_3D, 510, customLayout, self.hideUltrasoundSliceIn3DView)

    # Add existing Slicer layouts with callbacks
    layoutNode = self.layoutManager.layoutLogic().GetLayoutNode()

    ultrasoundViewId = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView
    self.registerLayout(self.VIEW_ULTRASOUND, ultrasoundViewId, \
      layoutNode.GetLayoutDescription(ultrasoundViewId), self.delayedFitAndHideUltrasoundSliceIn3dView)

    threeDViewId = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
    self.registerLayout(self.VIEW_3D, threeDViewId, \
      layoutNode.GetLayoutDescription(threeDViewId), self.showUltrasoundSliceIn3DView)

  def onSceneLoaded(self):
    """ Derived classes can override this function
    """
    pass

  def setupScene(self):
    """ setup feature scene
    """
    self.ultrasound.setupScene()

  def onSaveDirectoryPreferencesChanged(self):
    sceneSaveDirectory = str(self.saveDirectoryLineEdit.currentPath)
    self.logic.updateSettings({'SavedScenesDirectory' : sceneSaveDirectory}, self.configurationName)
    node = self.logic.getParameterNode()
    self.logic.updateParameterNodeFromUserPreferences(node, {'SavedScenesDirectory' : sceneSaveDirectory})

  def onSaveSceneClicked(self):#common
    #
    # save the mrml scene to a temp directory, then zip it
    #
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
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
    qt.QApplication.restoreOverrideCursor()
    slicer.util.showStatusMessage("Saved!", 2000)

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
    self.saveDirectoryLineEdit.connect('currentPathChanged(QString)', self.onSaveDirectoryPreferencesChanged)

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
    self.saveDirectoryLineEdit.disconnect('currentPathChanged(QString)', self.onSaveDirectoryPreferencesChanged)

  def showFullScreen(self):
    # We hide all toolbars, etc. which is inconvenient as a default startup setting,
    # therefore disable saving of window setup.
    settings = qt.QSettings()
    settings.setValue('MainWindow/RestoreGeometry', 'false')

    self.showToolbars(False)
    self.showModulePanel(False)
    self.showMenuBar(False)
    self.showPythonConsole(False)

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
    command.SetCommandAttribute('Name', command.GetCommandName())
    command.RemoveObservers(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent)
    command.AddObserver(slicer.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent, commandResponseCallback)
    self.connectorNode.SendCommand(command)

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
    if self.fitUltrasoundImageToViewOnConnect:
      self.delayedFitUltrasoundImageToView(3000)

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
    if self.connectorNode.GetState() == slicer.vtkMRMLIGTLConnectorNode.StateConnected:
      self.onConnectorNodeConnected(None, None, True)
    else:
      self.onConnectorNodeDisconnected(None, None, True)

    # Add observers for connect/disconnect events
    events = [[slicer.vtkMRMLIGTLConnectorNode.ConnectedEvent, self.onConnectorNodeConnected],
              [slicer.vtkMRMLIGTLConnectorNode.DisconnectedEvent, self.onConnectorNodeDisconnected]]
    for tagEventHandler in events:
      connectorNodeObserverTag = self.connectorNode.AddObserver(tagEventHandler[0], tagEventHandler[1])
      self.connectorNodeObserverTagList.append(connectorNodeObserverTag)

  def showUltrasoundSliceIn3DView(self):
    self.setUltrasoundSliceVisibilityIn3dView(True)

  def hideUltrasoundSliceIn3DView(self):
    self.setUltrasoundSliceVisibilityIn3dView(False)

  def setUltrasoundSliceVisibilityIn3dView(self, visible):
    redNode = slicer.mrmlScene.GetNodeByID('vtkMRMLSliceNodeRed')
    if visible:
      redNode.SetSliceVisible(1)
    else:
      redNode.SetSliceVisible(0)

  def fitUltrasoundImageToView(self):
    redWidget = self.layoutManager.sliceWidget('Red')
    redWidget.sliceController().fitSliceToBackground()

  def delayedFitUltrasoundImageToView(self, delayMsec=500):
    qt.QTimer.singleShot(delayMsec, self.fitUltrasoundImageToView)

  def delayedFitAndShowUltrasoundSliceIn3dView(self):
    self.delayedFitUltrasoundImageToView()
    self.showUltrasoundSliceIn3DView()

  def delayedFitAndHideUltrasoundSliceIn3dView(self):
    self.delayedFitUltrasoundImageToView()
    self.hideUltrasoundSliceIn3DView()

  def selectView(self, viewName):
    index = self.viewSelectorComboBox.findText(viewName)
    if index == -1:
      index = 0
    self.viewSelectorComboBox.setCurrentIndex(index)
    self.onViewSelect(index)

  def onViewSelect(self, layoutIndex):
    layoutName = self.viewSelectorComboBox.currentText
    logging.debug('onViewSelect: {0}'.format(layoutName))

    if layoutName not in self.layoutNameToIdMap:
      logging.error('Layout called ' + str(layoutName) + ' has not been registered to the guidelet')
      return

    layoutId = self.layoutNameToIdMap[layoutName]
    callback = self.layoutNameToSelectCallbackMap[layoutName]

    self.layoutManager.setLayout(layoutId)
    callback()

  def onUltrasoundPanelToggled(self, toggled):
    logging.debug('onUltrasoundPanelToggled: {0}'.format(toggled))
    if not toggled:
      # deactivate placement mode
      interactionNode = slicer.app.applicationLogic().GetInteractionNode()
      interactionNode.SetCurrentInteractionMode(interactionNode.ViewTransform)
      return

    self.showDefaultView()

  def showDefaultView(self):
    self.selectView(self.defaultLayoutName) # Red only layout by default
