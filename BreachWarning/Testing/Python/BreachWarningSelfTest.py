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
    self.parent.title = "BreachWarningSelfTest"
    self.parent.categories = ["Testing.IGT Tests"]
    self.parent.dependencies = ["CreateModels", "BreachWarning"]
    self.parent.contributors = ["Tamas Ungi, Jaime Garcia-Guevara, Andras Lasso (Queen's University)"]
    self.parent.helpText = """This is a self test for the breach warning module."""
    self.parent.acknowledgementText = """This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)"""

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['BreachWarningSelfTest'] = self.runTest

  def runTest(self):
    tester = BreachWarningSelfTestTest()
    tester.runTest()

#
# BreachWarningSelfTestWidget
#

class BreachWarningSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

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
  def __init__(self):
    pass

class BreachWarningSelfTestTest(ScriptedLoadableModuleTest):
  """This is the test case for your scripted module.
  """

  def setUp(self):
    """Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_BreachWarningSelfTest1()

  def test_BreachWarningSelfTest1(self):
    """Ideally you should have several levels of tests.  At the lowest level
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

    self.delayDisplay("Create models")
    modelNodes = []
    createModelsLogic = slicer.modules.createmodels.logic()
    sphereRadius = 10.0
    sphereModel = createModelsLogic.CreateSphere(sphereRadius) # watched model
    toolModel = createModelsLogic.CreateNeedle(50.0, 1.5, 0.0, False)

    self.delayDisplay("Set up module GUI")
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('BreachWarning')
    bwWidget = slicer.modules.breachwarning.widgetRepresentation()
    bwColorPickerButton = slicer.util.findChildren(widget=bwWidget, className='ctkColorPickerButton', name='ColorPickerButton')[0]
    bwModelNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ModelNodeComboBox')[0]
    bwToolNodeCombobox = slicer.util.findChildren(widget=bwWidget, name='ToolComboBox')[0]

    bwModelNodeCombobox.setCurrentNodeID(sphereModel.GetID())

    warningColor = (1.0,0.0,0.0)
    color = qt.QColor(warningColor[0]*255,warningColor[1]*255,warningColor[2]*255,1)
    bwColorPickerButton.setColor(color)

    # Transform the sphere somewhere
    sphereTransform = slicer.vtkMRMLLinearTransformNode()
    sphereTransformMatrix = vtk.vtkMatrix4x4()
    sphereTransformMatrix.SetElement(0, 3, 80)
    sphereTransformMatrix.SetElement(1, 3, 40)
    sphereTransformMatrix.SetElement(2, 3, 30)
    sphereTransform.SetMatrixTransformToParent(sphereTransformMatrix)
    slicer.mrmlScene.AddNode(sphereTransform)
    sphereModel.SetAndObserveTransformNodeID(sphereTransform.GetID())

    # Create transforms node hierarchy for the tool
    transforms=[]
    numberOfTransforms = 3
    for i in range(numberOfTransforms):
        transforms.append(slicer.vtkMRMLLinearTransformNode())
        transformName = "Tool_"+str(i)
        transforms[i].SetName(slicer.mrmlScene.GenerateUniqueName(transformName))
        slicer.mrmlScene.AddNode(transforms[i])
        if i>0:
          transforms[i].SetAndObserveTransformNodeID(transforms[i-1].GetID())

    # Tool transform is the one at the bottom of the transform hierarchy
    # (to make sure transform changes in the middle of the transform hierarchy are used correctly)
    toolModel.SetAndObserveTransformNodeID(transforms[-1].GetID())
    bwToolNodeCombobox.setCurrentNodeID(transforms[-1].GetID())

    # Pick a transform in the middle of the transform hierarchy that we change to simulate tool motion,
    # leave the rest of the transforms unchanged
    toolToWorldTransform = transforms[1]

    transformMatrixOutside = vtk.vtkMatrix4x4()
    transformMatrixOutside.DeepCopy(sphereTransformMatrix)
    transformMatrixOutside.SetElement(0, 3, transformMatrixOutside.GetElement(0,3) + sphereRadius*2.1)
    transformMatrixOutside.SetElement(1, 3, transformMatrixOutside.GetElement(1,3) + sphereRadius*1.3)
    transformMatrixOutside.SetElement(2, 3, transformMatrixOutside.GetElement(2, 3) + sphereRadius*3.2)

    transformMatrixInside = vtk.vtkMatrix4x4()
    transformMatrixInside.DeepCopy(sphereTransformMatrix)
    transformMatrixInside.SetElement(0, 3, transformMatrixInside.GetElement(0,3) + sphereRadius*0.1)
    transformMatrixInside.SetElement(1, 3, transformMatrixInside.GetElement(1,3) + sphereRadius*0.3)
    transformMatrixInside.SetElement(2, 3, transformMatrixInside.GetElement(2,3) + sphereRadius*0.2)

    # Start breach warning checks

    self.delayDisplay('Tool is outside the sphere')
    toolToWorldTransform.SetMatrixTransformToParent(transformMatrixOutside)
    sphereColor = sphereModel.GetDisplayNode().GetColor()
    self.assertNotEqual(sphereColor, warningColor)

    self.delayDisplay('Tool is inside the sphere')
    toolToWorldTransform.SetMatrixTransformToParent(transformMatrixInside)
    sphereColor = sphereModel.GetDisplayNode().GetColor()
    self.assertEqual(sphereColor, warningColor)

    self.delayDisplay('Tool is outside the sphere')
    toolToWorldTransform.SetMatrixTransformToParent(transformMatrixOutside)
    sphereColor = sphereModel.GetDisplayNode().GetColor()
    self.assertNotEqual(sphereColor, warningColor)

    self.delayDisplay('Test passed!')
