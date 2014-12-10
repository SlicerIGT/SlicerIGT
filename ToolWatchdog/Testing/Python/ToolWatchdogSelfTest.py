import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# ToolWatchdogSelfTest
# 

class ToolWatchdogSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "ToolWatchdogSelfTest" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Testing.IGT Tests"]
    self.parent.dependencies = []
    self.parent.contributors = ["Tamas Ungi (Queen's University) Jaime Garcia-Guevara (Queen's University)"]
    self.parent.helpText = """
    This is a self test for the tool watchdog module.
    """
    self.parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in
    Radiation Oncology (OCAIRO)
"""

#
# ToolWatchdogSelfTestWidget
#

class ToolWatchdogSelfTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...


    self.usePlusServerCheckBox = qt.QCheckBox()
    self.usePlusServerCheckBox.setText("Use plus server")
    speedLabel = qt.QLabel()
    speedLabel.setText("Speed to execute the test")
    self.speedSpinBox = qt.QSpinBox()
    self.speedSpinBox
    self.speedSpinBox.toolTip = "Speed to execute the test."
    self.speedSpinBox.name = "ToolWatchdogSelfTest Speed"
    self.speedSpinBox.setValue(3)
    hLayout =qt.QHBoxLayout()
    hLayout.addStretch(1)
    hLayout.addWidget(self.usePlusServerCheckBox)
    #hLayout.addItem(qt.QSpacerItem(20, 40, qt.QSizePolicy.Minimum, qt.QSizePolicy.Expanding))
    hLayout.addWidget(speedLabel)
    hLayout.addWidget(self.speedSpinBox)
    self.layout.addLayout(hLayout)
    #self.speedSpinBox.connect('changed()', self.onSpeedChanged)
    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = ToolWatchdogSelfTestLogic()
    print("Run the algorithm")
    logic.run()

  def onReload(self,moduleName="ToolWatchdogSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def setSpeed(self,moduleName="ToolWatchdogSelfTest"):
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)


  def onReloadAndTest(self,moduleName="ToolWatchdogSelfTest"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.setSpeed(self.speedSpinBox.value)
      tester.setUsePlusServer(self.usePlusServerCheckBox.isChecked())
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")

#
# ToolWatchdogSelfTestLogic
#
class ToolWatchdogSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def run(self):
    """
    Run the actual algorithm
    """
    print("Test the algorith logic here, not implemented yet")

#
# ToolWatchdogSelfTestTest
#
class ToolWatchdogSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """
  def setSpeed(self, speed=3):
      self.Speed=speed

  def setUsePlusServer(self, usePlusServer=False):
      self.UsePlusServer=usePlusServer

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_ToolWatchdogSelfTest1()

  def test_ToolWatchdogSelfTest1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test at speed "+ str(self.Speed))
    scene = slicer.mrmlScene
    mainWindow = slicer.util.mainWindow()

    mainWindow.moduleSelector().selectModule('ToolWatchdog')
    watchdogWidget = slicer.modules.toolwatchdog.widgetRepresentation()

    tools=[]
    watchdogToolNodeCombobox = slicer.util.findChildren(widget=watchdogWidget, name='ToolComboBox')[0]  
    watchdogToolAddToolButton = slicer.util.findChildren(widget=watchdogWidget, name='AddToolButton')[0]
    #if plus server connected open IGTLink
    if(self.UsePlusServer):
        openIGTLinkIFWidget = slicer.modules.openigtlinkif.widgetRepresentation()
        mainWindow.moduleSelector().selectModule('OpenIGTLinkIF')
        openIGTLinkIFAddToolButton = slicer.util.findChildren(widget=openIGTLinkIFWidget, name='AddConnectorButton')[0]
        openIGTLinkIFAddToolButton.click()
        openIGTLinkIFStatusCheckbox = slicer.util.findChildren(widget=openIGTLinkIFWidget, name='ConnectorStateCheckBox')[0]
        openIGTLinkIFStatusCheckbox.click()
        self.delayDisplay('Wait...to add IGT link',3000)
        mainWindow.moduleSelector().selectModule('ToolWatchdog')
        i=0
        while i<3:
            watchdogToolNodeCombobox.setCurrentNodeIndex(i)
            watchdogToolAddToolButton.click()
            i=i+1
    
    #add several transforms put them on the tool watchdog list
    i=0
    while i<3:
        # Create transform node, add it to the scene and set it in the toolComboBox
        transformName = "Tool_"+str(i)
        tools.append(slicer.vtkMRMLLinearTransformNode())
        tools[i].SetName(slicer.mrmlScene.GenerateUniqueName(transformName))
        slicer.mrmlScene.AddNode(tools[i])
        watchdogToolNodeCombobox.setCurrentNodeID(tools[i].GetID())
        watchdogToolAddToolButton.click()
        i=i+1

    self.delayDisplay('Wait...', 1100/self.Speed)
    print( watchdogToolNodeCombobox.nodeTypes )

    #Get gui widgets, move two nodes items in the list 
    watchdogToolToolsTableWidget = slicer.util.findChildren(widget=watchdogWidget, name='ToolsTableWidget')[0]
    watchdogToolToolsTableWidget.selectRow(3)
    self.delayDisplay('Table selection up 3rd row...', 2100)
    watchdogToolUpToolButton = slicer.util.findChildren(widget=watchdogWidget, name='UpToolButton')[0]
    watchdogToolUpToolButton.click()
    watchdogToolDownToolButton = slicer.util.findChildren(widget=watchdogWidget, name='DownToolButton')[0]
    watchdogToolToolsTableWidget.selectRow(0)
    self.delayDisplay('Table selection down first element...', 2100)
    watchdogToolDownToolButton.click()

    #if PlusServer used delete two of the nodes and move the one left to the bottom of the list
    watchdogToolDeleteToolButton = slicer.util.findChildren(widget=watchdogWidget, name='DeleteToolButton')[0]
    if(self.UsePlusServer):
        i=0
        while i<3:
            watchdogToolToolsTableWidget.selectRow(i*2)
            watchdogToolDeleteToolButton.click()
            i=i+1
        i=0
        while i<3:
            watchdogToolToolsTableWidget.selectRow(i)
            watchdogToolDownToolButton.click()
            i=i+1

    #put the status labels in a container to check if they update correctly
    i=0
    it=[]
    while i<3:
        it.append(watchdogToolToolsTableWidget.item(i,3))
        i=i+1
    if(self.UsePlusServer):
        it.append(watchdogToolToolsTableWidget.item(4,3))

    #verify if the tool watchdog indicator is updating according to the transform update
    n=50
    b=0
    while b<2:
        b=b+1
        a=0
        while a < n:
            a=a+2
            toParent = vtk.vtkMatrix4x4()
            tools[b].GetMatrixTransformFromParent(toParent)
            toParent.SetElement(0 ,3, a)
            tools[b].SetMatrixTransformFromParent(toParent)
            
            if a>=n/2:
                if a==(n/2+10):
                    self.delayDisplay('Tool_'+str(b)+' should be up to date until watchdog time',6000/self.Speed)
                    if(it[b].background().color().name()!='#2de05a' & it[4].background().color().name()!='#2de05a'):
                        print("Error not same color ON "+'Tool_'+str(b))
                    if(it[0].background().color().name()!='#ff0000'):
                        print("Error not same color OFF "+'Tool_0')
                self.delayDisplay('Tool_'+str(b)+ ' should be up to date',100)
        self.delayDisplay('wait...',1100/self.Speed)
        self.delayDisplay('Tools should be out of date',5000/self.Speed)
        i=0
        while i<3:
            if(it[i].background().color().name()!='#ff0000'):
                print("Error not same color OFF "+'Tool_'+str(i))
            i=i+1
    
    #Delete all items on the list
    watchdogToolToolsTableWidget.selectAll()
    watchdogToolDeleteToolButton.click()
    self.delayDisplay('Deleted',5000/self.Speed)
    self.delayDisplay('Test passed!')
