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

    speedLabel = qt.QLabel()
    speedLabel.setText("Speed to execute the test")
    self.speedSpinBox = qt.QSpinBox()
    self.speedSpinBox
    self.speedSpinBox.toolTip = "Speed to execute the test."
    self.speedSpinBox.name = "WatchdogSelfTest Speed"
    self.speedSpinBox.setValue(3)
    hLayout =qt.QHBoxLayout()
    hLayout.addStretch(1)
    hLayout.addWidget(speedLabel)
    hLayout.addWidget(self.speedSpinBox)
    self.layout.addLayout(hLayout)

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

  def setSpeed(self,moduleName="BreachWarningSelfTest"):
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="BreachWarningSelfTest"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.setSpeed(self.speedSpinBox.value)
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
  def setSpeed(self, speed=3):
    self.Speed=speed

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
    createCylinderButton = slicer.util.findChildren(widget=createModelsWidget, className='QPushButton')[1]
    createCylinderButton.click()

    mainWindow.moduleSelector().selectModule('BreachWarning')
    bwWidget = slicer.modules.breachwarning.widgetRepresentation()
    
    bwColorPickerButton = slicer.util.findChildren(widget=bwWidget, className='ctkColorPickerButton', name='ColorPickerButton')[0]
    color = qt.QColor(255,0,0,1)
    bwColorPickerButton.setColor(color)

    modelNodes=[]
    modelNodes.append(slicer.util.getNode(pattern="SphereModel"))
    modelNodes.append(slicer.util.getNode(pattern="CylinderModel"))
    bwModelNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ModelNodeComboBox')[0]
    bwModelNodeCombobox.setCurrentNodeID(modelNodes[0].GetID())

    bwToolNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ToolComboBox')[0]  
    # Create transforms node
    i=0
    transforms=[]
    while i<2:
        transforms.append(slicer.vtkMRMLLinearTransformNode())
        transformName = "Tool_"+str(i)
        transforms[i].SetName(slicer.mrmlScene.GenerateUniqueName(transformName))
        slicer.mrmlScene.AddNode(transforms[i])
        i=i+1
    bwToolNodeCombobox.setCurrentNodeID(transforms[0].GetID())

    #Modify second transform to be outside
    toParent1 = vtk.vtkMatrix4x4()
    transforms[1].GetMatrixTransformToParent(toParent1)
    toParent1.SetElement(0 ,3, 100)
    transforms[1].SetMatrixTransformToParent(toParent1)


    self.delayDisplay('At the begining Tool_1 is INSIDE the sphere',3000/self.Speed)
    colorD=modelNodes[0].GetDisplayNode().GetColor()
    if colorD != (1.0, 0.0, 0.0):
        print("1) Error begin not same color INSIDE")
    print (colorD)
            


    numberOfModels=1
    while numberOfModels>=0:
        bwModelNodeCombobox.setCurrentNodeIndex(numberOfModels)

       #switch tools
        i=1
        while i>=0:
            bwToolNodeCombobox.setCurrentNodeID(transforms[i].GetID())
            colorD=modelNodes[numberOfModels].GetDisplayNode().GetColor()
            if(i==0):
                if colorD != (1.0, 0.0, 0.0):
                    print("2) Error begin not same color INSIDE "+'Tool_'+str( i))
                self.delayDisplay('Switch tools while INSIDE '+'Tool_'+str( i),3000/self.Speed)
            if (i==1):
                if colorD == (1.0, 0.0, 0.0):
                    print("3) Error begin not same color OUTSIDE "+'Tool_'+str( i))
                self.delayDisplay('Switch tools while OUTSIDE '+'Tool_'+str( i),3000/self.Speed)
            i=i-1

        self.delayDisplay('Switch model while the tool it is INSIDE ',3000/self.Speed)
        a=0
        n=20

        limits=[]
        limits.append(10)
        limits.append(12)
        while a < n:
            a=a+2
            toParent = vtk.vtkMatrix4x4()
            transforms[0].GetMatrixTransformToParent(toParent)
            toParent.SetElement(0 ,3, a)
            transforms[0].SetMatrixTransformToParent(toParent)
            if a>=limits[numberOfModels]:
                if a==limits[numberOfModels]:
                    self.delayDisplay('It should be OUTSIDE Model_'+str(numberOfModels),3000/self.Speed)
                colorD=modelNodes[numberOfModels].GetDisplayNode().GetColor()
                if colorD == (1.0, 0.0, 0.0,):
                    print('4) Error not same color OUTSIDE Model_' +str(numberOfModels))
                print(colorD) 
        n=0
        while a > n:
            a=a-2
            toParent = vtk.vtkMatrix4x4()
            transforms[0].GetMatrixTransformToParent(toParent)
            toParent.SetElement(0 ,3, a)
            transforms[0].SetMatrixTransformToParent(toParent)

            if a<10:
                if a==8:
                    self.delayDisplay('Now it should be INSIDE Model_'+str(numberOfModels),3000/self.Speed)
                colorD=modelNodes[numberOfModels].GetDisplayNode().GetColor()
                if colorD != (1.0, 0.0, 0.0):
                    print("5) Error not same color INSIDE Model_" +str(numberOfModels))
                print(colorD) 
        numberOfModels=numberOfModels-1

    self.delayDisplay('Test passed!')
