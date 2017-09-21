import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# StandardizeLandmarks
#

class StandardizeLandmarks(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "StandardizeLandmarks" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Scoliosis"]
    self.parent.dependencies = []
    self.parent.contributors = ["Ben Church (PerkLab - Queen's University)"]
    self.parent.helpText = "This module can be used to center and normalize landmarks, and extrapolate bilaterally symmetric spinal landmarks to completion (for omission repair, see PreProcessLandmarks)"
    self.parent.acknowledgementText = """ """ # replace with organization, grant and thanks.

#
# StandardizeLandmarksWidget
#

class StandardizeLandmarksWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    self.reloadCollapsibleButton.collapsed = 1
    # Instantiate and connect widgets ...

    #
    # Data selection area
    #
    
    self.DataCollapsibleButton = ctk.ctkCollapsibleButton()
    self.DataCollapsibleButton.text = "Data Selection"
    self.layout.addWidget(self.DataCollapsibleButton)
    self.DataInterfaceLayout = qt.QGridLayout(self.DataCollapsibleButton)
    self.DataInterfaceLayout.setHorizontalSpacing(12)
    
    # Checkbox to indicate whether to operate on all MarkupsNodes, or to select one
    self.ModifyAllNodesCheckBox = qt.QCheckBox()
    self.ModifyAllNodesCheckBox.checked = 0
    self.ModifyAllNodesCheckBox.toolTip = "Check if you want to operate on all MarkupsNodes in the scene, uncheck if you want to select one"
    self.DataInterfaceLayout.addWidget(qt.QLabel("All Nodes?"), 0, 0, 1, 1)
    self.DataInterfaceLayout.addWidget(self.ModifyAllNodesCheckBox, 0, 1, 1, 1)
    
    # Single node ComboBox selector
    self.SingleNodeSelector = slicer.qMRMLNodeComboBox()
    self.SingleNodeSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.SingleNodeSelector.enabled = True
    self.SingleNodeSelector.selectNodeUponCreation = True
    self.SingleNodeSelector.addEnabled = False
    self.SingleNodeSelector.removeEnabled = False
    self.SingleNodeSelector.noneEnabled = True
    self.SingleNodeSelector.showHidden = False
    self.SingleNodeSelector.showChildNodeTypes = False
    self.SingleNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.SingleNodeSelector.setToolTip( "Pick the input to the algorithm." )
    self.DataInterfaceLayout.addWidget(qt.QLabel("Node selector"), 0, 2, 1, 1)
    self.DataInterfaceLayout.addWidget(self.SingleNodeSelector, 0, 3, 1, 1)
    
    #
    # Operations Interface
    #
    self.OperationsCollapsibleButton = ctk.ctkCollapsibleButton()
    self.OperationsCollapsibleButton.text = "Operations"
    self.layout.addWidget(self.OperationsCollapsibleButton)
    self.DataInterfaceLayout = qt.QGridLayout(self.OperationsCollapsibleButton)
    self.DataInterfaceLayout.setHorizontalSpacing(12)
   
    # Extrapolate button
    self.ExtrapolateLandmarksButton = qt.QPushButton("Extrapolate Landmarks")
    self.ExtrapolateLandmarksButton.toolTip = "Extend incomplete (but gap-free) landmark set to include T1-L5"
    self.ExtrapolateLandmarksButton.enabled = True
    self.ExtrapolateLandmarksButton.checkable = True
    self.DataInterfaceLayout.addWidget(self.ExtrapolateLandmarksButton, 1, 1, 1, 2)

    # Center button
    self.CenterLandmarksButton = qt.QPushButton("Center Landmarks")
    self.CenterLandmarksButton.toolTip = "Center the landmarks about the origin (subtract average locations)"
    self.CenterLandmarksButton.enabled = True
    self.CenterLandmarksButton.checkable = True
    self.DataInterfaceLayout.addWidget(self.CenterLandmarksButton, 2, 1, 1, 2)
    
    # Normalize button
    self.NormalizeLandmarksButton = qt.QPushButton("Normalize Landmarks")
    self.NormalizeLandmarksButton.toolTip = "Normalize landmarks coordinate over range [0,1] (divide by largest coordinate in each node)"
    self.NormalizeLandmarksButton.enabled = True
    self.NormalizeLandmarksButton.checkable = True
    self.DataInterfaceLayout.addWidget(self.NormalizeLandmarksButton, 3, 1, 1, 2)
    
    # Apply Button
    self.PerformOperationsButton = qt.QPushButton("Perform Operations")
    self.PerformOperationsButton.toolTip = "Run the algorithm."
    self.PerformOperationsButton.enabled = False
    self.DataInterfaceLayout.addWidget(self.PerformOperationsButton, 4, 1, 1, 2)

    #
    # Connections
    #
    self.ModifyAllNodesCheckBox.connect('stateChanged(int)', self.OnAllNodesChecked)
    self.SingleNodeSelector.connect('currentNodeChanged(bool)', self.OnSelectedNodeChanged)
    self.PerformOperationsButton.connect('clicked(bool)', self.OnPerformOperationsButton)
    
    #
    # Seperate reload button
    #
    self.ReloadButton = qt.QPushButton("Reload Module")
    self.ReloadButton.toolTip = "Reload this module, reflecting any changes in its code"
    self.ReloadButton.enabled = True
    self.layout.addWidget(self.ReloadButton)
    self.ReloadButton.connect('clicked(bool)', self.OnReloadButtonClicked)
    
  def cleanup(self):
    pass

  def OnAllNodesChecked(self):
    self.SingleNodeSelector.setMRMLScene(slicer.mrmlScene)
    self.SingleNodeSelector.enabled = not self.ModifyAllNodesCheckBox.isChecked()
    self.PerformOperationsButton.enabled = (self.SingleNodeSelector.currentNode() != None) or (self.ModifyAllNodesCheckBox.isChecked())
    
  def OnSelectedNodeChanged(self):
    # Check that ModifyAllNodesCheckBox is not checked, shouldn't be if SingleNodeSelector was able to change
    if self.ModifyAllNodesCheckBox.isChecked():
      print "ERROR - All Nodes should not be checked if self.SingleNodeSelector changed nodes"
      return
    else:
      self.PerformOperationsButton.enabled = (self.SingleNodeSelector.currentNode() != None)
    
  def OnPerformOperationsButton(self):
    logic = StandardizeLandmarksLogic()

    if self.ModifyAllNodesCheckBox.checked:
      logic.PerformSelectedOperations(self.ExtrapolateLandmarksButton.checked, self.CenterLandmarksButton.checked, self.NormalizeLandmarksButton.checked)
    else:
      SingleNode = self.SingleNodeSelector.currentNode()
      logic.PerformSelectedOperations(self.ExtrapolateLandmarksButton.checked, self.CenterLandmarksButton.checked, self.NormalizeLandmarksButton.checked, SingleNode)

  def OnReloadButtonClicked(self):
    print str(slicer.moduleNames.StandardizeLandmarks) + " reloaded"
    slicer.util.reloadScriptedModule(slicer.moduleNames.StandardizeLandmarks)
    
#
# StandardizeLandmarksLogic
#

class StandardizeLandmarksLogic(ScriptedLoadableModuleLogic):
  def __init__(self):
    self.ExpectedLabelsInOrder = ['T1L','T1R','T2L','T2R','T3L','T3R','T4L','T4R','T5L','T5R','T6L','T6R','T7L','T7R','T8L','T8R','T9L','T9R','T10L','T10R','T11L','T11R','T12L','T12R','L1L','L1R','L2L','L2R','L3L','L3R','L4L','L4R','L5L','L5R']

  def GetAverageTopVector(self, MarkupsNode):
    # Get point coords required for extrapolation vector
    TopLeftPointCoords = MarkupsNode.GetMarkupPointVector(0,0)
    TopRightPointCoords = MarkupsNode.GetMarkupPointVector(1,0)
    
    SecondLeftPointCoords = MarkupsNode.GetMarkupPointVector(2,0)
    SecondRightPointCoords = MarkupsNode.GetMarkupPointVector(3,0)
    
    TopLeftVector = [(TopLeftPointCoords[dim] - SecondLeftPointCoords[dim]) for dim in range(3)]
    TopRightVector = [(TopRightPointCoords[dim] - SecondRightPointCoords[dim]) for dim in range(3)]
    AverageTopVector = [((TopLeftVector[dim] + TopRightVector[dim])/2.0) for dim in range(3)]
    
    return AverageTopVector
    
  def GetAverageBottomVector(self, MarkupsNode):
    NumPoints = MarkupsNode.GetNumberOfFiducials()
    
    # Get point coords required for extrapolation vector
    BottomLeftPointCoords = MarkupsNode.GetMarkupPointVector(NumPoints-2,0)
    BottomRightPointCoords = MarkupsNode.GetMarkupPointVector(NumPoints-1,0)
    
    SecondLastLeftPointCoords = MarkupsNode.GetMarkupPointVector(NumPoints-4,0)
    SecondLastRightPointCoords = MarkupsNode.GetMarkupPointVector(NumPoints-3,0)
    
    BottomLeftVector = [(BottomLeftPointCoords[dim] - SecondLastLeftPointCoords[dim]) for dim in range(3)]
    BottomRightVector = [(BottomRightPointCoords[dim] - SecondLastRightPointCoords[dim]) for dim in range(3)]
    AverageBottomVector = [((BottomLeftVector[dim] + BottomRightVector[dim])/2.0) for dim in range(3)]
    
    return AverageBottomVector
    
  def Extrapolate(self, MarkupsNode):
    
    # Get all required MarkupsNode structure before modifying it
    
    NumOriginalPoints = MarkupsNode.GetNumberOfFiducials()
    
    # Find current top vertebra, to know how many points to add
    TopVertebraLabel = MarkupsNode.GetNthFiducialLabel(0)
    TopVertebraIndex = self.ExpectedLabelsInOrder.index(TopVertebraLabel)
    AverageTopVector = self.GetAverageTopVector(MarkupsNode)
    
    # Before any points are added, TopLeft and TopRight points are at the beginning of the MarkupsNode point list
    TopLeftPointCoords = MarkupsNode.GetMarkupPointVector(0,0)
    TopRightPointCoords = MarkupsNode.GetMarkupPointVector(1,0)

    # Find bottom vertebra
    BottomVertebraLabel = MarkupsNode.GetNthFiducialLabel(NumOriginalPoints-1)
    BottomVertebraIndex = self.ExpectedLabelsInOrder.index(BottomVertebraLabel)
    AverageBottomVector = self.GetAverageBottomVector(MarkupsNode)
    
    # Before any points are added, BottomLeft and BottomRight points are at the end of the MarkupsNode point list
    BottomLeftPointCoords = MarkupsNode.GetMarkupPointVector(NumOriginalPoints-2,0)
    BottomRightPointCoords = MarkupsNode.GetMarkupPointVector(NumOriginalPoints-1,0)
    
    # Extrapolate upwards
    if not TopVertebraLabel.__contains__("T1"):   # Check to make sure it's not already complete at top
      # Extraplate from top until T1 is added
      PointsRemaining = TopVertebraIndex
      
      while PointsRemaining > 0:            # These should be translated into FOR loops
        # Add right point
        NewRightCoords = [(TopRightPointCoords[dim] + AverageTopVector[dim]) for dim in range(3)]
        MarkupsNode.AddFiducialFromArray(NewRightCoords)
        LatestAdditionIndex = MarkupsNode.GetNumberOfFiducials()
        MarkupsNode.SetNthFiducialLabel(LatestAdditionIndex-1, self.ExpectedLabelsInOrder[PointsRemaining-1])
        TopRightPointCoords = NewRightCoords    # Update for next iteration
        PointsRemaining -= 1
        
        # Add left point
        NewLeftCoords = [(TopLeftPointCoords[dim] + AverageTopVector[dim]) for dim in range(3)]
        MarkupsNode.AddFiducialFromArray(NewLeftCoords)
        LatestAdditionIndex = MarkupsNode.GetNumberOfFiducials()
        MarkupsNode.SetNthFiducialLabel(LatestAdditionIndex-1, self.ExpectedLabelsInOrder[PointsRemaining-1])
        TopLeftPointCoords = NewLeftCoords    # Update for next iteration
        PointsRemaining -= 1
      
    # Extrapolate downwards
    if not BottomVertebraLabel.__contains__("L5"):
      # Extrapolate from bottom until L5 is reached
      PointsRemaining = len(self.ExpectedLabelsInOrder) - BottomVertebraIndex
      
      while PointsRemaining > 0:
        #print "Adding bottom points"
        # Add left point
        NewLeftCoords = [(BottomLeftPointCoords[dim] + AverageBottomVector[dim]) for dim in range(3)]
        MarkupsNode.AddFiducialFromArray(NewLeftCoords)
        LatestAdditionIndex = MarkupsNode.GetNumberOfFiducials()
        MarkupsNode.SetNthFiducialLabel(LatestAdditionIndex-1, self.ExpectedLabelsInOrder[-1*(PointsRemaining-1)])
        BottomLeftPointCoords = NewLeftCoords    # Update for next iteration
        PointsRemaining -= 1
        
        # Add right point
        NewRightCoords = [(BottomRightPointCoords[dim] + AverageBottomVector[dim]) for dim in range(3)]
        MarkupsNode.AddFiducialFromArray(NewRightCoords)
        LatestAdditionIndex = MarkupsNode.GetNumberOfFiducials()
        MarkupsNode.SetNthFiducialLabel(LatestAdditionIndex-1, self.ExpectedLabelsInOrder[-1*(PointsRemaining-1)])
        BottomRightPointCoords = NewRightCoords    # Update for next iteration
        PointsRemaining -= 1
        
  
    return
    
  def OrderLandmarks(self, MarkupsNode):
    Labels = [MarkupsNode.GetNthFiducialLabel(i) for i in range(MarkupsNode.GetNumberOfFiducials())]
    Coords = [MarkupsNode.GetMarkupPointVector(i,0) for i in range(MarkupsNode.GetNumberOfFiducials())]
    
    MarkupsNode.RemoveAllMarkups()
    for ProperIndex, ExpectedLabel in enumerate(self.ExpectedLabelsInOrder[:]):
      MarkupsNode.AddFiducialFromArray(Coords[Labels.index(ExpectedLabel)])
      MarkupsNode.SetNthFiducialLabel(ProperIndex, self.ExpectedLabelsInOrder[ProperIndex])
      
    return
  
  def Center(self, MarkupsNode):
    NumPoints = MarkupsNode.GetNumberOfFiducials()
    
    AllLabels = [MarkupsNode.GetNthFiducialLabel(i) for i in range(NumPoints)]
    AllPointCoords = [MarkupsNode.GetMarkupPointVector(i,0) for i in range(NumPoints)]
  
    # Find the average location (center-of-mass) of all points, so it can be subtracted
    COM = [(sum([AllPointCoords[i][dim] for i in range(NumPoints)])/NumPoints) for dim in range(3)]
    
    MarkupsNode.RemoveAllMarkups()
    for i, UncenterCoords in enumerate(AllPointCoords):
      CenteredPointCoords = [(UncenterCoords[dim] - COM[dim]) for dim in range(3)]
      MarkupsNode.AddFiducialFromArray(CenteredPointCoords)
      MarkupsNode.SetNthFiducialLabel(i, AllLabels[i])
      
    #print COM
    return
  
  def Normalize(self, MarkupsNode):
    NumPoints = MarkupsNode.GetNumberOfFiducials()
    
    AllLabels = [MarkupsNode.GetNthFiducialLabel(i) for i in range(NumPoints)]
    AllPointCoords = [MarkupsNode.GetMarkupPointVector(i,0) for i in range(NumPoints)]
  
    # Find the max point coord of all points (in all dims), so coords can be divided
    MaxCoord = max([max([AllPointCoords[i][dim] for i in range(NumPoints)]) for dim in range(3)])
    
    MarkupsNode.RemoveAllMarkups()
    for i, UnnormalizedCoords in enumerate(AllPointCoords):
      NormalizedPointCoords = [(UnnormalizedCoords[dim]/MaxCoord) for dim in range(3)]
      MarkupsNode.AddFiducialFromArray(NormalizedPointCoords)
      MarkupsNode.SetNthFiducialLabel(i, AllLabels[i])
    
    print MaxCoord
    return

  def PerformSelectedOperations(self, DoExtrapolate, DoCenter, DoNormalize, SingleNode=None):
    """
    Run the actual algorithm
    """

    if SingleNode == None:
      NodesToModify = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    else:
      NodesToModify = [SingleNode]
    
    if DoExtrapolate:
      for MarkupsNode in NodesToModify:
        self.Extrapolate(MarkupsNode)
        self.OrderLandmarks(MarkupsNode)    # Added landmarks go to end of fiducial list, even if they are added upwards, re-establish correspondence with self.ExpectedLabelsInOrder
        
    if DoCenter:
      for MarkupsNode in NodesToModify:
        self.Center(MarkupsNode)
    
    if DoNormalize:
      for MarkupsNode in NodesToModify:
        self.Normalize(MarkupsNode)

    return True


class StandardizeLandmarksTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_StandardizeLandmarks1()

  def test_StandardizeLandmarks1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
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
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = StandardizeLandmarksLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
