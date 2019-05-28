import os
import unittest
import math
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# FiducialsToModelRegistration
#

class FiducialsToModelRegistration(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Fiducials-Model Registration" # TODO make this more human readable by adding spaces
    self.parent.categories = ["IGT"]
    self.parent.dependencies = []
    self.parent.contributors = ["Tamas Ungi (Queen's University"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This module applies Iterative Closest Points registration from a fiducial list to a model surface.
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
    fiducialWarningLabel = qt.QLabel( "Note: Parent transforms of fiducials are not used. Fiducials should be defined in the coordinate system that is being registered." )
    fiducialWarningLabel.setWordWrap( True )
    parametersFormLayout.addRow(fiducialWarningLabel)

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
    self.outputSelector.renameEnabled = True
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

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = False
    parametersFormLayout.addRow(self.applyButton)

    #
    # Output panel
    #
    outputCollapsibleButton = ctk.ctkCollapsibleButton()
    outputCollapsibleButton.text = "Output"
    self.layout.addWidget( outputCollapsibleButton )
    outputFormLayout = qt.QFormLayout( outputCollapsibleButton )

    self.outputLine = qt.QLineEdit()
    self.outputLine.setReadOnly( True )
    outputFormLayout.addRow( "Mean distance after registration:", self.outputLine )

    #
    # Advanced parameters
    #
    advancedCollapsibleButton = ctk.ctkCollapsibleButton()
    advancedCollapsibleButton.text = "Advanced"
    self.layout.addWidget(advancedCollapsibleButton)

    # Layout
    advancedCollapsibleButton.collapsed = True
    advancedFormLayout = qt.QFormLayout(advancedCollapsibleButton)

    #
    # Transform type selector
    #
    self.typeSelector = qt.QComboBox()
    self.typeSelector.insertItem( 0, "Rigid" )
    self.typeSelector.insertItem( 1, "Similarity" )
    self.typeSelector.insertItem( 2, "Affine" )
    advancedFormLayout.addRow("Transform type: ", self.typeSelector)

    #
    # Iteration selector
    #
    self.iterationSpin = qt.QSpinBox()
    self.iterationSpin.setMaximum( 1000 )
    self.iterationSpin.setValue( 100 )
    advancedFormLayout.addRow("Number of iterations:", self.iterationSpin)

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

    inputFiducials = self.inputFiducialSelector.currentNode()
    inputModel = self.inputModelSelector.currentNode()
    outputTransform = self.outputSelector.currentNode()

    logic.run(inputFiducials, inputModel, outputTransform, self.typeSelector.currentIndex, self.iterationSpin.value )

    self.outputLine.setText( logic.ComputeMeanDistance(inputFiducials, inputModel, outputTransform) )

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

  def run(self, inputFiducials, inputModel, outputTransform, transformType=0, numIterations=100 ):

    self.delayDisplay('Running iterative closest point registration')

    fiducialsPolyData = vtk.vtkPolyData()
    self.FiducialsToPolyData(inputFiducials, fiducialsPolyData)

    icpTransform = vtk.vtkIterativeClosestPointTransform()
    icpTransform.SetSource( fiducialsPolyData )
    icpTransform.SetTarget( inputModel.GetPolyData() )
    icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    if transformType == 1:
      icpTransform.GetLandmarkTransform().SetModeToSimilarity()
    if transformType == 2:
      icpTransform.GetLandmarkTransform().SetModeToAffine()
    icpTransform.SetMaximumNumberOfIterations( numIterations )
    icpTransform.Modified()
    icpTransform.Update()

    outputTransform.SetMatrixTransformToParent( icpTransform.GetMatrix() )
    if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion >= 4 and slicer.app.minorVersion >= 11):
      outputTransform.AddNodeReferenceID(slicer.vtkMRMLTransformNode.GetMovingNodeReferenceRole(), inputFiducials.GetID())
      outputTransform.AddNodeReferenceID(slicer.vtkMRMLTransformNode.GetFixedNodeReferenceRole(), inputModel.GetID())

    return True


  def ComputeMeanDistance(self, inputFiducials, inputModel, transform ):
    surfacePoints = vtk.vtkPoints()
    cellId = vtk.mutable(0)
    subId = vtk.mutable(0)
    dist2 = vtk.mutable(0.0)
    locator = vtk.vtkCellLocator()
    locator.SetDataSet( inputModel.GetPolyData() )
    locator.SetNumberOfCellsPerBucket( 1 )
    locator.BuildLocator()
    totalDistance = 0.0

    n = inputFiducials.GetNumberOfFiducials()
    m = vtk.vtkMath()
    for fiducialIndex in range( 0, n ):
      originalPoint = [0, 0, 0]
      inputFiducials.GetNthFiducialPosition( fiducialIndex, originalPoint )
      transformedPoint = [0, 0, 0, 1]
      #transform.GetTransformToParent().TransformVector( originalPoint, transformedPoint )
      originalPoint.append(1)
      transform.GetTransformToParent().MultiplyPoint( originalPoint, transformedPoint )
      #transformedPoints.InsertNextPoint( transformedPoint )
      surfacePoint = [0, 0, 0]
      transformedPoint.pop()
      locator.FindClosestPoint( transformedPoint, surfacePoint, cellId, subId, dist2 )
      totalDistance = totalDistance + math.sqrt(dist2)

    return ( totalDistance / n )


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

    self.delayDisplay('Tests are not implemented for FiducialsToModelRegistration')
