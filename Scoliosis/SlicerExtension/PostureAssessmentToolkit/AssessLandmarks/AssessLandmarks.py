import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import numpy as np

#
# AssessLandmarks
#

class AssessLandmarks(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Assess Landmarks"
    self.parent.categories = ["Scoliosis"]
    self.parent.dependencies = ["PreProcessLandmarks"]
    self.parent.contributors = ["Ben Church (PerkLab - Queen's University)"]
    self.parent.helpText = "This module interprets the geometry of bilaterally symmetric spinal landmarks, providing scoliosis/posture assessment and visualization"
    self.parent.acknowledgementText = """ """ # replace with organization, grant and thanks.


#
# AssessLandmarksWidget
#

class AssessLandmarksWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def RollbackNodeName(self, LabelRoot):
    # Will check up to 100, to avoid infinite loop
    LabelsSearched = 0
    while LabelsSearched < 100 and slicer.util.getNode(LabelRoot + "_" + str(LabelsSearched)) == None:
      LabelsSearched += 1
    
    # Roll back the node's name
    if LabelsSearched == 100 and slicer.util.getNode(LabelRoot + "_" + str(LabelsSearched)) == None:
      print "Could not find node with name " + LabelRoot + "_[0...100]"
      return False
    else:
      slicer.util.getNode(LabelRoot + "_" + str(LabelsSearched)).SetName(LabelRoot)
      
    return True
  
  def ImportModuleData(self):
    # Load data used by module
    ModuleFilePath = slicer.modules.assesslandmarks.path
    ModuleDirPath = ModuleFilePath[:ModuleFilePath.index("AssessLandmarks.py")]
    
    # Load model transverse process markups node
    if not slicer.util.loadMarkupsFiducialList(ModuleDirPath + "ModuleData/ModelLandmarks.fcsv"):
      print "ERROR - could not load model spine landmarks from " + ModuleDirPath + "/ModuleData/ModelLandmarks.fcsv"
      return False
    else:
      slicer.util.getNode("ModelLandmarks").SetDisplayVisibility(0)
      
    # Load average spine model
    if not slicer.util.loadModel(ModuleDirPath + "ModuleData/AverageModel.vtp"):
      print "ERROR - could not load average spine model from " + ModuleDirPath + "ModuleData/AverageModel.vtp"
      return False
    elif slicer.util.getNode("AverageModel") == None:
      self.RollbackNodeName("AverageModel")
    slicer.util.getNode("AverageModel").SetDisplayVisibility(0)
    """
    # Load vertebrae surface models
    VertebraLabels = ['T01','T02','T03','T04','T05','T06','T07','T08','T09','T10','T11','T12','L01','L02','L03','L04','L05']
    for Label in VertebraLabels:
      if not slicer.util.loadModel(ModuleDirPath + "ModuleData/" + Label + ".vtp"):
        print "ERROR - could not load vertebra " + Label + " surface model from " + ModuleDirPath + "ModuleData/" + Label + ".vtp"
        return False
      elif slicer.util.getNode(Label) == None:
        self.RollbackNodeName(Label)
      # ASSERT model [Label] is imported and named correctly
      slicer.util.getNode(Label).SetDisplayVisibility(0)
    """
    
    return True
    
  def ClearModuleData(self):
    # Clear model markups node
    slicer.mrmlScene.RemoveNode(slicer.util.getNode('ModelLandmarks'))
    
    # Clear average spine surface model
    slicer.mrmlScene.RemoveNode(slicer.util.getNode('AverageModel'))
    
    # Clear visualizations generated
    ModelNodes = slicer.util.getNodesByClass('vtkMRMLModelNode')
    IsVisualizationNode = [ModelNodes[i].GetName().__contains__("_Visualization") for i in range(len(ModelNodes))]
    for i, ShouldDeleteNode in enumerate(IsVisualizationNode):
      if ShouldDeleteNode:
        slicer.mrmlScene.RemoveNode(ModelNodes[i])
    
    TransformNodes = slicer.util.getNodesByClass('vtkMRMLTransformNode')
    if len(TransformNodes) > 0:
      IsVisualizedTransform = [TransformNodes[i].GetName().__contains__("L5ToT7OffsetInAxialPlane") for i in range(len(TransformNodes))]
      for i, ShouldDeleteNode in enumerate(IsVisualizedTransform):
        if ShouldDeleteNode:
          slicer.mrmlScene.RemoveNode(TransformNodes[i])
    
    """
    # Clear vertebra surface models
    VertebraLabels = ['T01','T02','T03','T04','T05','T06','T07','T08','T09','T10','T11','T12','L01','L02','L03','L04','L05']
    for Label in VertebraLabels:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode(Label))
    """
    
  def setup(self):
    # Import module data
    self.ImportModuleData()
  
    # Instantiate logic as None to check if functions were run later
    self.logic = None
  
    ScriptedLoadableModuleWidget.setup(self)
    self.reloadCollapsibleButton.collapsed = 1
    # Instantiate and connect widgets ...

    #
    # Data Interface
    #
    DataCollapsibleButton = ctk.ctkCollapsibleButton()
    DataCollapsibleButton.text = "Data"
    self.layout.addWidget(DataCollapsibleButton)

    # Layout within the dummy collapsible button
    self.DataFormLayout = qt.QGridLayout(DataCollapsibleButton)
    
    # input volume selector
    self.MarkupsNodeSelector = slicer.qMRMLNodeComboBox()
    self.MarkupsNodeSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.MarkupsNodeSelector.enabled = True
    self.MarkupsNodeSelector.selectNodeUponCreation = True
    self.MarkupsNodeSelector.addEnabled = False
    self.MarkupsNodeSelector.removeEnabled = True
    self.MarkupsNodeSelector.noneEnabled = True
    self.MarkupsNodeSelector.showHidden = False
    self.MarkupsNodeSelector.showChildNodeTypes = False
    self.MarkupsNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.MarkupsNodeSelector.setToolTip( "Pick the input to the algorithm." )
    self.DataFormLayout.addWidget(qt.QLabel("Selected markups node: "), 0, 0, 1, 1)
    self.DataFormLayout.addWidget(self.MarkupsNodeSelector, 0, 1, 1, 3)

    #
    # Connections
    #

    self.MarkupsNodeSelector.connect('currentNodeChanged(bool)', self.OnLandmarksNodeSelectorChanged)
    
    #
    # Visualization Interface
    #
    
    #*********Initialize*********#
    
    self.VisualizationCollapsibleButton = ctk.ctkCollapsibleButton()
    self.VisualizationCollapsibleButton.text = "Visualization"
    self.VisualizationCollapsibleButton.collapsed = True
    self.layout.addWidget(self.VisualizationCollapsibleButton)
    
    # Layout within the dummy collapsible button
    self.VisualizationFormLayout = qt.QGridLayout(self.VisualizationCollapsibleButton)
    
    #
    # Embedded config panel
    #
    
    #*********Initialize*********#
    
    self.VisualizationConfigInterface = ctk.ctkCollapsibleButton()
    self.VisualizationConfigInterface.text = "Configuration"
    self.VisualizationConfigInterface.collapsed = True
    self.VisualizationFormLayout.addWidget(self.VisualizationConfigInterface, 1, 0, 1, 4)
    
    self.VisualizationConfigInterfaceLayout = qt.QGridLayout(self.VisualizationConfigInterface)
    self.VisualizationConfigInterfaceLayout.setHorizontalSpacing(12)
    
    #*********Operations*********#
    
    # Slider to specify whether to skew landmark registration into lateral or anterior-posterior direction
    self.ScaleSkewSlider =  ctk.ctkSliderWidget()
    self.ScaleSkewSlider.enabled = True
    self.ScaleSkewSlider.singleStep = 0.01
    self.ScaleSkewSlider.minimum = 0.0
    self.ScaleSkewSlider.maximum = 1.0
    self.ScaleSkewSlider.value = 0
    self.ScaleSkewSlider.setToolTip("Set magnitude of rotation applied to anchor points about axial vector from original landmark")
    self.VisualizationConfigInterfaceLayout.addWidget(qt.QLabel("Spine Scale Skew"), 1, 0, 1, 1)
    self.VisualizationConfigInterfaceLayout.addWidget(self.ScaleSkewSlider, 1, 1, 1, 3)

    #*********Connections*********#
    
    self.ScaleSkewSlider.connect("valueChanged(double)", self.OnGenerateVisualizationButtonClicked)
    
    #*********Operations*********#
    
    # Generate surface visualization button
    self.GenerateVisualizationButton = qt.QPushButton("Generate Visualization")
    self.GenerateVisualizationButton.toolTip = "Produce surface visualization based on selected markups node"
    self.GenerateVisualizationButton.enabled = False      # Wait for valid data selection first
    self.VisualizationFormLayout.addWidget(self.GenerateVisualizationButton, 2, 0, 1, 4)
    
    # Visualize L5 to T7 axial plane offset button
    self.VisualizeTranslationalDeformationButton = qt.QPushButton("Visualize Translational Deformation")
    self.VisualizeTranslationalDeformationButton.toolTip = "Display several vectors illustrating tranlational posture abonormalitites, relative to the AverageModel data"
    self.VisualizeTranslationalDeformationButton.enabled = False    # Enable after generating main visualization
    self.VisualizationFormLayout.addWidget(self.VisualizeTranslationalDeformationButton, 3, 0, 1, 4)
    
    #*********Connections*********#
    
    self.GenerateVisualizationButton.connect('clicked(bool)', self.OnGenerateVisualizationButtonClicked)
    self.VisualizeTranslationalDeformationButton.connect('clicked(bool)', self.OnVisualizeTranslationalDeformationButtonClicked)
    
    #
    # Assessment interface
    #
    
    #*********Initialize*********#
    self.AssessmentCollapsibleButton = ctk.ctkCollapsibleButton()
    self.AssessmentCollapsibleButton.text = "Assessment"
    self.layout.addWidget(self.AssessmentCollapsibleButton)
    self.AssessmentFormLayout = qt.QGridLayout(self.AssessmentCollapsibleButton)
    
    #
    # Embedded config panel
    #
    
    #*********Initialize*********#
    
    self.AssessmentConfigInterface = ctk.ctkCollapsibleButton()
    self.AssessmentConfigInterface.text = "Configuration"
    self.AssessmentConfigInterface.collapsed = True
    self.AssessmentFormLayout.addWidget(self.AssessmentConfigInterface, 1, 0, 1, 4)
    
    self.AssessmentConfigInterfaceLayout = qt.QGridLayout(self.AssessmentConfigInterface)
    self.AssessmentConfigInterfaceLayout.setHorizontalSpacing(12)
    
    #*********Operations*********#
    
    self.DoVisualizeMetricsCheckbox = qt.QCheckBox()
    self.DoVisualizeMetricsCheckbox.enabled = True
    self.DoVisualizeMetricsCheckbox.setToolTip("Check to display lines in 3D view showing postural abnormalities")
    self.DoVisualizeMetricsCheckbox.setChecked(1)    
    self.AssessmentConfigInterfaceLayout.addWidget(qt.QLabel("Visualize Posture Metrics"), 1, 0, 1, 1)
    self.AssessmentConfigInterfaceLayout.addWidget(self.DoVisualizeMetricsCheckbox, 1, 1, 1, 1)
    
    # Slider to specify polynomial degrees to fit to points
    self.PolyfitDegreeSpinBox = qt.QSpinBox()
    self.PolyfitDegreeSpinBox.enabled = True
    self.PolyfitDegreeSpinBox.setSingleStep(1)
    self.PolyfitDegreeSpinBox.setRange(1,10)
    self.PolyfitDegreeSpinBox.value = 5
    self.PolyfitDegreeSpinBox.setToolTip("Set point-fit polymoials' degrees")
    self.AssessmentConfigInterfaceLayout.addWidget(qt.QLabel("Polyfit Degree"), 2, 0, 1, 1)
    self.AssessmentConfigInterfaceLayout.addWidget(self.PolyfitDegreeSpinBox, 2, 1, 1, 3)
    
    # Slider to specify the number of points to consider along polynomial curves (resolution per interval)
    self.PointsPerPolyFitSlider = ctk.ctkSliderWidget()
    self.PointsPerPolyFitSlider.enabled = True
    self.PointsPerPolyFitSlider.singleStep = 1
    self.PointsPerPolyFitSlider.minimum = 25
    self.PointsPerPolyFitSlider.maximum = 500
    self.PointsPerPolyFitSlider.value = 100
    self.PointsPerPolyFitSlider.setToolTip("Set number of points used in polynomial-fit evaluations")
    self.AssessmentConfigInterfaceLayout.addWidget(qt.QLabel("Points Per PolyInterval"), 3, 0, 1, 1)
    self.AssessmentConfigInterfaceLayout.addWidget(self.PointsPerPolyFitSlider, 3, 1, 1, 3)
    
    #*********Operations*********#

    self.AssessPostureButton = qt.QPushButton("Assess Posture")
    self.AssessPostureButton.toolTip = "Fill table with posture assessment metrics"
    self.AssessPostureButton.enabled = False      # Wait for valid data selection first
    self.AssessmentFormLayout.addWidget(self.AssessPostureButton, 2, 0, 1, 4)
    
    #*********Connections*********#

    self.AssessPostureButton.connect('clicked(bool)',  self.OnAssessPostureButtonClicked)
    
    #*********Results*********#
    self.ResultsTableWidget = qt.QTableWidget()
    self.ResultsTableWidget.setRowCount(10)
    self.ResultsTableWidget.setVerticalHeaderLabels(10*" ")
    self.ResultsTableWidget.setColumnCount(3)
    self.ResultsTableWidget.setHorizontalHeaderLabels(["Group", "Metric", "Value"])
    self.AssessmentFormLayout.addWidget(self.ResultsTableWidget, 4, 0, 1, 4)
    
    
    #
    # Seperate reload button
    #
    self.ReloadButton = qt.QPushButton("Reload Module")
    self.ReloadButton.toolTip = "Reload this module, reflecting any changes in its code"
    self.ReloadButton.enabled = True
    self.layout.addWidget(self.ReloadButton)
    self.ReloadButton.connect('clicked(bool)', self.OnReloadButtonClicked)
    
    # Add vertical spacer
    self.layout.addStretch(1)

  def OnLandmarksNodeSelectorChanged(self):
    NewNode = self.MarkupsNodeSelector.currentNode()
    if NewNode == None:
      self.GenerateVisualizationButton.enabled = False
      self.VisualizeTranslationalDeformationButton.enabled = False
      self.AssessPostureButton.enabled  = False
      
    else:  
      if NewNode.GetNumberOfFiducials() < 34:
        print "ERROR - module does not currently support incomplete TrP sets, please use PreProcessLandmarks to impute omissions, and StandardizeLandmarks to extrapolate incomplete sets"
        return False
        
      if NewNode.GetNumberOfFiducials() == 34:
        if self.logic != None:
          self.logic = AssessLandmarksLogic(self.MarkupsNodeSelector.currentNode(), self.PolyfitDegreeSpinBox.value, int(self.PointsPerPolyFitSlider.value))
        self.GenerateVisualizationButton.enabled = True
        self.AssessPostureButton.enabled = True
    
  def OnGenerateVisualizationButtonClicked(self):
    OldVisualizationModelNode = slicer.util.getNode(self.MarkupsNodeSelector.currentNode().GetName()+"_Visualization")
    if OldVisualizationModelNode != None:
      slicer.mrmlScene.RemoveNode(OldVisualizationModelNode)
    
    if self.logic == None:
      self.logic = AssessLandmarksLogic(self.MarkupsNodeSelector.currentNode(), self.PolyfitDegreeSpinBox.value, int(self.PointsPerPolyFitSlider.value))
    
    self.logic.GenerateVisualization(self.ScaleSkewSlider.value)
    slicer.mrmlScene.AddNode(self.logic.VisualizationSurfaceModel)
    
    self.VisualizeTranslationalDeformationButton.enabled = True
    
  """
  def OnScaleSkewSliderChanged(self):
    CurrentMakupsNode = self.MarkupsNodeSelector.currentNode()
    if self.logic != None and slicer.util.getNode(CurrentMakupsNode.GetName()+"_Visualization") != None:
      #print "Updatingg visualization"
      self.logic.GenerateVisualization(self.ScaleSkewSlider.value)
  """
  
  #def OnAssessmentConfigChanged(self):
    
  
  def OnVisualizeTranslationalDeformationButtonClicked(self):
    CurrentMakupsNode = self.MarkupsNodeSelector.currentNode()
    if CurrentMakupsNode == None or slicer.util.getNode(CurrentMakupsNode.GetName()+"_Visualization") == None:
      print "ERROR - Primary visualization not found"
      print " Not generating secondary visualiztions"
      return False
    
    self.logic.VisualizeL5T7RadialOffset(CurrentMakupsNode)
    return True
    
  def OnAssessPostureButtonClicked(self):
    self.PatientPostureAssessment = self.logic.PostureAsssessment()
    self.logic.GetCenterlinePlumblineDeviationMetrics(self.logic.PatientMarkupsNode, self.logic.AlignedScaledModelMarkupsNode, self.PatientPostureAssessment, self.PolyfitDegreeSpinBox.value, self.PointsPerPolyFitSlider.value, self.DoVisualizeMetricsCheckbox.isChecked())
    
    self.logic.GetCobbFromLandmarks(self.logic.PatientMarkupsNode, self.PatientPostureAssessment, self.DoVisualizeMetricsCheckbox.isChecked())
    
    for i, ((Group, MetricG), (MetricV, Value)) in enumerate(zip(self.PatientPostureAssessment.GroupsMetrics, self.PatientPostureAssessment.MetricsValues)):
      self.ResultsTableWidget.setItem(i, 0, qt.QTableWidgetItem(str(Group)))
      self.ResultsTableWidget.setItem(i, 1, qt.QTableWidgetItem(str(MetricG)))
      #self.ResultsTableWidget.setItem(i, 2, qt.QTableWidgetItem(str(Value)))
    
    self.ResultsTableWidget.setItem(0, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.CobbAngle)[:5]))
    self.ResultsTableWidget.setItem(1, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.MaxDevR)[:5]))
    self.ResultsTableWidget.setItem(2, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.MaxDevL)[:5]))
    self.ResultsTableWidget.setItem(3, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.RmsDevRl)[:5]))
    self.ResultsTableWidget.setItem(4, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.MaxDevA)[:5]))
    self.ResultsTableWidget.setItem(5, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.MaxDevP)[:5]))
    self.ResultsTableWidget.setItem(6, 2, qt.QTableWidgetItem(str(self.PatientPostureAssessment.RmsDevAp)[:5]))
    
    return True
    
  def cleanup(self):
    pass

  def OnVeiwModified(self):
    if self.logic == None:
      return
    #if self.logic.PatientPostureAssessment.CobbAngle
      

  """
  def onApplyButton(self):
    logic = AssessLandmarksLogic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    imageThreshold = self.imageThresholdSliderWidget.value
    logic.run(self.MarkupsNodeSelector.currentNode(), self.outputSelector.currentNode(), imageThreshold, enableScreenshotsFlag)
  """
  def OnReloadButtonClicked(self):
    self.ClearModuleData()
    if self.logic != None:
      self.logic.ClearMrmlNodes()
      self.logic = None
    print str(slicer.moduleNames.AssessLandmarks) + " reloaded"
    slicer.util.reloadScriptedModule(slicer.moduleNames.AssessLandmarks)
    
