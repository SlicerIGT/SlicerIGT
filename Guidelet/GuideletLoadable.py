import os
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

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
    self.selectedConfigurationName = 'Default'

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

  def addLauncherWidgets(self):#Overriding this you can add user preferences to the launcher widgets
    self.addConfigurationsSelector()
    self.addPlusServerPreferences()

  def addPlusServerPreferences(self):
    # PlusServer
    try:
      slicer.modules.plusremote
    except:
      self.errorLabel = qt.QLabel("Error: Could not find Plus Remote module. Please install the SlicerIGT extension.")
      self.layout.addWidget(self.errorLabel)
      return

    self.plusServerHostNamePortLineEdit = qt.QLineEdit()
    leLabel = qt.QLabel()
    leLabel.setText("Set the Plus Server Host and Name Port:")
    hbox = qt.QHBoxLayout()
    hbox.addWidget(leLabel)
    hbox.addWidget(self.plusServerHostNamePortLineEdit)
    self.launcherFormLayout.addRow(hbox)

    lnNode = slicer.util.getNode(self.moduleName)
    if lnNode is not None and lnNode.GetParameter('PlusServerHostNamePort'):
        #logging.debug("There is already a connector PlusServerHostNamePort parameter " + lnNode.GetParameter('PlusServerHostNamePort'))
        self.plusServerHostNamePortLineEdit.setDisabled(True)
        self.plusServerHostNamePortLineEdit.setText(lnNode.GetParameter('PlusServerHostNamePort'))
    else:
        #self.plusServerHostNamePortLineEdit.setDisabled(False)
        settings = slicer.app.userSettings()
        plusServerHostNamePort = settings.value(self.moduleName + '/Configurations/' + self.selectedConfigurationName + '/PlusServerHostNamePort')
        self.plusServerHostNamePortLineEdit.setText(plusServerHostNamePort)

    self.plusServerHostNamePortLineEdit.connect('editingFinished()', self.onPlusServerPreferencesChanged)

  # Adds a list box populated with the available configurations in the Slicer.ini file
  def addConfigurationsSelector(self):
    self.configurationsComboBox = qt.QComboBox()
    configurationsLabel = qt.QLabel("Select Configuration: ")
    hBox = qt.QHBoxLayout()
    hBox.addWidget(configurationsLabel)
    hBox.addWidget(self.configurationsComboBox)
    hBox.setStretch(1,2)
    self.launcherFormLayout.addRow(hBox)

    # Populate configurationsComboBox with available configurations
    settings = slicer.app.userSettings()
    settings.beginGroup(self.moduleName + '/Configurations')
    configurations = settings.childGroups()
    for configuration in configurations:
      self.configurationsComboBox.addItem(configuration)
    settings.endGroup()

    # Set latest used configuration
    if settings.value(self.moduleName + '/MostRecentConfiguration'):
      self.selectedConfigurationName = settings.value(self.moduleName + '/MostRecentConfiguration')
      idx = self.configurationsComboBox.findText(settings.value(self.moduleName + '/MostRecentConfiguration'))
      self.configurationsComboBox.setCurrentIndex(idx)

    self.configurationsComboBox.connect('currentIndexChanged(const QString &)', self.onConfigurationChanged)

  def onConfigurationChanged(self, selectedConfigurationName):
    self.selectedConfigurationName = selectedConfigurationName
    settings = slicer.app.userSettings()
    self.guideletLogic.updateSettings({'MostRecentConfiguration' : self.selectedConfigurationName})
    plusServerHostNamePort = settings.value(self.moduleName + '/Configurations/' + self.selectedConfigurationName + '/PlusServerHostNamePort')
    self.plusServerHostNamePortLineEdit.setText(plusServerHostNamePort)

  def cleanup(self):
    self.launchGuideletButton.disconnect('clicked()', self.onLaunchGuideletButtonClicked)
    if self.guideletLogic:
      self.guideletLogic.cleanup()
    if self.guideletInstance:
      self.guideletInstance.cleanup()

  def onLaunchGuideletButtonClicked(self):
    logging.debug('onLaunchGuideletButtonClicked')

    if not self.guideletInstance:
      self.guideletInstance = self.createGuideletInstance()
    self.guideletInstance.setupScene()
    self.guideletInstance.showFullScreen()

  def onPlusServerPreferencesChanged(self):
    self.guideletLogic.updateSettings({'PlusServerHostNamePort' : self.plusServerHostNamePortLineEdit.text}, self.selectedConfigurationName)

  def createGuideletInstance(self):
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

    self.setupDefaultConfiguration()

  def createParameterNode(self):
    node = ScriptedLoadableModuleLogic.createParameterNode(self)
    self.updateParameterNodeFromSettings(node, 'Default')
    return node

  def cleanup(self):
    pass

  def setupDefaultConfiguration(self):
    settings = slicer.app.userSettings()
    settings.beginGroup(self.moduleName + '/Configurations')
    childs = settings.childGroups()
    settings.endGroup()
    if not 'Default' in childs:
      self.addValuesToDefaultConfiguration()

  # Adds a default configurations to Slicer.ini
  def addValuesToDefaultConfiguration(self):
    moduleDir = os.path.dirname(__file__)
    defaultSavePath = os.path.join(moduleDir, 'SavedScenes')

    settingList = {'StyleSheet' : 'DefaultStyle.qss',
                   'LiveUltrasoundNodeName' : 'Image_Reference',
                   'LiveUltrasoundNodeName_Needle' : 'Image_Needle',
                   'PlusServerHostNamePort' : 'localhost:18944',
                   'RecordingFilenamePrefix' : 'GuideletRecording-',
                   'RecordingFilenameExtension' : '.mhd',
                   'SavedScenesDirectory' : defaultSavePath
                   }
    self.updateSettings(settingList, 'Default')

  def updateUserPreferencesFromParameterNode(self, settingsNameValueMap, paramNode):
    raise NotImplementedError("not implemented, to-do")

  def updateParameterNodeFromUserPreferences(self, paramNode, settingsNameValueMap):
    for name in settingsNameValueMap:
      paramNode.SetParameter(name, settingsNameValueMap[name])

  def updateParameterNodeFromSettings(self, paramNode, configurationName):
    settings = slicer.app.userSettings()
    settings.beginGroup(self.moduleName + '/Configurations/' + configurationName)
    keys = settings.allKeys()
    for key in keys:
      paramNode.SetParameter(key, settings.value(key))
    settings.endGroup()

  def updateSettings(self, settingsNameValueMap, configurationName = None):#updateSettingsFromUserPreferences
    settings = slicer.app.userSettings()
    if not configurationName:
      groupString = self.moduleName
    else:
      groupString = self.moduleName + '/Configurations/' + configurationName

    settings.beginGroup(groupString)
    for name in settingsNameValueMap:
      settings.setValue(name, settingsNameValueMap[name])
    settings.endGroup()

  def writeTransformToSettings(self, transformName, transformMatrix, configurationName):
    transformMatrixArray = []
    for r in xrange(4):
      for c in xrange(4):
        transformMatrixArray.append(transformMatrix.GetElement(r,c))
    transformMatrixString = ' '.join(map(str, transformMatrixArray)) # string, numbers are separated by spaces
    settings = slicer.app.userSettings()
    settingString = self.moduleName + '/Configurations/' + configurationName + '/{0}' # Write to selected configuration
    settings.setValue(settingString.format(transformName), transformMatrixString)

  def createMatrixFromString(self, transformMatrixString):
    transformMatrix = vtk.vtkMatrix4x4()
    transformMatrixArray = map(float, transformMatrixString.split(' '))
    for r in xrange(4):
      for c in xrange(4):
        transformMatrix.SetElement(r,c, transformMatrixArray[r*4+c])
    return transformMatrix

  def readTransformFromSettings(self, transformName, configurationName):
    transformMatrix = vtk.vtkMatrix4x4()
    settings = slicer.app.userSettings()
    settingString = self.moduleName + '/Configurations/' + configurationName + '/{0}' # Read from selected configuration
    transformMatrixString = settings.value(settingString.format(transformName))
    if not transformMatrixString:
      settingString = self.moduleName + '/Configurations/Default/{0}' # Read from default configuration
      transformMatrixString = settings.value(settingString.format(transformName))
      if not transformMatrixString:
        return None
    return self.createMatrixFromString(transformMatrixString)

#
# GuideletTest
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