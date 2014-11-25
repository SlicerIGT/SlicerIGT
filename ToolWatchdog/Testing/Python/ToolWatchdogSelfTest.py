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
    self.parent.contributors = ["Tamas Ungi (Queen's University) Jaime Garcia-Guevara (Queen's University)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is a self test for the tool watchdog module.
    """
    self.parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in
    Radiation Oncology (OCAIRO)
""" # replace with organization, grant and thanks.

#
# ToolWatchdogSelfTestWidget
#

class ToolWatchdogSelfTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

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
    self.reloadButton.name = "ToolWatchdogSelfTest Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    
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

  def onReloadAndTest(self,moduleName="ToolWatchdogSelfTest"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
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


class ToolWatchdogSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

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

    self.delayDisplay("Starting the test")

    scene = slicer.mrmlScene
    mainWindow = slicer.util.mainWindow()

    mainWindow.moduleSelector().selectModule('ToolWatchdog')
    watchdogWidget = slicer.modules.toolwatchdog.widgetRepresentation()

    i=0
    transforms=[]
    watchdogToolNodeCombobox = slicer.util.findChildren(widget=watchdogWidget, name='TransformComboBox')[0]  
    watchdogToolAddTransformButton = slicer.util.findChildren(widget=watchdogWidget, name='AddTransformButton')[0]

    while i<3:
        # Create transform node, add it to the scene and set it in the toolComboBox
        transformName = "Tool_"+str(i)
        transforms.append(slicer.vtkMRMLLinearTransformNode())
        transforms[i].SetName(slicer.mrmlScene.GenerateUniqueName(transformName))
        slicer.mrmlScene.AddNode(transforms[i])
        watchdogToolNodeCombobox.setCurrentNodeID(transforms[i].GetID())
        watchdogToolAddTransformButton.click()
        i=i+1

    self.delayDisplay('Wait...',1100)
    print(  watchdogToolNodeCombobox.nodeTypes )
    
    #add several transform put them on the tool watchdog list

    #verify if the tool watchdog incitator is updateing according to the transform update
    n=50
    b=0
    while b<2:
        b=b+1
        a=0
        while a < n:
            a=a+2
            toParent = vtk.vtkMatrix4x4()
            transforms[b].GetMatrixTransformToParent(toParent)
            toParent.SetElement(0 ,3, a)
            transforms[b].SetMatrixTransformToParent(toParent)
            if a>=n/2:
                if a==(n/2+10):
                    self.delayDisplay('Tool_'+str(b)+' should be up to date until watchdog time',6000)
                self.delayDisplay('Tool_'+str(b)+ 'should be up to date',100)
        self.delayDisplay('wait...',1100)
        self.delayDisplay('Tools should be out of date',5000)

    watchdogToolTransformsTableWidget = slicer.util.findChildren(widget=watchdogWidget, name='TransformsTableWidget')[0]
    square = qt.QRect()
    watchdogToolDeleteTransformButton = slicer.util.findChildren(widget=watchdogWidget, name='DeleteTransformButton')[0]
    square.setTopLeft(qt.QPoint(0,0))
    square.setBottomRight(qt.QPoint(0,0))
    i=0
    while i<3:
        watchdogToolTransformsTableWidget.setSelection(square,3)
        watchdogToolDeleteTransformButton.click()
        i=i+1
    self.delayDisplay('Deleted',5000/speed)


    #this adds a toolbar but is not needed
    #actionIcon = self.parent.icon
    #a = qt.QAction(actionIcon, 'Download Sample Data', slicer.util.mainWindow())
    #a.setToolTip('Go to the SampleData module to download data from the network')
    #a.connect('triggered()', self.select)
    #mainWin = slicer.util.mainWindow()
    #newToolBar = qt.QToolBar
    #mainWin.addToolBar(newToolBar)
    #fileMenu = slicer.util.lookupTopLevelWidget('FileMenu')
    #if fileMenu:
    #  for action in fileMenu.actions():
    #    if action.text == 'Save':
    #      fileMenu.insertAction(action,a)
    
    self.delayDisplay('Test passed!')
