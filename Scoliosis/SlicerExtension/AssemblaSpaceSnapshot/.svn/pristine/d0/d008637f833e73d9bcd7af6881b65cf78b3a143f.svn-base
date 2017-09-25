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
    moduleInterface.text = "Model to patient registration"
    self.layout.addWidget(moduleInterface)
    moduleInterfaceLayout = qt.QFormLayout(moduleInterface)
    
    # Input data dropdown list selector
    self.FromInputSelector = slicer.qMRMLNodeComboBox()
    self.FromInputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
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
    self.ToInputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
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
    self.FromOutputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
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
    self.ToOutputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
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
    
    self.FromInputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect(self.FromInputSelector))
    self.ToInputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect(self.ToInputSelector))
    self.FromOutputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect(self.FromOutputSelector))
    self.ToOutputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect(self.ToOutputSelector))
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    moduleInterfaceLayout.addRow(self.reloadButton)
    self.GeneratePoints.connect('clicked(bool)', self.onGeneratePoints)
    self.reloadButton.connect('clicked(bool)', self.onReloadButton)
    
  def onSelect(self, ComboBox):
    ComboBox.setMRMLScene(slicer.mrmlScene)
    logic = ModelToPatientRegistrationLogic(self.FromInputSelector.currentNode, self.ToInputSelector.currentNode, self.FromOutputSelector.currentNode, self.ToOutputSelector.currentNode)
    
  def onGeneratePoints(self):
    logic = ModelToPatientRegistrationLogic(self.FromInputSelector.currentNode, self.ToInputSelector.currentNode, self.FromOutputSelector.currentNode, self.ToOutputSelector.currentNode)
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
    #self.OriginalModelPoints = slicer.util.getNode('UsLandmarks_Atlas')
    self.OriginalModelPoints = OriginalModelPoints
    #self.OriginalPatientPoints = slicer.util.getNode('TrXFiducials')
    self.OriginalPatientPoints = OriginalPatientPoints
    self.ModelRegistrationPoints = []
    self.ModelRegistrationPointsLeft = []
    self.ModelRegistrationPointsRight = []
    self.PatientRegistrationPoints = []
    self.PatientRegistrationPointsLeft = []
    self.PatientRegistrationPointsRight = []
    self.NamesIntersection = []
    #self.ModelRegistrationPointsNode = slicer.vtkMRMLMarkupsFiducialNode()
    #self.ModelRegistrationPointsNode.SetName('ModelRegistrationPointsNode')
    self.ModelRegistrationPointsNode = ModelRegistrationPointsNode
    #self.PatientRegistrationPointsNode = slicer.vtkMRMLMarkupsFiducialNode()
    #self.PatientRegistrationPointsNode.SetName('PatientRegistrationPointsNode')
    self.PatientRegistrationPointsNode = PatientRegistrationPointsNode
    self.UseVertebraWiseScaling = slicer.modules.ModelToPatientRegistrationWidget.UseLocalFactors.isChecked()
    self.GlobalVertebralScalingFactor = 30       # Distance (mm) in anterior direction to duplicate fiducial points for registration accuracy
    self.LocalVertebralScalingFactorsLeft = []
    self.LocalVertebralScalingFactorsRight = []
    self.PatientAnchorPointsLeft = []
    self.PatientAnchorPointsRight = []
    
  def AnchorPointSets(self):
    import math, numpy
    
    self.nOriginalPatientPoints = self.OriginalPatientPoints.GetNumberOfFiducials()
    
    for i in range(self.nOriginalPatientPoints):
      name = self.OriginalPatientPoints.GetNthFiducialLabel(i)
      location = self.OriginalPatientPoints.GetMarkupPointVector(i,0)
      self.PatientRegistrationPoints.append((name, location))
     
    # self.nOriginalModelPoints = self.OriginalModelPoints.GetNumberOfFiducials()  
    # for i in range(self.nOriginalModelPoints):
      # name = self.OriginalModelPoints.GetNthFiducialLabel(i)
      # location = self.OriginalModelPoints.GetMarkupPointVector(i,0)
      # self.ModelRegistrationPoints.append((name, location))
    
    self.NamesValid = self.CheckNamingConvention()       # This ensures that PatientRegistrationPoints is a subset of ModelRegistrationPoints
    if not self.NamesValid:
      # The input points contain an invalid name label
      return
    
    # Re-order points grabbed from patient anatomy to allow input to be in any order
    self.OrderPoints()
    
    self.EstablishCorrespondence()
    
    if self.UseVertebraWiseScaling:
      print "DEBUG: Using local scaling factors"
      self.ComputeAnatomicScaleFactorsLocally()
    else:
      print "DEBUG: Use global scaling factors"
      self.scale = self.ComputeSpineLengths()
    
    # Construct a point set at patient-spine space of points with correspondence
    self.AnchorPatientSpine()

    # Construct a point set at average-spine model of points with correspondence
    self.AnchorAverageSpine()
    
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
        print "   Check input landmark names."
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
      self.PatientRegistrationPointsNode.AddFiducial(CtPoint[1][0], CtPoint[1][1], CtPoint[1][2])
      self.PatientRegistrationPointsNode.SetNthFiducialLabel(i, CtPoint[0])
  
    for i in range(self.OriginalModelPoints.GetNumberOfFiducials()):
      CurrentLabel = self.OriginalModelPoints.GetNthFiducialLabel(i)
      CurrentPoint = self.OriginalModelPoints.GetMarkupPointVector(i,0)
      if(CurrentLabel[1] == "0"):
        if((CurrentLabel[0]+CurrentLabel[2:]) in self.CtNames):
          self.NamesIntersection.append((CurrentLabel[0]+CurrentLabel[2:]))
          self.ModelRegistrationPoints.append((CurrentLabel[0]+CurrentLabel[2:], [CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          self.ModelRegistrationPointsNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
          self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.ModelRegistrationPoints[-1][0])
          if(CurrentLabel[-1] == "L"):
            self.ModelRegistrationPointsLeft.append((CurrentLabel[0]+CurrentLabel[2:], CurrentPoint))
          else:
            self.ModelRegistrationPointsRight.append((CurrentLabel[0]+CurrentLabel[2:], CurrentPoint))
      else:
        if(CurrentLabel in self.CtNames):
          self.NamesIntersection.append(CurrentLabel)
          self.ModelRegistrationPoints.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          self.ModelRegistrationPointsNode.AddFiducial(CurrentPoint[0], CurrentPoint[1], CurrentPoint[2])
          self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, CurrentLabel)
          if(CurrentLabel[-1] == "L"):
            self.ModelRegistrationPointsLeft.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
          else:
            self.ModelRegistrationPointsRight.append((CurrentLabel,[CurrentPoint[0], CurrentPoint[1], CurrentPoint[2]]))
  
  def ComputeSpineLengths(self):
    import math
    PatientSpineLength = 0
    for i, LeftCtPoint in enumerate(self.PatientRegistrationPointsLeft[:-1], start=1):
      CurrentLength = math.sqrt((self.PatientRegistrationPointsLeft[i][1][0] - LeftCtPoint[1][0])**2 + (self.PatientRegistrationPointsLeft[i][1][1] - LeftCtPoint[1][1])**2 + (self.PatientRegistrationPointsLeft[i][1][2] - LeftCtPoint[1][2])**2)
      PatientSpineLength = PatientSpineLength + CurrentLength
    for i, RightCtPoint in enumerate(self.PatientRegistrationPointsRight[:-1], start=1):
      CurrentLength = math.sqrt((self.PatientRegistrationPointsRight[i][1][0] - RightCtPoint[1][0])**2 + (self.PatientRegistrationPointsRight[i][1][1] - RightCtPoint[1][1])**2 + (self.PatientRegistrationPointsRight[i][1][2] - RightCtPoint[1][2])**2)
      PatientSpineLength = PatientSpineLength + CurrentLength
    PatientSpineLength = PatientSpineLength/2           # To average the right and left sides
    
    AverageSpineLength = 0
    for i, LeftAtlasPoint in enumerate(self.ModelRegistrationPointsLeft[:-1], start=1):
      CurrentLength = math.sqrt((self.ModelRegistrationPointsLeft[i][1][0] - LeftAtlasPoint[1][0])**2 + (self.ModelRegistrationPointsLeft[i][1][1] - LeftAtlasPoint[1][1])**2 + (self.ModelRegistrationPointsLeft[i][1][2] - LeftAtlasPoint[1][2])**2)
      AverageSpineLength = AverageSpineLength + CurrentLength
    for i, RightAtlasPoint in enumerate(self.ModelRegistrationPointsRight[:-1], start=1):
      CurrentLength = math.sqrt((self.ModelRegistrationPointsRight[i][1][0] - RightAtlasPoint[1][0])**2 + (self.ModelRegistrationPointsRight[i][1][1] - RightAtlasPoint[1][1])**2 + (self.ModelRegistrationPointsRight[i][1][2] - RightAtlasPoint[1][2])**2)
      AverageSpineLength = AverageSpineLength + CurrentLength
    AverageSpineLength = AverageSpineLength/2
    
    print "\n Atlas model curve length(mm) = " + str(AverageSpineLength) 
    print "Ct model curve length(mm) = " + str(PatientSpineLength)
    self.scale = PatientSpineLength/AverageSpineLength
    return self.scale
    scale
    
  def ComputeAnatomicScaleFactorsLocally(self):
    import math
    
    # Top most point is a special boundary condition
    ModelDistanceMetric = math.sqrt((self.ModelRegistrationPointsLeft[0][1][0] - self.ModelRegistrationPointsLeft[1][1][0])**2 + \
      (self.ModelRegistrationPointsLeft[0][1][1] - self.ModelRegistrationPointsLeft[1][1][1])**2 + \
      (self.ModelRegistrationPointsLeft[0][1][2] - self.ModelRegistrationPointsLeft[1][1][2])**2)
    CurrentScalingFactor = math.sqrt((self.PatientRegistrationPointsLeft[0][1][0] - self.PatientRegistrationPointsLeft[1][1][0])**2 + \
      (self.PatientRegistrationPointsLeft[0][1][1] - self.PatientRegistrationPointsLeft[1][1][1])**2 + \
      (self.PatientRegistrationPointsLeft[0][1][2] - self.PatientRegistrationPointsLeft[1][1][2])**2)/ModelDistanceMetric
    self.LocalVertebralScalingFactorsLeft.append(CurrentScalingFactor)
    ModelDistanceMetric = math.sqrt((self.ModelRegistrationPointsRight[0][1][0] - self.ModelRegistrationPointsRight[1][1][0])**2 + \
      (self.ModelRegistrationPointsRight[0][1][1] - self.ModelRegistrationPointsRight[1][1][1])**2 + \
      (self.ModelRegistrationPointsRight[0][1][2] - self.ModelRegistrationPointsRight[1][1][2])**2)
    CurrentScalingFactor = math.sqrt((self.PatientRegistrationPointsRight[0][1][0] - self.PatientRegistrationPointsRight[1][1][0])**2 + \
      (self.PatientRegistrationPointsRight[0][1][1] - self.PatientRegistrationPointsRight[1][1][1])**2 + \
      (self.PatientRegistrationPointsRight[0][1][2] - self.PatientRegistrationPointsRight[1][1][2])**2)/ModelDistanceMetric
    self.LocalVertebralScalingFactorsRight.append(CurrentScalingFactor)
    
    # Deal with the left-sided points
    for i, (ModelPointLeft, PatientPointLeft) in enumerate(zip(self.ModelRegistrationPointsLeft[1:-1], self.PatientRegistrationPointsLeft[1:-1]), start=1):
      ModelDistanceMetric = math.sqrt((ModelPointLeft[1][0] - self.ModelRegistrationPointsLeft[i-1][1][0])**2 + \
        (ModelPointLeft[1][1] - self.ModelRegistrationPointsLeft[i-1][1][1])**2 + \
        (ModelPointLeft[1][2] - self.ModelRegistrationPointsLeft[i-1][1][2])**2)
      ModelDistanceMetric = ModelDistanceMetric + math.sqrt((self.ModelRegistrationPointsLeft[i+1][1][0] - ModelPointLeft[1][0])**2 + \
        (self.ModelRegistrationPointsLeft[i+1][1][1] - ModelPointLeft[1][1])**2 + \
        (self.ModelRegistrationPointsLeft[i+1][1][2] - ModelPointLeft[1][2])**2)
      CurrentScalingFactor = math.sqrt((PatientPointLeft[1][0] - self.PatientRegistrationPointsLeft[i-1][1][0])**2 + \
        (PatientPointLeft[1][1] - self.PatientRegistrationPointsLeft[i-1][1][1])**2 + \
        (PatientPointLeft[1][2] - self.PatientRegistrationPointsLeft[i-1][1][2])**2)/ModelDistanceMetric
      CurrentScalingFactor = CurrentScalingFactor + math.sqrt((self.PatientRegistrationPointsLeft[i+1][1][0] - PatientPointLeft[1][0])**2 + \
        (self.PatientRegistrationPointsLeft[i+1][1][1] - PatientPointLeft[1][1])**2 + \
        (self.PatientRegistrationPointsLeft[i+1][1][2] - PatientPointLeft[1][2])**2)/ModelDistanceMetric
      self.LocalVertebralScalingFactorsLeft.append(CurrentScalingFactor)
   
    # Then with the right-sided points
    for i, (ModelPointRight, PatientPointRight) in enumerate(zip(self.ModelRegistrationPointsRight[1:-1], self.PatientRegistrationPointsRight[1:-1]), start=1):
      ModelDistanceMetric = math.sqrt((ModelPointRight[1][0] - self.ModelRegistrationPointsRight[i-1][1][0])**2 + \
        (ModelPointRight[1][1] - self.ModelRegistrationPointsRight[i-1][1][1])**2 + \
        (ModelPointRight[1][2] - self.ModelRegistrationPointsRight[i-1][1][2])**2)
      ModelDistanceMetric = ModelDistanceMetric + math.sqrt((self.ModelRegistrationPointsRight[i+1][1][0] - ModelPointRight[1][0])**2 + \
        (self.ModelRegistrationPointsRight[i+1][1][1] - ModelPointRight[1][1])**2 + \
        (self.ModelRegistrationPointsRight[i+1][1][2] - ModelPointRight[1][2])**2)
      CurrentScalingFactor = math.sqrt((PatientPointRight[1][0] - self.PatientRegistrationPointsRight[i-1][1][0])**2 + \
        (PatientPointRight[1][1] - self.PatientRegistrationPointsRight[i-1][1][1])**2 + \
        (PatientPointRight[1][2] - self.PatientRegistrationPointsRight[i-1][1][2])**2)/ModelDistanceMetric
      CurrentScalingFactor = CurrentScalingFactor + math.sqrt((self.PatientRegistrationPointsRight[i+1][1][0] - PatientPointRight[1][0])**2 + \
        (self.PatientRegistrationPointsRight[i+1][1][1] - PatientPointRight[1][1])**2 + \
        (self.PatientRegistrationPointsRight[i+1][1][2] - PatientPointRight[1][2])**2)/ModelDistanceMetric
      self.LocalVertebralScalingFactorsRight.append(CurrentScalingFactor)
      
    # Bottom most point is a special boundary condition
    ModelDistanceMetric = math.sqrt((self.ModelRegistrationPointsLeft[-2][1][0] - self.ModelRegistrationPointsLeft[-1][1][0])**2 + \
      (self.ModelRegistrationPointsLeft[-2][1][1] - self.ModelRegistrationPointsLeft[-1][1][1])**2 + \
      (self.ModelRegistrationPointsLeft[-2][1][2] - self.ModelRegistrationPointsLeft[-1][1][2])**2)
    CurrentScalingFactor = math.sqrt((self.PatientRegistrationPointsLeft[-2][1][0] - self.PatientRegistrationPointsLeft[-1][1][0])**2 + \
      (self.PatientRegistrationPointsLeft[-2][1][1] - self.PatientRegistrationPointsLeft[-1][1][1])**2 + \
      (self.PatientRegistrationPointsLeft[-2][1][2] - self.PatientRegistrationPointsLeft[-1][1][2])**2)/ModelDistanceMetric
    self.LocalVertebralScalingFactorsLeft.append(CurrentScalingFactor)
    ModelDistanceMetric = math.sqrt((self.ModelRegistrationPointsRight[-2][1][0] - self.ModelRegistrationPointsRight[-1][1][0])**2 + \
      (self.ModelRegistrationPointsRight[-2][1][1] - self.ModelRegistrationPointsRight[-1][1][1])**2 + \
      (self.ModelRegistrationPointsRight[-2][1][2] - self.ModelRegistrationPointsRight[-1][1][2])**2)
    CurrentScalingFactor = math.sqrt((self.PatientRegistrationPointsRight[-2][1][0] - self.PatientRegistrationPointsRight[-1][1][0])**2 + \
      (self.PatientRegistrationPointsRight[-2][1][1] - self.PatientRegistrationPointsRight[-1][1][1])**2 + \
      (self.PatientRegistrationPointsRight[-2][1][2] - self.PatientRegistrationPointsRight[-1][1][2])**2)/ModelDistanceMetric
    self.LocalVertebralScalingFactorsRight.append(CurrentScalingFactor)
  
  def AnchorPatientSpine(self):
    import math, numpy 
    # Compute vectors normal to spine curve for offset points
    # The top points on each side must be done specially since they have only 1 neighbor
    TopPoint = self.PatientRegistrationPointsLeft[0]
    BottomPoint = self.PatientRegistrationPointsLeft[1]                                                                                   # Assumes there's more than on fid. on left side
    VerticalVector = [TopPoint[1][0] - BottomPoint[1][0], TopPoint[1][1] - BottomPoint[1][1], TopPoint[1][2] - BottomPoint[1][2]]
    AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    #if(self.PatientRegistrationPointsLeft[0][0][:-1] == self.PatientRegistrationPointsRight[0][0][:-1]):  # If symetric partner is present 
    RightPoint = self.ChooseClosestPointFrom(TopPoint, self.PatientRegistrationPointsRight)
    print TopPoint
    print RightPoint
    HorizontalVector = [TopPoint[1][0] - RightPoint[1][0], TopPoint[1][1] - RightPoint[1][1], TopPoint[1][2] - RightPoint[1][2]]
    OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
    if self.UseVertebraWiseScaling:
      for dim in range(3):
        OffsetVector[dim] = self.LocalVertebralScalingFactorsLeft[0]*AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.scale*self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
    self.PatientAnchorPointsLeft.append((self.PatientRegistrationPointsNode.GetNthFiducialLabel(0) + "A", OffsetVector))
   
    # The same thing on the right side point
    TopPoint = self.PatientRegistrationPointsRight[0]
    BottomPoint = self.PatientRegistrationPointsRight[1]                                                                               # Assuming there's more than one fid on right side  
    VerticalVector = [TopPoint[1][0] - BottomPoint[1][0], TopPoint[1][1] - BottomPoint[1][1], TopPoint[1][2] - BottomPoint[1][2]]
    AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    LeftPoint = self.ChooseClosestPointFrom(TopPoint,self.PatientRegistrationPointsLeft)
    HorizontalVector = [LeftPoint[1][0] - TopPoint[1][0], LeftPoint[1][1] - TopPoint[1][1], LeftPoint[1][2] - TopPoint[1][2]]
    OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
    if self.UseVertebraWiseScaling:
      for dim in range(3):
        OffsetVector[dim] = self.LocalVertebralScalingFactorsRight[0]*AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.scale*self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
    self.PatientAnchorPointsRight.append((self.PatientRegistrationPointsNode.GetNthFiducialLabel(1) + "A", OffsetVector))
        
    # Now for the rest of the fiducials (except the bottom two)
    LeftIterator = 1
    RightIterator = 1
    for i, CurrentPoint in enumerate(self.PatientRegistrationPoints[2:-2], start = 2):
      if(CurrentPoint[0][-1] == "L"):
        AbovePoint = self.PatientRegistrationPointsLeft[LeftIterator-1]
        VerticalVectorAbove = [AbovePoint[1][0] - CurrentPoint[1][0], AbovePoint[1][1] - CurrentPoint[1][1], AbovePoint[1][2] - CurrentPoint[1][2]]
        BelowPoint = self.PatientRegistrationPointsLeft[LeftIterator+1]
        VerticalVectorBelow = [CurrentPoint[1][0] - BelowPoint[1][0], CurrentPoint[1][1] - BelowPoint[1][1], CurrentPoint[1][2] - BelowPoint[1][2]]
        for dim in range(3):
          #VerticalVectorAbove[dim] = VerticalVectorAbove[dim]/(int(CurrentPoint[0][1:-1]) - int(AbovePoint[0][1:-1]))
          #VerticalVectorBelow[dim] = VerticalVectorBelow[dim]/(int(BelowPoint[0][1:-1]) - int(CurrentPoint[0][1:-1]))
          VerticalVector[dim] = (VerticalVectorAbove[dim] + VerticalVectorBelow[dim])/2
        AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
        RightPoint = self.ChooseClosestPointFrom(CurrentPoint, self.PatientRegistrationPointsRight)               # NOT NECESSARILY CURRENTPOINT'S VERTEBRAL NEIGHBOR
        HorizontalVector = [RightPoint[1][0] - CurrentPoint[1][0], RightPoint[1][1] - CurrentPoint[1][1], RightPoint[1][2] - CurrentPoint[1][2]]
        OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
        OffsetNorm = math.sqrt(OffsetVector[0]**2 + OffsetVector[1]**2 + OffsetVector[2]**2)
        if(self.UseVertebraWiseScaling):
          for dim in range(3):
            #OffsetVector[dim] = self.LocalVertebralScalingFactorsLeft[LeftIterator]*(self.AnatomicScalingFactor)*OffsetVector[dim]/OffsetNorm
            OffsetVector[dim] = (AnatomicScalingFactor)*OffsetVector[dim]/OffsetNorm
        else:
          for dim in range(3):
            OffsetVector[dim] = self.scale*self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
        self.PatientAnchorPointsLeft.append((CurrentPoint[0] + "A", OffsetVector))
        LeftIterator = LeftIterator + 1
      else:
        AbovePoint = self.PatientRegistrationPointsRight[RightIterator-1]
        VerticalVectorAbove = [AbovePoint[1][0] - CurrentPoint[1][0], AbovePoint[1][1] - CurrentPoint[1][1], AbovePoint[1][2] - CurrentPoint[1][2]]
        BelowPoint = self.PatientRegistrationPointsRight[RightIterator+1]
        VerticalVectorBelow = [CurrentPoint[1][0] - BelowPoint[1][0], CurrentPoint[1][1] - BelowPoint[1][1], CurrentPoint[1][2] - BelowPoint[1][2]]
        for dim in range(3):
          #VerticalVectorAbove[dim] = VerticalVectorAbove[dim]/(int(CurrentPoint[0][1:-1]) - int(AbovePoint[0][1:-1]))
          #VerticalVectorBelow[dim] = VerticalVectorBelow[dim]/(int(BelowPoint[0][1:-1]) - int(CurrentPoint[0][1:-1]))
          VerticalVector[dim] = (VerticalVectorAbove[dim] + VerticalVectorBelow[dim])/2
        AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
        LeftPoint = self.ChooseClosestPointFrom(CurrentPoint, self.PatientRegistrationPointsLeft)               # NOT NECESSARILY CURRENT POINT'S VERTEBRAL NEIGHBOR
        HorizontalVector = [LeftPoint[1][0] - CurrentPoint[1][0], LeftPoint[1][1] - CurrentPoint[1][1], LeftPoint[1][2] - CurrentPoint[1][2]]
        OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
        OffsetNorm = math.sqrt(OffsetVector[0]**2 + OffsetVector[1]**2 + OffsetVector[2]**2)
        if(self.UseVertebraWiseScaling):
          for dim in range(3):
            #OffsetVector[dim] = self.LocalVertebralScalingFactorsLeft[RightIterator]*(self.AnatomicScalingFactor)*OffsetVector[dim]/OffsetNorm
            OffsetVector[dim] = (AnatomicScalingFactor)*OffsetVector[dim]/OffsetNorm
        else:
          for dim in range(3):
            OffsetVector[dim] = self.scale*self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
        self.PatientAnchorPointsRight.append((CurrentPoint[0] + "A", OffsetVector))
        RightIterator = RightIterator + 1
       
    # Include similar method for last two points 
    AbovePoint = self.PatientRegistrationPointsLeft[-2]
    BesidePoint = self.PatientRegistrationPointsRight[-1]
    LastLeft = self.PatientRegistrationPointsLeft[-1]
    VerticalVector = [AbovePoint[1][0] - LastLeft[1][0], AbovePoint[1][1] - LastLeft[1][1], AbovePoint[1][2] - LastLeft[1][2]]
    OffsetScaleFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    HorizontalVector = [BesidePoint[1][0] - LastLeft[1][0], BesidePoint[1][1] - LastLeft[1][1], BesidePoint[1][2] - LastLeft[1][2]]
    OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2) 
    if self.UseVertebraWiseScaling:
      for dim in range(3):
        OffsetVector[dim] = self.LocalVertebralScalingFactorsLeft[-1]*OffsetScaleFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.scale*OffsetScaleFactor*OffsetVector[dim]/OffsetNorm
    self.PatientAnchorPointsLeft.append((self.PatientRegistrationPointsLeft[-1][0] + "A", OffsetVector))
    
    AbovePoint = self.PatientRegistrationPointsRight[-2]
    LastRight = BesidePoint
    BesidePoint = LastLeft
    VerticalVector = [AbovePoint[1][0] - LastRight[1][0], AbovePoint[1][1] - LastRight[1][1], AbovePoint[1][2] - LastRight[1][2]]
    OffsetScaleFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    HorizontalVector = [BesidePoint[1][0] - LastRight[1][0], BesidePoint[1][1] - LastRight[1][1], BesidePoint[1][2] - LastRight[1][2]]
    OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2) 
    if self.UseVertebraWiseScaling:
      for dim in range(3):
        OffsetVector[dim] = self.LocalVertebralScalingFactorsRight[-1]*OffsetScaleFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.scale*OffsetScaleFactor*OffsetVector[dim]/OffsetNorm
    self.PatientAnchorPointsRight.append((self.PatientRegistrationPointsRight[-1][0] + "A", OffsetVector))
    
    i = 0
    while(i < len(self.PatientRegistrationPointsLeft) and i < len(self.PatientRegistrationPointsRight)):
      print self.PatientRegistrationPointsLeft[i][0] + " vector: " + str(self.PatientRegistrationPointsLeft[i][1][0]) + " " + str(self.PatientRegistrationPointsLeft[i][1][1]) + " " + str(self.PatientRegistrationPointsLeft[i][1][2])
      print self.PatientAnchorPointsLeft[i][0] + " vector: " + str(self.PatientRegistrationPointsLeft[i][1][0] + self.PatientAnchorPointsLeft[i][1][0]) + " " + str(self.PatientRegistrationPointsLeft[i][1][1] + \
        self.PatientAnchorPointsLeft[i][1][1]) + " " + str(self.PatientRegistrationPointsLeft[i][1][2] + self.PatientAnchorPointsLeft[i][1][2])
      print self.PatientRegistrationPointsRight[i][0] + " vector: " + str(self.PatientRegistrationPointsRight[i][1][0]) + " " + str(self.PatientRegistrationPointsRight[i][1][1]) + " " + str(self.PatientRegistrationPointsRight[i][1][2])
      print self.PatientAnchorPointsRight[i][0] + " vector: " + str(self.PatientRegistrationPointsRight[i][1][0] + self.PatientAnchorPointsRight[i][1][0]) + " " + str(self.PatientRegistrationPointsRight[i][1][1] + \
        self.PatientAnchorPointsRight[i][1][1]) + " " + str(self.PatientRegistrationPointsRight[i][1][2] + self.PatientAnchorPointsRight[i][1][2])
      i = i+1
      
    if(i < len(self.PatientRegistrationPointsLeft)):
      while(i < len(self.PatientRegistrationPointsLeft)):
        print self.PatientRegistrationPointsLeft[i][0] + " vector: " + str(self.PatientRegistrationPointsLeft[i][1][0]) + " " + str(self.PatientRegistrationPointsLeft[i][1][1]) + " " + str(self.PatientRegistrationPointsLeft[i][1][2])
        print self.PatientAnchorPointsLeft[i][0] + " vector: " + str(self.PatientRegistrationPointsLeft[i][1][0] + self.PatientAnchorPointsLeft[i][1][0]) + " " + str(self.PatientRegistrationPointsLeft[i][1][1] + \
          self.PatientAnchorPointsLeft[i][1][1]) + " " + str(self.PatientRegistrationPointsLeft[i][1][2] + self.PatientAnchorPointsLeft[i][1][2])
        i = i+1
    elif(i < len(self.PatientRegistrationPointsRight)):
      while(i < len(self.PatientRegistrationPointsRight)):
        print self.PatientRegistrationPointsRight[i][0] + " vector: " + str(self.PatientRegistrationPointsRight[i][1][0]) + " " + str(self.PatientRegistrationPointsRight[i][1][1]) + " " + str(self.PatientRegistrationPointsRight[i][1][2])
        print self.PatientAnchorPointsRight[i][0] + " vector: " + str(self.PatientRegistrationPointsRight[i][1][0] + self.PatientAnchorPointsRight[i][1][0]) + " " + str(self.PatientRegistrationPointsRight[i][1][1] + \
          self.PatientAnchorPointsRight[i][1][1]) + " " + str(self.PatientRegistrationPointsRight[i][1][2] + self.PatientAnchorPointsRight[i][1][2])
        i = i+1
    
    LeftIterator = 0
    RightIterator = 0
    for i in range(self.PatientRegistrationPointsNode.GetNumberOfFiducials()):
      OriginalCtPoint = self.PatientRegistrationPointsNode.GetMarkupPointVector(i,0)
      if(self.PatientRegistrationPointsNode.GetNthFiducialLabel(i)[-1] == "L"):
        self.PatientRegistrationPointsNode.AddFiducial(OriginalCtPoint[0] + self.PatientAnchorPointsLeft[LeftIterator][1][0],OriginalCtPoint[1] + self.PatientAnchorPointsLeft[LeftIterator][1][1],OriginalCtPoint[2] + self.PatientAnchorPointsLeft[LeftIterator][1][2])
        self.PatientRegistrationPointsNode.SetNthFiducialLabel(self.PatientRegistrationPointsNode.GetNumberOfFiducials() - 1, self.PatientAnchorPointsLeft[LeftIterator][0])
        LeftIterator = LeftIterator + 1
      else:
        self.PatientRegistrationPointsNode.AddFiducial(OriginalCtPoint[0] + self.PatientAnchorPointsRight[RightIterator][1][0],OriginalCtPoint[1] + self.PatientAnchorPointsRight[RightIterator][1][1],OriginalCtPoint[2] + self.PatientAnchorPointsRight[RightIterator][1][2])
        self.PatientRegistrationPointsNode.SetNthFiducialLabel(self.PatientRegistrationPointsNode.GetNumberOfFiducials() - 1, self.PatientAnchorPointsRight[RightIterator][0])
        RightIterator = RightIterator + 1
         
    slicer.mrmlScene.AddNode(self.PatientRegistrationPointsNode)
    return
  
  def AnchorAverageSpine(self):
  
    # TODO: Take into account possibility of missing points, making sure to normalize dilated ScalingFactor
  
    import math, numpy      
    
    # Top most points are a special boundary condition
    #First, the top-left point:
    TopPoint = self.ModelRegistrationPointsLeft[0]
    BelowPoint = self.ModelRegistrationPointsLeft[1]
    VerticalVector = [TopPoint[1][0] - BelowPoint[1][0], TopPoint[1][1] - BelowPoint[1][1], TopPoint[1][2] - BelowPoint[1][2]]
    #for dim in range(3):                                            # Normalization step in case next left sided point is a few vertebrae down
      #VerticalVector[dim] = VerticalVector[dim]/(int(BelowPoint[0][1:-1]) - int(TopPoint[0][1:-1]))
    AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    #BesidePoint = self.ModelRegistrationPointsRight[0]              # NOT NECESSARILY ON SAME VERTEBRA AS TOP-LEFT POINT - CONSIDER SOME NORMALIZATION METHOD
    BesidePoint = self.ChooseClosestPointFrom(TopPoint, self.ModelRegistrationPointsRight)
    HorizontalVector = [BesidePoint[1][0] - TopPoint[1][0], BesidePoint[1][1] - TopPoint[1][1], BesidePoint[1][2] - TopPoint[1][2]]
    OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
    if(self.UseVertebraWiseScaling):
      for dim in range(3):
        OffsetVector[dim] = AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
    self.ModelRegistrationPointsNode.AddFiducial(TopPoint[1][0] + OffsetVector[0], TopPoint[1][1] + OffsetVector[1], TopPoint[1][2] + OffsetVector[2])
    self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.ModelRegistrationPointsNode.GetNthFiducialLabel(0) + "A")

    # For the top-right point:
    TopPoint = self.ModelRegistrationPointsRight[0]
    BelowPoint = self.ModelRegistrationPointsRight[1]
    VerticalVector = [TopPoint[1][0] - BelowPoint[1][0], TopPoint[1][1] - BelowPoint[1][1], TopPoint[1][2] - BelowPoint[1][2]]
    #for dim in range(3):                                            # Normalization step in case next left sided point is a few vertebrae down
      #VerticalVector[dim] = VerticalVector[dim]/(int(BelowPoint[0][1:-1]) - int(TopPoint[0][1:-1]))
    AnatomicScalingFactor = math.sqrt(VerticalVector[0]**2 + VerticalVector[1]**2 + VerticalVector[2]**2)
    #BesidePoint = self.ModelRegistrationPointsLeft[0]         # NOT NECESSARILY ON SAME VERTEBRA AS TOP-LEFT POINT - CONSIDER SOME NORMALIZATION METHOD
    BesidePoint = self.ChooseClosestPointFrom(TopPoint, self.ModelRegistrationPointsLeft)
    HorizontalVector = [BesidePoint[1][0] - TopPoint[1][0], BesidePoint[1][1] - TopPoint[1][1], BesidePoint[1][2] - TopPoint[1][2]]
    OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
    OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
    if(self.UseVertebraWiseScaling):
      for dim in range(3):
        OffsetVector[dim] = AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
    else:
      for dim in range(3):
        OffsetVector[dim] = self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
    self.ModelRegistrationPointsNode.AddFiducial(TopPoint[1][0] + OffsetVector[0], TopPoint[1][1] + OffsetVector[1], TopPoint[1][2] + OffsetVector[2])
    self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.ModelRegistrationPointsNode.GetNthFiducialLabel(1) + "A")

    # For the rest of the points (except bottom two)
    LeftIterator = 1
    RightIterator = 1
    for i, OriginalPoint in enumerate(self.ModelRegistrationPoints[2:-2], start=2):
      if(OriginalPoint[0][-1] == "L"):
        AbovePoint = self.ModelRegistrationPointsLeft[LeftIterator-1]
        TopVector = [AbovePoint[1][0] - OriginalPoint[1][0], AbovePoint[1][1] - OriginalPoint[1][1], AbovePoint[1][2] - OriginalPoint[1][2]]
        BelowPoint = self.ModelRegistrationPointsLeft[LeftIterator+1]
        LeftIterator = LeftIterator + 1
        BottomVector = [OriginalPoint[1][0] - BelowPoint[1][0], OriginalPoint[1][1] - BelowPoint[1][1], OriginalPoint[1][2] - BelowPoint[1][2]]
        #VerticalVector = [(TopVector[0] + BottomVector[0])/2, (TopVector[1] + BottomVector[1])/2, (TopVector[2] + BottomVector[2])/2]
        for dim in range(3):
          #TopVector[dim] = TopVector[dim]/(int(OriginalPoint[0][1:-1]) - int(AbovePoint[0][1:-1]))
          #BottomVector[dim] = BottomVector[dim]/(int(BelowPoint[0][1:-1]) - int(OriginalPoint[0][1:-1]))
          VerticalVector[dim] = (TopVector[dim] + BottomVector[dim])/2
        AnatomicScalingFactor = math.sqrt((VerticalVector[0]**2) + VerticalVector[1]**2 + VerticalVector[2]**2)
        #RightPoint = self.ModelRegistrationPointsRight[RightIterator]                       # NOT NECESSARILY BESIDE IN THE CASE OF INCOMPLETE POINT SETS
        RightPoint = self.ChooseClosestPointFrom(OriginalPoint, self.ModelRegistrationPointsRight)
        HorizontalVector = [RightPoint[1][0] - OriginalPoint[1][0], RightPoint[1][1] - OriginalPoint[1][1], RightPoint[1][2] - OriginalPoint[1][2]]
        OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
        OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
        if(self.UseVertebraWiseScaling):
          for dim in range(3):
            OffsetVector[dim] = AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
        else:
          for dim in range(3):
            OffsetVector[dim] = self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
        self.ModelRegistrationPointsNode.AddFiducial(OriginalPoint[1][0] + OffsetVector[0], OriginalPoint[1][1] + OffsetVector[1], OriginalPoint[1][2] + OffsetVector[2])
        self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.ModelRegistrationPointsNode.GetNthFiducialLabel(i) + "A")
      else:
        AbovePoint = self.ModelRegistrationPointsRight[RightIterator-1]
        TopVector = [AbovePoint[1][0] - OriginalPoint[1][0], AbovePoint[1][1] - OriginalPoint[1][1], AbovePoint[1][2] - OriginalPoint[1][2]]
        BelowPoint = self.ModelRegistrationPointsRight[RightIterator+1]
        RightIterator = RightIterator + 1
        BottomVector = [OriginalPoint[1][0] - BelowPoint[1][0], OriginalPoint[1][1] - BelowPoint[1][1], OriginalPoint[1][2] - BelowPoint[1][2]]
        #VerticalVector = [(TopVector[0] + BottomVector[0])/2, (TopVector[1] + BottomVector[1])/2, (TopVector[2] + BottomVector[2])/2]
        for dim in range(3):
          #TopVector[dim] = TopVector[dim]/(int(OriginalPoint[0][1:-1]) - int(AbovePoint[0][1:-1]))
          #BottomVector[dim] = BottomVector[dim]/(int(BelowPoint[0][1:-1]) - int(OriginalPoint[0][1:-1]))
          VerticalVector[dim] = (TopVector[dim] + BottomVector[dim])/2
        AnatomicScalingFactor = math.sqrt((VerticalVector[0]**2) + VerticalVector[1]**2 + VerticalVector[2]**2)
        #LeftPoint = self.ModelRegistrationPointsLeft[LeftIterator-1]                       # NOT NECESSARILY BESIDE IN THE CASE OF INCOMPLETE POINT SETS
        LeftPoint = self.ChooseClosestPointFrom(OriginalPoint, self.ModelRegistrationPointsLeft)
        HorizontalVector = [LeftPoint[1][0] - OriginalPoint[1][0], LeftPoint[1][1] - OriginalPoint[1][1], LeftPoint[1][2] - OriginalPoint[1][2]]
        OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
        OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
        if(self.UseVertebraWiseScaling):
          for dim in range(3):
            OffsetVector[dim] = AnatomicScalingFactor*OffsetVector[dim]/OffsetNorm
        else:
          for dim in range(3):
            OffsetVector[dim] = self.GlobalVertebralScalingFactor*OffsetVector[dim]/OffsetNorm
        self.ModelRegistrationPointsNode.AddFiducial(OriginalPoint[1][0] + OffsetVector[0], OriginalPoint[1][1] + OffsetVector[1], OriginalPoint[1][2] + OffsetVector[2])
        self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.ModelRegistrationPointsNode.GetNthFiducialLabel(i) + "A")
        
    # Special method for last two model's points' offsets  
    self.ModelRegistrationPointsNode.AddFiducial(self.ModelRegistrationPointsLeft[-1][1][0], self.ModelRegistrationPointsLeft[-1][1][1] + self.GlobalVertebralScalingFactor, self.ModelRegistrationPointsLeft[-1][1][2])
    self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.PatientRegistrationPointsLeft[-1][0] + "A")
    self.ModelRegistrationPointsNode.AddFiducial(self.ModelRegistrationPointsRight[-1][1][0], self.ModelRegistrationPointsRight[-1][1][1] + self.GlobalVertebralScalingFactor, self.ModelRegistrationPointsRight[-1][1][2])
    self.ModelRegistrationPointsNode.SetNthFiducialLabel(self.ModelRegistrationPointsNode.GetNumberOfFiducials() - 1, self.PatientRegistrationPointsRight[-1][0] + "A")
 
    slicer.mrmlScene.AddNode(self.ModelRegistrationPointsNode)
    return
      
  
  def ChooseClosestPointFrom(self, CurrentPoint, SelectionPoints):
    if(CurrentPoint[0][-1] == "L"):
      if(CurrentPoint[0][0] == "T"):                      # We are marrying up a point on the left-thoracic spine
        BestGuess = SelectionPoints[0]
        if(BestGuess[0][0] == "L"):                       # The highest point we could find is necessarily below the CurrentPoint
          return BestGuess
        elif(int(BestGuess[0][1:-1]) > int(CurrentPoint[0][1:-1])):
          return BestGuess
        else:
          RightIterator = 1
          while(BestGuess[0][:-1] != CurrentPoint[0][:-1] and RightIterator < len(SelectionPoints)):
            NextTry = SelectionPoints[RightIterator]
            if(NextTry[0][0] == "T"):
              if(abs(int(NextTry[0][1:-1]) - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
              RightIterator = RightIterator + 1
              continue
            else:                                         # If a lumbar is closer to our thoracic point than any other thoracic point, we don't need to check any more
              if(abs(int(NextTry[0][1:-1]) + 12 - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
                return BestGuess
              else:
                return BestGuess
          return BestGuess
      else:                                             # Our point is on the left-lumbar spine
        BestGuess = SelectionPoints[-1]
        if(BestGuess[0][0] == "T"):                     # The lowest partner point is in the thoracic spine
          return BestGuess
        elif(int(BestGuess[0][1:-1] < int(CurrentPoint[0][1:-1]))): # The lowest point is still above our CurrentPoint
          return BestGuess
        else:
          RightIterator = len(SelectionPoints) - 2
          while(BestGuess[0][:-1] != CurrentPoint[0][:-1] and RightIterator >= 0):
            NextTry = SelectionPoints[RightIterator]
            if(NextTry[0][0] == "L"):
              if(abs(int(NextTry[0][1:-1]) - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
              RightIterator = RightIterator - 1
              continue
            else:
              if(abs(int(NextTry[0][1:-1]) - 12 - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
                return BestGuess
              else:
                return BestGuess
          return BestGuess
    else:                                                   
      if(CurrentPoint[0][0] == "T"):                      # We are marrying up a point on the right-thoracic spine
        BestGuess = SelectionPoints[0]
        if(BestGuess[0][0] == "L"):                       # The highest point we could find is necessarily below the CurrentPoint
          return BestGuess
        elif(int(BestGuess[0][1:-1]) > int(CurrentPoint[0][1:-1])):
          return BestGuess
        else:
          LeftIterator = 1
          while(BestGuess[0][:-1] != CurrentPoint[0][:-1] and LeftIterator < len(SelectionPoints)):
            NextTry = SelectionPoints[LeftIterator]
            if(NextTry[0][0] == "T"):
              if(abs(int(NextTry[0][1:-1]) - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
              LeftIterator = LeftIterator + 1
              continue
            else:                                         # If a lumbar is closer to our thoracic point than any other thoracic point, we don't need to check any more
              if(abs(int(NextTry[0][1:-1]) + 12 - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
                return BestGuess
              else:
                return BestGuess
          return BestGuess
      else:                                             # Our point is on the right-lumbar spine
        BestGuess = SelectionPoints[-1]
        if(BestGuess[0][0] == "T"):                     # The lowest partner point is in the thoracic spine
          return BestGuess
        elif(int(BestGuess[0][1:-1] < int(CurrentPoint[0][1:-1]))): # The lowest point is still above our CurrentPoint
          return BestGuess
        else:
          LeftIterator = len(SelectionPoints) - 2
          while(BestGuess[0][:-1] != CurrentPoint[0][:-1] and LeftIterator >= 0):
            NextTry = SelectionPoints[LeftIterator]
            if(NextTry[0][0] == "L"):
              if(abs(int(NextTry[0][1:-1]) - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
              LeftIterator = LeftIterator - 1
              continue
            else:
              if(abs(int(NextTry[0][1:-1]) - 12 - int(CurrentPoint[0][1:-1])) < abs(int(BestGuess[0][1:-1]) - int(CurrentPoint[0][1:-1]))):
                BestGuess = NextTry
                return BestGuess
              else:
                return BestGuess
          return BestGuess
 
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
