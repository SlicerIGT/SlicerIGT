import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# BreachWarningSelfTest
#

class BreachWarningSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "BreachWarningSelfTest" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Testing.IGT Tests"]
    self.parent.dependencies = []
    self.parent.contributors = ["Tamas Ungi (Queen's University) Jaime Garcia-Guevara (Queen's University)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is a self test for the breach warning module.
    """
    self.parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in
    Radiation Oncology (OCAIRO)
""" 

#
# BreachWarningSelfTestWidget
#


class BreachWarningSelfTestWidget(ScriptedLoadableModuleWidget):

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
    self.reloadButton.name = "BreachWarningSelfTest Reload"
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
    
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = BreachWarningSelfTestLogic()
    print("Run the algorithm")
    logic.run()

  def onReload(self,moduleName="BreachWarningSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="BreachWarningSelfTest"):
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
# BreachWarningSelfTestLogic
#

class BreachWarningSelfTestLogic(ScriptedLoadableModuleLogic):
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


class BreachWarningSelfTestTest(ScriptedLoadableModuleTest):
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
    self.test_BreachWarningSelfTest1()

  def test_BreachWarningSelfTest1(self):
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

    mainWindow.moduleSelector().selectModule('CreateModels')
    createModelsWidget = slicer.modules.createmodels.widgetRepresentation()
    createSphereButton = slicer.util.findChildren(widget=createModelsWidget, className='QPushButton', name='CreateSphereButton')[0]
    createSphereButton.click()

    mainWindow.moduleSelector().selectModule('BreachWarning')
    bwWidget = slicer.modules.breachwarning.widgetRepresentation()
    
    bwColorPickerButton = slicer.util.findChildren(widget=bwWidget, className='ctkColorPickerButton', name='ColorPickerButton')[0]
    color = qt.QColor(255,0,0,1)
    bwColorPickerButton.setColor(color)

    modelNode = slicer.util.getNode(pattern="SphereModel")
    bwModelNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ModelNodeComboBox')[0]
    bwModelNodeCombobox.setCurrentNodeID(modelNode.GetID())

    # Create transform node
    transform = slicer.vtkMRMLLinearTransformNode()
    transform.SetName(slicer.mrmlScene.GenerateUniqueName("ToolTipToRAS"))
    slicer.mrmlScene.AddNode(transform)
    bwToolNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ToolComboBox')[0]  
    bwToolNodeCombobox.setCurrentNodeID(transform.GetID())

    self.delayDisplay('At the begining is inside',1000)
    colorD=modelNode.GetDisplayNode().GetColor()
    if colorD != (1.0, 0.0, 0.0):
        print("Error begin not same color INSIDE")
    print (colorD)
            
    a=0
    n=20
    while a < n:
        a=a+2
        toParent = vtk.vtkMatrix4x4()
        transform.GetMatrixTransformToParent(toParent)
        toParent.SetElement(0 ,3, a)
        transform.SetMatrixTransformToParent(toParent)
        if a>=10:
            if a==10:
                self.delayDisplay('It should be outside',3000)
            colorD=modelNode.GetDisplayNode().GetColor()
            if colorD == (1.0, 0.0, 0.0,):
                print("Error not same color OUTSIDE")
            print(colorD) 
    n=0
    while a > n:
        a=a-2
        toParent = vtk.vtkMatrix4x4()
        transform.GetMatrixTransformToParent(toParent)
        toParent.SetElement(0 ,3, a)
        transform.SetMatrixTransformToParent(toParent)

        if a<10:
            if a==8:
                self.delayDisplay('Now it should be inside',3000)
            colorD=modelNode.GetDisplayNode().GetColor()
            if colorD != (1.0, 0.0, 0.0):
                print("Error not same color INSIDE")
            print(colorD) 

    self.delayDisplay('Test passed!')
