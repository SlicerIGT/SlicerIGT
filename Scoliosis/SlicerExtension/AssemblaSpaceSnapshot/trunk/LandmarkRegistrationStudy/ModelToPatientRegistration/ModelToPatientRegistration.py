from __main__ import vtk, qt, ctk, slicer
import numpy as np
#
# ModelToPatientRegistration
#

class ModelToPatientRegistration:
  def __init__(self, parent):
    parent.title = "ModelToPatientRegistration"
    parent.categories = ["Scoliosis"]
    parent.dependencies = []
    parent.contributors = ["Ben Church - Queen's University, PerkLab"]
    parent.helpText = """
    This scripted loadable module was initially created for the B-spline (?)
    spinal model registration project.
    """
    parent.acknowledgementText = """ """ 

#
# ModelToPatientRegistrationWidget
#

class ModelToPatientRegistrationWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qtQVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parnet.show()

  def setup(self):
    # Another panel for modifying landmark sets, deleting points and such
    LandmarkModification = ctk.ctkCollapsibleButton()
    LandmarkModification.text = "Landmark Set Modification"
    self.layout.addWidget(LandmarkModification)
    LandmarkModificationLayout = qt.QFormLayout(LandmarkModification)
    LandmarkModification.collapsed = True
    
    self.ToModifySelector = slicer.qMRMLNodeComboBox()
    self.ToModifySelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.ToModifySelector.selectNodeUponCreation = False
    self.ToModifySelector.enabled = True
    self.ToModifySelector.addEnabled = True
    self.ToModifySelector.noneEnabled = True
    self.ToModifySelector.removeEnabled = True
    self.ToModifySelector.renameEnabled = True
    self.ToModifySelector.toolTip = "Choose which landmark set to perform operation on"
    LandmarkModificationLayout.addRow("Input node:", self.ToModifySelector)
    self.ToModifySelector.setMRMLScene(slicer.mrmlScene)
    
    self.ModifiedStorageNode = slicer.qMRMLNodeComboBox()
    self.ModifiedStorageNode.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.ModifiedStorageNode.selectNodeUponCreation = True
    self.ModifiedStorageNode.enabled = True
    self.ModifiedStorageNode.addEnabled = True
    self.ModifiedStorageNode.noneEnabled = True
    self.ModifiedStorageNode.removeEnabled = True
    self.ModifiedStorageNode.renameEnabled = True
    self.ModifiedStorageNode.toolTip = "Choose markups node to contain modified landmarks"
    LandmarkModificationLayout.addRow("Output node:", self.ModifiedStorageNode)
    self.ModifiedStorageNode.setMRMLScene(slicer.mrmlScene)
    
    self.RemoveThirdLandmarks = qt.QCheckBox("Remove every third vertebra's landmarks")
    self.RemoveThirdLandmarks.toolTip = "Creates markups node with one third of original node's vertebrae removed"
    self.RemoveThirdLandmarks.enabled = True
    LandmarkModificationLayout.addRow(self.RemoveThirdLandmarks)
    self.RemoveThirdLandmarks.connect('toggled(bool)', self.onReconfigure)
    
    self.RemoveTwoThirdsLandmarks = qt.QCheckBox("Remove two of every three vertebrae's landmarks")
    self.RemoveTwoThirdsLandmarks.toolTip = "Creates markups node with one third of original node's vertebrae remaining"
    self.RemoveTwoThirdsLandmarks.enabled = True
    LandmarkModificationLayout.addRow(self.RemoveTwoThirdsLandmarks)
    self.RemoveTwoThirdsLandmarks.connect('toggled(bool)', self.onReconfigure)
    
    self.RemoveHalfLandmarks = qt.QCheckBox("Remove every other vertebra's landmarks")
    self.RemoveHalfLandmarks.toolTip = "Creates markups node with half the inputs vertebra's landmarks removed"
    self.RemoveHalfLandmarks.enabled = True
    LandmarkModificationLayout.addRow(self.RemoveHalfLandmarks)
    self.RemoveHalfLandmarks.connect('toggled(bool)', self.onReconfigure)
    
    self.ModifyLandmarkSet = qt.QPushButton("Modify landmark set")
    self.ModifyLandmarkSet.toolTip = "Run chosen operation on chosen landmark set"
    LandmarkModificationLayout.addRow(self.ModifyLandmarkSet)
    
    self.ToModifySelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ToModifySelector))
    self.ModifiedStorageNode.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ModifiedStorageNode))
    self.ModifyLandmarkSet.connect('clicked(bool)', self.OnModifyMarkupsNode)
    
    # Set up user interface panel
    moduleInterface = ctk.ctkCollapsibleButton()
    moduleInterface.text = "Model to patient registration"
    self.layout.addWidget(moduleInterface)
    moduleInterfaceLayout = qt.QFormLayout(moduleInterface)
    
    # Input data dropdown list selector
    self.FromInputSelector = slicer.qMRMLNodeComboBox()
    self.FromInputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.FromInputSelector.selectNodeUponCreation = True
    self.FromInputSelector.enabled  = True
    self.FromInputSelector.addEnabled = True
    self.FromInputSelector.noneEnabled = False
    self.FromInputSelector.removeEnabled = True
    self.FromInputSelector.renameEnabled = True
    self.FromInputSelector.toolTip = "Choose the landmark list to be registered to the other"
    moduleInterfaceLayout.addRow("From-landmark input list:", self.FromInputSelector)
    self.FromInputSelector.setMRMLScene(slicer.mrmlScene)

    # Input data dropdown list selector
    self.ToInputSelector = slicer.qMRMLNodeComboBox()
    self.ToInputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.ToInputSelector.selectNodeUponCreation = True
    self.ToInputSelector.enabled  = True
    self.ToInputSelector.addEnabled = True
    self.ToInputSelector.noneEnabled = False
    self.ToInputSelector.removeEnabled = True
    self.ToInputSelector.renameEnabled = True
    self.ToInputSelector.toolTip = "Choose the landmark list the other will be registered to"
    moduleInterfaceLayout.addRow("To-landmark input list:", self.ToInputSelector)
    self.ToInputSelector.setMRMLScene(slicer.mrmlScene)

    # Output continer storage selector
    self.FromOutputSelector = slicer.qMRMLNodeComboBox()
    self.FromOutputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.FromOutputSelector.selectNodeUponCreation = True
    self.FromOutputSelector.enabled  = True
    self.FromOutputSelector.addEnabled = True
    self.FromOutputSelector.noneEnabled = False
    self.FromOutputSelector.removeEnabled = True
    self.FromOutputSelector.renameEnabled = True
    self.FromOutputSelector.toolTip = "Select a vtkMRMLMarkupsFiducialNode to store landmarks to be registered to the other"
    moduleInterfaceLayout.addRow("From-landmark output storage:", self.FromOutputSelector)
    self.FromOutputSelector.setMRMLScene(slicer.mrmlScene)
    
    # Output continer storage selector
    self.ToOutputSelector = slicer.qMRMLNodeComboBox()
    self.ToOutputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.ToOutputSelector.selectNodeUponCreation = True
    self.ToOutputSelector.enabled  = True
    self.ToOutputSelector.addEnabled = True
    self.ToOutputSelector.noneEnabled = False
    self.ToOutputSelector.removeEnabled = True
    self.ToOutputSelector.renameEnabled = True
    self.ToOutputSelector.toolTip = "Select a vtkMRMLMarkupsFiducialNode to store landmarks the other will be registered to"
    moduleInterfaceLayout.addRow("To-landmark output storage:", self.ToOutputSelector)
    self.ToOutputSelector.setMRMLScene(slicer.mrmlScene)
    
    # Scaling factors option
    self.UseLocalFactors = qt.QCheckBox("Compute scaling factors individually")
    self.UseLocalFactors.toolTip = "Uses information near to each landmark to compute individual scaling factors, rather than one global scaling factor"
    self.UseLocalFactors.enabled = True
    moduleInterfaceLayout.addRow(self.UseLocalFactors)
    
    self.UseAverageAnatomicScaling = qt.QCheckBox("Use average of individual scaling factors")
    self.UseAverageAnatomicScaling.toolTip = "Uses average anatomic scaling factor for uniform scaling of model to patient models"
    self.UseAverageAnatomicScaling.enabled = True
    moduleInterfaceLayout.addRow(self.UseAverageAnatomicScaling)
    
    self.UseVerticalAnatomicScaling = qt.QCheckBox("Use vertical inter-landmark distances to scale anchor offsets")
    self.UseVerticalAnatomicScaling.toolTip = "One possible way of scaling the model spine to the patient's"
    self.UseVerticalAnatomicScaling.enabled = True
    moduleInterfaceLayout.addRow(self.UseVerticalAnatomicScaling)
    
    self.UseScalingPoints = qt.QCheckBox("Use scale points nodes to calculate anchor point offset magnitude")
    self.UseScalingPoints.toolTip = "Uses superior and inferior scaling points placed on vertebral bodies (in seperate nodes from TrX) for anchor point offset scaling"
    self.UseScalingPoints.enabled = True
    moduleInterfaceLayout.addRow(self.UseScalingPoints)
    
    # Find-list-intersection button
    self.GeneratePoints = qt.QPushButton("Generate registration points")
    self.GeneratePoints.toolTip = "Finds intersection of model and patient anatomic landmark sets, then supplements them with anchor points."
    if(self.FromInputSelector.currentNode and self.ToInputSelector.currentNode and (self.FromInputSelector.currentNode != self.ToInputSelector.currentNode)):
      if(self.FromOutputSelector.currentNode and self.ToOutputSelector.currentNode and (self.FromOutputSelector.currentNode != self.ToOutputSelector.currentNode)):
        self.GeneratePoints.enabled = True
      else:
        self.GeneratePoints.enabled = False
    else:
      self.GeneratePoints.enabled = False
    moduleInterfaceLayout.addRow(self.GeneratePoints)
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    moduleInterfaceLayout.addRow(self.reloadButton)
    
    self.FromInputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.FromInputSelector))
    self.ToInputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ToInputSelector))
    self.FromOutputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.FromOutputSelector))
    self.ToOutputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ToOutputSelector))
    self.GeneratePoints.connect('clicked(bool)', self.onGeneratePoints)
    self.reloadButton.connect('clicked(bool)', self.onReloadButton)
    
  def onSelect(self, ComboBox):
    ComboBox.setMRMLScene(slicer.mrmlScene)
    #logic = ModelToPatientRegistrationLogic(self.FromInputSelector.currentNode, self.ToInputSelector.currentNode, self.FromOutputSelector.currentNode, self.ToOutputSelector.currentNode)
    
  def onReconfigure(self):
    Check = [self.RemoveThirdLandmarks.checkState(), self.RemoveHalfLandmarks.checkState(), self.RemoveTwoThirdsLandmarks.checkState()]
    if Check.count(2) > 1:
      print "One operation may be selected at a time."
      return
    if Check.count(2) == 0:
      print "Select one operation"
      return

    
  def OnModifyMarkupsNode(self):
    logic = MarkupsNodeModificationLogic(self.ToModifySelector.currentNode(), self.ModifiedStorageNode.currentNode())
    if self.RemoveThirdLandmarks.checkState():
      logic.RemoveThirdVertebrae()
    elif self.RemoveHalfLandmarks.checkState():
      logic.RemoveHalfVertebrae()
    elif self.RemoveTwoThirdsLandmarks.checkState():
      logic.RemoveTwoThirdsVertebrae()
  
  def onGeneratePoints(self):
    logic = ModelToPatientRegistrationLogic(self.FromInputSelector.currentNode(), self.ToInputSelector.currentNode(), self.FromOutputSelector.currentNode(), self.ToOutputSelector.currentNode())
    #Reanme run and pass parameters
    logic.AnchorPointSets()
  
  def cleanup(self):
    pass
    
  def onReloadButton(self):
    if(slicer.util.getNode('ModelRegistrationPointsNode') != None):
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('ModelRegistrationPointsNode'))
    if(slicer.util.getNode('PatientRegistrationPointsNode') != None):  
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('PatientRegistrationPointsNode'))
    slicer.util.reloadScriptedModule(slicer.moduleNames.ModelToPatientRegistration)

