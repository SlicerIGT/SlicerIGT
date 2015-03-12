import random
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# TexturedMesh
#

class TexturedMesh(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "TexturedMesh" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Examples"]
    self.parent.dependencies = []
    self.parent.contributors = [] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    It performs a simple thresholding on the input volume and optionally captures a screenshot.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.

#
# TexturedMeshWidget
#

class TexturedMeshWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

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
    # input volume selector
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
    self.inputModelSelector.setToolTip( "Pick the input to the algorithm." )
    parametersFormLayout.addRow("Input Model: ", self.inputModelSelector)

    #input texture selector
    self.inputTextureSelector = slicer.qMRMLNodeComboBox()
    self.inputTextureSelector.nodeTypes = ( ("vtkMRMLVectorVolumeNode"), "" )
    self.inputTextureSelector.selectNodeUponCreation = True
    self.inputTextureSelector.addEnabled = False
    self.inputTextureSelector.removeEnabled = False
    self.inputTextureSelector.noneEnabled = False
    self.inputTextureSelector.showHidden = False
    self.inputTextureSelector.showChildNodeTypes = False
    self.inputTextureSelector.setMRMLScene( slicer.mrmlScene )
    self.inputTextureSelector.setToolTip( "Pick the input to the algorithm." )
    parametersFormLayout.addRow("Input Texture: ", self.inputTextureSelector)


    #
    # output volume selector
    #
    self.outputSelector = slicer.qMRMLNodeComboBox()
    self.outputSelector.nodeTypes = ( ("vtkMRMLModelNode"), "" )
    self.outputSelector.selectNodeUponCreation = True
    self.outputSelector.addEnabled = True
    self.outputSelector.removeEnabled = True
    self.outputSelector.noneEnabled = True
    self.outputSelector.showHidden = False
    self.outputSelector.showChildNodeTypes = False
    self.outputSelector.setMRMLScene( slicer.mrmlScene )
    self.outputSelector.setToolTip( "Pick the output to the algorithm." )
    parametersFormLayout.addRow("Output Model: ", self.outputSelector)


    # Parameters Area
    #
    parametersRGBCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersRGBCollapsibleButton.text = "RGB Parameters"
    self.layout.addWidget(parametersRGBCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersRGBFormLayout = qt.QFormLayout(parametersRGBCollapsibleButton)
    
    # redThreshold value
    #
    self.redValueSliderWidget = ctk.ctkSliderWidget()
    self.redValueSliderWidget.singleStep = 1
    self.redValueSliderWidget.minimum = 0
    self.redValueSliderWidget.maximum = 256
    self.redValueSliderWidget.value = 188
    self.redValueSliderWidget.setToolTip("Set threshold value for computing the output image. Voxels that have intensities lower than this value will set to zero.")
    parametersRGBFormLayout.addRow("Red Value", self.redValueSliderWidget)

    # greenThreshold value
    self.greenValueSliderWidget = ctk.ctkSliderWidget()
    self.greenValueSliderWidget.singleStep = 1
    self.greenValueSliderWidget.minimum = 0
    self.greenValueSliderWidget.maximum = 256
    self.greenValueSliderWidget.value = 128
    self.greenValueSliderWidget.setToolTip("Set threshold value for computing the output image. Voxels that have intensities lower than this value will set to zero.")
    parametersRGBFormLayout.addRow("Green Value", self.greenValueSliderWidget)

    # blueThreshold value
    self.blueValueSliderWidget = ctk.ctkSliderWidget()
    self.blueValueSliderWidget.singleStep = 1
    self.blueValueSliderWidget.minimum = 0
    self.blueValueSliderWidget.maximum = 256
    self.blueValueSliderWidget.value = 60
    self.blueValueSliderWidget.setToolTip("Set threshold value for computing the output image. Voxels that have intensities lower than this value will set to zero.")
    parametersRGBFormLayout.addRow("Blue Value", self.blueValueSliderWidget)
    
    
    # threshold value
    #
    self.imageThresholdSliderWidget = ctk.ctkSliderWidget()
    self.imageThresholdSliderWidget.singleStep = 1
    self.imageThresholdSliderWidget.minimum = 0
    self.imageThresholdSliderWidget.maximum = 256
    self.imageThresholdSliderWidget.value = 30
    self.imageThresholdSliderWidget.setToolTip("Set threshold value for computing the output image. Voxels that have intensities lower than this value will set to zero.")
    parametersRGBFormLayout.addRow("Color Threshold", self.imageThresholdSliderWidget)

    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QRadioButton()
    self.enableScreenshotsFlagCheckBox.checked = 0
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    parametersFormLayout.addRow("Screen shots", self.enableScreenshotsFlagCheckBox)

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
    self.inputTextureSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.outputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Apply button state
    self.onSelect()

  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputModelSelector.currentNode() and self.inputTextureSelector.currentNode() and self.outputSelector.currentNode()
    self.applyButton.enabled = self.inputTextureSelector.currentNode() and self.outputSelector.currentNode()

  def onApplyButton(self):
    logic = TexturedMeshLogic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    colorTolerance = self.imageThresholdSliderWidget.value
    colorRgb = (self.redValueSliderWidget.value,self.greenValueSliderWidget.value,self.blueValueSliderWidget.value)
    logic.run(self.inputModelSelector.currentNode(), self.inputTextureSelector.currentNode(), self.outputSelector.currentNode(), colorRgb, colorTolerance, enableScreenshotsFlag)
#
# TexturedMeshLogic
#

class TexturedMeshLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def hasImageData(self,volumeNode):
    """This is an example logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      logging.debug('hasImageData failed: no volume node')
      return False
    if volumeNode.GetImageData() == None:
      logging.debug('hasImageData failed: no image data in volume node')
      return False
    return True


  def takeScreenshot(self,name,description,type=-1):
    # show the message even if not taking a screen shot
    slicer.util.delayDisplay('Take screenshot: '+description+'.\nResult is available in the Annotations module.', 3000)

    lm = slicer.app.layoutManager()
    # switch on the type to get the requested window
    widget = 0
    if type == slicer.qMRMLScreenShotDialog.FullLayout:
      # full layout
      widget = lm.viewport()
    elif type == slicer.qMRMLScreenShotDialog.ThreeD:
      # just the 3D window
      widget = lm.threeDWidget(0).threeDView()
    elif type == slicer.qMRMLScreenShotDialog.Red:
      # red slice window
      widget = lm.sliceWidget("Red")
    elif type == slicer.qMRMLScreenShotDialog.Yellow:
      # yellow slice window
      widget = lm.sliceWidget("Yellow")
    elif type == slicer.qMRMLScreenShotDialog.Green:
      # green slice window
      widget = lm.sliceWidget("Green")
    else:
      # default to using the full window
      widget = slicer.util.mainWindow()
      # reset the type so that the node is set correctly
      type = slicer.qMRMLScreenShotDialog.FullLayout

    # grab and convert to vtk image data
    qpixMap = qt.QPixmap().grabWidget(widget)
    qimage = qpixMap.toImage()
    imageData = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(qimage,imageData)

    annotationLogic = slicer.modules.annotations.logic()
    annotationLogic.CreateSnapShot(name, description, type, 1, imageData)

  def run(self, modelNode, textureImageNode, outputModelNode, colorRgb, colorTolerance, enableScreenshots=0):
    """
    Run the actual algorithm
    """

    logging.info('Processing started')
    
    ShowTextureOnModel(modelNode, textureImageNode)
    ConvertTextureToPointAttribute(modelNode, textureImageNode)
    #extract mesh is new
    ExtractMesh(outputModelNode,modelNode,colorRgb,colorTolerance)
    ShowTextureOnModel(outputModelNode, textureImageNode)
    logging.info('Processing Complete')
    return True


class TexturedMeshTest(ScriptedLoadableModuleTest):
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
    self.test_TexturedMesh1()

  def test_TexturedMesh1(self):
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
    logic = TexturedMeshLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')

# Show texture
def ShowTextureOnModel(modelNode, textureImageNode):
  modelDisplayNode=modelNode.GetDisplayNode()
  modelDisplayNode.SetBackfaceCulling(0)
  textureImageFlipVert=vtk.vtkImageFlip()
  textureImageFlipVert.SetFilteredAxis(1)
  textureImageFlipVert.SetInputConnection(textureImageNode.GetImageDataConnection())
  modelDisplayNode.SetTextureImageDataConnection(textureImageFlipVert.GetOutputPort())

# Add texture data to scalars
def ConvertTextureToPointAttribute(modelNode, textureImageNode):
  polyData=modelNode.GetPolyData()
  textureImageFlipVert=vtk.vtkImageFlip()
  textureImageFlipVert.SetFilteredAxis(1)
  textureImageFlipVert.SetInputConnection(textureImageNode.GetImageDataConnection())
  textureImageFlipVert.Update()
  textureImageData=textureImageFlipVert.GetOutput()
  pointData=polyData.GetPointData()
  tcoords=pointData.GetTCoords()
  numOfPoints=pointData.GetNumberOfTuples()
  assert numOfPoints==tcoords.GetNumberOfTuples(), "Number of texture coordinates does not equal number of points"
  textureSamplingPointsUv=vtk.vtkPoints()
  textureSamplingPointsUv.SetNumberOfPoints(numOfPoints)
  for pointIndex in xrange(numOfPoints):
    uv=tcoords.GetTuple2(pointIndex)
    textureSamplingPointsUv.SetPoint(pointIndex, uv[0], uv[1], 0)

  textureSamplingPointDataUv=vtk.vtkPolyData()
  uvToXyz=vtk.vtkTransform()
  textureImageDataSpacingSpacing=textureImageData.GetSpacing()
  textureImageDataSpacingOrigin=textureImageData.GetOrigin()
  textureImageDataSpacingDimensions=textureImageData.GetDimensions()
  uvToXyz.Scale(textureImageDataSpacingDimensions[0]/textureImageDataSpacingSpacing[0], textureImageDataSpacingDimensions[1]/textureImageDataSpacingSpacing[1], 1)
  uvToXyz.Translate(textureImageDataSpacingOrigin)
  textureSamplingPointDataUv.SetPoints(textureSamplingPointsUv)
  transformPolyDataToXyz=vtk.vtkTransformPolyDataFilter()
  transformPolyDataToXyz.SetInputData(textureSamplingPointDataUv)
  transformPolyDataToXyz.SetTransform(uvToXyz)
  probeFilter=vtk.vtkProbeFilter()
  probeFilter.SetInputConnection(transformPolyDataToXyz.GetOutputPort())
  probeFilter.SetSourceData(textureImageData)
  probeFilter.Update()
  rgbPoints=probeFilter.GetOutput().GetPointData().GetArray('ImageScalars')
  colorArrayRed=vtk.vtkDoubleArray()
  colorArrayRed.SetName('ColorRed')
  colorArrayRed.SetNumberOfTuples(numOfPoints)
  colorArrayGreen=vtk.vtkDoubleArray()
  colorArrayGreen.SetName('ColorGreen')
  colorArrayGreen.SetNumberOfTuples(numOfPoints)
  colorArrayBlue=vtk.vtkDoubleArray()
  colorArrayBlue.SetName('ColorBlue')
  colorArrayBlue.SetNumberOfTuples(numOfPoints)
  for pointIndex in xrange(numOfPoints):
    rgb=rgbPoints.GetTuple3(pointIndex)
    colorArrayRed.SetValue(pointIndex,rgb[0])
    colorArrayGreen.SetValue(pointIndex,rgb[1])
    colorArrayBlue.SetValue(pointIndex,rgb[2])
  colorArrayRed.Modified()
  colorArrayGreen.Modified()
  colorArrayBlue.Modified()
  pointData.AddArray(colorArrayRed)
  pointData.AddArray(colorArrayGreen)
  pointData.AddArray(colorArrayBlue)
  pointData.Modified()
  polyData.Modified()

def ExtractMesh(outputModelNode, modelNode, colorRgb, colorTolerance):
  
  fullPolyData = modelNode.GetPolyData()
  pointData=fullPolyData.GetPointData()

  redValues = pointData.GetArray('ColorRed')
  greenValues = pointData.GetArray('ColorGreen')
  blueValues = pointData.GetArray('ColorBlue')

  redMaxTolerance = colorRgb[0] + colorTolerance
  redMinTolerance = colorRgb[0] - colorTolerance
  greenMaxTolerance = colorRgb[1] + colorTolerance
  greenMinTolerance = colorRgb[1] - colorTolerance
  blueMaxTolerance = colorRgb[2] + colorTolerance
  blueMinTolerance = colorRgb[2] - colorTolerance

  lengthTuples = int(redValues.GetNumberOfTuples()) 
  selectedPointIds = vtk.vtkIdTypeArray()

  for pointId in range(lengthTuples):
    if redValues.GetValue(pointId) >= redMinTolerance and redValues.GetValue(pointId) <= redMaxTolerance and greenValues.GetValue(pointId) >= greenMinTolerance and greenValues.GetValue(pointId) <= greenMaxTolerance and blueValues.GetValue(pointId) >= blueMinTolerance and blueValues.GetValue(pointId) <= blueMaxTolerance:
      selectedPointIds.InsertNextValue(pointId)  
      selectedPointIds.InsertNextValue(pointId)  

  selectionNode = vtk.vtkSelectionNode()
  selectionNode.SetFieldType(vtk.vtkSelectionNode.POINT)
  selectionNode.SetContentType(vtk.vtkSelectionNode.INDICES)
  selectionNode.SetSelectionList(selectedPointIds)
  selectionNode.GetProperties().Set(vtk.vtkSelectionNode.CONTAINING_CELLS(), 1);

  selection = vtk.vtkSelection()
  selection.AddNode(selectionNode)

  extractSelection = vtk.vtkExtractSelection()
  extractSelection.SetInputData(0,fullPolyData)
  extractSelection.SetInputData(1,selection);
  extractSelection.Update();

  convertToPolydata = vtk.vtkDataSetSurfaceFilter()
  convertToPolydata.SetInputConnection(extractSelection.GetOutputPort())
  convertToPolydata.Update()
  outputModelNode.SetAndObservePolyData(convertToPolydata.GetOutput())

  if not outputModelNode.GetDisplayNode():
    md2 = slicer.vtkMRMLModelDisplayNode()
    slicer.mrmlScene.AddNode(md2)
    outputModelNode.SetAndObserveDisplayNodeID(md2.GetID()) 
  