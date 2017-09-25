from __main__ import vtk, qt, ctk, slicer

#
# ModelToPatientRegistration
#

class ModelToPatientRegistration:
  def __init__(self, parent):
    parent.title = "ModelToPatientRegistration"
    parent.categories = ["Landmark Registration"]
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
    # Set up user interface panel
    moduleInterface = ctk.ctkCollapsibleButton()
    moduleInterface.text = "Model to Patient Registration"
    self.layout.addWidget(moduleInterface)
    moduleInterfaceLayout = qt.QFormLayout(moduleInterface)
    
    # Input data dropdown list selector
    self.FromInputSelector = slicer.qMRMLNodeComboBox()
    self.FromInputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.FromInputSelector.selectNodeUponCreation = False
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
    self.ToInputSelector.selectNodeUponCreation = False
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
    self.FromOutputSelector.selectNodeUponCreation = False
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
    self.ToOutputSelector.selectNodeUponCreation = False
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
    
    self.FromInputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.FromInputSelector))
    self.ToInputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ToInputSelector))
    self.FromOutputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.FromOutputSelector))
    self.ToOutputSelector.connect("currentNodeChanged(vtkMRMLMarkupsFiducialNode*)", self.onSelect(self.ToOutputSelector))
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    moduleInterfaceLayout.addRow(self.reloadButton)
    self.GeneratePoints.connect('clicked(bool)', self.onGeneratePoints)
    self.reloadButton.connect('clicked(bool)', self.onReloadButton)
    
  def onSelect(self, ComboBox):
    ComboBox.setMRMLScene(slicer.mrmlScene)
    #logic = ModelToPatientRegistrationLogic(self.FromInputSelector.currentNode, self.ToInputSelector.currentNode, self.FromOutputSelector.currentNode, self.ToOutputSelector.currentNode)
    
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
    
    # Scaling factors
    self.PatientScalingFactorsLeft = []
    self.PatientScalingFactorsRight = []
    self.ModelScalingFactorsLeft = []
    self.ModelScalingFactorsRight = []

    
    # Input and output (anchor) points
    self.ModelRegistrationPoints = []
    self.ModelRegistrationPointsLeft = []
    self.ModelRegistrationPointsRight = []
    self.PatientRegistrationPoints = []
    self.PatientRegistrationPointsLeft = []
    self.PatientRegistrationPointsRight = []
    self.NamesIntersection = []
    self.OriginalPatientPoints = OriginalPatientPoints
    self.OriginalModelPoints = OriginalModelPoints

    # Reinitialize
    slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().RemoveAllMarkups()
    slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().RemoveAllMarkups()
    
    self.UseVertebraWiseScaling = slicer.modules.ModelToPatientRegistrationWidget.UseLocalFactors.isChecked()
    self.UseAverageScaling = slicer.modules.ModelToPatientRegistrationWidget.UseAverageAnatomicScaling.isChecked()
    self.UseVerticalScaling = slicer.modules.ModelToPatientRegistrationWidget.UseVerticalAnatomicScaling.isChecked()
    self.GlobalVertebralScalingFactor = 30       # Distance (mm) in anterior direction to duplicate fiducial points for registration accuracy
    
  def AnchorPointSets(self):
    import math, numpy
    
    for i in range(self.OriginalPatientPoints.GetNumberOfFiducials()):
      name = self.OriginalPatientPoints.GetNthFiducialLabel(i)
      location = self.OriginalPatientPoints.GetMarkupPointVector(i,0)
      self.PatientRegistrationPoints.append((name, location))
    
    self.NamesValid = self.CheckNamingConvention()       # This ensures that PatientRegistrationPoints is a subset of ModelRegistrationPoints
    if not self.NamesValid:
      # The input points contain an invalid name label
      return
    
    # Re-order points grabbed from patient anatomy to allow input to be in any order
    self.OrderPoints()
    
    self.EstablishCorrespondence()
    
    self.ComputeVectors()
    
    if self.UseVertebraWiseScaling or self.UseAverageScaling:
      self.ComputeLocalAnatomicScalingFactors()
    
    # Construct a point set at patient-spine space of points with correspondence
    self.AnchorPatientSpine()

    # Construct a point set at average-spine model of points with correspondence
    self.AnchorModelSpine()
    
    if(len(self.NamesIntersection) == 0):   # For whatever reason, no points in common could be found between the CT and atlas
      print "Warning - intersection of CT and atlas points is the empty set."
      print "   Maybe points are named incorrectly."
      print "   Correspondence lists not genereated."
      return
    
    print "\n Intersection of CT and atlas landmarks:"
    for i, name in enumerate(self.NamesIntersection[:-1]):
      if((name[:-1] == self.NamesIntersection[i+1][:-1]) and (name[-1] != self.NamesIntersection[i+1][-1])):
        print name + "  " + self.NamesIntersection[i+1]
          
      else:
        if(i>0 and ((name[:-1] + "L" not in self.NamesIntersection) or (name[:-1] + "R" not in self.NamesIntersection))):
          print name

    # This should fix bug where last landmark is not printed if it has no partner
    if(self.NamesIntersection[-1][:-1] != self.NamesIntersection[-2][:-1]): # and (self.NamesIntersection[-1][-1] != self.NamesIntersection))
      print self.NamesIntersection[-1]
    return    
  
  def CheckNamingConvention(self):
    NamesValid = True
    self.ValidNames = ['T1L','T1R','T2L','T2R','T3L','T3R','T4L','T4R','T5L','T5R','T6L','T6R','T7L','T7R','T8L','T8R',
      'T9L','T9R','T10L','T10R','T11L','T11R','T12L','T12R','L1L','L1R','L2L','L2R','L3L','L3R','L4L','L4R','L5L','L5R']
    self.CtNames = [x[0] for x in self.PatientRegistrationPoints]
    for CtPoint in self.PatientRegistrationPoints:
      if CtPoint[0] not in self.ValidNames:
        print "Naming convention not followed."
        print " " + CtPoint[0] + " not allowed"
        NamesValid = False
        return NamesValid
      if(self.CtNames.count(CtPoint[0]) != 1):
        print "Duplicated point present in input."
        NamesValid = False
        return NamesValid
    return NamesValid
    
  def OrderPoints(self):
    self.ThoracicPoints = []
    self.LumbarPoints = []
    for CtPoint in self.PatientRegistrationPoints:
      if CtPoint[0][0] == "T":
        self.ThoracicPoints.append((CtPoint[0][1:], CtPoint[1], int(CtPoint[0][1:-1])))
      else:
        self.LumbarPoints.append((CtPoint[0][1:], CtPoint[1], int(CtPoint[0][1:-1])))
        
    self.ThoracicPoints.sort(key=lambda tup: tup[2])
    self.LumbarPoints.sort(key=lambda tup: tup[2])
    for i, ThoracicPoint in enumerate(self.ThoracicPoints):
      self.ThoracicPoints[i] = ("T" + ThoracicPoint[0], ThoracicPoint[1])
    for i, LumbarPoint in enumerate(self.LumbarPoints):
      self.LumbarPoints[i] = ("L" + LumbarPoint[0], LumbarPoint[1])
    
    self.PatientRegistrationPoints = self.ThoracicPoints + self.LumbarPoints
    self.CtNames = [x[0] for x in self.PatientRegistrationPoints]
    return
    
  def EstablishCorrespondence(self):
    for i, CtPoint in enumerate(self.PatientRegistrationPoints):
      if(CtPoint[0][-1] == "L"):
        self.PatientRegistrationPointsLeft.append(CtPoint)
      else:
        self.PatientRegistrationPointsRight.append(CtPoint)
      slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().AddFiducial(CtPoint[1][0], CtPoint[1][1], CtPoint[1][2])
      slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().SetNthFiducialLabel(i, CtPoint[0])
  
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
  
  def ComputeVectors(self):
    import numpy
    # Assumes that landmark points are correctly ordered and COMPLETE
    for PatientPoint in self.PatientRegistrationPointsLeft:
      self.PatientSupInfVectorsLeft.append(self.ComputeSupInfVector(PatientPoint, self.PatientRegistrationPointsLeft))
      self.PatientRightLeftVectorsLeft.append(self.ComputeRightLeftVector(PatientPoint, self.PatientRegistrationPointsRight))
      self.PatientAntPostVectorsLeft.append(numpy.cross(self.PatientRightLeftVectorsLeft[-1], self.PatientSupInfVectorsLeft[-1]))
    for PatientPoint in self.PatientRegistrationPointsRight:
      self.PatientSupInfVectorsRight.append(self.ComputeSupInfVector(PatientPoint, self.PatientRegistrationPointsRight))
      self.PatientRightLeftVectorsRight.append(self.ComputeRightLeftVector(PatientPoint, self.PatientRegistrationPointsLeft))
      self.PatientAntPostVectorsRight.append(numpy.cross(self.PatientRightLeftVectorsRight[-1], self.PatientSupInfVectorsRight[-1]))
    for ModelPoint in self.ModelRegistrationPointsLeft:
      self.ModelSupInfVectorsLeft.append(self.ComputeSupInfVector(ModelPoint, self.ModelRegistrationPointsLeft))
      self.ModelRightLeftVectorsLeft.append(self.ComputeRightLeftVector(ModelPoint, self.ModelRegistrationPointsRight))
      self.ModelAntPostVectorsLeft.append(numpy.cross(self.ModelRightLeftVectorsLeft[-1], self.ModelSupInfVectorsLeft[-1]))
    for ModelPoint in self.ModelRegistrationPointsRight:
      self.ModelSupInfVectorsRight.append(self.ComputeSupInfVector(ModelPoint, self.ModelRegistrationPointsRight))
      self.ModelRightLeftVectorsRight.append(self.ComputeRightLeftVector(ModelPoint, self.ModelRegistrationPointsLeft))
      self.ModelAntPostVectorsRight.append(numpy.cross(self.ModelRightLeftVectorsRight[-1], self.ModelSupInfVectorsRight[-1]))
  
  def ComputeSupInfVector(self, Point, PointSet):
    import math
    SupInfVector = [0, 0, 0]
    
    if Point[0] == PointSet[0][0]:
      # Boundary conition, top point
      for dim in range(3):
        SupInfVector[dim] += PointSet[1][1][dim] - Point[1][dim]
    elif Point[0] == PointSet[-1][0]:
      # Boundary condition, bottom point
      for dim in range(3):
        SupInfVector[dim] += Point[1][dim] - PointSet[-2][1][dim]
    else:
      # Non-bounary, expecting a point above and below
      SearchIterator = 1
      while Point[0] != PointSet[SearchIterator][0]: # Search by label until point is found
        SearchIterator += 1
        if SearchIterator > len(PointSet):
          print "Error - could not compute SupInfVector for point " + Point[0]
          print " Point not found in registration points - Returning 0-vector."
          return [0,0,0]
      for dim in range(3):
        # Use average since we have points above and below
        SupInfVector[dim] += ((Point[1][dim] - PointSet[SearchIterator - 1][1][dim]) + (PointSet[SearchIterator + 1][1][dim] - Point[1][dim])) / 2.0
   
    return SupInfVector
    
  def ComputeRightLeftVector(self, Point, ParallelPointSet):
    import math
    RightLeftVecor = [0, 0, 0]
    SearchIterator = 0
    
    while Point[0][:-1] != ParallelPointSet[SearchIterator][0][:-1]:    # Compare labels from beginning, slow
      SearchIterator += 1
      if SearchIterator > len(ParallelPointSet):
        print "Error - could not find point symmetric to point " + Point[0]
        print " Point not found in registration points - Returning 0-vector."
        return [0,0,0]
    for dim in range(3):
      RightLeftVecor[dim] = ParallelPointSet[SearchIterator][1][dim] - Point[1][dim]
      
    if Point[0][-1] == "R":
      # Ensure vectors are always Left to Right, convention for easy crossing
      for i, Coord in enumerate(RightLeftVecor):
        RightLeftVecor[i] = -1 * Coord
        
    return RightLeftVecor
 
  def ComputeLocalAnatomicScalingFactors(self):
    import numpy
    if self.UseVerticalScaling:
      # Uses lengths of SupInfVectors as anatomic scaling factors
      SumPatientScalingFactors = 0
      SumModelScalingFactors = 0
      for i in range(len(self.PatientRegistrationPointsLeft)):
        self.PatientScalingFactorsLeft.append(numpy.linalg.norm(self.PatientSupInfVectorsLeft[i]))
        SumPatientScalingFactors += self.PatientScalingFactorsLeft[-1]
      for i in range(len(self.PatientRegistrationPointsRight)):
        self.PatientScalingFactorsRight.append(numpy.linalg.norm(self.PatientSupInfVectorsRight[i]))
        SumPatientScalingFactors += self.PatientScalingFactorsRight[-1]
      for i in range(len(self.ModelRegistrationPointsLeft)):
        self.ModelScalingFactorsLeft.append(numpy.linalg.norm(self.ModelSupInfVectorsLeft[i]))
        SumModelScalingFactors += self.ModelScalingFactorsLeft[-1]
      for i in range(len(self.ModelRegistrationPointsRight)):
        self.ModelScalingFactorsRight.append(numpy.linalg.norm(self.ModelSupInfVectorsRight[i]))
        SumModelScalingFactors += self.ModelScalingFactorsRight[-1]
      
      if self.UseAverageScaling:
        AveragePatientScalingFactor = SumPatientScalingFactors / (len(self.PatientScalingFactorsLeft) + len(self.PatientScalingFactorsRight))
        AverageModelScalingFactor = SumModelScalingFactors / (len(self.ModelScalingFactorsLeft) + len(self.ModelScalingFactorsRight))
        for i in range(len(self.PatientScalingFactorsLeft)):
          self.PatientScalingFactorsLeft[i] = AveragePatientScalingFactor
        for i in range(len(self.PatientScalingFactorsRight)):
          self.PatientScalingFactorsRight[i] = AveragePatientScalingFactor
        for i in range(len(self.ModelScalingFactorsLeft)):
          self.ModelScalingFactorsLeft[i] = AverageModelScalingFactor
        for i in range(len(self.ModelScalingFactorsRight)):
          self.ModelScalingFactorsRight[i] = AverageModelScalingFactor
        
      for i in range(len(self.PatientScalingFactorsLeft)):
        # The multiplication by * (AveragePatientScalingFactor / AverageModelScalingFactor) is the VSF, to factor in the relative lengths of model/patient
        self.PatientScalingFactorsLeft[i] = self.PatientScalingFactorsLeft[i] * (self.PatientScalingFactorsLeft[i] / self.ModelScalingFactorsLeft[i])
      for i in range(len(self.PatientScalingFactorsRight)):
        self.PatientScalingFactorsRight[i] = self.PatientScalingFactorsRight[i] * (self.PatientScalingFactorsRight[i] / self.ModelScalingFactorsRight[i])
 
  def AnchorPatientSpine(self):
    import numpy
    LeftIt = 0
    RightIt = 0
    print self.PatientScalingFactorsLeft
    print self.PatientScalingFactorsRight
    for i in range(len(self.PatientRegistrationPoints)):
      if self.PatientRegistrationPoints[i][0][-1] == "L":
        AnchorPoint = (self.PatientRegistrationPoints[i][0] + "A", [0, 0, 0])
        AntPostVectorNorm = numpy.linalg.norm(self.PatientAntPostVectorsLeft[LeftIt])
        for dim in range(3):
          AnchorPoint[1][dim] = self.PatientRegistrationPoints[i][1][dim] + (self.PatientAntPostVectorsLeft[LeftIt][dim] * self.PatientScalingFactorsLeft[LeftIt] / AntPostVectorNorm)
        slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().AddFiducial(AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2])
        slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().GetNumberOfFiducials() - 1, AnchorPoint[0])
        LeftIt += 1
      else: #self.PatientRegistrationPoints[i][0][-1] == "R":
        AnchorPoint = (self.PatientRegistrationPoints[i][0] + "A", [0, 0, 0])
        AntPostVectorNorm = numpy.linalg.norm(self.PatientAntPostVectorsRight[RightIt])
        for dim in range(3):
          AnchorPoint[1][dim] = self.PatientRegistrationPoints[i][1][dim] + (self.PatientAntPostVectorsRight[RightIt][dim] * self.PatientScalingFactorsRight[RightIt] / AntPostVectorNorm)
        slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().AddFiducial(AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2])
        slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.ToOutputSelector.currentNode().GetNumberOfFiducials() - 1, AnchorPoint[0])
        RightIt += 1
      self.PatientRegistrationPoints.append((AnchorPoint[0] + "A", [AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2]]))
  
  def AnchorModelSpine(self):
    import numpy
    LeftIt = 0
    RightIt = 0
    for i in range(len(self.ModelRegistrationPoints)):
      if self.ModelRegistrationPoints[i][0][-1] == "L":
        AnchorPoint = (self.ModelRegistrationPoints[i][0] + "A", [0, 0, 0])
        AntPostVectorNorm = numpy.linalg.norm(self.ModelAntPostVectorsLeft[LeftIt])
        for dim in range(3):
          AnchorPoint[1][dim] = self.ModelRegistrationPoints[i][1][dim] + (self.ModelAntPostVectorsLeft[LeftIt][dim] * self.ModelScalingFactorsLeft[LeftIt] / AntPostVectorNorm)
        slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().AddFiducial(AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2])
        slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().GetNumberOfFiducials() - 1, AnchorPoint[0])
        LeftIt += 1
      else: #self.ModelRegistrationPoints[i][0][-1] == "R":
        AnchorPoint = (self.ModelRegistrationPoints[i][0] + "A", [0, 0, 0])
        AntPostVectorNorm = numpy.linalg.norm(self.ModelAntPostVectorsRight[RightIt])
        for dim in range(3):
          AnchorPoint[1][dim] = self.ModelRegistrationPoints[i][1][dim] + (self.ModelAntPostVectorsRight[RightIt][dim] * self.ModelScalingFactorsRight[RightIt] / AntPostVectorNorm)
        slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().AddFiducial(AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2])
        slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().SetNthFiducialLabel(slicer.modules.ModelToPatientRegistrationWidget.FromOutputSelector.currentNode().GetNumberOfFiducials() - 1, AnchorPoint[0])
        RightIt += 1
      self.ModelRegistrationPoints.append((AnchorPoint[0] + "A", [AnchorPoint[1][0], AnchorPoint[1][1], AnchorPoint[1][2]]))
 
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