#
# ModelToPatientRegistrationLogic
#

class ModelToPatientRegistrationLogic:
  
  def __init__(self, OriginalModelPoints, OriginalPatientPoints, ModelRegistrationPointsNode, PatientRegistrationPointsNode):
    import numpy as np
    self.OriginalPatientPoints = OriginalPatientPoints
    self.OriginalModelPoints = OriginalModelPoints
    self.ModelRegistrationPointsNode = ModelRegistrationPointsNode
    self.PatientRegistrationPointsNode = PatientRegistrationPointsNode
    
    """
    # Unit vectors
    self.PatientSupInfVectorsLeft = []
    self.PatientRightLeftVectorsLeft = []
    self.PatientAntPostVectorsLeft = []
    self.PatientSupInfVectorsRight = []
    self.PatientRightLeftVectorsRight = []
    self.PatientAntPostVectorsRight = []
    self.ModelSupInfVectorsLeft = []
    self.ModelRightLeftVectorsLeft = []
    self.ModelAntPostVectorsLeft = []
    self.ModelSupInfVectorsRight = []
    self.ModelRightLeftVectorsRight = []
    self.ModelAntPostVectorsRight = []
    

    
    # Input and output (anchor) points
    self.ModelRegistrationPoints = []
    self.PatientRegistrationPoints = []
    self.NamesIntersection = []

    
    # Scale points in (Label, [R,A,S]) format
    self.PatientScalePoints = []
    self.ModelScalePoints = []
    """
    
    self.ModelRegistrationPointsLeft = []
    
    # Partially redundant variables - required for certain scaling conditions
    # TODO: MERGE REDUNDANT VARIABLES INTO STREAMLINED FUNCTIONALITY
    self.PatientScalingFactorsLeft = []
    self.PatientScalingFactorsRight = []
    self.ModelScalingFactorsLeft = []
    self.ModelScalingFactorsRight = []
    self.ModelRegistrationPointsRight = []
    self.PatientRegistrationPointsLeft = []
    self.PatientRegistrationPointsRight = []
    
    # Reinitialize
    slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().RemoveAllMarkups()
    slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().RemoveAllMarkups()
    
    self.UseVertebraWiseScaling = slicer.modules.ModelToPatientRegistrationWidget.UseLocalFactors.isChecked()
    self.UseAverageScaling = slicer.modules.ModelToPatientRegistrationWidget.UseAverageAnatomicScaling.isChecked()
    self.UseVerticalScaling = slicer.modules.ModelToPatientRegistrationWidget.UseVerticalAnatomicScaling.isChecked()
    self.UseScalingPoints = slicer.modules.ModelToPatientRegistrationWidget.UseScalingPoints.isChecked()
    if self.UseScalingPoints:
      self.PatientScalePointsNode = slicer.util.getNode('ScalePoints' + self.OriginalPatientPoints.GetName()[-3:])
      self.AllModelScalePointsNode = slicer.util.getNode('ModelScalePoints')
    self.GlobalVertebralScalingFactor = 30       # Distance (mm) in anterior direction to duplicate fiducial points for registration accuracy
  
  class Patient:
    def __init__(self, ParentLogic, LandmarksNode, RegistrationPointsNode, ScalePointsNode=None):
      import numpy as np
      # Sort landmarks, left to right, superior to inferior, into a new node
      self.RawLandmarksNode = LandmarksNode     # 'Raw' meaning may be missing points, or be out of order
      self.LandmarksNode = self.OrderRawLandmarksIntoRegistration()
      self.ScalePointsNode = ScalePointsNode
      
      # Make sure landmarks have valid labels
      if not self.CheckNamingConvention():
        print "Naming convention not followed - Patient object initialized but left empty"
        return
      
      self.AnchorPointsNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.AnchorPointsNode.SetName("PatientAnchorPoints" + LandmarksNode.GetName()[-3:])
      
      #self.AnchorOffsetScaleFactors = np.zeros(self.LandmarksNode.GetNumberOfFiducials())
      self.AnchorOffsetScaleFactors = []
      self.AnchorOffsetDirectionVectors = []
      
      ParentLogic.ComputeOffsetUnitVectors(self)
      
      self.RegistrationPointsNode = RegistrationPointsNode                 # Will contain sorted landmarks and anchor points, in that order
      
    def CheckNamingConvention(self):       # Returns bool
      NamesValid = True
      
      ValidTrPNames = ['T1L','T1R','T2L','T2R','T3L','T3R','T4L','T4R','T5L','T5R','T6L','T6R','T7L','T7R','T8L','T8R',
        'T9L','T9R','T10L','T10R','T11L','T11R','T12L','T12R','L1L','L1R','L2L','L2R','L3L','L3R','L4L','L4R','L5L','L5R']
      for i in range(self.LandmarksNode.GetNumberOfFiducials()-1):
        if self.LandmarksNode.GetNthFiducialLabel(i) not in ValidTrPNames:
          print "TrP naming convention not followed."
          print " Name " + PatientScalePoint[0] + " not allowed"
          NamesValid = False
          return NamesValid
          
      if self.ScalePointsNode != None:
        ValidScalePointNames = ['T1S','T1I','T2S','T2I','T3S','T3I','T4S','T4I','T5S','T5I','T6S','T6I','T7S','T7I','T8S','T8I',
          'T9S','T9I','T10S','T10I','T11S','T11I','T12S','T12I','L1S','L1I','L2S','L2I','L3S','L3I','L4S','L4I','L5S','L5I']
        for i in range(self.ScalePointsNode.GetNumberOfFiducials()-1):
          if self.ScalePointsNode.GetNthFiducialLabel(i) not in ValidScalePointNames:
            print "ScalePoints naming convention not followed."
            print " Name " + self.ScalePointsNode.GetNthFiducialLabel(i) + " not allowed"
            NamesValid = False
            return NamesValid
            
      return NamesValid
      
    def OrderRawLandmarksIntoRegistration(self):              # Returns vtkMRMLMarkupsFiducialNode
      RawThoracicPoints = []   # Will contain landmark points, represented as (Label, [R, A, S]) for sorting
      RawLumbarPoints = []
      for i in range(self.RawLandmarksNode.GetNumberOfFiducials()):
        if self.RawLandmarksNode.GetNthFiducialLabel(i)[0] == "T":
          RawThoracicPoints.append((self.RawLandmarksNode.GetNthFiducialLabel(i), self.RawLandmarksNode.GetMarkupPointVector(i, 0)))
        else:
          RawLumbarPoints.append((self.RawLandmarksNode.GetNthFiducialLabel(i), self.RawLandmarksNode.GetMarkupPointVector(i, 0)))
      SortedThoracicPoints = sorted(RawThoracicPoints, key=lambda tup: int(tup[0][1:-1]))  # Sort by labels alphabetically
      SortedLumbarPoints = sorted(RawLumbarPoints, key=lambda tup: int(tup[0][1:-1]))  # Sort by labels alphabetically
      SortedLandmarks =  SortedThoracicPoints + SortedLumbarPoints
      SortedLandmarksNode = slicer.vtkMRMLMarkupsFiducialNode()

      for i, Landmark in enumerate(SortedLandmarks):
        SortedLandmarksNode.AddFiducialFromArray(Landmark[1])
        SortedLandmarksNode.SetNthFiducialLabel(i, Landmark[0])
      return SortedLandmarksNode
  
  class Model:
    def __init__(self, Parent, Patient, FullLandmarksNode, RegistrationPointsNode, ScalePointsNode=None):
      self.Parent = Parent
      import numpy as np
      self.UseScalingPoints = Parent.UseScalingPoints
      
      self.FullLandmarksNode = FullLandmarksNode    # Contains all landmarks in UsLandmarks_Atlas, not trimmed to correspond to patient
      self.FullScalePointsNode = ScalePointsNode
      (self.LandmarksNode, self.ScalePointsNode) = self.EstablishCorrespondence(Patient)
      
      self.AnchorPointsNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.AnchorPointsNode.SetName("ModelAnchorPoints" + Patient.RawLandmarksNode.GetName()[-3:])
      
      #self.AnchorOffsetScaleFactors = np.zeros(self.LandmarksNode.GetNumberOfFiducials())
      self.AnchorOffsetScaleFactors = []
      self.AnchorOffsetDirectionVectors = []
      
      Parent.ComputeOffsetUnitVectors(self)
      
      self.RegistrationPointsNode = RegistrationPointsNode
      
    def EstablishCorrespondence(self, Patient):     # Returns (vtkMRMLMarkupsFiducialNode(), vtkMRMLMarkupsFiducialNode()) ---- (LandmarksNode, ScalePointsNode)
      CorrespondingLandmarksNode = slicer.vtkMRMLMarkupsFiducialNode()
      
      # Use labels to find model points with corresponding patient points
      PatientLandmarkPointCounter = 0
      NumPatientLandmarks = Patient.LandmarksNode.GetNumberOfFiducials()
      for (ModelPointLabel, ModelPointCoords) in (zip([self.FullLandmarksNode.GetNthFiducialLabel(i) for i in range(self.FullLandmarksNode.GetNumberOfFiducials())], [self.FullLandmarksNode.GetMarkupPointVector(i, 0) for i in range(self.FullLandmarksNode.GetNumberOfFiducials())])):
        (PatientPointLabel, PatientPointCoords) = (Patient.LandmarksNode.GetNthFiducialLabel(PatientLandmarkPointCounter), Patient.LandmarksNode.GetMarkupPointVector(PatientLandmarkPointCounter, 0))
        if PatientPointLabel == ModelPointLabel:
          CorrespondingLandmarksNode.AddFiducialFromArray(ModelPointCoords)
          CorrespondingLandmarksNode.SetNthFiducialLabel(PatientLandmarkPointCounter, PatientPointLabel)
          PatientLandmarkPointCounter += 1

      for (PatientPointLabel, PatientPointCoords) in zip([Patient.LandmarksNode.GetNthFiducialLabel(i) for i in range(NumPatientLandmarks)], [Patient.LandmarksNode.GetMarkupPointVector(i, 0) for i in range(NumPatientLandmarks)]):
        if(PatientPointLabel[-1] == "L"):
          self.Parent.PatientRegistrationPointsLeft.append((PatientPointLabel, PatientPointCoords))
        else:
          self.Parent.PatientRegistrationPointsRight.append((PatientPointLabel, PatientPointCoords))
        
      # Use labels to find model scale points with corresponding patient scale points
      CorrespondingScalePointsNode = slicer.vtkMRMLMarkupsFiducialNode()
      if self.UseScalingPoints:
        PatientScalePointCounter = 0
        FullNumModelScalePoints = self.FullScalePointsNode.GetNumberOfFiducials()
        for (ModelScalePointLabel, ModelScalePointCoords) in (zip([self.FullScalePointsNode.GetNthFiducialLabel(i) for i in range(FullNumModelScalePoints)], [self.FullScalePointsNode.GetMarkupPointVector(i, 0) for i in range(FullNumModelScalePoints)])):
          PatientPointLabel = Patient.ScalePointsNode.GetNthFiducialLabel(PatientScalePointCounter)
          if PatientPointLabel == ModelScalePointLabel:
            CorrespondingScalePointsNode.AddFiducialFromArray(ModelScalePointCoords)
            CorrespondingScalePointsNode.SetNthFiducialLabel(PatientScalePointCounter, ModelScalePointLabel)
            PatientScalePointCounter += 1
      #print CorrespondingLandmarksNode
      return (CorrespondingLandmarksNode, CorrespondingScalePointsNode)
      
  # Actual program
  def AnchorPointSets(self):
    # Anatomy models, patient and average model
    if self.UseScalingPoints:
      self.PatientAnatomy = self.Patient(self, self.OriginalPatientPoints, self.PatientRegistrationPointsNode, self.PatientScalePointsNode)
      self.ModelAnatomy = self.Model(self, self.PatientAnatomy, self.OriginalModelPoints, self.ModelRegistrationPointsNode, self.AllModelScalePointsNode)
    else:
      self.PatientAnatomy = self.Patient(self, self.OriginalPatientPoints, self.PatientRegistrationPointsNode)
      self.ModelAnatomy = self.Model(self, self.PatientAnatomy, self.OriginalModelPoints, self.ModelRegistrationPointsNode)
    
    if self.UseVertebraWiseScaling or self.UseAverageScaling:
      self.ComputeScalingFactorsFromTransverseProcesses(self.ModelAnatomy)
      self.ComputeScalingFactorsFromTransverseProcesses(self.PatientAnatomy)
      
    if self.UseScalingPoints:
      self.ComputeScalingFactorsFromScalePoints(self.ModelAnatomy)
      self.ComputeScalingFactorsFromScalePoints(self.PatientAnatomy)
    
    #self.ScalePatientToModel(self.PatientAnatomy, self.ModelAnatomy)
    
    self.AddAnchorPoints(self.ModelAnatomy)
    self.AddAnchorPoints(self.PatientAnatomy)

  """
    
  def EstablishCorrespondence(self):
    for i, PatientPoint in enumerate(self.PatientRegistrationPoints):
      if(PatientPoint[0][-1] == "L"):
        self.PatientRegistrationPointsLeft.append(PatientPoint)
      else:
        self.PatientRegistrationPointsRight.append(PatientPoint)
      slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().AddFiducial(PatientPoint[1][0], PatientPoint[1][1], PatientPoint[1][2])
      slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().SetNthFiducialLabel(i, PatientPoint[0])
  
    for i in range(self.OriginalModelPoints.GetNumberOfFiducials()):
      CurrentLabel = self.OriginalModelPoints.GetNthFiducialLabel(i)
      CurrentPoint = self.OriginalModelPoints.GetMarkupPointVector(i,0)
      if(CurrentLabel[1] == "0"):
        if((CurrentLabel[0]+CurrentLabel[2:]) in self.CtNames):
          self.NamesIntersection.append((CurrentLabel[0]+CurrentLabel[2:]))
          self.ModelRegistrationPoints.append((CurrentLabel[0]+CurrentLabel[2:], [CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
          slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().GetNumberOfFiducials() - 1, self.ModelRegistrationPoints[-1][0])
          if(CurrentLabel[-1] == "L"):
            self.ModelRegistrationPointsLeft.append((CurrentLabel[0]+CurrentLabel[2:], CurrentPoint))
          else:
            self.ModelRegistrationPointsRight.append((CurrentLabel[0]+CurrentLabel[2:], CurrentPoint))
      else:
        if(CurrentLabel in self.CtNames):
          self.NamesIntersection.append(CurrentLabel)
          self.ModelRegistrationPoints.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
          slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().GetNumberOfFiducials() - 1, CurrentLabel)
          if(CurrentLabel[-1] == "L"):
            self.ModelRegistrationPointsLeft.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          else:
            self.ModelRegistrationPointsRight.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
            
    if self.UseScalingPoints:
      TopScalePointStart = self.ValidScalePointNames.index(self.PatientScalePoints[0][0])
      for i in range(TopScalePointStart, TopScalePointStart + len(self.PatientScalePoints)):
        CurrentLabel = self.AllModelScalePointsNode.GetNthFiducialLabel(i)
        CurrentPoint = self.AllModelScalePointsNode.GetMarkupPointVector(i, 0)
        self.ModelScalePoints.append((CurrentLabel, CurrentPoint))
  """
  
  def ComputeOffsetUnitVectors(self, Anatomy):
    import numpy as np
    
    # Treat top (superior-most) boundary condition
    # This method can also search through the rest of the spine, eliminating the boundary condition
    LeftTopSearch = 0
    LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(LeftTopSearch)
    while LandmarkLabel[-1] != "L":
      LeftTopSearch += 1
      LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(LeftTopSearch)
    (TopLeftLabel, TopLeftCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(LeftTopSearch), Anatomy.LandmarksNode.GetMarkupPointVector(LeftTopSearch, 0))
    
    SecondLeftSearch = LeftTopSearch + 1
    LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLeftSearch)
    while LandmarkLabel[-1] != "L":
      SecondLeftSearch += 1
      LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLeftSearch)
    (SecondLeftLabel, SecondLeftCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLeftSearch), Anatomy.LandmarksNode.GetMarkupPointVector(SecondLeftSearch, 0))
    
    # Same thing to find the right-side top boundary condition
    RightTopSearch = 0
    LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(RightTopSearch)
    while LandmarkLabel[-1] != "R":
      RightTopSearch += 1
      LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(RightTopSearch)
    (TopRightLabel, TopRightCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(RightTopSearch), Anatomy.LandmarksNode.GetMarkupPointVector(RightTopSearch, 0))
    
    SecondRightSearch = RightTopSearch + 1
    LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(SecondRightSearch)
    while LandmarkLabel[-1] != "R":
      SecondRightSearch += 1
      LandmarkLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(SecondRightSearch)
    (SecondRightLabel, SecondRightCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondRightSearch), Anatomy.LandmarksNode.GetMarkupPointVector(SecondRightSearch, 0))
    
    print (TopLeftLabel, TopRightLabel)
    
    # Top-end boundary requires only Top and Middle points
    CurrentAxialVector = [0, 0, 0]
    CurrentLateralVector = [0, 0, 0]
    for dim in range(3):
      CurrentAxialVector[dim] = ((TopLeftCoords[dim] - SecondLeftCoords[dim]) + (TopRightCoords[dim] - SecondRightCoords[dim])) / 2.0
      CurrentLateralVector[dim] = TopRightCoords[dim] - TopLeftCoords[dim]
    
    CurrentOffsetVector = np.cross(CurrentAxialVector, CurrentLateralVector)
    OffsetVectorNorm = np.linalg.norm(CurrentOffsetVector)
    for dim in range(3):
      CurrentOffsetVector[dim] = CurrentOffsetVector[dim] / OffsetVectorNorm
    #print " " + str(CurrentOffsetVector)
    
    # Append OffsetVector twice, once for each top point ASSUMES FIRST TWO POINTS IN LANDMARKS ARE TOP LEFT AND TOP RIGHT - defeats purpose of above search
    # Consider having one offset vector per vertebra
    Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)
    Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)
    
    # ASSERT top-end boundary condition dealt with - offset direction calculated
    
    for Point in range(2, Anatomy.LandmarksNode.GetNumberOfFiducials() - 2, 2):  # Assumes first point in set is left points - skips boundary conditions
      # ASSUMES ALL POINTS ARE PRESENT
      CurrentLeftPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(Point), Anatomy.LandmarksNode.GetMarkupPointVector(Point, 0))
      
      LeftIndexAbove = self.FindNegihborPointAbove(Anatomy, (CurrentLeftPoint[0], Point))
      LeftPointAbove = (Anatomy.LandmarksNode.GetNthFiducialLabel(LeftIndexAbove), Anatomy.LandmarksNode.GetMarkupPointVector(LeftIndexAbove, 0))
      
      LeftIndexBelow = self.FindNegihborPointBelow(Anatomy, (CurrentLeftPoint[0], Point))
      LeftPointBelow = (Anatomy.LandmarksNode.GetNthFiducialLabel(LeftIndexBelow), Anatomy.LandmarksNode.GetMarkupPointVector(LeftIndexBelow, 0))
      
      NeighborIndexBeside = self.FindNegihborPointBeside(Anatomy, (CurrentLeftPoint[0], Point))
      CurrentRightPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(NeighborIndexBeside), Anatomy.LandmarksNode.GetMarkupPointVector(NeighborIndexBeside, 0))
      
      RightIndexAbove = self.FindNegihborPointAbove(Anatomy, (CurrentRightPoint[0], NeighborIndexBeside))
      RightPointAbove = (Anatomy.LandmarksNode.GetNthFiducialLabel(RightIndexAbove), Anatomy.LandmarksNode.GetMarkupPointVector(RightIndexAbove, 0))
      
      RightIndexBelow = self.FindNegihborPointBelow(Anatomy, (CurrentRightPoint[0], NeighborIndexBeside))
      RightPointBelow = (Anatomy.LandmarksNode.GetNthFiducialLabel(RightIndexBelow), Anatomy.LandmarksNode.GetMarkupPointVector(RightIndexBelow, 0))
      
      print (CurrentLeftPoint[0], CurrentRightPoint[0])
      
      TopAxialVector = [0, 0, 0]
      BottomAxialVector = [0, 0, 0]
      AverageAxialVector = [0, 0, 0]
      CurrentLateralVector = [0, 0, 0]
      for dim in range(3):
        TopAxialVector[dim] = ((LeftPointAbove[1][dim] - CurrentLeftPoint[1][dim]) + (RightPointAbove[1][dim] - CurrentRightPoint[1][dim])) / 2.0
        BottomAxialVector[dim] = ((CurrentLeftPoint[1][dim] - LeftPointBelow[1][dim]) + (CurrentRightPoint[1][dim] - RightPointBelow[1][dim])) / 2.0
        AverageAxialVector[dim] = (TopAxialVector[dim] + BottomAxialVector[dim]) / 2.0
        CurrentLateralVector[dim] = CurrentRightPoint[1][dim] - CurrentLeftPoint[1][dim]
      
      CurrentOffsetVector = np.cross(AverageAxialVector, CurrentLateralVector)
      OffsetVectorNorm = np.linalg.norm(CurrentOffsetVector)
      
      for dim in range(3):
        CurrentOffsetVector[dim] = CurrentOffsetVector[dim] / OffsetVectorNorm
      #print " " + str(CurrentOffsetVector)

      # Append OffsetVector twice, once for each top point ASSUMES FIRST TWO POINTS IN LANDMARKS ARE TOP LEFT AND TOP RIGHT - defeats purpose of above search
      # Consider having one offset vector per vertebra
      Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)
      Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)
    
    # Bottom points are a boundary condition, like the top
    LeftBottomSearch = Anatomy.LandmarksNode.GetNumberOfFiducials() - 1
    while Anatomy.LandmarksNode.GetNthFiducialLabel(LeftBottomSearch)[-1] != "L":
      LeftBottomSearch -= 1
    LeftBottomPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(LeftBottomSearch), Anatomy.LandmarksNode.GetMarkupPointVector(LeftBottomSearch, 0))
    
    SecondLastLeftIndex = self.FindNegihborPointAbove(Anatomy, (LeftBottomPoint[0], LeftBottomSearch))
    SecondLastLeftPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLastLeftIndex), Anatomy.LandmarksNode.GetMarkupPointVector(SecondLastLeftIndex, 0))
    
    RightBottomSearch = Anatomy.LandmarksNode.GetNumberOfFiducials() - 1
    while Anatomy.LandmarksNode.GetNthFiducialLabel(RightBottomSearch)[-1] != "R":
      RightBottomSearch -= 1
    RightBottomPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(RightBottomSearch), Anatomy.LandmarksNode.GetMarkupPointVector(RightBottomSearch, 0))
    
    SecondLastRightIndex = self.FindNegihborPointAbove(Anatomy, (RightBottomPoint[0], RightBottomSearch))
    SecondLastRightPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLastRightIndex), Anatomy.LandmarksNode.GetMarkupPointVector(SecondLastRightIndex, 0))
    
    print (LeftBottomPoint[0], RightBottomPoint[0])
    print ""
    
    CurrentAxialVector = [0, 0, 0]
    CurrentLateralVector = [0, 0, 0]
    for dim in range(3):
      CurrentAxialVector[dim] = ((SecondLastLeftPoint[1][dim] - LeftBottomPoint[1][dim]) + (SecondLastRightPoint[1][dim] - RightBottomPoint[1][dim])) / 2.0
      CurrentLateralVector[dim] = SecondLastRightPoint[1][dim] - SecondLastLeftPoint[1][dim]
    
    CurrentOffsetVector = np.cross(CurrentAxialVector, CurrentLateralVector)
    OffsetVectorNorm = np.linalg.norm(CurrentOffsetVector)
    for dim in range(3):
      CurrentOffsetVector[dim] = CurrentOffsetVector[dim] / OffsetVectorNorm
    #print " " + str(CurrentOffsetVector)  
      
    Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)
    Anatomy.AnchorOffsetDirectionVectors.append(CurrentOffsetVector)

  def FindNegihborPointBelow(self, Anatomy, Point):   # Point is of the form ('Label', index) --- returns PointSearchIndex, indicating neighbor location
    OriginalLabel = Point[0]
    PointSearchIndex = Point[1] + 1                   # Start at Point location, for speed
    CandidateLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex)
    
    while CandidateLabel[-1] != OriginalLabel[-1]:
      #print Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex)
      if PointSearchIndex >= Anatomy.LandmarksNode.GetNumberOfFiducials():
        print "ERROR - could not find neighbor below " + OriginalLabel
        print " Returning nothing - results invalid"
        return
      PointSearchIndex += 1
      CandidateLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex)
    return PointSearchIndex
    
  def FindNegihborPointAbove(self, Anatomy, Point):   # Point is of the form ('Label', index) --- returns PointSearchIndex, indicating neighbor location
    OriginalLabel = Point[0]
    PointSearchIndex = Point[1] - 1                   # Start at Point location, for speed
    CandidateLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex)
    
    #print (Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex), Point)
    while CandidateLabel[-1] != OriginalLabel[-1]:
      #print (Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex), Point)
      if PointSearchIndex < 0:
        print "ERROR - could not find neighbor above " + Point[0]
        print " Returning nothing - results invalid"
        return
      PointSearchIndex -= 1
      CandidateLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchIndex)
    return PointSearchIndex
    
  def FindNegihborPointBeside(self, Anatomy, Point):
    OriginalLabel = Point[0]
    PointSearchUpIndex = Point[1] - 1       # Search out from start point, up and down, for speed
    CandidateUpLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchUpIndex)
    PointSearchDownIndex = Point[1] + 1
    CandidateDownLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchDownIndex)
    
    while CandidateUpLabel[:-1] != OriginalLabel[:-1] and CandidateDownLabel[:-1] != OriginalLabel[:-1]:
      #print "SearchUp: " + str((Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchUpIndex), Point[0]))
      #print "SearchDown: " + str((Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchDownIndex), Point[0]))
      if PointSearchUpIndex > 0:
        PointSearchUpIndex -= 1
      if PointSearchDownIndex < Anatomy.LandmarksNode.GetNumberOfFiducials()-1:
        PointSearchDownIndex += 1
      CandidateUpLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchUpIndex)
      CandidateDownLabel = Anatomy.LandmarksNode.GetNthFiducialLabel(PointSearchDownIndex)
      #print "SearchUpIndex: " + str(PointSearchUpIndex)
      #print "SearchDownIndex: " + str(PointSearchDownIndex)
    if CandidateUpLabel[:-1] == Point[0][:-1]:
      return PointSearchUpIndex
    if CandidateDownLabel[:-1] == Point[0][:-1]:
      return PointSearchDownIndex
    print "ERROR - could not find symmetric partner for " + Point[0]
    print " Returning nothing - results invalid"
    return

  def ComputeScalingFactorsFromTransverseProcesses(self, Anatomy):      # ASSUMES TOP TWO POINTS ARE LEFT AND RIGHT NEIGHBORS
    
    if self.UseVerticalScaling:
      # Uses lengths of SupInfVectors as anatomic scaling factors
      SumPatientScalingFactors = 0
      SumModelScalingFactors = 0
      NumLandmarks = Anatomy.LandmarksNode.GetNumberOfFiducials()
      
      # Top of spine has boundary condition, points only have one vertical neighbor, below
      (TopLeftLabel, TopLeftCoords) = Anatomy.LandmarksNode.GetNthFiducialLabel(0), Anatomy.LandmarksNode.GetMarkupPointVector(0,0)
      SecondLeftIndex = self.FindNegihborPointBelow(Anatomy, (TopLeftLabel, 0))
      (SecondLeftLabel, SecondLeftCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondLeftIndex), Anatomy.LandmarksNode.GetMarkupPointVector(SecondLeftIndex,0))
      TopLeftSupInfVector = [0,0,0]
      for dim in range(3):
        TopLeftSupInfVector[dim] = TopLeftCoords[dim] - SecondLeftCoords[dim]
      Anatomy.AnchorOffsetScaleFactors.append(np.linalg.norm(TopLeftSupInfVector))
      
      (TopRightLabel, TopRightCoords) = Anatomy.LandmarksNode.GetNthFiducialLabel(1), Anatomy.LandmarksNode.GetMarkupPointVector(1,0)
      SecondRightIndex = self.FindNegihborPointBelow(Anatomy, (TopRightLabel, 1))
      (SecondRightLabel, SecondRightCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(SecondRightIndex), Anatomy.LandmarksNode.GetMarkupPointVector(SecondRightIndex,0))
      TopRightSupInfVector = [0,0,0]
      for dim in range(3):
        TopRightSupInfVector[dim] = TopRightCoords[dim] - SecondRightCoords[dim]
      Anatomy.AnchorOffsetScaleFactors.append(np.linalg.norm(TopRightSupInfVector))
      
      for j, (MidLandmarkLabel, MidLandmarkCoords) in enumerate(zip([Anatomy.LandmarksNode.GetNthFiducialLabel(i) for i in range(1, NumLandmarks-3)], [Anatomy.LandmarksNode.GetMarkupPointVector(i, 0) for i in range(1, NumLandmarks-3)])):
        AboveLandmarkIndex = self.FindNegihborPointAbove(Anatomy, (MidLandmarkLabel, j+2))
        (AboveLandmarkLabel, AboveLandmarkCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(AboveLandmarkIndex), Anatomy.LandmarksNode.GetMarkupPointVector(AboveLandmarkIndex,0))
        AboveSupInfVector = [0,0,0]
        for dim in range(3):
          AboveSupInfVector[dim] = AboveLandmarkCoords[dim] - MidLandmarkCoords[dim]
        
        BelowLandmarkIndex = self.FindNegihborPointBelow(Anatomy, (MidLandmarkLabel, j+2))
        (BelowLandmarkLabel, BelowLandmarkCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(BelowLandmarkIndex), Anatomy.LandmarksNode.GetMarkupPointVector(BelowLandmarkIndex,0))
        BelowSupInfVector = [0,0,0]
        for dim in range(3):
          BelowSupInfVector[dim] = MidLandmarkCoords[dim] - BelowLandmarkCoords[dim]
          
        MeanSupInfVector = [0,0,0]
        for dim in range(3):
          MeanSupInfVector[dim] = (AboveSupInfVector[dim] + BelowSupInfVector[dim]) / 2.0
        Anatomy.AnchorOffsetScaleFactors.append(np.linalg.norm(MeanSupInfVector))
      
      # Last two landmark points in spine are a boundary condition, each having only one vertical neighbor, above
      (BottomLeftLabel, BottomLeftCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(NumLandmarks-2), Anatomy.LandmarksNode.GetMarkupPointVector(NumLandmarks-2,0))
      PenultimateLeftIndex = self.FindNegihborPointAbove(Anatomy, (BottomLeftLabel, NumLandmarks-2))
      (PenultimateLeftLabel, PenultimateLeftCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(PenultimateLeftIndex), Anatomy.LandmarksNode.GetMarkupPointVector(PenultimateLeftIndex,0))
      BottomLeftSupInfVector = [0,0,0]
      for dim in range(3):
        BottomLeftSupInfVector[dim] = PenultimateLeftCoords[dim] - BottomLeftCoords[dim]
      Anatomy.AnchorOffsetScaleFactors.append(np.linalg.norm(BottomLeftSupInfVector))
      
      (BottomRightLabel, BottomRightCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(NumLandmarks-1), Anatomy.LandmarksNode.GetMarkupPointVector(NumLandmarks-1,0))
      PenultimateRightIndex = self.FindNegihborPointAbove(Anatomy, (BottomRightLabel, NumLandmarks-1))
      (PenultimateRightLabel, PenultimateRightCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(PenultimateRightIndex), Anatomy.LandmarksNode.GetMarkupPointVector(PenultimateRightIndex,0))
      BottomRightSupInfVector = [0,0,0]
      for dim in range(3):
        BottomRightSupInfVector[dim] = PenultimateRightCoords[dim] - BottomRightCoords[dim]
      Anatomy.AnchorOffsetScaleFactors.append(np.linalg.norm(BottomRightSupInfVector))
      
      # Use the average of the individual scaling factors, if specified
      if self.UseAverageScaling:
        IndividualScalingFactors = Anatomy.AnchorOffsetScaleFactors
        AveragedScalingFactor = np.mean(IndividualScalingFactors)
        Anatomy.AnchorOffsetScaleFactors = NumLandmarks * [AveragedScalingFactor]

  def ComputeScalingFactorsFromScalePoints(self, Anatomy):
    # Computes offset magnitudes as distance between CurrentVertebraAveragePostPoint and CurrentVertebraAverageAntPoint
    # Assumes all TrPs have symmetric partners, and all corresponding scale points are present
    
    CurrentVertebraAverageAntPoint = [0, 0, 0]
    CurrentVertebraAveragePostPoint = [0, 0, 0]
    AntPostScaleVector = [0, 0, 0]
    
    NumScalePoints = Anatomy.ScalePointsNode.GetNumberOfFiducials
    NumLandmarkPoints = Anatomy.LandmarksNode.GetNumberOfFiducials()
    
    for (VertebraAnt, VertebraPost) in zip(range(0, Anatomy.ScalePointsNode.GetNumberOfFiducials() - 1, 2), range(0, Anatomy.LandmarksNode.GetNumberOfFiducials() - 1, 2)):
    #for i, (SupScalePoint, InfScalePoint) in enumerate(zip([(Anatomy.ScalePointsNode.GetNthFiducialLabel(i), Anatomy.ScalePointsNode.GetMarkupPointVector(i,0)) for i in range(0, NumScalePoints-1, 2)], [(Anatomy.ScalePointsNode.GetNthFiducialLabel(i), Anatomy.ScalePointsNode.GetMarkupPointVector(i,0)) for i in range(0, NumScalePoints-1, 2)]))
      #(SupScalePointLabel, SupScalePointCoords) = SupScalePoint
      #(InfScalePointLabel, InfScalePointCoords) = InfScalePoint
      SupScalePoint = (Anatomy.ScalePointsNode.GetNthFiducialLabel(VertebraAnt), Anatomy.ScalePointsNode.GetMarkupPointVector(VertebraAnt, 0))
      InfScalePoint = (Anatomy.ScalePointsNode.GetNthFiducialLabel(VertebraAnt + 1), Anatomy.ScalePointsNode.GetMarkupPointVector(VertebraAnt + 1, 0))
      (LeftTrPLabel, LeftTrPCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(i), Anatomy.LandmarksNode.GetMarkupPointVector(i, 0))
      (RightTrPLabel, RightTrPCoords) = (Anatomy.LandmarksNode.GetNthFiducialLabel(i + 1), Anatomy.LandmarksNode.GetMarkupPointVector(i + 1, 0))
      for dim in range(3):
        CurrentVertebraAverageAntPoint[dim] = (SupScalePointCoords[dim] + InfScalePointCoords[dim]) / 2.0
        CurrentVertebraAveragePostPoint[dim] = (LeftTrPCoords[dim] + RightTrPCoords[dim]) / 2.0
        AntPostScaleVector[dim] = CurrentVertebraAverageAntPoint[dim] - CurrentVertebraAveragePostPoint[dim]
      #print np.linalg.norm(AntPostScaleVector)
      Anatomy.AnchorOffsetScaleFactors[i] = (np.linalg.norm(AntPostScaleVector)) # Once for the left anchor point
      Anatomy.AnchorOffsetScaleFactors[i + 1] = (np.linalg.norm(AntPostScaleVector)) # And once for the right
    #print Anatomy.AnchorOffsetScaleFactors
  
  def AddAnchorPoints(self, Anatomy):
    # Assumes Anatomy's anchor offset magnitudes and directions are instantiated
    #print ""
    #print Anatomy
    #print Anatomy.AnchorOffsetDirectionVectors
    #print " "
    #print Anatomy.AnchorOffsetScaleFactors
    #print " "
    
    #print Anatomy.LandmarksNode
    NumOriginalLandmarks = Anatomy.LandmarksNode.GetNumberOfFiducials()
    # Add original landmarks to RegistrationPoints
    for LandmarkIndex in range(NumOriginalLandmarks):
      OriginalLandmarkPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(LandmarkIndex), Anatomy.LandmarksNode.GetMarkupPointVector(LandmarkIndex, 0))
      Anatomy.RegistrationPointsNode.AddFiducialFromArray(OriginalLandmarkPoint[1])
      Anatomy.RegistrationPointsNode.SetNthFiducialLabel(LandmarkIndex, OriginalLandmarkPoint[0])

    # Add anchor points to RegistrationPoints
    for LandmarkIndex in range(NumOriginalLandmarks):
      OriginalLandmarkPoint = (Anatomy.LandmarksNode.GetNthFiducialLabel(LandmarkIndex), Anatomy.LandmarksNode.GetMarkupPointVector(LandmarkIndex, 0))
      CorrespondingAnchorCoord = [0, 0, 0]
      for dim in range(3):
        CorrespondingAnchorCoord[dim] = OriginalLandmarkPoint[1][dim] + (Anatomy.AnchorOffsetDirectionVectors[LandmarkIndex][dim] * Anatomy.AnchorOffsetScaleFactors[LandmarkIndex])
      CorrespondingAnchorPoint = (OriginalLandmarkPoint[0] + "A", CorrespondingAnchorCoord)
      Anatomy.RegistrationPointsNode.AddFiducialFromArray(CorrespondingAnchorCoord)
      Anatomy.RegistrationPointsNode.SetNthFiducialLabel(NumOriginalLandmarks+LandmarkIndex, CorrespondingAnchorPoint[0])
    
