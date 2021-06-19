import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# WatchdogSelfTest
# 

class WatchdogSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "WatchdogSelfTest"
    self.parent.categories = ["Testing.IGT Tests"]
    self.parent.dependencies = ["Watchdog"]
    self.parent.contributors = ["Andras Lasso, Tamas Ungi, Jaime Garcia-Guevara (Queen's University)"]
    self.parent.helpText = """This is a self test for the tool watchdog module."""
    self.parent.acknowledgementText = """This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)"""

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['WatchdogSelfTest'] = self.runTest

  def runTest(self):
    tester = WatchdogSelfTestTest()
    tester.runTest()
    
#
# WatchdogSelfTestWidget
#

class WatchdogSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
#
# WatchdogSelfTestLogic
#
class WatchdogSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

#
# WatchdogSelfTestTest
#
class WatchdogSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)
    
    self.transformNode1 = slicer.vtkMRMLTransformNode()
    self.transformNode1.SetName("Transform 1")
    slicer.mrmlScene.AddNode(self.transformNode1)
    
    self.transformNode2 = slicer.vtkMRMLTransformNode()
    self.transformNode2.SetName("Transform 2")
    slicer.mrmlScene.AddNode(self.transformNode2)
    
    self.transformNode3 = slicer.vtkMRMLTransformNode()
    self.transformNode3.SetName("Transform 3")
    slicer.mrmlScene.AddNode(self.transformNode3)

    self.delayMs = 700

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_WatchdogSelfTest1()

  def test_WatchdogSelfTest1(self):
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

    slicer.util.delayDisplay("Test Watchdog widget buttons",self.delayMs)

    slicer.util.selectModule('Watchdog')
    watchdogModuleWidget = slicer.modules.watchdog.widgetRepresentation()

    watchdogNodeSelectorComboBox = slicer.util.findChildren(widget=watchdogModuleWidget,name='ModuleNodeComboBox')[0]
    toolComboBox = slicer.util.findChildren(widget=watchdogModuleWidget,name='ToolComboBox')[0]
    addToolButton = slicer.util.findChildren(widget=watchdogModuleWidget,name='AddToolButton')[0]
    deleteToolButton = slicer.util.findChildren(widget=watchdogModuleWidget,name='DeleteToolButton')[0]
    toolsTableWidget = slicer.util.findChildren(widget=watchdogModuleWidget,name='ToolsTableWidget')[0]
    visibilityCheckBox = slicer.util.findChildren(widget=watchdogModuleWidget,name='VisibilityCheckBox')[0]
    displayNodeViewComboBox = slicer.util.findChildren(widget=watchdogModuleWidget,name='DisplayNodeViewComboBox')[0]

    # Check if a watchdog node is created by default
    watchdogNode = watchdogNodeSelectorComboBox.currentNode()
    self.assertTrue( watchdogNode is not None )    
    
    # Check node add
    slicer.util.delayDisplay("Test add watched nodes",self.delayMs)

    toolComboBox.setCurrentNodeID(self.transformNode1.GetID())
    addToolButton.click()
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 1 )

    toolComboBox.setCurrentNodeID(self.transformNode2.GetID())
    addToolButton.click()
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 2 )
    
    toolComboBox.setCurrentNodeID(self.transformNode3.GetID())
    addToolButton.click()
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 3 )

    # Check node remove
    slicer.util.delayDisplay("Test remove watched nodes",self.delayMs)
    # Using GUI
    toolsTableWidget.selectRow(0) # transformNode1
    deleteToolButton.click()
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 2 )
    # Through MRML scene    
    slicer.mrmlScene.RemoveNode(self.transformNode2)
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 1 )
    # Add back the two removed nodes
    slicer.mrmlScene.AddNode(self.transformNode2)
    watchdogNode.AddWatchedNode(self.transformNode1)
    watchdogNode.AddWatchedNode(self.transformNode2)
    self.assertEqual( watchdogNode.GetNumberOfWatchedNodes(), 3 )
    
    # Check table contents
    # Watched node order should be: transformNode3, transformNode1, transformNode2
    slicer.util.delayDisplay("Test watched node table contents",self.delayMs)
    watchedNodeIndex=1
    # Node name column
    self.assertEqual(toolsTableWidget.item(watchedNodeIndex,0).text(), 'Transform 1')
    watchdogNode.GetWatchedNode(watchedNodeIndex).SetName('Transform 1x')
    slicer.util.delayDisplay("Wait for the node name to update",1000.0)
    self.assertEqual(toolsTableWidget.item(watchedNodeIndex,0).text(), 'Transform 1x')
    # Warning text column
    self.assertEqual(toolsTableWidget.item(watchedNodeIndex,1).text(), 'Transform 1 is not up-to-date')
    watchdogNode.SetWatchedNodeWarningMessage(watchedNodeIndex, 'outdated')
    self.assertEqual(toolsTableWidget.item(watchedNodeIndex,1).text(), 'outdated')
    # Sound column
    soundCheckBox = slicer.util.findChildren(widget=toolsTableWidget.cellWidget(watchedNodeIndex,2),name='Sound')[0]
    self.assertEqual(soundCheckBox.checked, False)
    watchdogNode.SetWatchedNodePlaySound(watchedNodeIndex, True)
    self.assertEqual(soundCheckBox.checked, True)
    # Status column
    statusCheckBox = slicer.util.findChildren(widget=toolsTableWidget.cellWidget(watchedNodeIndex,3),name='StatusIcon')[0]
    slicer.util.delayDisplay("Wait for the node to become outdated",1000.0)
    self.assertEqual(statusCheckBox.toolTip, "<p>invalid</p>")
    isValid = False
    for i in range(10):
      # In theory, the tooltip should be updated immediately, and this is what happens in Slicer
      # however, in the test environment, the field isn't changed until a bit later, so we check multiple times
      # It's not perfect, but otherwise this test fails when the mechanism is actually working
      slicer.util.delayDisplay("Wait for the transform name to update", 100.0)
      if statusCheckBox.toolTip == "<p>valid</p>":
        isValid = True
        break
    self.assertEqual(isValid, True)
    slicer.util.delayDisplay("Wait for the node to become outdated",1000.0)
    self.assertEqual(statusCheckBox.toolTip, "<p>invalid</p>")

    #remove module node
    slicer.mrmlScene.RemoveNode(watchdogNode)
    self.assertEqual(toolsTableWidget.rowCount, 0)
    
    slicer.util.delayDisplay('Test passed!')
