import random
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# TextureModel
#

class TextureModel(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Texture Model"
    self.parent.categories = ["Surface Models"]
    self.parent.dependencies = []
    self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)", "Amani Ibrahim (PerkLab, Queen's)" ]
    self.parent.helpText = """This module applies a texture (stored in a volume node) to a model node.
It is typically used to display colored surfaces, provided by surface scanners, exported in OBJ format.
The model must contain texture coordinates. Only a single texture file per model is supported.
"""
    self.parent.acknowledgementText = """ """ # replace with organization, grant and thanks.

#
# TextureModelWidget
#

class TextureModelWidget(ScriptedLoadableModuleWidget):
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
    self.inputModelSelector.nodeTypes = [ "vtkMRMLModelNode" ]
    self.inputModelSelector.addEnabled = False
    self.inputModelSelector.removeEnabled = True
    self.inputModelSelector.renameEnabled = True
    self.inputModelSelector.noneEnabled = False
    self.inputModelSelector.showHidden = False
    self.inputModelSelector.showChildNodeTypes = False
    self.inputModelSelector.setMRMLScene( slicer.mrmlScene )
    self.inputModelSelector.setToolTip( "Model node containing geometry and texture coordinates." )
    parametersFormLayout.addRow("Model: ", self.inputModelSelector)

    #input texture selector
    self.inputTextureSelector = slicer.qMRMLNodeComboBox()
    self.inputTextureSelector.nodeTypes = [ "vtkMRMLVectorVolumeNode" ]
    self.inputTextureSelector.addEnabled = False
    self.inputTextureSelector.removeEnabled = True
    self.inputTextureSelector.renameEnabled = True
    self.inputTextureSelector.noneEnabled = False
    self.inputTextureSelector.showHidden = False
    self.inputTextureSelector.showChildNodeTypes = False
    self.inputTextureSelector.setMRMLScene( slicer.mrmlScene )
    self.inputTextureSelector.setToolTip( "Color image containing texture image." )
    parametersFormLayout.addRow("Texture: ", self.inputTextureSelector)

    self.addColorAsPointAttributeCheckbox = qt.QCheckBox()
    self.addColorAsPointAttributeCheckbox.checked = False
    self.addColorAsPointAttributeCheckbox.setToolTip("If checked, color information will be saved as point data."
      " It is useful if further color-based filtering will be performed on the model.")
    parametersFormLayout.addRow("Set color information as point data: ", self.addColorAsPointAttributeCheckbox)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Apply texture to selected model."
    self.applyButton.enabled = False
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)
    self.inputModelSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.inputTextureSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Apply button state
    self.onSelect()

  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputTextureSelector.currentNode() and self.inputModelSelector.currentNode()

  def onApplyButton(self):
    logic = TextureModelLogic()
    logic.applyTexture(self.inputModelSelector.currentNode(), self.inputTextureSelector.currentNode(), self.addColorAsPointAttributeCheckbox.checked)