class ModelToPatientRegistrationTest:
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
    self.test_ModelToPatientRegistration1()

  def test_ModelToPatientRegistration1(self):
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
    logic = CtToAtlasRegistrationLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')

class MarkupsNodeModificationLogic:
  def __init__(self, Input, Output):
    self.MarkupsNode = Input
    self.OutputNode = Output
    
  def RemoveThirdVertebrae(self):
    print "Creating landmark set every third vertebra removed"
    for it, i in enumerate(range(0, self.MarkupsNode.GetNumberOfFiducials(), 2)):
      if it % 3 == 0 or it % 3 == 1:
        CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i,0)
        self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
        self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i))
        CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i+1,0)
        self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
        self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i+1))
        print "Landmark " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-2) + " and " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1) + " created."
       
  def RemoveHalfVertebrae(self):
    print "Creating landmark set with half vertebrae removed"
    for i in range(0, self.MarkupsNode.GetNumberOfFiducials(), 4):
      CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i,0)
      self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
      self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i))
      CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i+1,0)
      self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
      self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i+1))
      print "Landmark " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-2) + " and " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1) + " created."
  
  def RemoveTwoThirdsVertebrae(self):
    print "Creating landmark set with two-thirds of vertebrae removed"
    for i in range(0, self.MarkupsNode.GetNumberOfFiducials(), 6):
      CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i,0)
      self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
      self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i))
      CurrentPoint = self.MarkupsNode.GetMarkupPointVector(i+1,0)
      self.OutputNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
      self.OutputNode.SetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1, self.MarkupsNode.GetNthFiducialLabel(i+1))
      print "Landmark " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-2) + " and " + self.OutputNode.GetNthFiducialLabel(self.OutputNode.GetNumberOfFiducials()-1) + " created."
