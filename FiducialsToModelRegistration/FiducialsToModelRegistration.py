import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# FiducialsToModelRegistration
#

class FiducialsToModelRegistration(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "FiducialsToModelRegistration" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Examples"]
    self.parent.dependencies = []
    self.parent.contributors = ["John Doe (AnyWare Corp.)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.

#
# FiducialsToModelRegistrationWidget
#

class FiducialsToModelRegistrationWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # input fiducial list selector
    #
    self.inputFiducialSelector = slicer.qMRMLNodeComboBox()
    self.inputFiducialSelector.nodeTypes = ( ("vtkMRMLMarkupsFiducialNode"), "" )
    self.inputFiducialSelector.selectNodeUponCreation = True
    self.inputFiducialSelector.addEnabled = False
    self.inputFiducialSelector.removeEnabled = False
    self.inputFiducialSelector.noneEnabled = False
    self.inputFiducialSelector.showHidden = False
    self.inputFiducialSelector.showChildNodeTypes = False
    self.inputFiducialSelector.setMRMLScene( slicer.mrmlScene )
    self.inputFiducialSelector.setToolTip( "Pick the input fiducial list for the algorithm." )
    parametersFormLayout.addRow("Input fiducials: ", self.inputFiducialSelector)

    #
    # input model selector
    #
    self.inputModelSelector = slicer.qMRMLNodeComboBox()
    self.inputModelSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.inputModelSelector.selectNodeUponCreation = True
    self.inputModelSelector.addEnabled = False
    self.inputModelSelector.removeEnabled = False
    self.inputModelSelector.noneEnabled = False
    self.inputModelSelector.showHidden = False
    self.inputModelSelector.showChildNodeTypes = False
    self.inputModelSelector.setMRMLScene( slicer.mrmlScene )
    self.inputModelSelector.setToolTip( "Pick the input model for the algorithm." )
    parametersFormLayout.addRow("Input model: ", self.inputModelSelector)

    #
    # output transform selector
    #
    self.outputSelector = slicer.qMRMLNodeComboBox()
    self.outputSelector.nodeTypes = ( ("vtkMRMLLinearTransformNode"), "" )
    self.outputSelector.selectNodeUponCreation = True
    self.outputSelector.addEnabled = True
    self.outputSelector.removeEnabled = True
    self.outputSelector.noneEnabled = False
    self.outputSelector.showHidden = False
    self.outputSelector.showChildNodeTypes = False
    self.outputSelector.setMRMLScene( slicer.mrmlScene )
    self.outputSelector.setToolTip( "Pick the output to the algorithm." )
    parametersFormLayout.addRow("Output transform: ", self.outputSelector)

    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
    self.enableScreenshotsFlagCheckBox.checked = 0
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    # parametersFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # scale factor for screen shots
    #
    self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
    self.screenshotScaleFactorSliderWidget.singleStep = 1.0
    self.screenshotScaleFactorSliderWidget.minimum = 1.0
    self.screenshotScaleFactorSliderWidget.maximum = 50.0
    self.screenshotScaleFactorSliderWidget.value = 1.0
    self.screenshotScaleFactorSliderWidget.setToolTip("Set scale factor for the screen shots.")
    #parametersFormLayout.addRow("Screenshot scale factor", self.screenshotScaleFactorSliderWidget)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = False
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)
    self.inputModelSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.inputFiducialSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.outputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputModelSelector.currentNode() and self.outputSelector.currentNode() and self.inputFiducialSelector.currentNode()

  def onApplyButton(self):
    logic = FiducialsToModelRegistrationLogic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)
    print("Run the algorithm")
    logic.run(self.inputFiducialSelector.currentNode(), self.inputModelSelector.currentNode(), self.outputSelector.currentNode(), enableScreenshotsFlag,screenshotScaleFactor)


#
# FiducialsToModelRegistrationLogic
#

class FiducialsToModelRegistrationLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True

  def run(self, inputFiducials, inputModel, outputTransform, enableScreenshots=0, screenshotScaleFactor=1):
    """
    Run the actual algorithm
    """

    self.delayDisplay('Running the aglorithm')

    fiducialsPolyData = vtk.vtkPolyData()
    self.FiducialsToPolyData(inputFiducials, fiducialsPolyData)

    icpTransform = vtk.vtkIterativeClosestPointTransform()
    icpTransform.SetSource( fiducialsPolyData )
    icpTransform.SetTarget( inputModel.GetPolyData() )
    icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    icpTransform.SetMaximumNumberOfIterations( 100 )
    icpTransform.Modified()
    icpTransform.Update()

    print 'Number of source points'
    print fiducialsPolyData.GetNumberOfPoints()

    print "The resulting transform is"
    print icpTransform.GetMatrix()

    outputTransform.SetMatrixTransformToParent( icpTransform.GetMatrix() )

    return True

  def FiducialsToPolyData(self, fiducials, polyData):

    points = vtk.vtkPoints()
    n = fiducials.GetNumberOfFiducials()
    for fiducialIndex in range( 0, n ):
      p = [0, 0, 0]
      fiducials.GetNthFiducialPosition( fiducialIndex, p )
      points.InsertNextPoint( p )

    tempPolyData = vtk.vtkPolyData()
    tempPolyData.SetPoints( points )

    vertex = vtk.vtkVertexGlyphFilter()
    vertex.SetInputData( tempPolyData )
    vertex.Update()

    polyData.ShallowCopy( vertex.GetOutput() )


class FiducialsToModelRegistrationTest(ScriptedLoadableModuleTest):
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
    self.test_FiducialsToModelRegistration1()

  def test_FiducialsToModelRegistration1(self):
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
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = FiducialsToModelRegistrationLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
