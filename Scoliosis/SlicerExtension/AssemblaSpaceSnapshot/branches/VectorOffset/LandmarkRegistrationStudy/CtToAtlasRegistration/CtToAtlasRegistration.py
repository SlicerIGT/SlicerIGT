from __main__ import vtk, qt, ctk, slicer

#
# CtToAtlasRegistration
#

class CtToAtlasRegistration:
  def __init__(self, parent):
    parent.title = "CtToAtlasRegistration"
    parent.categories = ["Landmark Registration"]
    parent.dependencies = []
    parent.contributors = ["Ben Church - Queen's University, PerkLab"]
    parent.helpText = """
    This scripted loadable module was initially created for the B-spline (?)
    spinal model registration project.
    """
    parent.acknowledgementText = """ """ 

#
# CtToAtlasRegistrationWidget
#

class CtToAtlasRegistrationWidget:
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
    moduleInterface.text = "Model to atlas registration"
    self.layout.addWidget(moduleInterface)
    moduleInterfaceLayout = qt.QGridLayout(moduleInterface)
    
    # Find-list-intersection button
    self.TrimButton = qt.QPushButton("Trim lists")
    self.TrimButton.toolTip = "Finds intersection of atlas and CT scan fiducial sets."
    self.TrimButton.enabled = True
    moduleInterfaceLayout.addWidget(self.TrimButton)
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    moduleInterfaceLayout.addWidget(self.reloadButton)
    self.TrimButton.connect('clicked(bool)', self.onTrimButton)
    self.reloadButton.connect('clicked(bool)', self.onReloadButton)
    
  def onTrimButton(self):
    logic = CtToAtlasRegistrationLogic()
    self.AtlasFids = slicer.util.getNode('UsLandmarks_Atlas')
    self.CtFids = slicer.util.getNode('TrXFiducials')
    self.nCtFids = self.CtFids.GetNumberOfFiducials()
    self.nAtalsFids = self.AtlasFids.GetNumberOfFiducials()
    (self.CtNames, self.NamesIntersection) = logic.run(self.CtFids, self.AtlasFids)
  
  def cleanup(self):
    pass
    
  def onReloadButton(self):
    if(slicer.util.getNode('CommonFiducialsAtlas') != None):
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('CommonFiducialsAtlas'))
    if(slicer.util.getNode('CommonFiducialsCt') != None):  
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('CommonFiducialsCt'))
    slicer.util.reloadScriptedModule(slicer.moduleNames.CtToAtlasRegistration)

#
# CtToAtlasRegistrationLogic
#

