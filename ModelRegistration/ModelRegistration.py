import os
import unittest
import math
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# ModelRegistration
#

class ModelRegistration(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Model Registration"
    self.parent.categories = ["IGT"]
    self.parent.dependencies = []
    self.parent.contributors = ["Andras Lasso, Tamas Ungi (PerkLab, Queen's University"]
    self.parent.helpText = """
    This module applies Iterative Closest Points registration between two surface models.
    """
    self.parent.acknowledgementText = """
    This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
    """

#
# ModelRegistrationWidget
#

class ModelRegistrationWidget(ScriptedLoadableModuleWidget):

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

    transformWarningLabel = qt.QLabel( "Note: Parent transforms of models are not used. Models should be defined in the coordinate system that is being registered." )
    transformWarningLabel.setWordWrap( True )
    parametersFormLayout.addRow(transformWarningLabel)

    #
    # input target (fixed, dense) model selector
    #
    self.inputTargetModelSelector = slicer.qMRMLNodeComboBox()
    self.inputTargetModelSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.inputTargetModelSelector.selectNodeUponCreation = True
    self.inputTargetModelSelector.addEnabled = False
    self.inputTargetModelSelector.removeEnabled = False
    self.inputTargetModelSelector.noneEnabled = False
    self.inputTargetModelSelector.showHidden = False
    self.inputTargetModelSelector.showChildNodeTypes = False
    self.inputTargetModelSelector.setMRMLScene( slicer.mrmlScene )
    self.inputTargetModelSelector.setToolTip( "Select the model the other will be transformed to. This model required to contain a dense set of points." )
    parametersFormLayout.addRow("Input fixed (dense) model: ", self.inputTargetModelSelector)

    #
    # input source (moving) model selector
    #
    self.inputSourceModelSelector = slicer.qMRMLNodeComboBox()
    self.inputSourceModelSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.inputSourceModelSelector.selectNodeUponCreation = True
    self.inputSourceModelSelector.addEnabled = False
    self.inputSourceModelSelector.removeEnabled = False
    self.inputSourceModelSelector.noneEnabled = False
    self.inputSourceModelSelector.showHidden = False
    self.inputSourceModelSelector.showChildNodeTypes = False
    self.inputSourceModelSelector.setMRMLScene( slicer.mrmlScene )
    self.inputSourceModelSelector.setToolTip( "Select the model that will be transformed. This model may require a sparse set of points." )
    parametersFormLayout.addRow("Input moving (sparse) model: ", self.inputSourceModelSelector)

    #
    # output transform selector
    #
    self.outputSourceToTargetTransformSelector = slicer.qMRMLNodeComboBox()
    self.outputSourceToTargetTransformSelector.nodeTypes = ( ("vtkMRMLLinearTransformNode"), "" )
    self.outputSourceToTargetTransformSelector.selectNodeUponCreation = True
    self.outputSourceToTargetTransformSelector.addEnabled = True
    self.outputSourceToTargetTransformSelector.removeEnabled = True
    self.outputSourceToTargetTransformSelector.noneEnabled = False
    self.outputSourceToTargetTransformSelector.showHidden = False
    self.outputSourceToTargetTransformSelector.showChildNodeTypes = False
    self.outputSourceToTargetTransformSelector.renameEnabled = True
    self.outputSourceToTargetTransformSelector.setMRMLScene( slicer.mrmlScene )
    self.outputSourceToTargetTransformSelector.setToolTip( "Pick the moving to fixed transform computed by the algorithm." )
    parametersFormLayout.addRow("Output transform: ", self.outputSourceToTargetTransformSelector)

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
    self.inputTargetModelSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.inputSourceModelSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.outputSourceToTargetTransformSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)


  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputTargetModelSelector.currentNode() and self.outputSourceToTargetTransformSelector.currentNode() and self.inputSourceModelSelector.currentNode()

  def onApplyButton(self):
    logic = ModelRegistrationLogic()

    inputSourceModel = self.inputSourceModelSelector.currentNode()
    inputTargetModel = self.inputTargetModelSelector.currentNode()
    outputSourceToTargetTransform = self.outputSourceToTargetTransformSelector.currentNode()

    logic.run(inputSourceModel, inputTargetModel, outputSourceToTargetTransform, self.typeSelector.currentIndex, self.iterationSpin.value )

    self.outputLine.setText( logic.ComputeMeanDistance(inputSourceModel, inputTargetModel, outputSourceToTargetTransform) )

#
# ModelRegistrationLogic
#

class ModelRegistrationLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """

  def run(self, inputSourceModel, inputTargetModel, outputSourceToTargetTransform, transformType=0, numIterations=100 ):

    self.delayDisplay('Running iterative closest point registration')

    icpTransform = vtk.vtkIterativeClosestPointTransform()
    icpTransform.SetSource( inputSourceModel.GetPolyData() )
    icpTransform.SetTarget( inputTargetModel.GetPolyData() )
    icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    if transformType == 1:
      icpTransform.GetLandmarkTransform().SetModeToSimilarity()
    if transformType == 2:
      icpTransform.GetLandmarkTransform().SetModeToAffine()
    icpTransform.SetMaximumNumberOfIterations( numIterations )
    icpTransform.Modified()
    icpTransform.Update()

    outputSourceToTargetTransform.SetMatrixTransformToParent( icpTransform.GetMatrix() )
    if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion >= 4 and slicer.app.minorVersion >= 11):
      outputSourceToTargetTransform.AddNodeReferenceID(slicer.vtkMRMLTransformNode.GetMovingNodeReferenceRole(), inputSourceModel.GetID())
      outputSourceToTargetTransform.AddNodeReferenceID(slicer.vtkMRMLTransformNode.GetFixedNodeReferenceRole(), inputTargetModel.GetID())

    return True


  def ComputeMeanDistance(self, inputSourceModel, inputTargetModel, transform ):
    sourcePolyData = inputSourceModel.GetPolyData()
    targetPolyData = inputTargetModel.GetPolyData()

    cellId = vtk.mutable(0)
    subId = vtk.mutable(0)
    dist2 = vtk.mutable(0.0)
    locator = vtk.vtkCellLocator()
    locator.SetDataSet( targetPolyData )
    locator.SetNumberOfCellsPerBucket( 1 )
    locator.BuildLocator()

    totalDistance = 0.0

    sourcePoints = sourcePolyData.GetPoints()
    n = sourcePoints.GetNumberOfPoints()
    m = vtk.vtkMath()
    for sourcePointIndex in xrange(n):
      sourcePointPos = [0, 0, 0]
      sourcePoints.GetPoint( sourcePointIndex, sourcePointPos )
      transformedSourcePointPos = [0, 0, 0, 1]
      #transform.GetTransformToParent().TransformVector( sourcePointPos, transformedSourcePointPos )
      sourcePointPos.append(1)
      transform.GetTransformToParent().MultiplyPoint( sourcePointPos, transformedSourcePointPos )
      #transformedPoints.InsertNextPoint( transformedSourcePointPos )
      surfacePoint = [0, 0, 0]
      transformedSourcePointPos.pop()
      locator.FindClosestPoint( transformedSourcePointPos, surfacePoint, cellId, subId, dist2 )
      totalDistance = totalDistance + math.sqrt(dist2)

    return ( totalDistance / n )

class ModelRegistrationTest(ScriptedLoadableModuleTest):
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
    self.test_ModelRegistration1()

  def test_ModelRegistration1(self):
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

    self.delayDisplay('Test is not implemented for ModelRegistration')
