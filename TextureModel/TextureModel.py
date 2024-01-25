import random
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
from slicer.i18n import translate
from slicer.i18n import tr as _
#
# TextureModel
#

class TextureModel(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = _("Texture Model")
    self.parent.categories = [translate("qSlicerAbstractCoreModule","Surface Models")]
    self.parent.dependencies = []
    self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)", "Amani Ibrahim (PerkLab, Queen's)" ]
    self.parent.helpText = _("""This module applies a texture (stored in a volume node) to a model node.
It is typically used to display colored surfaces, provided by surface scanners, exported in OBJ format.
The model must contain texture coordinates. Only a single texture file per model is supported.
For more information, visit <a href='https://github.com/SlicerIGT/SlicerIGT/#user-documentation'>SlicerIGT project website</a>.
""")
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
    parametersCollapsibleButton.text = _("Parameters")
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
    self.inputModelSelector.setToolTip(_( "Model node containing geometry and texture coordinates." ))
    parametersFormLayout.addRow(_("Model: ", self.inputModelSelector))

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
    self.inputTextureSelector.setToolTip(_( "Color image containing texture image." ))
    parametersFormLayout.addRow(_("Texture: ", self.inputTextureSelector))

    self.addColorAsPointAttributeComboBox = qt.QComboBox()
    self.addColorAsPointAttributeComboBox.addItem(_("disabled"))
    self.addColorAsPointAttributeComboBox.addItem(_("RGB vector", "uchar-vector"))
    self.addColorAsPointAttributeComboBox.addItem(_("RGB float vector", "float-vector"))
    self.addColorAsPointAttributeComboBox.addItem(_("RGB float components", "float-components"))
    self.addColorAsPointAttributeComboBox.setCurrentIndex(0)
    self.addColorAsPointAttributeComboBox.setToolTip(_('Save color in point data.'
      ' "RGB vector" is recommended for compatibility with most software.'
      ' The point data may be used for thresholding or color-based processing.'))
    parametersFormLayout.addRow(_("Save color information as point data: ", self.addColorAsPointAttributeComboBox))

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton(_("Apply"))
    self.applyButton.toolTip = _("Apply texture to selected model.")
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
    try:
      qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
      logic = TextureModelLogic()
      logic.applyTexture(self.inputModelSelector.currentNode(), self.inputTextureSelector.currentNode(),
        self.addColorAsPointAttributeComboBox.currentData)
      qt.QApplication.restoreOverrideCursor()
    except Exception as e:
      qt.QApplication.restoreOverrideCursor()
      slicer.util.errorDisplay("Failed to compute results: "+str(e))
      import traceback
      traceback.print_exc() 

#
# TextureModelLogic
#
class TextureModelLogic(ScriptedLoadableModuleLogic):
  """This class implements all the actual computations.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def applyTexture(self, modelNode, textureImageNode, saveAsPointData=None):
    """
    Apply texture to model node
    :param saveAsPointData: None (not saved), `vector`, `float-vector`, `float-components`
    """
    self.showTextureOnModel(modelNode, textureImageNode)
    if saveAsPointData:
      print(f"saveAsPointData: {saveAsPointData}")
      self.convertTextureToPointAttribute(modelNode, textureImageNode, saveAsPointData)

  # Show texture
  def showTextureOnModel(self, modelNode, textureImageNode):
    modelDisplayNode = modelNode.GetDisplayNode()
    modelDisplayNode.SetBackfaceCulling(0)
    textureImageFlipVert = vtk.vtkImageFlip()
    textureImageFlipVert.SetFilteredAxis(1)
    textureImageFlipVert.SetInputConnection(textureImageNode.GetImageDataConnection())
    modelDisplayNode.SetTextureImageDataConnection(textureImageFlipVert.GetOutputPort())

  # Add texture data to scalars
  def convertTextureToPointAttribute(self, modelNode, textureImageNode, saveAsPointData):
    """
    :param saveAsPointData: None (not saved), `vector`, `float-vector`, `float-components`
    """
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
    for pointIndex in range(numOfPoints):
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

    if saveAsPointData=='uchar-vector':
      colorArray = vtk.vtkUnsignedCharArray()
      colorArray.SetName('RGB')
      colorArray.SetNumberOfComponents(3)
      colorArray.SetNumberOfTuples(numOfPoints)
      for pointIndex in range(numOfPoints):
        rgb = rgbPoints.GetTuple3(pointIndex)
        colorArray.SetTuple3(pointIndex, rgb[0], rgb[1], rgb[2])
      colorArray.Modified()
      pointData.AddArray(colorArray)
    elif saveAsPointData=='float-vector':
      colorArray = vtk.vtkDoubleArray()
      colorArray.SetName('Color')
      colorArray.SetNumberOfComponents(3)
      colorArray.SetNumberOfTuples(numOfPoints)
      for pointIndex in range(numOfPoints):
        rgb = rgbPoints.GetTuple3(pointIndex)
        colorArray.SetTuple3(pointIndex, rgb[0]/255., rgb[1]/255., rgb[2]/255.)
      colorArray.Modified()
      pointData.AddArray(colorArray)
    elif saveAsPointData=='float-components':
      colorArrayRed = vtk.vtkDoubleArray()
      colorArrayRed.SetName('ColorRed')
      colorArrayRed.SetNumberOfTuples(numOfPoints)
      colorArrayGreen = vtk.vtkDoubleArray()
      colorArrayGreen.SetName('ColorGreen')
      colorArrayGreen.SetNumberOfTuples(numOfPoints)
      colorArrayBlue = vtk.vtkDoubleArray()
      colorArrayBlue.SetName('ColorBlue')
      colorArrayBlue.SetNumberOfTuples(numOfPoints)
      for pointIndex in range(numOfPoints):
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
    else:
      raise ValueError(f"Invalid saveAsPointData: {saveAsPointData}")

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

    slicer.util.delayDisplay("Starting the test")

    # Download
    import urllib
    url = 'https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/752ce9afe8b708fcd4f8448612170f8e730670d845f65177860edc0e08004ecf'
    zipFilePath = slicer.app.temporaryPath + '/' + 'FemurHeadSurfaceScan.zip'
    extractPath = slicer.app.temporaryPath + '/' + 'FemurHeadSurfaceScan'
    if not os.path.exists(zipFilePath) or os.stat(zipFilePath).st_size == 0:
      logging.info('Requesting download from %s...\n' % url)
      urllib.request.urlretrieve(url, zipFilePath)
      slicer.util.delayDisplay('Finished with download\n')

    # Unzip
    slicer.util.delayDisplay("Unzipping to %s" % (extractPath))
    qt.QDir().mkpath(extractPath)
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.Unzip(zipFilePath, extractPath)

    # Load
    slicer.util.loadModel(extractPath+"/head_obj.obj")
    slicer.util.loadVolume(extractPath+"/head_obj_0.png")

    slicer.util.delayDisplay(_('Finished with download and loading'))

    # Test
    modelNode = slicer.util.getNode("head_obj")
    textureNode = slicer.util.getNode("head_obj_0")
    logic = TextureModelLogic()
    logic.applyTexture(modelNode, textureNode)
    slicer.util.delayDisplay(_('Test passed!'))