class CtToAtlasRegistrationLogic():
  def __init__(self):
    self.CtNames = []
    self.NamesIntersection = []
    self.AtlasNames = []
    self.CommonFiducialsAtlas = slicer.vtkMRMLMarkupsFiducialNode()
    self.CommonFiducialsAtlas.SetName('CommonFiducialsAtlas')
    self.CommonFiducialsCt = slicer.vtkMRMLMarkupsFiducialNode()
    self.CommonFiducialsCt.SetName('CommonFiducialsCt')
    self.CtSpineLength = 0         # Will be lengthened with the addition of each common vertebrae as a running average of right and left side lengths
    self.AtlasSpineLength = 0       # Will be computed for the atlas spine as for the Ct spine
    self.AntiSqishOffset = 30       # Distance in anterior direction to duplicate fiducial points for registration accuracy
    self.LeftAtlasCommon = []
    self.RightAtlasCommon = []
    self.LeftCtCommon = []
    self.RightCtCommon = []
    self.LeftOffsetPoints = []
    self.RightOffsetPoints = []
    
  def run(self, CtFids, AtlasFids):
    import math, numpy
    nCtFids = CtFids.GetNumberOfFiducials()
    nAtlasFids = AtlasFids.GetNumberOfFiducials()
    for i in range(nCtFids):
      name = CtFids.GetNthFiducialLabel(i)
      self.CtNames.append(name)
      
    for i in range(nAtlasFids):
      name = AtlasFids.GetNthFiducialLabel(i)
      self.AtlasNames.append(name)
      
    # Make sure Ct has no points the atlas doesn't  
    for CtPoint in self.CtNames:
      if(int(CtPoint[1:-1])<10):         # If the atlas fiducial name has a "0" as the second char
        if((CtPoint[0]+"0"+CtPoint[1:]) not in self.AtlasNames):
          self.CtNames.remove(CtPoint)
      else:
        if(CtPoint not in self.AtlasNames):
          self.CtNames.remove(CtPoint)
    
    # Still need to make sure the atlas has no more points than CtNames
    for i, AtlasPoint in enumerate(self.AtlasNames):
      if(AtlasPoint[1] == "0"):
        if((AtlasPoint[0]+AtlasPoint[2:]) in self.CtNames):     
          self.NamesIntersection.append(AtlasPoint)
          newAtlasPoint = AtlasFids.GetMarkupPointVector(i,0)
          # This if-structure eliminates need for same-side point searching for spine length computation
          if(AtlasPoint[-1] == "L"):
            self.LeftAtlasCommon.append((AtlasPoint, newAtlasPoint))
          else:
            self.RightAtlasCommon.append((AtlasPoint, newAtlasPoint))          
          self.CommonFiducialsAtlas.AddFiducial(newAtlasPoint[0], newAtlasPoint[1], newAtlasPoint[2])
          self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, AtlasFids.GetNthFiducialLabel(i))
          
      else:
        if(AtlasPoint in self.CtNames):  
          self.NamesIntersection.append(AtlasPoint)
          newAtlasPoint = AtlasFids.GetMarkupPointVector(i,0)
          if(AtlasPoint[-1] == "L"):
            self.LeftAtlasCommon.append((AtlasPoint, newAtlasPoint))
          else:
            self.RightAtlasCommon.append((AtlasPoint, newAtlasPoint))       
          self.CommonFiducialsAtlas.AddFiducial(newAtlasPoint[0], newAtlasPoint[1], newAtlasPoint[2])
          self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, AtlasFids.GetNthFiducialLabel(i))

    CtPointIt = 0
    for i, CommonFid in enumerate(self.NamesIntersection):
      if(int(CommonFid[1]) == 0):
        while((CtFids.GetNthFiducialLabel(CtPointIt) != CommonFid[0] + CommonFid[2:]) and (CtPointIt < nCtFids)):
          CtPointIt = CtPointIt + 1
          if(CtPointIt == nCtFids):
            break

        newCtPoint = CtFids.GetMarkupPointVector(CtPointIt,0)
        if(CommonFid[-1] == "L"):
          self.LeftCtCommon.append((CommonFid, newCtPoint))
        else:
          self.RightCtCommon.append((CommonFid, newCtPoint))
          
        self.CommonFiducialsCt.AddFiducial(newCtPoint[0], newCtPoint[1], newCtPoint[2])
        self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, CtFids.GetNthFiducialLabel(CtPointIt))
      else:
        while((CtFids.GetNthFiducialLabel(CtPointIt) != CommonFid) and (CtPointIt < nCtFids)):
          CtPointIt = CtPointIt + 1
          if(CtPointIt == nCtFids):
            break

        newCtPoint = CtFids.GetMarkupPointVector(CtPointIt,0)
        if(CommonFid[-1] == "L"):
          self.LeftCtCommon.append((CommonFid, newCtPoint))
        else:
          self.RightCtCommon.append((CommonFid, newCtPoint))
          
        self.CommonFiducialsCt.AddFiducial(newCtPoint[0], newCtPoint[1], newCtPoint[2])
        self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, CtFids.GetNthFiducialLabel(CtPointIt))
    
    if(len(self.NamesIntersection) != 0):
      LeftPointsWeight = float(len(self.LeftAtlasCommon))/(len(self.NamesIntersection))
      RightPointsWeight = 1.0 - LeftPointsWeight
      for i, LeftAtlasPoint in enumerate(self.LeftAtlasCommon[1:]):
        self.AtlasSpineLength = self.AtlasSpineLength + (math.sqrt((LeftAtlasPoint[1][0]-self.LeftAtlasCommon[i][1][0])**2 + (LeftAtlasPoint[1][1]-self.LeftAtlasCommon[i][1][1])**2 + (LeftAtlasPoint[1][2]-self.LeftAtlasCommon[i][1][2])**2)) * LeftPointsWeight 
      for i, RightAtlasPoint in enumerate(self.RightAtlasCommon[1:]):
        self.AtlasSpineLength = self.AtlasSpineLength + (math.sqrt((RightAtlasPoint[1][0]-self.RightAtlasCommon[i][1][0])**2 + (RightAtlasPoint[1][1]-self.RightAtlasCommon[i][1][1])**2 + (RightAtlasPoint[1][2]-self.RightAtlasCommon[i][1][2])**2)) * RightPointsWeight
      
      for i, LeftCtPoint in enumerate(self.LeftCtCommon[1:]):
        self.CtSpineLength = self.CtSpineLength + (math.sqrt((LeftCtPoint[1][0]-self.LeftCtCommon[i][1][0])**2 + (LeftCtPoint[1][1]-self.LeftCtCommon[i][1][1])**2 + (LeftCtPoint[1][2]-self.LeftCtCommon[i][1][2])**2)) * LeftPointsWeight
      for i, RightCtPoint in enumerate(self.RightCtCommon[1:]):
        self.CtSpineLength = self.CtSpineLength + (math.sqrt((RightCtPoint[1][0]-self.RightCtCommon[i][1][0])**2 + (RightCtPoint[1][1]-self.RightCtCommon[i][1][1])**2 + (RightCtPoint[1][2]-self.RightCtCommon[i][1][2])**2)) * RightPointsWeight
      
      scale = self.CtSpineLength/self.AtlasSpineLength
      
      # Compute vectors normal to spine curve for offset points
      # The top points on each side must be done specially since they have only 1 neighbor
      TopPoint = self.LeftCtCommon[0][1]
      BottomPoint = self.LeftCtCommon[1][1]                                                                                    # Assumes there's more than on fid. on left side
      VerticalVector = [TopPoint[0] - BottomPoint[0], TopPoint[1] - BottomPoint[1], TopPoint[2] - BottomPoint[2]]
      if(self.CommonFiducialsAtlas.GetNthFiducialLabel(0)[1] == "0"):     # This condition might be unecessary if self.CommonFiducialsCt is used
        if(self.CommonFiducialsCt.GetNthFiducialLabel(0)[:-1] == self.CommonFiducialsCt.GetNthFiducialLabel(1)[:-1]):  # If symetric partner is present
          if(self.CommonFiducialsCt.GetNthFiducialLabel(0)[0] == self.CommonFiducialsCt.GetNthFiducialLabel(1)[0]):  
            RightPoint = self.RightCtCommon[0][1]
            HorizontalVector = [TopPoint[0] - RightPoint[0], TopPoint[1] - RightPoint[1], TopPoint[2] - RightPoint[2]]
            OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
            OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
            for dim in range(3):
              OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
              
              
            self.LeftOffsetPoints.append((self.CommonFiducialsCt.GetNthFiducialLabel(0)[0] + "0" + self.CommonFiducialsCt.GetNthFiducialLabel(0)[1:] + "A", OffsetVector))
            
            # The same thing on the right side point
            LeftPoint = TopPoint
            TopPoint = RightPoint
            BottomPoint = self.RightCtCommon[1][1]                                                                                # Assuming there's more than one fid on right side  
            VerticalVector = [TopPoint[0] - BottomPoint[0], TopPoint[1] - BottomPoint[1], TopPoint[2] - BottomPoint[2]]
            OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
            OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
            for dim in range(3):
              OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
            self.RightOffsetPoints.append((self.CommonFiducialsCt.GetNthFiducialLabel(0)[0] + "0" + self.CommonFiducialsCt.GetNthFiducialLabel(0)[1:] + "A", OffsetVector))
        
        """
        else:                                                                                                           # We have just one vector to work with
          #OffsetVector = [VerticalVector[0],,VerticalVector[2]]
        """
        
      else:   # When fidLabel[1] != 0
        if(self.CommonFiducialsCt.GetNthFiducialLabel(0)[2:-1] == self.CommonFiducialsCt.GetNthFiducialLabel(1)[2:-1]):  # If symetric partner is present
          if(self.CommonFiducialsCt.GetNthFiducialLabel(0)[0] == self.CommonFiducialsCt.GetNthFiducialLabel(1)[0]):
            RightPoint = self.RightCtCommon[0][1]
            HorizontalVector = [TopPoint[0] - RightPoint[0], TopPoint[1] - RightPoint[1], TopPoint[2] - RightPoint[2]]
            OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
            OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
            for dim in range(3):
              OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
            self.LeftOffsetPoints.append((self.CommonFiducialsCt.GetNthFiducialLabel(0) + "A", OffsetVector))
            
            # The same thing on the right side point
            LeftPoint = TopPoint
            TopPoint = RightPoint
            BottomPoint = self.RightCtCommon[1][1]                                                                                # Assuming there's more than one fid on right side  
            VerticalVector = [TopPoint[0] - BottomPoint[0], TopPoint[1] - BottomPoint[1], TopPoint[2] - BottomPoint[2]]
            for dim in range(3):
              HorizontalVector[dim] = (-1)*HorizontalVector[dim]    
            
            OffsetVector = numpy.cross(HorizontalVector, VerticalVector) 
            OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
            for dim in range(3):
              OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
            self.RightOffsetPoints.append((self.CommonFiducialsCt.GetNthFiducialLabel(0) + "A", OffsetVector))
          
      # Now for the rest of the fiducials (except the bottom two)
      for i, leftPoint in enumerate(self.LeftCtCommon[1:-1]):   # This loop structure should only require that the atlas points are ordered
        RightSideSearch = 0
        AbovePoint = self.LeftCtCommon[i]         
        BelowPoint = self.LeftCtCommon[i+2]
        VerticalVectorBelow = [leftPoint[1][0] - BelowPoint[1][0], leftPoint[1][1] - BelowPoint[1][1], leftPoint[1][2] - BelowPoint[1][2]]
        VerticalVectorAbove = [AbovePoint[1][0] - leftPoint[1][0], AbovePoint[1][1] - leftPoint[1][1], AbovePoint[1][2] - leftPoint[1][2]]
        for dim in range(3):
          VerticalVector[dim] = (VerticalVectorBelow[dim] + VerticalVectorAbove[dim])/2
        while(RightSideSearch < len(self.RightCtCommon) and self.RightCtCommon[RightSideSearch][0][:-1] != leftPoint[0][:-1]):
          RightSideSearch = RightSideSearch + 1
        if(RightSideSearch < len(self.RightCtCommon)):                                                                         # We find the symmetric match  
          BesidePoint = self.RightCtCommon[RightSideSearch]
          HorizontalVector = [leftPoint[1][0] - BesidePoint[1][0], leftPoint[1][1] - BesidePoint[1][1], leftPoint[1][2] - BesidePoint[1][2]]
        OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
        OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)      
        for dim in range(3):
          OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
        self.LeftOffsetPoints.append((leftPoint[0] + "A", OffsetVector))
        
        print leftPoint[0] + "A" + " vector: " + str(OffsetVector[0]) + "  " + str(OffsetVector[1]) + "  " + str(OffsetVector[2])
        print "   Length = " + str(math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2))
        
      for i, rightPoint in enumerate(self.RightCtCommon[1:-1]):
        LeftSideSearch = 0
        AbovePoint = self.RightCtCommon[i]
        BelowPoint = self.RightCtCommon[i+2]
        VerticalVectorBelow = [rightPoint[1][0] - BelowPoint[1][0], rightPoint[1][1] - BelowPoint[1][1], rightPoint[1][2] - BelowPoint[1][2]]
        VerticalVectorAbove = [AbovePoint[1][0] - rightPoint[1][0], AbovePoint[1][1] - rightPoint[1][1], AbovePoint[1][2] - rightPoint[1][2]]
        for dim in range(3):
          VerticalVector[dim] = (VerticalVectorBelow[dim] + VerticalVectorAbove[dim])/2
        while(LeftSideSearch < len(self.LeftCtCommon) and self.LeftCtCommon[LeftSideSearch][0][:-1] != rightPoint[0][:-1]):
          LeftSideSearch = LeftSideSearch + 1
        if(LeftSideSearch < len(self.LeftCtCommon)):
          BesidePoint = self.LeftCtCommon[LeftSideSearch]
          HorizontalVector = [rightPoint[1][0] - BesidePoint[1][0], rightPoint[1][1] - BesidePoint[1][1], rightPoint[1][2] - BesidePoint[1][2]]
        OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
        OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)      
        for dim in range(3):
          OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm   
        self.RightOffsetPoints.append((rightPoint[0] + "A", OffsetVector))
        
        print rightPoint[0] + "A" + " vector: " + str(OffsetVector[0]) + "  " + str(OffsetVector[1]) + "  " + str(OffsetVector[2])
        print "   Length = " + str(math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2))
         
      # Include similar method for last two points 
      AbovePoint = self.LeftCtCommon[-2]
      BesidePoint = self.RightCtCommon[-1]
      LastLeft = self.LeftCtCommon[-1]
      VerticalVector = [AbovePoint[1][0] - LastLeft[1][0], AbovePoint[1][1] - LastLeft[1][1], AbovePoint[1][2] - LastLeft[1][2]]
      HorizontalVector = [BesidePoint[1][0] - LastLeft[1][0], BesidePoint[1][1] - LastLeft[1][1], BesidePoint[1][2] - LastLeft[1][2]]
      OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
      OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2) 
      for dim in range(3):
        OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
      self.LeftOffsetPoints.append((self.LeftCtCommon[-1][0] + "A", OffsetVector))
      
      AbovePoint = self.RightCtCommon[-2]
      LastRight = BesidePoint
      BesidePoint = LastLeft
      VerticalVector = [AbovePoint[1][0] - LastRight[1][0], AbovePoint[1][1] - LastRight[1][1], AbovePoint[1][2] - LastRight[1][2]]
      HorizontalVector = [BesidePoint[1][0] - LastRight[1][0], BesidePoint[1][1] - LastRight[1][1], BesidePoint[1][2] - LastRight[1][2]]
      OffsetVector = numpy.cross(HorizontalVector, VerticalVector)
      OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2) 
      for dim in range(3):
        OffsetVector[dim] = scale*(self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
      self.RightOffsetPoints.append((self.RightCtCommon[-1][0] + "A", OffsetVector))

      LeftIterator = 0
      RightIterator = 0
      for i in range(self.CommonFiducialsCt.GetNumberOfFiducials()):
        OriginalCtPoint = self.CommonFiducialsCt.GetMarkupPointVector(i,0)
        if(self.CommonFiducialsCt.GetNthFiducialLabel(i)[-1] == "L"):
          self.CommonFiducialsCt.AddFiducial(OriginalCtPoint[0] + self.LeftOffsetPoints[LeftIterator][1][0],OriginalCtPoint[1] + self.LeftOffsetPoints[LeftIterator][1][1],OriginalCtPoint[2] + self.LeftOffsetPoints[LeftIterator][1][2])
          self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, self.LeftOffsetPoints[LeftIterator][0])
          LeftIterator = LeftIterator + 1
        else:
          self.CommonFiducialsCt.AddFiducial(OriginalCtPoint[0] + self.RightOffsetPoints[RightIterator][1][0],OriginalCtPoint[1] + self.RightOffsetPoints[RightIterator][1][1],OriginalCtPoint[2] + self.RightOffsetPoints[RightIterator][1][2])
          self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, self.RightOffsetPoints[RightIterator][0])
          RightIterator = RightIterator + 1
       
      # The atlas points should have their offsets added by normal vectors rather than a constant anterior offset
      LeftIterator = 0
      RightIterator = 0
      for i in range(self.CommonFiducialsAtlas.GetNumberOfFiducials()-2):
        TopPoint = (self.CommonFiducialsAtlas.GetNthFiducialLabel(i),self.CommonFiducialsAtlas.GetMarkupPointVector(i,0))
        BelowPoint = (self.CommonFiducialsAtlas.GetNthFiducialLabel(i+2), self.CommonFiducialsAtlas.GetMarkupPointVector(i+2,0))
        VerticalVector = [TopPoint[1][0] - BelowPoint[1][0], TopPoint[1][1] - BelowPoint[1][1], TopPoint[1][2] - BelowPoint[1][2]]
        # This if-condition assumes points are ordered, all have a partner, and begin with a left point
        if(TopPoint[0][-1] == "L"):
          BesidePoint = (self.CommonFiducialsAtlas.GetNthFiducialLabel(i+1),self.CommonFiducialsAtlas.GetMarkupPointVector(i+1,0))
          HorizontalVector = [BesidePoint[1][0] - TopPoint[1][0], BesidePoint[1][1] - TopPoint[1][1], BesidePoint[1][2] - TopPoint[1][2]]
        else:
          BesidePoint = (self.CommonFiducialsAtlas.GetNthFiducialLabel(i-1),self.CommonFiducialsAtlas.GetMarkupPointVector(i-1,0))
          HorizontalVector = [TopPoint[1][0] - BesidePoint[1][0], TopPoint[1][1] - BesidePoint[1][1], TopPoint[1][2] - BesidePoint[1][2]]
        OffsetVector = numpy.cross(VerticalVector, HorizontalVector)
        OffsetNorm = math.sqrt((OffsetVector[0])**2 + (OffsetVector[1])**2 + (OffsetVector[2])**2)
        for dim in range(3):
          OffsetVector[dim] = (self.AntiSqishOffset)*OffsetVector[dim]/OffsetNorm
        self.CommonFiducialsAtlas.AddFiducial(TopPoint[1][0] + OffsetVector[0], TopPoint[1][1] + OffsetVector[1], TopPoint[1][2] + OffsetVector[2])
        self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, self.CommonFiducialsAtlas.GetNthFiducialLabel(i) + "A")

      # Special method for last two atlas points' offsets  
      self.CommonFiducialsAtlas.AddFiducial(self.LeftAtlasCommon[-1][1][0], self.LeftAtlasCommon[-1][1][1] + self.AntiSqishOffset, self.LeftAtlasCommon[-1][1][2])
      self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, self.LeftAtlasCommon[-1][0] + "A")
      self.CommonFiducialsAtlas.AddFiducial(self.RightAtlasCommon[-1][1][0], self.RightAtlasCommon[-1][1][1] + self.AntiSqishOffset, self.RightAtlasCommon[-1][1][2])
      self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, self.RightAtlasCommon[-1][0] + "A")
      
      slicer.mrmlScene.AddNode(self.CommonFiducialsCt)
      slicer.mrmlScene.AddNode(self.CommonFiducialsAtlas)
      print "\n Atlas model curve length(mm) = " + str(self.AtlasSpineLength) 
      print "Ct model curve length(mm) = " + str(self.CtSpineLength)
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
      return (self.CtNames, self.NamesIntersection)
    
    else: # For whatever reason, no points in common could be found between the CT and atlas
      print "Warning - intersection of CT and atlas points is the empty set."
      print "   Maybe points are named incorrectly."
      print "   Returning empty lists."
      return ([], [])
    


class CtToAtlasRegistrationTest():
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
    self.test_CtToAtlasRegistration1()

  def test_CtToAtlasRegistration1(self):
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