#
# AssessLandmarksLogic
#

class AssessLandmarksLogic(ScriptedLoadableModuleLogic):
  class PostureAsssessment:     # PostureAsssessment (noun, no-functions) data structure to hold metric values for GUI table
    def __init__(self):
      self.GroupsMetrics = [('Scoliosis','Cobb Angle'), ('Scoliosis','Max R Dist.'), ('Scoliosis','Max L Dist.'), ('Scoliosis','RMS R-L Dist.'),\
        ('Scoliosis','Max A Dist.'), ('Scoliosis','Max P Dist.'), ('Scoliosis','RMS A-P Dist')]
      self.MetricsValues = [['Cobb Angle', 0.0], ['Max R Dist.',0.0], ['Max L Dist.',0.0], ['RMS R-L Dist.',0.0], \
        ['Max A Dist.',0.0], ['Max P Dist.',0.0], ['RMS A-P Dist.',0.0]]
      
      self.CobbAngle = None
      self.MaxDevR = None
      self.MaxDevL = None
      self.RmsDevRl = None
      self.MaxDevA = None
      self.MaxDevP = None
      self.RmsDevAp = None
      
      
  def __init__(self, MarkupsNode, PolyDegree, PointsPerInterval):
    self.PatientMarkupsNode = MarkupsNode
    
    # PostureAsssessment, for convenient storage of posture metrics
    self.PatientPostureAssessment = self.PostureAsssessment()
    
    # Average-model anatomy to register to patient anatomy
    self.ModelMarkupsNode = slicer.util.getNode("ModelLandmarks")
    self.AverageSpineSurfaceModel = slicer.util.getNode("AverageModel")
    
    # A model markups node which can be scaled to patient size seperately from the visualization registration for quantitative assessement
    ScaledMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    ScaledMarkupsNode.Copy(self.ScaleModelToPatient(self.ModelMarkupsNode, self.PatientMarkupsNode, PolyDegree, PointsPerInterval))
    
    self.AlignedScaledModelMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    self.AlignedScaledModelMarkupsNode.Copy(self.AlignModelToPatient(ScaledMarkupsNode, self.PatientMarkupsNode))
    self.AlignedScaledModelMarkupsNode.CreateDefaultDisplayNodes()
    #slicer.mrmlScene.AddNode(self.AlignedScaledModelMarkupsNode)
    
    # Model node to display patient visualization
    self.VisualizationSurfaceModel = slicer.vtkMRMLModelNode()
    PolyData = vtk.vtkPolyData()
    PolyData.DeepCopy(self.AverageSpineSurfaceModel.GetPolyData())
    self.VisualizationSurfaceModel.SetAndObservePolyData(PolyData)
    self.VisualizationSurfaceModel.SetName(self.PatientMarkupsNode.GetName() + "_Visualization")
    
    """
    # Models require distinct  display nodes
    dn = slicer.vtkMRMLModelDisplayNode()
    self.VisualizationSurfaceModel.SetAndObserveDisplayNodeID(dn.GetID())
    self.VisualizationSurfaceModel.SetDisplayVisibility(0)
    """
    
    #self.VisualizationSurfaceModel.Copy(self.AverageSpineSurfaceModel)
    
    # GlyphMarkupsNode to store location(s) to display transform glyphs
    self.L5T7RadialVectorGlyphNode = slicer.vtkMRMLMarkupsFiducialNode()
    self.L5T7RadialVectorGlyphNode.SetName("L5T7RadialVector")
    #slicer.mrmlScene.AddNode(self.L5T7RadialVectorGlyphNode)
    
  def ClearMrmlNodes(self):
    #slicer.mrmlScene.RemoveNode(self.L5T7RadialVectorGlyphNode)
    
    OldModelRegistrationPoints = slicer.util.getNode("MRP")
    if OldModelRegistrationPoints != None:
      slicer.mrmlScene.RemoveNode(OldModelRegistrationPoints)
    
    OldPatientRegistrationPoints = slicer.util.getNode("PRP")
    if OldPatientRegistrationPoints != None:
      slicer.mrmlScene.RemoveNode(OldPatientRegistrationPoints)
    
    OldRulerAnnotationNodes = slicer.util.getNodesByClass('vtkMRMLAnnotationRulerNode')
    for OldNode in OldRulerAnnotationNodes:
      print OldNode.GetName()
      #if OldNode.GetName().__contains__(self.PatientMarkupsNode.GetName()):
      slicer.mrmlScene.RemoveNode(OldNode)
    
    return True
  
  #
  # Initialization functions
  #
  
  def GetCenterlinePolynomial(self, Node, PolyDegree):
    # Get Left and Right landmark coords, average them for centerline by bilateral symmetry
    LeftCoords = [Node.GetMarkupPointVector(i,0) for i in range(0,Node.GetNumberOfFiducials(),2)]
    RightCoords = [Node.GetMarkupPointVector(i,0) for i in range(1,Node.GetNumberOfFiducials(),2)]
    CenterlineCoords = [[(LeftCoords[Vertebra][dim] + RightCoords[Vertebra][dim])/2.0 for dim in range(3)] for Vertebra in range(len(LeftCoords))]
    
    # Extract dimension components for polynomial fit
    CenterRlCoords = [CenterlineCoords[Vertebra][0] for Vertebra in range(len(LeftCoords))]
    CenterApCoords = [CenterlineCoords[Vertebra][1] for Vertebra in range(len(LeftCoords))]
    CenterSiCoords = [CenterlineCoords[Vertebra][2] for Vertebra in range(len(LeftCoords))]
    
    # Fit polynomials to both other directions with respect to S-I
    Rs_AsPolynomial = (np.polyfit(CenterSiCoords, CenterRlCoords, PolyDegree), np.polyfit(CenterSiCoords, CenterApCoords, PolyDegree))
    return Rs_AsPolynomial
    
  def AlignModelToPatient(self, ModelNode, PatientNode):
    # Clear old nodes and instantiate new ones
    OldCongruentMarkupsNode = slicer.util.getNode("AvgModel for " + PatientNode.GetName())
    if OldCongruentMarkupsNode != None:
      slicer.mrmlScene.RemoveNode(OldCongruentMarkupsNode)
    
    # Get center-points of both spine's ends
    ModelBaseCenterCoords = [(ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-2,0)[dim] + ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    PatientBaseCenterCoords = [(PatientNode.GetMarkupPointVector(PatientNode.GetNumberOfFiducials()-2,0)[dim] + PatientNode.GetMarkupPointVector(PatientNode.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    ModelToPatientTranslation = [PatientBaseCenterCoords[dim] - ModelBaseCenterCoords[dim] for dim in range(3)]
        
    # Node to contain aligned and scaled markups
    AlignedModelMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    AlignedModelMarkupsNode.SetName("AvgModel for " + PatientNode.GetName())
    
    # Reposition average spine reference
    for PointIndex in range(ModelNode.GetNumberOfFiducials()):
      CurrentCoords = ModelNode.GetMarkupPointVector(PointIndex,0)
      AlignedCoords = [CurrentCoords[dim] + ModelToPatientTranslation[dim] for dim in range(3)]
      AlignedModelMarkupsNode.AddFiducialFromArray(AlignedCoords)
      AlignedModelMarkupsNode.SetNthFiducialLabel(PointIndex, ModelNode.GetNthFiducialLabel(PointIndex))
  
    """
    # DEBUGGING
    
    slicer.mrmlScene.AddNode(AlignedModelMarkupsNode)
    AlignmentVtkTransform = slicer.vtkMRMLTransformNode()
    AlignmentVtkMatrix = vtk.vtkMatrix4x4()
    for dim in range(3):
      AlignmentVtkMatrix.SetElement(dim, 3, ModelToPatientTranslation[dim])
    AlignmentVtkTransform.SetAndObserveMatrixTransformFromParent(AlignmentVtkMatrix)
    UndeformedModelCopy = slicer.mrmlScene.CopyNode(self.AverageSpineSurfaceModel)
    UndeformedModelCopy.SetName("AlignedModel")
    #UndeformedModelCopy.SetAndObservePolyData(self.AverageSpineSurfaceModel.GetPolyData())
    #slicer.mrmlScene.AddNode(UndeformedModelCopy)
    #UndeformedModelCopy.CreateDefaultDisplayNodes()
    UndeformedModelCopy.ApplyTransform(AlignmentVtkTransform.GetTransformFromParent())
    UndeformedModelCopy.HardenTransform()
    """
    return AlignedModelMarkupsNode
  
  def ScaleModelToPatient(self, ModelNode, PatientNode, PolyDegree, PointsPerPolyFit):    # Returns a markups node which is AssessLandmarksLogic's self.ModelMarkupsNode, aligned with annd scaled to self.PatientMarkupsNode
    # Clear old nodes and instantiate new ones
    OldCongruentMarkupsNode = slicer.util.getNode("AvgModel for " + PatientNode.GetName())
    if OldCongruentMarkupsNode != None:
      slicer.mrmlScene.RemoveNode(OldCongruentMarkupsNode)
    
    # Get center-points of both spine's ends
    ModelBaseCenterCoords = [(ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-2,0)[dim] + ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    PatientBaseCenterCoords = [(PatientNode.GetMarkupPointVector(PatientNode.GetNumberOfFiducials()-2,0)[dim] + PatientNode.GetMarkupPointVector(PatientNode.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    PatientTopCenterCoords = [(PatientNode.GetMarkupPointVector(0,0)[dim] + PatientNode.GetMarkupPointVector(1,0)[dim])/2.0 for dim in range(3)]
    
    #ModelTopCenterCoords = [(ModelNode.GetMarkupPointVector(0,0)[dim] + ModelNode.GetMarkupPointVector(1,0)[dim])/2.0 for dim in range(3)]
    
    ModelToPatientTranslation = [PatientBaseCenterCoords[dim] - ModelBaseCenterCoords[dim] for dim in range(3)]
    
    UnscaledModelBaseCenterCoords = [(ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-2,0)[dim] + ModelNode.GetMarkupPointVector(ModelNode.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    UnscaledModelTopCenterCoords = [(ModelNode.GetMarkupPointVector(0,0)[dim] + ModelNode.GetMarkupPointVector(1,0)[dim])/2.0 for dim in range(3)]
    
    # Get scales of both spines to compute ratio
    PatientPolynomials = self.GetCenterlinePolynomial(PatientNode, PolyDegree)
    PatientPolynomialCurveLength = self.GetCurveLength(PatientPolynomials, PatientBaseCenterCoords, PatientTopCenterCoords, PointsPerPolyFit)
    ModelPolynomials = self.GetCenterlinePolynomial(ModelNode, PolyDegree)
    ModelPolynomialCurveLength = self.GetCurveLength(ModelPolynomials, UnscaledModelBaseCenterCoords, UnscaledModelTopCenterCoords, PointsPerPolyFit)
    #print "Patient Polynomials: ", PatientPolynomials
    #print "Aligned model polynomials: ", ModelPolynomials 
    
    # Need to deal with negative shrink amounts? Current method should suffice?
    ModelShrinkAmountMm = abs(ModelPolynomialCurveLength - PatientPolynomialCurveLength) # self.MoveCoordsAlongCurve currently requires a positive SlidingDistance, use if-checks for different length (sign) cases
    #print "Patient/model curve lengths: ", PatientPolynomialCurveLength, ModelPolynomialCurveLength
    
    # Node to contain aligned and scaled markups
    ScaledModelMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    ScaledModelMarkupsNode.SetName("AvgModel for " + PatientNode.GetName())
    
    # Shift all AvgModel markup points towards center of their polynomial curve by amount proportioanl to their distance from it and the scaling needed
    ModelCenterSiCoords = self.MoveCoordsAlongCurve(UnscaledModelBaseCenterCoords, ModelPolynomials, ModelPolynomialCurveLength/2.0, PointsPerPolyFit, 1)
    TopPointDistFromCenter = self.GetCurveLength(ModelPolynomials, UnscaledModelTopCenterCoords, ModelCenterSiCoords, PointsPerPolyFit)
    BottomPointDistFromCenter = self.GetCurveLength(ModelPolynomials, ModelCenterSiCoords, UnscaledModelBaseCenterCoords, PointsPerPolyFit)
    for ModelPointIndex in range(ModelNode.GetNumberOfFiducials()):
      CurrentCoords = ModelNode.GetMarkupPointVector(ModelPointIndex,0)
      CurrentDistFromCenter = self.GetCurveLength(ModelPolynomials, CurrentCoords, ModelCenterSiCoords, PointsPerPolyFit)
      
      if ModelPolynomialCurveLength < PatientPolynomialCurveLength:   # Must expand spine outwards
        if CurrentCoords[2] > ModelCenterSiCoords[2]:       # We are above spine center, bring point down to shrink spine
          SlidingDistance = (CurrentDistFromCenter/TopPointDistFromCenter)*ModelShrinkAmountMm*0.5
          CurveShiftedCoords = self.MoveCoordsAlongCurve(CurrentCoords, ModelPolynomials, SlidingDistance, PointsPerPolyFit, 1)       
        else:                                               # We are below spine center, bring point upwards to shrink spine
          SlidingDistance = (CurrentDistFromCenter/BottomPointDistFromCenter)*ModelShrinkAmountMm*0.5
          CurveShiftedCoords = self.MoveCoordsAlongCurve(CurrentCoords, ModelPolynomials, SlidingDistance, PointsPerPolyFit, -1)
      else:   # IF ModelPolynomialCurveLength >= PatientPolynomialCurveLength; must shrink spine inwards
        if CurrentCoords[2] > ModelCenterSiCoords[2]:
          SlidingDistance = (CurrentDistFromCenter/TopPointDistFromCenter)*ModelShrinkAmountMm*0.5
          CurveShiftedCoords = self.MoveCoordsAlongCurve(CurrentCoords, ModelPolynomials, SlidingDistance, PointsPerPolyFit, -1)       
        else:
          SlidingDistance = (CurrentDistFromCenter/BottomPointDistFromCenter)*ModelShrinkAmountMm*0.5
          CurveShiftedCoords = self.MoveCoordsAlongCurve(CurrentCoords, ModelPolynomials, SlidingDistance, PointsPerPolyFit, 1)
          
      ScaledModelMarkupsNode.AddFiducialFromArray(CurveShiftedCoords)
      ScaledModelMarkupsNode.SetNthFiducialLabel(ModelPointIndex, ModelNode.GetNthFiducialLabel(ModelPointIndex))
      
    return ScaledModelMarkupsNode
    
  def GetCurveLength(self, Polynomial, IntervalStart, IntervalEnd, PointsInInterval):     # NOTE Polynomial is of form returned by self.GetCenterlinePolynomial
    # Get closest point on Polynomial curve to interval bounds for parallel curve definition
    CurveIntervalStart = self.GetClosestPolycurveCoordsToPoint(Polynomial, IntervalStart, PointsInInterval)
    CurveIntervalEnd = self.GetClosestPolycurveCoordsToPoint(Polynomial, IntervalEnd, PointsInInterval)
    Interval = np.linspace(CurveIntervalStart[2], CurveIntervalEnd[2], PointsInInterval)
    
    LengthSummation = 0
    for i, Point in enumerate(Interval[1:]):
      DeltaR = np.polyval(Polynomial[0], Point) - np.polyval(Polynomial[0], Interval[i])
      DeltaA = np.polyval(Polynomial[1], Point) - np.polyval(Polynomial[1], Interval[i])
      DeltaS = Point - Interval[i]
      LengthSummation += np.sqrt((DeltaR**2) + (DeltaA**2) + (DeltaS**2))
      
    return LengthSummation
    
  def GetClosestPolycurveCoordsToPoint(self, Polynomial, Point, PointsInInterval):  # Returns Coords, the closest coords to Point lying on Polynomial curves
    # Must find point on Polynomial curve minimally distant to Coords, to move along parallel curve
    PointToCurveVector = [np.polyval(Polynomial[0], Point[2]) - Point[0], np.polyval(Polynomial[1], Point[2]) - Point[1], 0.0]   # Start search for shortest line to curve with vector in axial plane
    
    # A search over an interval the width of DistToCurve in the axial plane guarantees that any point on the curve passing closer to Coords will be considered
    DistToCurve = np.linalg.norm(PointToCurveVector)
    #print ""
    #print DistToCurve, Point
    CurveSearchDomain = np.linspace(Point[2]-DistToCurve, Point[2]+DistToCurve, PointsInInterval)
    MinDistToCurve = DistToCurve
    CurvePointCorrespondingToPoint = [Point[dim] + PointToCurveVector[dim] for dim in range(3)]
    
    for CurvePoint in CurveSearchDomain:
      PointToCurvePointVector = [np.polyval(Polynomial[0], CurvePoint) - Point[0], np.polyval(Polynomial[1], CurvePoint) - Point[1], CurvePoint - Point[2]]
      DistToCurve = np.linalg.norm(PointToCurvePointVector)
      if DistToCurve < MinDistToCurve:
        MinDistToCurve = DistToCurve
        CurvePointCorrespondingToPoint = [Point[dim] + PointToCurvePointVector[dim] for dim in range(3)]
        PointToCurvePointVector = [CurvePointCorrespondingToPoint[dim] - Point[dim] for dim in range(3)]
    
    #print MinDistToCurve
    #print ""
    return CurvePointCorrespondingToPoint
    
  def MoveCoordsAlongCurve(self, Coords, Polynomial, Distance, PointsInInterval, DirectionSign):
    # Get amount to increment S-coord each step, as it is the indepedent variable in Polynomial
    AvgDistPerCurvePoint = Distance / PointsInInterval
    AvgDeltaSiPerPoint = AvgDistPerCurvePoint / 3.0
    
    DistanceRemaining = Distance
    
    # A point moving along Polynomial curve tells us how to move Coords
    MovingCoords = self.GetClosestPolycurveCoordsToPoint(Polynomial, Coords, PointsInInterval)
    CoordsToCurveVector = [MovingCoords[dim] - Coords[dim] for dim in range(3)]
    
    while DistanceRemaining > 0:
      # Get next coords which AvgDeltaSiPerPoint take the point to
      if DirectionSign > 0:
        NextSiCoord = MovingCoords[2] + AvgDeltaSiPerPoint
      else:
        NextSiCoord = MovingCoords[2] - AvgDeltaSiPerPoint
        
      NextRlCoord = np.polyval(Polynomial[0], NextSiCoord)
      NextApCoord = np.polyval(Polynomial[1], NextSiCoord)
      
      DeltaRl = NextRlCoord - MovingCoords[0]
      DeltaAp = NextApCoord - MovingCoords[1]
      DeltaSi = AvgDeltaSiPerPoint
      
      # Shift point along Polynomial curve
      MovingCoords = [NextRlCoord, NextApCoord, NextSiCoord]
      DistanceShifted = np.sqrt((DeltaRl**2) + (DeltaAp**2) + (DeltaSi**2))
      DistanceRemaining -= DistanceShifted
    
    NewCoords = [MovingCoords[dim] + CoordsToCurveVector[dim] for dim in range(3)]
    
    return NewCoords
    
  #
  # Visualization functions
  #
  
  def GenerateVisualization(self, ScaleSkew):
    
    # (Re) initialize
    OldVisualizationModelNode = slicer.util.getNode(self.PatientMarkupsNode.GetName() + "_Visualization")
    if OldVisualizationModelNode != None:
      slicer.mrmlScene.RemoveNode(OldVisualizationModelNode)
    
    self.VisualizationSurfaceModel = slicer.vtkMRMLModelNode()
    PolyData = vtk.vtkPolyData()
    PolyData.DeepCopy(self.AverageSpineSurfaceModel.GetPolyData())
    self.VisualizationSurfaceModel.SetAndObservePolyData(PolyData)
    self.VisualizationSurfaceModel.SetName(self.PatientMarkupsNode.GetName() + "_Visualization")
    
    """
    # Models require distinct  display nodes
    dn = slicer.vtkMRMLModelDisplayNode()
    dn.SetVisibility(1)
    self.VisualizationSurfaceModel.SetAndObserveDisplayNodeID(dn.GetID())
    """
    
    PatientLabelsCoords = [[self.PatientMarkupsNode.GetNthFiducialLabel(i), self.PatientMarkupsNode.GetMarkupPointVector(i,0)] for i in range(self.PatientMarkupsNode.GetNumberOfFiducials())]
    ModelLabelsCoords = [[self.ModelMarkupsNode.GetNthFiducialLabel(i), self.ModelMarkupsNode.GetMarkupPointVector(i,0)] for i in range(self.ModelMarkupsNode.GetNumberOfFiducials())]
    
    # Add anchor points to each of the AverageModel landmarks
    for i, (ModelPointLabel, ModelPointCoords) in enumerate(ModelLabelsCoords):
      if i == self.ModelMarkupsNode.GetNumberOfFiducials():
        break
      AnchorOffsetVector = self.GetAnchorOffsetVector(self.ModelMarkupsNode, i)
      AnchorPointCoords = [(ModelPointCoords[dim] + AnchorOffsetVector[dim]) for dim in range(3)]
      ModelLabelsCoords.append([ModelPointLabel, AnchorPointCoords])
    
   # Add achor points to each of the patient landmarks
    for i, (PatientPointLabel, PatientPointCoords) in enumerate(PatientLabelsCoords):
      if i == self.PatientMarkupsNode.GetNumberOfFiducials():
        break
      AnchorOffsetVector = self.GetAnchorOffsetVector(self.PatientMarkupsNode, i)
      AnchorPointCoords = [(PatientPointCoords[dim] + AnchorOffsetVector[dim]) for dim in range(3)]
      PatientLabelsCoords.append([PatientPointLabel, AnchorPointCoords])

    
    # Create Markups Nodes for anchored landmark sets
    ModelRegistrationMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    for i, (Label, Coords) in enumerate(ModelLabelsCoords):
      ModelRegistrationMarkupsNode.AddFiducialFromArray(Coords)
      ModelRegistrationMarkupsNode.SetNthFiducialLabel(i, Label)

    PatientRegistrationMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    for i, (Label, Coords) in enumerate(PatientLabelsCoords):
      PatientRegistrationMarkupsNode.AddFiducialFromArray(Coords)
      PatientRegistrationMarkupsNode.SetNthFiducialLabel(i, Label)

    # Try correcting for lateral-anterior skew
    SkewAngle = ScaleSkew * 90.0
    for RegistrationPointIndex in range(self.PatientMarkupsNode.GetNumberOfFiducials()):
      self.RotateAnchorPointAboutAxisThroughOriginal(PatientRegistrationMarkupsNode, RegistrationPointIndex, SkewAngle)

    for PointIndex in range(PatientRegistrationMarkupsNode.GetNumberOfFiducials()):
      PatientLabelsCoords[PointIndex][1] = PatientRegistrationMarkupsNode.GetMarkupPointVector(PointIndex, 0)
      
    """
    OldModelPointsNode = slicer.util.getNode("MRP")
    if OldModelPointsNode != None:
      slicer.mrmlScene.RemoveNode(OldModelPointsNode)
    slicer.mrmlScene.AddNode(ModelRegistrationMarkupsNode)
    ModelRegistrationMarkupsNode.SetName("MRP")
      
    OldPatientPointsNode = slicer.util.getNode("PRP")
    if OldPatientPointsNode != None:
      slicer.mrmlScene.RemoveNode(OldPatientPointsNode)
    slicer.mrmlScene.AddNode(PatientRegistrationMarkupsNode)
    PatientRegistrationMarkupsNode.SetName("PRP")
    """
    
    # Instantiate vtk counterparts to mrmlScene objects for thinplatespline registration
    ModelVtkCoords = vtk.vtkFloatArray()
    ModelVtkCoords.SetNumberOfComponents(3)
    ModelVtkCoords.SetNumberOfTuples(len(ModelLabelsCoords))
    for i, (Label, Coords) in enumerate(ModelLabelsCoords):
      ModelVtkCoords.SetTuple(i,Coords)
    ModelVtkPoints = vtk.vtkPoints()
    ModelVtkPoints.SetData(ModelVtkCoords)

    
    PatientVtkCoords = vtk.vtkFloatArray()
    PatientVtkCoords.SetNumberOfComponents(3)
    PatientVtkCoords.SetNumberOfTuples(len(PatientLabelsCoords))
    for i, (Label, Coords) in enumerate(PatientLabelsCoords):
      PatientVtkCoords.SetTuple(i,Coords)
    PatientVtkPoints = vtk.vtkPoints()
    PatientVtkPoints.SetData(PatientVtkCoords)
    
    # Compute vtk thin-plate spline transform, must convert mrmlScene objects to vtk counterparts
    TPST = vtk.vtkThinPlateSplineTransform()
    TPST.SetSourceLandmarks(ModelVtkPoints)
    TPST.SetTargetLandmarks(PatientVtkPoints)
    TPST.Update()
    
    # Transform un-deformed spine to generate visualization
    self.VisualizationSurfaceModel.ApplyTransform(TPST)
    self.VisualizationSurfaceModel.HardenTransform()
    
    slicer.mrmlScene.AddNode(self.VisualizationSurfaceModel)
    self.VisualizationSurfaceModel.CreateDefaultDisplayNodes()
    self.VisualizationSurfaceModel.SetDisplayVisibility(1)

    
  def GetAnchorOffsetVector(self, Node, PointIndex):
    # This is becomming disorganized - TODO: COme up with systematic way for applying geometry corrections
    #LabelsCoords = [(Node.GetNthFiducialLabel(i), Node.GetMarkupPointVector(i,0)) for i in range(Node.GetNumberOfFiducials())]
    LateralVector = self.GetLateralVector(Node, PointIndex)
    LateralVectorNorm = np.linalg.norm(LateralVector)
    LateralUnitVector = [(LateralVector[dim]/LateralVectorNorm) for dim in range(3)]
    
    AxialVector = self.GetAxialVector(Node, PointIndex)
    AxialVectorNorm = np.linalg.norm(AxialVector)
    AxialUnitVector = [(AxialVector[dim]/AxialVectorNorm) for dim in range(3)]
    
    InflationFactor = self.GetInflationFactor(Node, PointIndex)
    #InflationCorrectionFactor = max(InflationFactor-1.0,0)
    InflationCorrectionFactor = (abs(InflationFactor-1.0)**1.5) * np.sign((InflationFactor-1.0))
    #print Node.GetName() + " - " + Node.GetNthFiducialLabel(PointIndex) + " ICF: " + str(InflationCorrectionFactor)

    InflationCorrectionVector = [(LateralVector[dim] * (InflationCorrectionFactor)) for dim in range(3)]
    
    UnscaledAnteriorVector = np.cross(LateralUnitVector, AxialUnitVector)
    # Reflect UnscaledAnteriorVector if it points into posterior direction
    if UnscaledAnteriorVector[1] < 0:
      UnscaledAnteriorVector = [(-1*UnscaledAnteriorVector[dim]) for dim in range(3)]
      
    # Scale UnscaledAnteriorVector
    #AnchorOffsetScale = np.linalg.norm(AxialVector)
    AnchorOffsetScale = (self.GetSpineLength(Node)/(Node.GetNumberOfFiducials()/2))
    
    UnscaledAnteriorVectorNorm = np.linalg.norm(UnscaledAnteriorVector)
    #AnchorOffsetVector = [((UnscaledAnteriorVector[dim])*(InflationFactor*AnchorOffsetScale/UnscaledAnteriorVectorNorm)) for dim in range(3)]
    AnchorOffsetVector = [(((UnscaledAnteriorVector[dim])*(AnchorOffsetScale/(UnscaledAnteriorVectorNorm)))+InflationCorrectionVector[dim]) for dim in range(3)]
    
    return AnchorOffsetVector

  def GetLateralVector(self, Node, PointIndex):
    (CurentLabel, CurrentCoords) = (Node.GetNthFiducialLabel(PointIndex), Node.GetMarkupPointVector(PointIndex,0))
    
    if Node.GetNthFiducialLabel(PointIndex)[-1] == "L":
      LateralNeighborCoords = Node.GetMarkupPointVector(PointIndex+1, 0)
      LateralVector = [(CurrentCoords[dim] - LateralNeighborCoords[dim]) for dim in range(3)]
      return LateralVector
      
    if Node.GetNthFiducialLabel(PointIndex)[-1] == "R":
      LateralNeighborCoords = Node.GetMarkupPointVector(PointIndex-1, 0)
      LateralVector = [(CurrentCoords[dim] - LateralNeighborCoords[dim]) for dim in range(3)]
      return LateralVector

    
  def GetAxialVector(self, Node, PointIndex):
    ExpectedVertebraLabelsInOrder = ['T1','T2','T3','T4','T5','T6','T7','T8','T9','T10','T11','T12','L1','L2','L3','L4','L5']
    (CurentLabel, CurrentCoords) = (Node.GetNthFiducialLabel(PointIndex), Node.GetMarkupPointVector(PointIndex,0))
    ExpectedOrderIndex = ExpectedVertebraLabelsInOrder.index(CurentLabel[:-1])

    if PointIndex < 2:# or (Node.GetNthFiducialLabel(PointIndex-2)[:-1] == ExpectedVertebraLabelsInOrder[ExpectedOrderIndex-1] and Node.GetNthFiducialLabel(PointIndex-2)[-1] == CurentLabel[-1]):
      # Axial neighbor must be below current point 
      if CurentLabel[-1] == "L":
        LeftNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+2, 0)
        LeftAxialVector = [(CurrentCoords[dim] - LeftNeighborBelowCoords[dim])for dim in range(3)]
        RightNeighborBesideCoords = Node.GetMarkupPointVector(PointIndex+1, 0)
        RightNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+3, 0)
        RightAxialVector = [(RightNeighborBesideCoords[dim] - RightNeighborBelowCoords[dim])for dim in range(3)]
      else:
        LeftNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+1, 0)
        LeftNeighborBesideCoords = Node.GetMarkupPointVector(PointIndex-1, 0)
        LeftAxialVector = [(LeftNeighborBesideCoords[dim] - LeftNeighborBelowCoords[dim])for dim in range(3)]
        RightNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+2, 0)
        RightAxialVector = [(CurrentCoords[dim] - RightNeighborBelowCoords[dim])for dim in range(3)]        
      AxialVector = [(LeftAxialVector[dim] + RightAxialVector[dim])/2.0 for dim in range(3)]
      return AxialVector
    
    if PointIndex > Node.GetNumberOfFiducials() - 3:# (Node.GetNthFiducialLabel(PointIndex+2)[:-1] == ExpectedVertebraLabelsInOrder[ExpectedOrderIndex+1] and Node.GetNthFiducialLabel(PointIndex+2)[-1] == CurentLabel[-1]):
      # Axial neighbor must be above current point in markups list
      if CurentLabel[-1] == "L":
        LeftNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-2, 0)
        LeftAxialVector = [(LeftNeighborAboveCoords[dim] - CurrentCoords[dim])for dim in range(3)]
        RightNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-1, 0)
        RightNeighborBesideCoords = Node.GetMarkupPointVector(PointIndex+1, 0)
        RightAxialVector = [(RightNeighborAboveCoords[dim] - RightNeighborBesideCoords[dim])for dim in range(3)]
      else:
        LeftNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-3, 0)
        LeftNeighborBesideCoords = Node.GetMarkupPointVector(PointIndex-1, 0)
        LeftAxialVector = [(LeftNeighborAboveCoords[dim] - LeftNeighborBesideCoords[dim])for dim in range(3)]
        RightNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-2, 0)
        RightAxialVector = [(RightNeighborAboveCoords[dim] - CurrentCoords[dim])for dim in range(3)]
      AxialVector = [(LeftAxialVector[dim] + RightAxialVector[dim])/2.0 for dim in range(3)]
      return AxialVector
      
    # ELSE: There is an axial neighbor above and below our point, we should average
    if CurentLabel[-1] == "L":
      LeftNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-2, 0)
      LeftNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+2, 0)
      LeftAxialVector = [(LeftNeighborAboveCoords[dim] - LeftNeighborBelowCoords[dim])/2.0 for dim in range(3)]
      RightNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-1, 0)
      RightNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+3, 0)
      RightAxialVector = [(RightNeighborAboveCoords[dim] - RightNeighborBelowCoords[dim])/2.0 for dim in range(3)]
      AxialVector = [(LeftAxialVector[dim] + RightAxialVector[dim])/2.0 for dim in range(3)]
    else: # if CurentLabel[-1] == "R"
      LeftNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-3, 0)
      LeftNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+1, 0)
      LeftAxialVector = [(LeftNeighborAboveCoords[dim] - LeftNeighborBelowCoords[dim])/2.0 for dim in range(3)]
      RightNeighborAboveCoords = Node.GetMarkupPointVector(PointIndex-2, 0)
      RightNeighborBelowCoords = Node.GetMarkupPointVector(PointIndex+2, 0)
      RightAxialVector = [(RightNeighborAboveCoords[dim] - RightNeighborBelowCoords[dim])/2.0 for dim in range(3)]
      AxialVector = [(LeftAxialVector[dim] + RightAxialVector[dim])/2.0 for dim in range(3)]
    
    return AxialVector
    
  def GetInflationFactor(self, Node, PointIndex):
    # Returns (VertebraWidth / ModelVertebraWidth) / (VertebraLength / ModelVertebraLength)
    # This InflationFactor serves as an estimate of how much TrP scale variation, independent of other size variation, locally dilates visualization
    VertebraWidth = np.linalg.norm(self.GetLateralVector(Node, PointIndex))
    ModelVertebraWidth = np.linalg.norm(self.GetLateralVector(self.ModelMarkupsNode, PointIndex))
    
    ThisAxialVector = self.GetAxialVector(Node, PointIndex)
    ThisModelAxialVector = self.GetAxialVector(self.ModelMarkupsNode, PointIndex)
    
    if Node.GetNthFiducialLabel(PointIndex)[-1] == "L":
      ContraLateralAxialVector = self.GetAxialVector(Node, PointIndex+1)
      ContraLateralModelAxialVector = self.GetAxialVector(self.ModelMarkupsNode, PointIndex+1)
    else:
      ContraLateralAxialVector = self.GetAxialVector(Node, PointIndex-1)
      ContraLateralModelAxialVector = self.GetAxialVector(self.ModelMarkupsNode, PointIndex-1)
    
    PatientAxialVector = [(ThisAxialVector[dim] + ContraLateralAxialVector[dim])/2.0 for dim in range(3)]
    VertebraLength = np.linalg.norm(PatientAxialVector)
    
    ModelAxialVector = [(ThisModelAxialVector[dim] + ContraLateralModelAxialVector[dim])/2.0 for dim in range(3)]
    ModelVertebraLength = np.linalg.norm(ModelAxialVector)
    
    InflationFactor = ((VertebraWidth/ModelVertebraWidth) / (VertebraLength/ModelVertebraLength))
    
    #print Node.GetName() + " - " + Node.GetNthFiducialLabel(PointIndex) + " I.F. - " + str(InflationFactor)
    
    return InflationFactor
    
  def RotateAnchorPointAboutAxisThroughOriginal(self, Node, PointIndex, Angle):
    if Node.GetNthFiducialLabel(PointIndex)[-1] == "L":
      Angle *= -1
  
    OriginalCoords = Node.GetMarkupPointVector(PointIndex, 0)
    CorrespondingAnchorCoords = Node.GetMarkupPointVector(PointIndex+34, 0)
    OriginalToAnchorCoords = [(CorrespondingAnchorCoords[dim] - OriginalCoords[dim]) for dim in range(3)]
    
    Axis = self.GetAxialVector(Node, PointIndex)
    AxisNorm = np.linalg.norm(Axis)
    UnitAxis = [(Axis[dim])/AxisNorm for dim in range(3)]
    RotMat = self.GetVtkRotationMatrixAboutAxis(Angle, UnitAxis)
    
    RotatedAnchorCoords = OriginalCoords
    for row in range(3):
      #RotatedAnchorCoords[row] = OriginalCoords[dim]
      for col in range(3):
        RotatedAnchorCoords[row] += OriginalToAnchorCoords[col] * RotMat.GetElement(row,col)
    
    Node.SetNthFiducialPositionFromArray(PointIndex+34, RotatedAnchorCoords)
    return True
    
  def AnnotateDeviation(self, Node, DeviationLocumCoords, DeviationVector, DeviationName):
    # Re-initialize scene
    OldRulerNodes = slicer.util.getNodesByClass('vtkMRMLAnnotationRulerNode')
    for OldNode in OldRulerNodes:
      if OldNode.GetName() == Node.GetName() + "_" + DeviationName:
        slicer.mrmlScene.RemoveNode(OldNode)
    
    RecognizedNames = ['Max R','Max L','Max A','Max P']
    CorrespondingColors = [[100,50,255], [0,1,0], [0,0,1], [255,130,0]]
    IsName = [RecognizedNames[i]==DeviationName for i in range(len(RecognizedNames))]    # There should be at most 1 True in IsName
    if IsName.__contains__(True):
      NameIndex = IsName.index(True)
    else:
      NameIndex = -1
    NodeBaseCenterCoords = [(Node.GetMarkupPointVector(Node.GetNumberOfFiducials()-2,0)[dim] + Node.GetMarkupPointVector(Node.GetNumberOfFiducials()-1,0)[dim])/2.0 for dim in range(3)]
    
    # Initialize node
    RulerNode = slicer.vtkMRMLAnnotationRulerNode()
    slicer.mrmlScene.AddNode(RulerNode)
    RulerNode.SetName(Node.GetName() + "_" + DeviationName)
    
    # Set points
    RulerNode.SetPosition1(DeviationLocumCoords)
    RulerNode.SetPosition2([DeviationLocumCoords[dim] + DeviationVector[dim] for dim in range(3)])
    
    # Modify display nodes
    RulerNode.CreateDefaultDisplayNodes()
    
    RulerNode.CreateAnnotationTextDisplayNode()
    tdn = RulerNode.GetAnnotationTextDisplayNode()
    if NameIndex >= 0: 
      tdn.SetColor(CorrespondingColors[NameIndex])
    tdn.SetTextScale(12)
    
    RulerNode.CreateAnnotationPointDisplayNode()
    pdn = RulerNode.GetAnnotationPointDisplayNode()
    if NameIndex >= 0: 
      pdn.SetColor(CorrespondingColors[NameIndex])
    pdn.SetGlyphScale(7)  
    
    RulerNode.CreateAnnotationLineDisplayNode()
    ldn = RulerNode.GetAnnotationLineDisplayNode()
    if NameIndex >= 0: 
      ldn.SetColor(CorrespondingColors[NameIndex])
    ldn.SetLineThickness(7)
    
    
    return True

  def VisualizeCobbAngle(self, Node, Angle, MaxNegAngleIndex, MaxPosAngleIndex):
    CobbAngleRulerColor = [255,0,0]
    
    OldRulerNodes = slicer.util.getNodesByClass('vtkMRMLAnnotationRulerNode')
    for OldNode in OldRulerNodes:
      if OldNode.GetName().__contains__("Cobb"):
        slicer.mrmlScene.RemoveNode(OldNode)
    
    if MaxNegAngleIndex < MaxPosAngleIndex:
      TopVertebraIndex = MaxNegAngleIndex
      BottomVertebraIndex = MaxPosAngleIndex
    else:
      TopVertebraIndex = MaxPosAngleIndex
      BottomVertebraIndex = MaxNegAngleIndex
    
    # Get inside curve direction 
    TopLeftCoords = Node.GetMarkupPointVector(TopVertebraIndex*2,0)
    TopRightCoords = Node.GetMarkupPointVector(TopVertebraIndex*2 + 1,0)
    TopLeftToRightVector = [(TopRightCoords[dim] - TopLeftCoords[dim])for dim in range(3)]
      
    BottomLeftCoords = Node.GetMarkupPointVector(BottomVertebraIndex*2,0)
    BottomRightCoords = Node.GetMarkupPointVector(BottomVertebraIndex*2 + 1,0)
    BottomLeftToRightVector = [(BottomRightCoords[dim] - BottomLeftCoords[dim])for dim in range(3)]
    
    
    if TopLeftToRightVector[2] > 0:
      TopOutsideToInsideVector = [-1*TopLeftToRightVector[dim] for dim in range(3)]
      TopOutsidePoint = Node.GetMarkupPointVector(TopVertebraIndex*2+1,0)
      TopInsidePoint = [TopOutsidePoint[dim] + 2.0*TopOutsideToInsideVector[dim] for dim in range(3)]
      
      BottomOutsideToInsideVector = [-1*BottomLeftToRightVector[dim] for dim in range(3)]
      BottomOutsidePoint = Node.GetMarkupPointVector(BottomVertebraIndex*2+1,0)
      BottomInsidePoint = [BottomOutsidePoint[dim] + 2.0*BottomOutsideToInsideVector[dim] for dim in range(3)]
      
    else:
      TopOutsideToInsideVector = TopLeftToRightVector
      TopOutsidePoint = Node.GetMarkupPointVector(TopVertebraIndex*2,0)
      TopInsidePoint = [TopOutsidePoint[dim] + 2.0*TopOutsideToInsideVector[dim] for dim in range(3)]
      
      BottomOutsideToInsideVector = BottomLeftToRightVector
      BottomOutsidePoint = Node.GetMarkupPointVector(BottomVertebraIndex*2,0)
      BottomInsidePoint = [BottomOutsidePoint[dim] + 2.0*BottomOutsideToInsideVector[dim] for dim in range(3)]
    
    # Find furthest point of line from middle laterally, draw even angle annotation lines
    if abs(TopInsidePoint[0]) > abs(BottomInsidePoint[0]):
      InnermostRlCoords = TopInsidePoint
    else:
      InnermostRlCoords = BottomInsidePoint
    
    TopRulerNode = slicer.vtkMRMLAnnotationRulerNode()
    TopRulerNode.SetPosition1([InnermostRlCoords[0]]+TopInsidePoint[1:])
    TopRulerNode.SetPosition2(TopOutsidePoint)
    slicer.mrmlScene.AddNode(TopRulerNode)
    TopRulerNode.SetName(Node.GetName() + "_Cobb Angle: " + str(Angle)[:5])
    TopRulerNode.CreateDefaultDisplayNodes()
    
    TopRulerNode.CreateAnnotationLineDisplayNode()
    tldn = TopRulerNode.GetAnnotationLineDisplayNode()
    tldn.SetColor(CobbAngleRulerColor)
    tldn.SetLineThickness(7)
    
    TopRulerNode.CreateAnnotationPointDisplayNode()
    tpdn = TopRulerNode.GetAnnotationPointDisplayNode()
    #tpdn.SetColor(CobbAngleRulerColor)
    tpdn.SetGlyphScale(0)
    
    TopRulerNode.CreateAnnotationTextDisplayNode()
    ttdn = TopRulerNode.GetAnnotationTextDisplayNode()
    ttdn.SetColor(CobbAngleRulerColor)
    ttdn.SetTextScale(12)
    
    BottomRulerNode = slicer.vtkMRMLAnnotationRulerNode()
    BottomRulerNode.SetPosition1(BottomOutsidePoint)
    BottomRulerNode.SetPosition2([InnermostRlCoords[0]]+BottomInsidePoint[1:])
    slicer.mrmlScene.AddNode(BottomRulerNode)
    BottomRulerNode.SetName("")
    BottomRulerNode.CreateDefaultDisplayNodes()
    
    BottomRulerNode.CreateAnnotationLineDisplayNode()
    bldn = BottomRulerNode.GetAnnotationLineDisplayNode()
    bldn.SetColor(CobbAngleRulerColor)
    bldn.SetLineThickness(7)
    
    BottomRulerNode.CreateAnnotationPointDisplayNode()
    bpdn = BottomRulerNode.GetAnnotationPointDisplayNode()
    #tpdn.SetColor(CobbAngleRulerColor)
    bpdn.SetGlyphScale(0)
    
    BottomRulerNode.CreateAnnotationTextDisplayNode()
    btdn = BottomRulerNode.GetAnnotationTextDisplayNode()
    btdn.SetColor(CobbAngleRulerColor)
    btdn.SetTextScale(0)
  
  #
  # Posture assessment functions
  #
   
  def GetCobbFromLandmarks(self, Node, PostureAssessment, DoVisualizeCobb):
    LeftCoords = [Node.GetMarkupPointVector(i, 0) for i in range(0, Node.GetNumberOfFiducials(), 2)]
    RightCoords = [Node.GetMarkupPointVector(i, 0) for i in range(1, Node.GetNumberOfFiducials(), 2)]
    
    CoronalAngles = [(180.0/np.pi)*np.math.asin((RightCoords[i][2]-LeftCoords[i][2])/(RightCoords[i][0]-LeftCoords[i][0])) for i in range(len(LeftCoords))]
    MaxNegAngle = 90.0
    MaxPosAngle = -90.0
    for VertebraIndex, Angle in enumerate(CoronalAngles):
      if Angle > MaxPosAngle:
        MaxPosAngle = Angle
        MaxPosAngleIndex = VertebraIndex
      if Angle < MaxNegAngle:
        MaxNegAngle = Angle
        MaxNegAngleIndex = VertebraIndex
    
    CobbEstimate = MaxPosAngle - MaxNegAngle
    PostureAssessment.CobbAngle = CobbEstimate
    
    if DoVisualizeCobb:
      self.VisualizeCobbAngle(Node, CobbEstimate, MaxNegAngleIndex, MaxPosAngleIndex)
    
    return True
    
  def GetCobbFromCurve(self, RlPolynomial, PostureAssessment):
   # NOT IMPLEMENTED
   return True
   
   
  def GetCenterlinePlumblineDeviationMetrics(self, Node, CongruentAverageNode, PostureAssessment, PolyDegree, PointsPerPolyFit, DoVisualizeMetrics):
    # Get reference and patient anatomic polyfits
    CongruentAveragePolynomials = self.GetCenterlinePolynomial(CongruentAverageNode, PolyDegree)
    PatientPolynomials = self.GetCenterlinePolynomial(Node, PolyDegree)
    
    CongruentToPatientRlPolynomial = np.polysub(PatientPolynomials[0], CongruentAveragePolynomials[0])
    CongruentToPatientApPolynomial = np.polysub(PatientPolynomials[1], CongruentAveragePolynomials[1])
    
    SiIntervalStart = min(Node.GetMarkupPointVector(Node.GetNumberOfFiducials()-1,0)[2], Node.GetMarkupPointVector(Node.GetNumberOfFiducials()-2,0)[2])
    SiIntervalEnd = max(Node.GetMarkupPointVector(0,0)[2], Node.GetMarkupPointVector(1,0)[2])
    
    IntervalSearchSpace = np.linspace(SiIntervalEnd, SiIntervalStart, PointsPerPolyFit)
    
    MaxDeviationsRsLs = [[0,0],[0,0]]
    MaxDeviationsAsPs = [[0,0],[0,0]]
    SumSquaredRlDeviations = 0
    SumSquaredApDeviations = 0
    
    for SearchSpacePoint in IntervalSearchSpace:
      # Check for greatest right and left deviations
      CorrespondingRlDeviation = np.polyval(CongruentToPatientRlPolynomial, SearchSpacePoint)
      if CorrespondingRlDeviation > MaxDeviationsRsLs[0][0]:
        MaxDeviationsRsLs[0][0] = CorrespondingRlDeviation
        MaxDeviationsRsLs[0][1] = SearchSpacePoint
      elif CorrespondingRlDeviation < MaxDeviationsRsLs[1][0]:
        MaxDeviationsRsLs[1][0] = CorrespondingRlDeviation
        MaxDeviationsRsLs[1][1] = SearchSpacePoint
    
      # Check for greatest anterior posterior deviations
      CorrespondingApDeviation = np.polyval(CongruentToPatientApPolynomial, SearchSpacePoint)
      if CorrespondingApDeviation > MaxDeviationsAsPs[0][0]:
        MaxDeviationsAsPs[0][0] = CorrespondingApDeviation
        MaxDeviationsAsPs[0][1] = SearchSpacePoint
      elif CorrespondingApDeviation < MaxDeviationsAsPs[1][0]:
        MaxDeviationsAsPs[1][0] = CorrespondingApDeviation
        MaxDeviationsAsPs[1][1] = SearchSpacePoint
    
      # Sum squarred deviation
      SumSquaredRlDeviations += CorrespondingRlDeviation ** 2
      SumSquaredApDeviations += CorrespondingApDeviation ** 2
      
    # Take root mean of sum squarred
    RlRmsDeviation = np.sqrt(SumSquaredRlDeviations/len(IntervalSearchSpace))
    ApRmsDeviation = np.sqrt(SumSquaredApDeviations/len(IntervalSearchSpace))
    
    # Set posture assessement attributes
    PostureAssessment.MaxDevR = MaxDeviationsRsLs[0][0]
    PostureAssessment.MaxDevL = MaxDeviationsRsLs[1][0]
    PostureAssessment.RmsDevRl = RlRmsDeviation
    PostureAssessment.MaxDevA = MaxDeviationsAsPs[0][0]
    PostureAssessment.MaxDevP = MaxDeviationsAsPs[1][0]
    PostureAssessment.RmsDevAp = ApRmsDeviation
    """
    PostureAssessment.MetricsValues[0][1] = MaxDeviationsRsLs[0][0]
    PostureAssessment.MetricsValues[1][1] = abs(MaxDeviationsRsLs[1][0])
    PostureAssessment.MetricsValues[2][1] = RlRmsDeviation
    PostureAssessment.MetricsValues[3][1] = MaxDeviationsAsPs[0][0]
    PostureAssessment.MetricsValues[4][1] = abs(MaxDeviationsAsPs[1][0])
    PostureAssessment.MetricsValues[5][1] = ApRmsDeviation
    """
    if DoVisualizeMetrics:
      MaxRLocum = [np.polyval(PatientPolynomials[0], MaxDeviationsRsLs[0][1]), np.polyval(PatientPolynomials[1], MaxDeviationsRsLs[0][1]), MaxDeviationsRsLs[0][1]]
      MaxRVector = [-MaxDeviationsRsLs[0][0], 0, 0]
      self.AnnotateDeviation(Node, MaxRLocum, MaxRVector, 'Max R')
      
      MaxLLocum = [np.polyval(PatientPolynomials[0], MaxDeviationsRsLs[1][1]), np.polyval(PatientPolynomials[1], MaxDeviationsRsLs[1][1]), MaxDeviationsRsLs[1][1]]
      MaxLVector = [-MaxDeviationsRsLs[1][0], 0, 0]
      self.AnnotateDeviation(Node, MaxLLocum, MaxLVector, 'Max L')
      
      MaxALocum = [np.polyval(PatientPolynomials[0], MaxDeviationsAsPs[0][1]), np.polyval(PatientPolynomials[1], MaxDeviationsAsPs[0][1]), MaxDeviationsAsPs[0][1]]
      MaxAVector = [0, -MaxDeviationsAsPs[0][0], 0]
      self.AnnotateDeviation(Node, MaxALocum, MaxAVector, 'Max A')
      
      MaxPLocum = [np.polyval(PatientPolynomials[0], MaxDeviationsAsPs[1][1]), np.polyval(PatientPolynomials[1], MaxDeviationsAsPs[1][1]), MaxDeviationsAsPs[1][1]]
      MaxPVector = [0, -MaxDeviationsAsPs[1][0], 0]
      self.AnnotateDeviation(Node, MaxPLocum, MaxPVector, 'Max P')
      
    return True
    
  def GetVtkRotationMatrixAboutAxis(self, Angle, Axis):
    x = Axis[0]
    y = Axis[1]
    z = Axis[2]
    c = np.math.cos(Angle*(np.pi/180.0))
    s = np.math.sin(Angle*(np.pi/180.0))
    
    RotMat = vtk.vtkMatrix3x3()
    RotMat.SetElement(0,0,((x**2)*(1-c))+c)
    RotMat.SetElement(0,1,((x*y)*(1-c))-(z*s))
    RotMat.SetElement(0,2,((x*z)*(1-c))+(y*s))
    
    RotMat.SetElement(1,0,((x*y)*(1-c))+(z*s))
    RotMat.SetElement(1,1,((y**2)*(1-c))+c)
    RotMat.SetElement(1,2,((y*z)*(1-c))-(x*s))
    
    RotMat.SetElement(2,0,((x*z)*(1-c))-(y*s))
    RotMat.SetElement(2,1,((y*z)*(1-c))+(x*s))
    RotMat.SetElement(2,2,((z**2)*(1-c))+c)
  
    return RotMat
  
  def VisualizeL5T7RadialOffset(self, Node):
    # Get from transform
    L5T7RadialVector = self.GetT7L5RadialVector(Node)
    
    # Also get to transform (initial display glyph at L5)
    OrigL5MidpointVector = [(Node.GetMarkupPointVector(32,0)[dim] + Node.GetMarkupPointVector(33,0)[dim])/2.0 for dim in range(3)]
    
    # Represent transforms as vtkMatrix4x4
    L5T7RadialVectorTransformMatrix = vtk.vtkMatrix4x4()
    OrigL5MidpointTransformMatrix = vtk.vtkMatrix4x4()
    for dim in range(3):
      L5T7RadialVectorTransformMatrix.SetElement(dim, 3, L5T7RadialVector[dim])
      OrigL5MidpointTransformMatrix.SetElement(dim, 3, OrigL5MidpointVector[dim])
     
    # mrmlTransformNode representation
    L5T7RadialVectorTransformNode = slicer.vtkMRMLTransformNode()
    L5T7RadialVectorTransformNode.SetAndObserveMatrixTransformFromParent(L5T7RadialVectorTransformMatrix)
    L5T7RadialVectorTransformNode.SetName("L5ToT7OffsetInAxialPlane")
    
    # Update mrmlScene
    OldTransformNode = slicer.util.getNode("L5ToT7OffsetInAxialPlane")
    if OldTransformNode != None:
      slicer.mrmlScene.RemoveNode(OldTransformNode)
    slicer.mrmlScene.AddNode(L5T7RadialVectorTransformNode)
    
    # MarkupsNode to contain glyph visualization locations
    self.L5T7RadialVectorGlyphNode.AddFiducialFromArray(OrigL5MidpointVector)
    self.L5T7RadialVectorGlyphNode.SetNthFiducialLabel(self.L5T7RadialVectorGlyphNode.GetNumberOfFiducials()-1, "L5T7RadialVector")
    
    # Update transform display node
    L5T7RadialVectorTransformNode.CreateDefaultDisplayNodes()
    dn = L5T7RadialVectorTransformNode.GetDisplayNode()
    dn.SetVisualizationMode(dn.VIS_MODE_GLYPH)
    dn.SetGlyphType(dn.GLYPH_TYPE_ARROW)
    dn.SetAndObserveGlyphPointsNode(self.L5T7RadialVectorGlyphNode)
    dn.SetVisibility(1)
    dn.SetGlyphResolution(20)
    dn.SetGlyphShaftDiameterPercent(33)
    dn.SetGlyphTipLengthPercent(15)
    
      
    #L5T7RadialVectorTransformNod
    #dn.SetColor([0,1,0])
    
  def GetSpineLength(self, Node):
    # Uses Pythagorus, point to point along each side, and averages both sides' curves
    LeftRightCoords = [(Node.GetMarkupPointVector(point,0), Node.GetMarkupPointVector(point+1,0)) for point in range(0,Node.GetNumberOfFiducials(),2)]
    NumVertebra = len(LeftRightCoords)
    LengthSummation = 0
    for VertebraIndex, (LeftCoords, RightCoords) in enumerate(LeftRightCoords[:-1]):
      LeftIntervalLength = np.linalg.norm([(LeftRightCoords[VertebraIndex+1][0][dim] - LeftCoords[dim]) for dim in range(3)])
      RightIntervalLength = np.linalg.norm([(LeftRightCoords[VertebraIndex+1][1][dim] - RightCoords[dim]) for dim in range(3)])
      IntervalAverageLength = (LeftIntervalLength + RightIntervalLength)/2.0
      LengthSummation += IntervalAverageLength
    
    return LengthSummation
    
  def GetT7L5RadialVector(self, Node):
    # Compared to the reference radial vector of the model spine, this vector indicates translational deformation relative to the plumbline
    # L5T7RadialVector is the vector joining the mid points of T7 and L5 TrP pairs, projected onto the axial plane
    
    # Get midpoints
    T7Midpoint = [(Node.GetMarkupPointVector(12,0)[dim] + Node.GetMarkupPointVector(13,0)[dim])/2.0 for dim in range(3)]
    L5Midpoint = [(Node.GetMarkupPointVector(32,0)[dim] + Node.GetMarkupPointVector(33,0)[dim])/2.0 for dim in range(3)]
  
    L5T7RadialVector = [(L5Midpoint[0]-T7Midpoint[0]), (L5Midpoint[1]-T7Midpoint[1]), 0]
  
    return L5T7RadialVector
  
  def run(self, inputVolume, outputVolume, imageThreshold, enableScreenshots=0):
    """
    Run the actual algorithm
    """

    if not self.isValidInputOutputData(inputVolume, outputVolume):
      slicer.util.errorDisplay('Input volume is the same as output volume. Choose a different output volume.')
      return False

    logging.info('Processing started')

    # Compute the thresholded output volume using the Threshold Scalar Volume CLI module
    cliParams = {'InputVolume': inputVolume.GetID(), 'OutputVolume': outputVolume.GetID(), 'ThresholdValue' : imageThreshold, 'ThresholdType' : 'Above'}
    cliNode = slicer.cli.run(slicer.modules.thresholdscalarvolume, None, cliParams, wait_for_completion=True)

    # Capture screenshot
    if enableScreenshots:
      self.takeScreenshot('AssessLandmarksTest-Start','MyScreenshot',-1)

    logging.info('Processing completed')

    return True


class AssessLandmarksTest(ScriptedLoadableModuleTest):
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
    self.test_AssessLandmarks1()

  def test_AssessLandmarks1(self):
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
    logic = AssessLandmarksLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