#
# TextureModelLogic
#
class TextureModelLogic(ScriptedLoadableModuleLogic):
  """This class implements all the actual computations.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def applyTexture(self, modelNode, textureImageNode, addColorAsPointAttribute = False):
    """
    Apply texture to model node
    """
    self.showTextureOnModel(modelNode, textureImageNode)
    if addColorAsPointAttribute:
      self.convertTextureToPointAttribute(modelNode, textureImageNode)

  # Show texture
  def showTextureOnModel(self, modelNode, textureImageNode):
    modelDisplayNode = modelNode.GetDisplayNode()
    modelDisplayNode.SetBackfaceCulling(0)
    textureImageFlipVert = vtk.vtkImageFlip()
    textureImageFlipVert.SetFilteredAxis(1)
    textureImageFlipVert.SetInputConnection(textureImageNode.GetImageDataConnection())
    modelDisplayNode.SetTextureImageDataConnection(textureImageFlipVert.GetOutputPort())

  # Add texture data to scalars
  def convertTextureToPointAttribute(self, modelNode, textureImageNode):
    polyData = modelNode.GetPolyData()
    textureImageFlipVert = vtk.vtkImageFlip()
    textureImageFlipVert.SetFilteredAxis(1)
    textureImageFlipVert.SetInputConnection(textureImageNode.GetImageDataConnection())
    textureImageFlipVert.Update()
    textureImageData = textureImageFlipVert.GetOutput()
    pointData = polyData.GetPointData()
    tcoords = pointData.GetTCoords()
    numOfPoints = pointData.GetNumberOfTuples()
    assert numOfPoints == tcoords.GetNumberOfTuples(), "Number of texture coordinates does not equal number of points"
    textureSamplingPointsUv = vtk.vtkPoints()
    textureSamplingPointsUv.SetNumberOfPoints(numOfPoints)
    for pointIndex in xrange(numOfPoints):
      uv = tcoords.GetTuple2(pointIndex)
      textureSamplingPointsUv.SetPoint(pointIndex, uv[0], uv[1], 0)

    textureSamplingPointDataUv = vtk.vtkPolyData()
    uvToXyz = vtk.vtkTransform()
    textureImageDataSpacingSpacing = textureImageData.GetSpacing()
    textureImageDataSpacingOrigin = textureImageData.GetOrigin()
    textureImageDataSpacingDimensions = textureImageData.GetDimensions()
    uvToXyz.Scale(textureImageDataSpacingDimensions[0] / textureImageDataSpacingSpacing[0],
                  textureImageDataSpacingDimensions[1] / textureImageDataSpacingSpacing[1], 1)
    uvToXyz.Translate(textureImageDataSpacingOrigin)
    textureSamplingPointDataUv.SetPoints(textureSamplingPointsUv)
    transformPolyDataToXyz = vtk.vtkTransformPolyDataFilter()
    transformPolyDataToXyz.SetInputData(textureSamplingPointDataUv)
    transformPolyDataToXyz.SetTransform(uvToXyz)
    probeFilter = vtk.vtkProbeFilter()
    probeFilter.SetInputConnection(transformPolyDataToXyz.GetOutputPort())
    probeFilter.SetSourceData(textureImageData)
    probeFilter.Update()
    rgbPoints = probeFilter.GetOutput().GetPointData().GetArray('ImageScalars')
    colorArrayRed = vtk.vtkDoubleArray()
    colorArrayRed.SetName('ColorRed')
    colorArrayRed.SetNumberOfTuples(numOfPoints)
    colorArrayGreen = vtk.vtkDoubleArray()
    colorArrayGreen.SetName('ColorGreen')
    colorArrayGreen.SetNumberOfTuples(numOfPoints)
    colorArrayBlue = vtk.vtkDoubleArray()
    colorArrayBlue.SetName('ColorBlue')
    colorArrayBlue.SetNumberOfTuples(numOfPoints)
    for pointIndex in xrange(numOfPoints):
      rgb = rgbPoints.GetTuple3(pointIndex)
      colorArrayRed.SetValue(pointIndex, rgb[0])
      colorArrayGreen.SetValue(pointIndex, rgb[1])
      colorArrayBlue.SetValue(pointIndex, rgb[2])
    colorArrayRed.Modified()
    colorArrayGreen.Modified()
    colorArrayBlue.Modified()
    pointData.AddArray(colorArrayRed)
    pointData.AddArray(colorArrayGreen)
    pointData.AddArray(colorArrayBlue)
    pointData.Modified()
    polyData.Modified()

class TextureModelTest(ScriptedLoadableModuleTest):
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
    self.test_TextureModel1()

  def test_TextureModel1(self):
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

    # Download
    import urllib
    url = 'https://artec3d-production.s3-eu-west-1.amazonaws.com/3Dmodels/human_eye_obj.zip'
    zipFilePath = slicer.app.temporaryPath + '/' + 'Human_Eye_obj.zip'
    extractPath = slicer.app.temporaryPath + '/' + 'Human_Eye_obj'
    if not os.path.exists(zipFilePath) or os.stat(zipFilePath).st_size == 0:
      logging.info('Requesting download from %s...\n' % url)
      urllib.urlretrieve(url, zipFilePath)
      self.delayDisplay('Finished with download\n')

    # Unzip
    self.delayDisplay("Unzipping to %s" % (extractPath))
    qt.QDir().mkpath(extractPath)
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.Unzip(zipFilePath, extractPath)

    # Load
    slicer.util.loadModel(extractPath+"/Human_Eye_obj.obj")
    slicer.util.loadVolume(extractPath+"/Human_Eye_obj_0.jpg")

    self.delayDisplay('Finished with download and loading')

    # Test
    modelNode = slicer.util.getNode("Human_Eye_obj")
    textureNode = slicer.util.getNode("Human_Eye_obj_0")
    logic = TextureModelLogic()
    logic.applyTexture(modelNode, textureNode)
    self.delayDisplay('Test passed!')
