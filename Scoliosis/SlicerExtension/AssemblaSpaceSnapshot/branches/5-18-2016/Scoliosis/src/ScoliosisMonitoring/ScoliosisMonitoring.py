from __main__ import vtk, qt, ctk, slicer
from VolumesViewer import *
from VolumeRenderingViewer import *
from ToolsViewer import *
#from WorkInProgress import *
import os
#
# ScoliosisMonitoring
#


class ScoliosisMonitoring:
  def __init__(self, parent):
    parent.title = "Scoliosis Monitoring"
    parent.categories = ["Testing"]
    parent.contributors = ["Franklin King (Queen's University), Tamas Ungi (Queen's University)"]
    parent.helpText = """
    This is a module for measuring spinal curvature by measuring angles specified by ruler annotations. See <a>https://www.assembla.com/spaces/Scoliosis/wiki/Spinal_Curvature_Measurement</a> for more information.
    """
    parent.acknowledgementText = """
    This work was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)
"""
    #parent.icon = qt.QIcon(":Icons/ScoliosisMonitoring.png")
    self.parent = parent

    
#
# qScoliosisMonitoringWidget
#
class ScoliosisMonitoringWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE = 7
    self.CONST_NUMBER_OF_THORACIC_VERTERBRAE = 12
    self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE = 5  
    self.observerTags = []
    self.verterbraeNames = []    
    if not parent:
      self.setup()
      self.parent.show()

      
  def setup(self):
    for cervicalVertebrae in range(1, self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE + 1):
      self.verterbraeNames.append("C"+str(cervicalVertebrae))
      
    for thoracicVertebrae in range(1, self.CONST_NUMBER_OF_THORACIC_VERTERBRAE + 1):
      self.verterbraeNames.append("T"+str(thoracicVertebrae))
      
    for lumbarVertebrae in range(1, self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE + 1):
      self.verterbraeNames.append("L"+str(lumbarVertebrae))
  
    self.superiorAnglesList = [None]*self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE
    self.inferiorAnglesList = [None]*self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE
  
    # Instantiate and connect widgets
    #
    # Brief Explanation Text
    instructionsCollapsibleButton = ctk.ctkCollapsibleButton()
    instructionsCollapsibleButton.text = "Instructions"
    #self.layout.addWidget(instructionsCollapsibleButton)    
    
    instructionsLayout = qt.QVBoxLayout(instructionsCollapsibleButton)    
    
    self.namingInstructions = ctk.ctkFittedTextBrowser()
    self.namingInstructions.setReadOnly(True)
    self.namingInstructions.setPlainText("C - Cervical, T - Thoracic, L - Lumbar\ns - Superior, i - Inferior\nName ruler annotations like so: T1-s, T1-i, T2-s, etc.")
    instructionsLayout.addWidget(self.namingInstructions)
    
    
    # Show slicelet button
    sliceletStartButton = qt.QPushButton("Show slicelet")
    sliceletStartButton.toolTip = "Start the slicelet"
    self.layout.addWidget(sliceletStartButton)
    sliceletStartButton.connect('clicked(bool)', self.onShowSliceletButtonClicked)
    
    
    
    # Input Area
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Input"
    #self.layout.addWidget(parametersCollapsibleButton)

    # Layout
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    # Input Selector
    self.inputSelector = slicer.qMRMLNodeComboBox()
    self.inputSelector.nodeTypes = ( ("vtkMRMLAnnotationHierarchyNode"), "" )
    self.inputSelector.selectNodeUponCreation = True
    self.inputSelector.addEnabled = True
    self.inputSelector.removeEnabled = True
    self.inputSelector.noneEnabled = False
    self.inputSelector.showHidden = False
    self.inputSelector.showChildNodeTypes = False
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    self.inputSelector.setToolTip( "Pick the input to the algorithm." )
    parametersFormLayout.addRow("Input Rulers: ", self.inputSelector)
    
    self.projectionPlaneBox = qt.QComboBox()  
    self.projectionPlaneBox.addItems(["Coronal", "Axial", "Sagittal"])
    parametersFormLayout.addRow("Projection Plane: ", self.projectionPlaneBox)    
    
    #
    # Output Area
    #    
    outputCollapsibleButton = ctk.ctkCollapsibleButton()
    outputCollapsibleButton.text = "Output"
    #self.layout.addWidget(outputCollapsibleButton)
    
    # Layout
    outputVerticalLayout = qt.QGridLayout(outputCollapsibleButton)

    # Calculate Button
    self.calculateButton = qt.QPushButton("Start Calculating")
    outputVerticalLayout.addWidget(self.calculateButton, 0, 0, 1, 5)
    
    # Save Button
    self.saveButton = qt.QPushButton("Save as CSV")
    outputVerticalLayout.addWidget(self.saveButton, 1, 0, 1, 5)
      
    # Angle Table
    horizontalHeaders = ["Vertebrae", "Superior Angles (degrees)", "Inferior Angles (degrees)"]

    self.angleTable = qt.QTableWidget(self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE + self.CONST_NUMBER_OF_THORACIC_VERTERBRAE + self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE, 3)
    self.angleTable.sortingEnabled = False
    self.angleTable.setEditTriggers(0)
    self.angleTable.setMinimumHeight(self.angleTable.verticalHeader().length() + 25)
    self.angleTable.horizontalHeader().setResizeMode(qt.QHeaderView.Stretch)
    self.angleTable.setSizePolicy (qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.Preferred)
    outputVerticalLayout.addWidget(self.angleTable, 2, 0, 1, 5)
    
    self.angleTable.setHorizontalHeaderLabels(horizontalHeaders)

    self.labelReferences = []
    self.superiorAngleReferences = []
    self.inferiorAngleReferences = []
    for verterbraeIndex, verterbraeName in enumerate(self.verterbraeNames):
      vertebraeLabel = qt.QTableWidgetItem(verterbraeName)
      self.labelReferences.append(vertebraeLabel)
      self.angleTable.setItem(verterbraeIndex, 0, vertebraeLabel)
      
      superiorAngleItem = qt.QTableWidgetItem("")
      self.superiorAngleReferences.append(superiorAngleItem)
      self.angleTable.setItem(verterbraeIndex, 1, superiorAngleItem)     

      inferiorAngleItem = qt.QTableWidgetItem("")
      self.inferiorAngleReferences.append(inferiorAngleItem)
      self.angleTable.setItem(verterbraeIndex, 2, inferiorAngleItem)
      
    # connections
    self.calculateButton.connect('clicked(bool)', self.onCalculateButton)
    self.saveButton.connect('clicked(bool)', self.onSaveButton)
    self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    self.layout.addStretch(1)
    
    
    
  def onShowSliceletButtonClicked(self):   
    slicelet = ScoliosisSliceletTestSlicelet() 
    
      
  def onSelect(self):
    logic = ScoliosisMonitoringLogic()
  
  
  def onCalculateButton(self):
    logic = ScoliosisMonitoringLogic()
    (self.superiorAnglesList, self.inferiorAnglesList) = logic.calculateAngles(self.inputSelector.currentNode(), self.projectionPlaneBox.currentText)
    
    superiorAnglesListCount = 0
    for superiorAngle in self.superiorAnglesList:
      self.angleTable.item(superiorAnglesListCount, 1).setText(str(superiorAngle))
      superiorAnglesListCount += 1

    inferiorAnglesListCount = 0
    for inferiorAngle in self.inferiorAnglesList:
      self.angleTable.item(inferiorAnglesListCount, 2).setText(str(inferiorAngle))
      inferiorAnglesListCount += 1
    
    # Refresh observers
    for rulerNode, tag in self.observerTags:
      rulerNode.RemoveObserver(tag)
    
    rulerNodes = vtk.vtkCollection();
    self.inputSelector.currentNode().GetDirectChildren(rulerNodes)
    
    rulerNodesList = []
    for nodeIndex in range(rulerNodes.GetNumberOfItems()):
      if rulerNodes.GetItemAsObject(nodeIndex).GetClassName() == "vtkMRMLAnnotationRulerNode":
        rulerNodesList.append(rulerNodes.GetItemAsObject(nodeIndex)) 
 
    for rulerNode in rulerNodesList:
      def update(caller, ev):
        import re
        angle = logic.updateAngle(caller, self.projectionPlaneBox.currentText)
        
        rulerName = caller.GetName()
        #nameSearch.group(1) - Region (C, T, or L)
        #nameSearch.group(2) - Number (1, 2, 3, etc.)
        #nameSearch.group(3) - Superior or Inferior (s or i)
        nameSearch = re.search(r"^([CTL])(\d+)-([si])$", rulerName)
        if nameSearch != None:
          if nameSearch.group(1) == "C":
            index = int(nameSearch.group(2))-1
          elif nameSearch.group(1) == "T":
            index = int(nameSearch.group(2)) + self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE - 1
          elif nameSearch.group(1) == "L":
            index = int(nameSearch.group(2)) + self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE + self.CONST_NUMBER_OF_THORACIC_VERTERBRAE - 1
            
          if nameSearch.group(3) == "s":
            self.angleTable.item(index, 1).setText(str(angle))
          elif nameSearch.group(3) == "i":
            self.angleTable.item(index, 2).setText(str(angle))
        
      tag = rulerNode.AddObserver(rulerNode.ControlPointModifiedEvent, update)
      self.observerTags.append([rulerNode, tag])
      
  
  def onSaveButton(self):
    import csv
    
    fileName = qt.QFileDialog.getSaveFileName(0, "Save Angle Data", "", "CSV File (*.csv)")
    
    with open(fileName, 'wb') as csvfile:
      writer = csv.writer(csvfile, delimiter=',', quotechar='|')
      writer.writerow(['Vertebrae', 'Superior Angle (degrees)', 'Inferior Angle (degrees)'])
      
      for rowIndex in range(len(self.labelReferences)):
        writer.writerow([self.labelReferences[rowIndex].text(), self.superiorAngleReferences[rowIndex].text(), self.inferiorAngleReferences[rowIndex].text()])
    
    
#
# ScoliosisMonitoringLogic
#
class ScoliosisMonitoringLogic:
  def __init__(self):
    self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE = 7
    self.CONST_NUMBER_OF_THORACIC_VERTERBRAE = 12
    self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE = 5
    self.connectorNode = None 
    self.trackerToWallTransformationMatrix = None
    vrd = slicer.modules.volumereslicedriver
    self.vrdl = vrd.logic()
    self.vrdl.SetMRMLScene(slicer.mrmlScene)
    self.windows = 128
    self.level=192
   
  def calculateAngles(self, annotationHierarchy, planeOrientation):
    import re, math
  
    superiorCervicalAngles = [None]*self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE
    superiorThoracicAngles = [None]*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE
    superiorLumbarAngles = [None]*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE
    
    inferiorCervicalAngles = [None]*self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE
    inferiorThoracicAngles = [None]*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE
    inferiorLumbarAngles = [None]*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE    
  
    if planeOrientation == "Coronal":
      planeNormal = [0,1,0]
      baseAngleVector = [1,0,0]
    elif planeOrientation == "Axial":
      planeNormal = [0,0,1]
      baseAngleVector = [1,0,0]
    else:
      planeNormal = [1,0,0]
      baseAngleVector = [0,1,0]
      
    rulerNodes = vtk.vtkCollection();
    annotationHierarchy.GetDirectChildren(rulerNodes)
    
    rulerNodesList = []
    for nodeIndex in range(rulerNodes.GetNumberOfItems()):
      if rulerNodes.GetItemAsObject(nodeIndex).GetClassName() == "vtkMRMLAnnotationRulerNode":
        rulerNodesList.append(rulerNodes.GetItemAsObject(nodeIndex))

    for ruler in rulerNodesList:
      angle = self.updateAngle(ruler, planeOrientation)
      
      rulerName = ruler.GetName()
      #nameSearch.group(1) - Region (C, T, or L)
      #nameSearch.group(2) - Number (1, 2, 3, etc.)
      #nameSearch.group(3) - Superior or Inferior (s or i)
      nameSearch = re.search(r"^([CTL])(\d+)-([si])$", rulerName)
      if nameSearch != None:
        if nameSearch.group(1) == "C":
          if nameSearch.group(3) == "s":
            superiorCervicalAngles[int(nameSearch.group(2)) - 1] = angle
          elif nameSearch.group(3) == "i":
            inferiorCervicalAngles[int(nameSearch.group(2)) - 1] = angle
        elif nameSearch.group(1) == "T":
          if nameSearch.group(3) == "s":
            superiorThoracicAngles[int(nameSearch.group(2)) - 1] = angle
          elif nameSearch.group(3) == "i":
            inferiorThoracicAngles[int(nameSearch.group(2)) - 1] = angle     
        elif nameSearch.group(1) == "L":
          if nameSearch.group(3) == "s":
            superiorLumbarAngles[int(nameSearch.group(2)) - 1] = angle
          elif nameSearch.group(3) == "i":
            inferiorLumbarAngles[int(nameSearch.group(2)) - 1] = angle      
    
    return (superiorCervicalAngles + superiorThoracicAngles + superiorLumbarAngles, inferiorCervicalAngles + inferiorThoracicAngles + inferiorLumbarAngles)
  
  
  def updateAngle(self, ruler, planeOrientation):
    import math
    
    if planeOrientation == "Coronal":
      planeNormal = [0,1,0]
      baseAngleVector = [1,0,0]
    elif planeOrientation == "Axial":
      planeNormal = [0,0,1]
      baseAngleVector = [1,0,0]
    else:
      planeNormal = [1,0,0]
      baseAngleVector = [0,1,0]    
    
    position1 = [0,0,0]
    ruler.GetPosition1(position1)
    
    position2 = [0,0,0]
    ruler.GetPosition2(position2)
    
    vector = [position1[0] - position2[0], position1[1] - position2[1], position1[2] - position2[2]]
    vtk.vtkPlane.ProjectPoint(vector, [0,0,0], planeNormal, vector)
    
    norm = vtk.vtkMath.Norm(vector)
    if norm == 0:
      return None
    
    angle = vtk.vtkMath.DegreesFromRadians( math.acos( vtk.vtkMath.Dot(vector, baseAngleVector) / norm ) )
    if angle > 90:
      angle = angle - 180      
    if planeOrientation == "Coronal" or planeOrientation == "Sagittal":
      if position1[2] < position2[2]:
        angle = -angle
    else:
      if position1[1] < position2[1]:
        angle = -angle
        
    return angle

  def captureStylusTipPosition(self, fidName ):
    saml = slicer.modules.annotations.logic() 
    listNode = slicer.util.getNode("Key Points List")
    saml.SetActiveHierarchyNodeID(listNode.GetID())  
    StylusTipToReferenceNode = slicer.util.getNode("StylusTipToReference")
    validTransformation = self.isValidTransformation("StylusTipToReference") 
    
    if validTransformation == True: 
      transformation = StylusTipToReferenceNode.GetMatrixTransformToParent()
      fidPos=[transformation.GetElement(0,3),transformation.GetElement(1,3),transformation.GetElement(2,3)]
      keyPointNode=slicer.vtkMRMLAnnotationFiducialNode()
      slicer.mrmlScene.AddNode(keyPointNode)
      keyPointNode.SetFiducialWorldCoordinates(fidPos)
      keyPointNode.SetName(fidName)    
      keyPointDisplayNode=keyPointNode.GetAnnotationPointDisplayNode()
      keyPointTextDisplay=keyPointNode.GetAnnotationTextDisplayNode()
      keyPointDisplayNode.SetColor([0,0,1])
      keyPointTextDisplay.SetColor([0,0,1])
      self.setActiveAnnotationsList("Key Points List") 
      print("Key point position recorded")   
    else:
      print("Key point position is invalid")  
    return validTransformation
    
  def computeTransverseProcessesAngle(self, upperRuler, lowerRuler, RASToWallMatrixTransform):
    import math
    
    upperLeftPoint = [0,0,0]
    upperRightPoint = [0,0,0]
    
    # upper vector projection
  
    upperRuler.GetPosition1(upperLeftPoint)
    upperRuler.GetPosition2(upperRightPoint)
    
    upperLeftPointProjection = RASToWallMatrixTransform.MultiplyPoint([upperLeftPoint[0],upperLeftPoint[1],upperLeftPoint[2],1])
    upperRightPointProjection = RASToWallMatrixTransform.MultiplyPoint([upperRightPoint[0],upperRightPoint[1],upperRightPoint[2],1])
    
    projectedUpperVector = [upperLeftPointProjection[0]-upperRightPointProjection[0],upperLeftPointProjection[1]-upperRightPointProjection[1],0]
    UpperProjectedVectorNorm = vtk.vtkMath.Norm(projectedUpperVector)
    print "Upper ruler left position: " + str(upperLeftPoint)
    print "Upper ruler right position: " + str(upperRightPoint)
    print "Upper vector after projection: " + str(projectedUpperVector)
    
    # lower vector projection
    
    lowerLeftPoint = [0,0,0]
    lowerRightPoint = [0,0,0]
    
    lowerRuler.GetPosition1(lowerLeftPoint)
    lowerRuler.GetPosition2(lowerRightPoint)
    
    lowerLeftPointProjection = RASToWallMatrixTransform.MultiplyPoint([lowerLeftPoint[0],lowerLeftPoint[1],lowerLeftPoint[2],1])
    lowerRightPointProjection = RASToWallMatrixTransform.MultiplyPoint([lowerRightPoint[0],lowerRightPoint[1],lowerRightPoint[2],1])

    projectedLowerVector = [lowerLeftPointProjection[0]-lowerRightPointProjection[0],lowerLeftPointProjection[1]-lowerRightPointProjection[1],0]
    
    LowerProjectedVectorNorm = vtk.vtkMath.Norm(projectedLowerVector)
    print "Lower ruler left position: " + str(lowerLeftPoint)
    print "Lower ruler right position: " + str(lowerRightPoint)
    print "Lower vector after projection: " + str(projectedLowerVector)
    
    angle = vtk.vtkMath.DegreesFromRadians( math.acos( vtk.vtkMath.Dot(projectedUpperVector, projectedLowerVector)/(UpperProjectedVectorNorm*LowerProjectedVectorNorm)) )
    print "angle= " + str(angle)   
    return angle


  def computeTransverseProcessesAngleOld(self, upperRuler, lowerRuler, planeOrientation):
    import math
    
    if planeOrientation == "Coronal":
      planeNormal = [0,1,0]
      baseAngleVector = [1,0,0]
    elif planeOrientation == "Axial":
      planeNormal = [0,0,1]
      baseAngleVector = [1,0,0]
    else:
      planeNormal = [1,0,0]
      baseAngleVector = [0,1,0]    
    
    
    position1 = [0,0,0]
    position2 = [0,0,0]
    
    # upper vector projection
    projectedUpperVector = [0,0,0]
    upperRuler.GetPosition1(position1)
    upperRuler.GetPosition2(position2)
    upperVector = [position1[0] - position2[0], position1[1] - position2[1], position1[2] - position2[2]]
    
    
    vtk.vtkPlane.ProjectPoint(upperVector, [0,0,0], planeNormal, projectedUpperVector)
    UpperProjectedVectorNorm = vtk.vtkMath.Norm(projectedUpperVector)
    print "Upper ruler position 1: " + str(position1)
    print "Upper ruler position 2: " + str(position2)
    print "Projected Upper vector: " + str(projectedUpperVector)
    
    # lower vector projection
    lowerProjectedVector = [0,0,0]
    lowerRuler.GetPosition1(position1)
    lowerRuler.GetPosition2(position2)
    lowerVector = [position1[0] - position2[0], position1[1] - position2[1], position1[2] - position2[2]]
    
    vtk.vtkPlane.ProjectPoint(lowerVector, [0,0,0], planeNormal, lowerProjectedVector)   
    LowerProjectedVectorNorm = vtk.vtkMath.Norm(lowerProjectedVector)
    print "Lower ruler position 1: " + str(position1)
    print "Lower ruler position 2: " + str(position2)
    print "Lower Projected vector: " + str(lowerProjectedVector)
    
    angle = vtk.vtkMath.DegreesFromRadians( math.acos( vtk.vtkMath.Dot(upperVector, lowerVector)/(UpperProjectedVectorNorm*LowerProjectedVectorNorm)) )
    print "angle= " + str(angle)   
    return angle


  def populateRulers(self, annotationHierarchy, verterbraeNames):
    completeVerterbraeNames = []
    for verterbraeName in verterbraeNames:
      completeVerterbraeNames.append(verterbraeName + "-s")
      completeVerterbraeNames.append(verterbraeName + "-i")

    if annotationHierarchy != None:
      rulerNodes = vtk.vtkCollection()
      annotationHierarchy.GetDirectChildren(rulerNodes)
      
      rulerNodeNames = []
      for nodeIndex in range(rulerNodes.GetNumberOfItems()):
        if rulerNodes.GetItemAsObject(nodeIndex).GetClassName() == "vtkMRMLAnnotationRulerNode":
          rulerNodeNames.append(rulerNodes.GetItemAsObject(nodeIndex).GetName())
      
      addedRulerNodesList = []
      addedVerterbrae = 200
      for completeVerterbraeName in completeVerterbraeNames:
        if completeVerterbraeName not in rulerNodeNames:
          newRulerNode = slicer.vtkMRMLAnnotationRulerNode()
          newRulerNode.SetName(completeVerterbraeName)
          newRulerNode.SetPosition1(0, addedVerterbrae, 0)
          newRulerNode.SetPosition2(-300, addedVerterbrae, 0)
          newRulerNode.SetPointColour([0,0,1])
          newRulerNode.Initialize(slicer.mrmlScene)
          annotationHierarchy.SetAndObserveDisplayNodeID(newRulerNode.GetDisplayNodeID())
          addedVerterbrae -= 50
          addedRulerNodesList.append(newRulerNode)
          
    else:
      addedVerterbrae = 200
      for completeVerterbraeName in completeVerterbraeNames:
        newRulerNode = slicer.vtkMRMLAnnotationRulerNode()
        newRulerNode.SetName(completeVerterbraeName)
        newRulerNode.SetPosition1(0, addedVerterbrae, 0)
        newRulerNode.SetPosition2(-300, addedVerterbrae, 0)
        newRulerNode.Initialize(slicer.mrmlScene)
        annotationHierarchy.SetAndObserveDisplayNodeID(newRulerNode.GetDisplayNodeID())
        addedVerterbrae -= 50
   
    return addedRulerNodesList


  
  def addRulerNodeToTheRulerList(self, rulerListNode, rulerNodeName):
    saml = slicer.modules.annotations.logic()
    saml.SetActiveHierarchyNodeID(rulerListNode.GetID())
    addedVerterbrae = 200
    newRulerNode = slicer.vtkMRMLAnnotationRulerNode()
    newRulerNode.SetName(rulerNodeName)
    leftFiducial = slicer.util.getNode("Left"+rulerNodeName+"Fiducial")
    rightFiducial = slicer.util.getNode("Right"+rulerNodeName+"Fiducial")
    leftPos=[0, 0 ,0]
    rightPos=[0, 0 ,0]
    leftFiducial.GetFiducialCoordinates(leftPos)
    rightFiducial.GetFiducialCoordinates(rightPos)
    newRulerNode.SetPosition1(leftPos)
    newRulerNode.SetPosition2(rightPos)
    newRulerNode.Initialize(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(newRulerNode)
    print "A ruler node was added to the list"  
    #rulerListNode.SetAndObserveDisplayNodeID(newRulerNode.GetDisplayNodeID())      
  
  def createAndAssociateConectorNodeWithScene(self):
    cn = slicer.util.getNode('Plus Server Connection')  
    if cn == None:
       cn = slicer.vtkMRMLIGTLConnectorNode()
       slicer.mrmlScene.AddNode(cn)
       cn.SetName('Plus Server Connection')
       print("IGTL Connector node was created!")
    self.connectorNode = cn
       
  def connectWithTracker(self):
    self.connectorNode.SetTypeClient("localhost", 18944)
    print("Status before start(): " + str(self.connectorNode.GetState()))
    self.startTracking()
    print("Connected with Plus Server in Slicelet Class ")
    print("Status after start(): " + str(self.connectorNode.GetState()))   
    # self.associateTransformations()  
        
  def startTracking(self):
      self.connectorNode.Start()
      
  def stopTracking(self):
    self.connectorNode.Stop()  
    
  def getConnectorNode(self): 
      return self.connectorNode
  
  def getVolumeResliceDriverLogic(self):
    return self.vrdl

  def showRedSliceIn3D(self, isShown):  
    image_Reference = slicer.util.getNode("Image_Reference")  
    if image_Reference is not None:
      redNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
      #vrd = slicer.modules.volumereslicedriver
      #vrdl = vrd.logic()
      #vrdl.SetMRMLScene(slicer.mrmlScene)
      self.vrdl.SetDriverForSlice(image_Reference.GetID(), redNode)
      self.vrdl.SetModeForSlice(self.vrdl.MODE_TRANSVERSE180, redNode)
      
      # Set the background volume 
      redWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeRed")
      redWidgetCompNode.SetBackgroundVolumeID(image_Reference.GetID())
      # Show the volume in 3D
      redNode.SetSliceVisible(isShown)
      # Adjust the size of the US image in the Axial view
      sliceLogic = slicer.vtkMRMLSliceLogic()
      sliceLogic.SetName("Red")
      sliceLogic.SetMRMLScene(slicer.mrmlScene)
      # sliceLogic.FitSliceToAll()
      greenNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeGreen")
      yellowNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeYellow")
      greenNode.SetSliceVisible(False)
      yellowNode.SetSliceVisible(False)
      self.adjustDisplaySettings(image_Reference)


  def adjustDisplaySettings(self, imageNode):
   # set the display settings
   display=imageNode.GetDisplayNode()
   display.AutoWindowLevelOff()
   display.SetWindowLevel(self.windows,self.level)    
  
  def listenToImageSentToTheScene(self):
   self.sceneObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onImageSentToTheScene)
    

  def doNotListenToImageSentToTheScene(self):
    slicer.mrmlScene.RemoveObserver(self.sceneObserver)

  def onImageSentToTheScene(self, caller, event):
    image_Reference = slicer.util.getNode("Image_Reference")   
    if image_Reference is not None: 
      self.doNotListenToImageSentToTheScene()
      self.showRedSliceIn3D(True)    


  def takeUSSnapshot(self):
    image_RAS = slicer.util.getNode(self.USImageName)
    usn = slicer.modules.ultrasoundsnapshots
    usnl = usn.logic()
    usnl.AddSnapshot(image_RAS)
    
  def takeUSSnapshot2(self,name):
    snapshotDisp = slicer.vtkMRMLModelDisplayNode()
    slicer.mrmlScene.AddNode(snapshotDisp)
    snapshotDisp.SetScene(slicer.mrmlScene)
    snapshotDisp.SetDisableModifiedEvent(1)
    snapshotDisp.SetOpacity(1.0)
    snapshotDisp.SetColor(1.0, 1.0, 1.0)
    snapshotDisp.SetAmbient(1.0)
    snapshotDisp.SetBackfaceCulling(0)
    snapshotDisp.SetDiffuse(0)
    snapshotDisp.SetSaveWithScene(0)
    snapshotDisp.SetDisableModifiedEvent(0)
    
    snapshotModel = slicer.vtkMRMLModelNode()
    snapshotModel.SetName(name)
    snapshotModel.SetDescription("Live Ultrasound Snapshot")
    snapshotModel.SetScene(slicer.mrmlScene)
    snapshotModel.SetAndObserveDisplayNodeID(snapshotDisp.GetID())
    snapshotModel.SetHideFromEditors(0)
    snapshotModel.SetSaveWithScene(0)
    slicer.mrmlScene.AddNode(snapshotModel)
    
    image_RAS = slicer.util.getNode("Image_Reference")
    
    dim = [0, 0, 0]
    imageData = image_RAS.GetImageData()
    imageData.GetDimensions(dim)
    
    plane = vtk.vtkPlaneSource()
    plane.Update()
    snapshotModel.SetAndObservePolyData(plane.GetOutput())
    
    slicePolyData = snapshotModel.GetPolyData()
    slicePoints = slicePolyData.GetPoints()
    
    # In parent transform is saved the ReferenceToRAS transform
    parentTransform = vtk.vtkTransform()
    parentTransform.Identity()
    if not image_RAS.GetParentTransformNode() == None:
      parentMatrix = vtk.vtkMatrix4x4()
      parentTransformNode = image_RAS.GetParentTransformNode()
      parentTransformNode.GetMatrixTransformToWorld(parentMatrix)
      # aux=parentTransform.GetMatrix()
      # aux.DeepCopy(parentMatrix)
      # parentTransform.Update()
      parentTransform.SetMatrix(parentMatrix)
      
    inImageTransform = vtk.vtkTransform()
    inImageTransform.Identity()
    image_RAS.GetIJKToRASMatrix(inImageTransform.GetMatrix())
    
    tImageToRAS = vtk.vtkTransform()
    tImageToRAS.Identity()
    tImageToRAS.PostMultiply()
    tImageToRAS.Concatenate(inImageTransform)
    tImageToRAS.Concatenate(parentTransform)
   
    tImageToRAS.Update()
    
    point1Image = [0.0, 0.0, 0.0, 1.0]
    point2Image = [dim[0], 0.0, 0.0, 1.0]
    point3Image = [0.0, dim[1], 0.0, 1.0]
    point4Image = [dim[0], dim[1], 0.0, 1.0]
    
    point1RAS = [0.0, 0.0, 0.0, 0.0]
    point2RAS = [0.0, 0.0, 0.0, 0.0]
    point3RAS = [0.0, 0.0, 0.0, 0.0]
    point4RAS = [0.0, 0.0, 0.0, 0.0]
    tImageToRAS.MultiplyPoint(point1Image, point1RAS)
    tImageToRAS.MultiplyPoint(point2Image, point2RAS)
    tImageToRAS.MultiplyPoint(point3Image, point3RAS)
    tImageToRAS.MultiplyPoint(point4Image, point4RAS)  
    
    p1RAS = [point1RAS[0], point1RAS[1], point1RAS[2]]
    p2RAS = [point2RAS[0], point2RAS[1], point2RAS[2]]
    p3RAS = [point3RAS[0], point3RAS[1], point3RAS[2]]
    p4RAS = [point4RAS[0], point4RAS[1], point4RAS[2]]
    slicePoints.SetPoint(0, p1RAS)
    slicePoints.SetPoint(1, p2RAS)
    slicePoints.SetPoint(2, p3RAS)
    slicePoints.SetPoint(3, p4RAS)
    # # Add image texture.
    image = vtk.vtkImageData()
    image.DeepCopy(imageData)
    modelDisplayNode = snapshotModel.GetModelDisplayNode()
    modelDisplayNode.SetAndObserveTextureImageData(image)   
    
    snapshotTexture = slicer.vtkMRMLScalarVolumeNode()
    snapshotTexture.SetAndObserveImageData(image)
    snapshotTexture.SetName(name + "_Texture")
    slicer.mrmlScene.AddNode(snapshotTexture)
    snapshotTexture.CopyOrientation( image_RAS )
    
    snapshotTextureDisplayNode = slicer.vtkMRMLScalarVolumeDisplayNode()
    snapshotTextureDisplayNode.SetName(name + "_TextureDisplay")
    snapshotTextureDisplayNode.SetAutoWindowLevel(0);
    snapshotTextureDisplayNode.SetWindow(256);
    snapshotTextureDisplayNode.SetLevel(128);
    snapshotTextureDisplayNode.SetDefaultColorMap();
    slicer.mrmlScene.AddNode(snapshotTextureDisplayNode)
    
    snapshotTexture.AddAndObserveDisplayNodeID( snapshotTextureDisplayNode.GetID() )
    
    snapshotModel.SetAttribute( "TextureNodeID", snapshotTexture.GetID() )
    snapshotModelDisplayNode= snapshotModel.GetModelDisplayNode()
    snapshotModelDisplayNode.SetAndObserveTextureImageData( snapshotTexture.GetImageData() )
    
  def createTransverseProcessesList(self): 
    saml = slicer.modules.annotations.logic()
    saml.AddHierarchy()
    createdNode = saml.GetActiveHierarchyNode()
    createdNode.SetName("Transverse Processes List")

  def createRulerList(self): 
    saml = slicer.modules.annotations.logic()
    activeNode = saml.GetActiveHierarchyNode()
    parentNode=activeNode.GetParentNode()
    saml.SetActiveHierarchyNodeID(parentNode.GetID())
    saml.AddHierarchy()
    createdNode = saml.GetActiveHierarchyNode()
    createdNode.SetName("Ruler List")
    
  def createKeyPointsList(self): 
    saml = slicer.modules.annotations.logic()
    activeNode = saml.GetActiveHierarchyNode()
    parentNode=activeNode.GetParentNode()
    saml.SetActiveHierarchyNodeID(parentNode.GetID())
    saml.AddHierarchy()
    createdNode = saml.GetActiveHierarchyNode()
    createdNode.SetName("Key Points List")   
    
  def setActiveAnnotationsList(self, listName):
    saml = slicer.modules.annotations.logic()   
    listNode = slicer.util.getNode(listName)   
    if listNode is not None:
      saml.SetActiveHierarchyNodeID(listNode.GetID())
          
  def addFiducial(self, name):
    print("Place fiducial pressed")
    saml = slicer.modules.annotations.logic() 
    listNode = slicer.util.getNode("Transverse Processes List")
    if listNode is not None:
      saml.SetActiveHierarchyNodeID(listNode.GetID())
      snode = slicer.vtkMRMLSelectionNode.SafeDownCast(slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton"))
      inode = slicer.vtkMRMLInteractionNode.SafeDownCast(slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton"))
      #snode.SetActiveAnnotationID("vtkMRMLAnnotationFiducialNode")
      #snode.SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode")
      snode.SetReferenceActivePlaceNodeClassName("vtkMRMLAnnotationFiducialNode")
      #snode.SetReferenceActiveFiducialListID(listNode.GetID())
      inode.SwitchToSinglePlaceMode()
      print "Now we are in the place mode"
    else:
      print "Transverse Processes List" + "list does not exit!" 
      
  def setTrackerToWallTransformationMatrix(self):
      transformationNode = slicer.util.getNode("TrackerToWall")
      transformation = transformationNode.GetMatrixTransformToParent()
      self.trackerToWallTransformationMatrix = transformation   
    
  def getTrackerToWallTransformationMatrix(self):
      return self.trackerToWallTransformationMatrix
  
  def isValidTransformation(self, transformationNodeName):
 
      transformationNode = slicer.util.getNode(transformationNodeName)
      # transformation=vtk.vtkMatrix4x4()
      # transformationNode.GetMatrixTransformToWorld(transformation)
      transformation = transformationNode.GetMatrixTransformToParent()
      # print "Transformation is: "  
      # print transformation 
      
      validTransformation = True
      a00 = transformation.GetElement(0, 0)     
      a01 = transformation.GetElement(0, 1) 
      a02 = transformation.GetElement(0, 2) 
      a03 = transformation.GetElement(0, 3)
      
      a10 = transformation.GetElement(1, 0)     
      a11 = transformation.GetElement(1, 1) 
      a12 = transformation.GetElement(1, 2) 
      a13 = transformation.GetElement(1, 3) 
      
      a20 = transformation.GetElement(2, 0)     
      a21 = transformation.GetElement(2, 1) 
      a22 = transformation.GetElement(2, 2) 
      a23 = transformation.GetElement(2, 3)    
      
      a30 = transformation.GetElement(3, 0)     
      a31 = transformation.GetElement(3, 1) 
      a32 = transformation.GetElement(3, 2) 
      a33 = transformation.GetElement(3, 3)   
      
      if(a00 == 1 and a01 == 0 and a02 == 0 and a03 == 0 and 
         a10 == 0 and a11 == 1 and a12 == 0 and a13 == 0 and
         a20 == 0 and a21 == 0 and a22 == 1 and a32 == 0 and
         a30 == 0 and a31 == 0 and a23 == 0 and a33 == 1):
         validTransformation = False   
      
      # print "Transformation is valid:" + str(validTransformation)  
      return validTransformation

  
  def associateTransformations(self):
      
    referenceToRASNode = slicer.util.getNode("ReferenceToRAS")
    if referenceToRASNode == None:
      referenceToRASNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(referenceToRASNode)
      referenceToRASNode.SetName("ReferenceToRAS")  
    
    # # The nodes StylusTipToReference and ProbeToReference are added 
    stylusTipToReferenceNode = slicer.util.getNode("StylusTipToReference")
    if stylusTipToReferenceNode == None:
      stylusTipToReferenceNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(stylusTipToReferenceNode)
      stylusTipToReferenceNode.SetName("StylusTipToReference")
    
    probeToReferenceNode = slicer.util.getNode("ProbeToReference")
    if probeToReferenceNode == None:
      probeToReferenceNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(probeToReferenceNode)
      probeToReferenceNode.SetName("ProbeToReference")
      
    referenceToTrackerNode = slicer.util.getNode("ReferenceToTracker")
    if referenceToTrackerNode == None:
      referenceToTrackerNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(referenceToTrackerNode)
      referenceToTrackerNode.SetName("ReferenceToTracker") 
      
    r2ToTrackerNode = slicer.util.getNode("R2ToTracker")
    if r2ToTrackerNode == None:
      r2ToTrackerNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(r2ToTrackerNode)
      r2ToTrackerNode.SetName("R2ToTracker")  

    r3ToTrackerNode = slicer.util.getNode("R3ToTracker")
    if r3ToTrackerNode == None:
      r3ToTrackerNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(r3ToTrackerNode)
      r3ToTrackerNode.SetName("R3ToTracker")                
      
    wallToTrackerNode = slicer.util.getNode("WallToTracker")
    if wallToTrackerNode == None:
      wallToTrackerNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(wallToTrackerNode)
      wallToTrackerNode.SetName("WallToTracker")    
      
    imageToReferenceNode = slicer.util.getNode("ImageToReference")
    if imageToReferenceNode == None:
      imageToReferenceNode = slicer.vtkMRMLLinearTransformNode()
      slicer.mrmlScene.AddNode(imageToReferenceNode)
      imageToReferenceNode.SetName("ImageToReference")  
    
    imageReferenceNode = slicer.util.getNode("Image_Reference")
    if imageReferenceNode == None:
      imageReferenceNode = slicer.vtkMRMLScalarVolumeNode()
      slicer.mrmlScene.AddNode(imageReferenceNode)
      imageReferenceNode.SetName("Image_Reference")         
         
    # imageReferenceNode=slicer.util.getNode("Image_Reference")
    imageReferenceNode.SetAndObserveTransformNodeID(referenceToRASNode.GetID())
    
    # imageToReferenceNode=slicer.util.getNode("ImageToReference")
    imageToReferenceNode.SetAndObserveTransformNodeID(referenceToRASNode.GetID())
    
    # probeToReferenceNode=slicer.util.getNode("ProbeToReference")
    probeToReferenceNode.SetAndObserveTransformNodeID(referenceToRASNode.GetID())   
    
    # # Associate the stylus to reference transform with the reference to RAS
    stylusTipToReferenceNode.SetAndObserveTransformNodeID(referenceToRASNode.GetID())     
     
    self.stylusModelNode = slicer.util.getNode("Stylus_Example")    
    if self.stylusModelNode == None:    
        # Add the stylus model
        modelsModule = slicer.modules.models
        modelsModuleLogic = modelsModule.logic()
        modelsModuleLogic.SetMRMLScene(slicer.mrmlScene)
        path = slicer.modules.scoliosismonitoring.path
        modulePath = os.path.dirname(path)
        stylusModelFile = os.path.join(modulePath, "Resources/Models/Stylus_Example.stl")
        modelsModuleLogic.AddModel(stylusModelFile)
        self.stylusModelNode = slicer.util.getNode("Stylus_Example")
    
    self.stylusModelNode.RemoveAllNodeReferenceIDs("transform")
  
    # # Associate the model of the stylus with the stylus tip transforms
    stylusTipModelToStylusTipTransform = slicer.util.getNode("StylusTipModelToStylusTip")
    if stylusTipModelToStylusTipTransform == None:
        stylusTipModelToStylusTipTransform = slicer.vtkMRMLLinearTransformNode()
        slicer.mrmlScene.AddNode(stylusTipModelToStylusTipTransform) 
        stylusTipModelToStylusTipTransform.SetName("StylusTipModelToStylusTip") 
        
    matrix = vtk.vtkMatrix4x4()
    matrix.SetElement(0, 3, -210)
    stylusTipModelToStylusTipTransform.SetAndObserveMatrixTransformToParent(matrix)
    stylusTipModelToStylusTipTransform.SetAndObserveTransformNodeID(stylusTipToReferenceNode.GetID())
    
    self.stylusModelNode.SetAndObserveTransformNodeID(stylusTipModelToStylusTipTransform.GetID())


  def getStylusModel(self):
      return self.stylusModelNode


  def showReconstructedVolume(self, name):  
    recvol_Reference = slicer.util.getNode(name) 
    
    redNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
    redNode.SetSliceVisible(True)  
    redWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeRed")
    redWidgetCompNode.SetBackgroundVolumeID(recvol_Reference.GetID())
    redNode.SetOrientation("Axial")
    sliceLogicRed = slicer.vtkMRMLSliceLogic()
    sliceLogicRed.SetName("Red")
    sliceLogicRed.SetMRMLScene(slicer.mrmlScene)
    sliceLogicRed.FitSliceToAll()
    
    greenNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeGreen")
    greenNode.SetSliceVisible(True)  
    greenWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeGreen")
    greenWidgetCompNode.SetBackgroundVolumeID(recvol_Reference.GetID())
    greenNode.SetOrientation("Coronal")
    sliceLogicGreen = slicer.vtkMRMLSliceLogic()
    sliceLogicGreen.SetName("Green")
    sliceLogicGreen.SetMRMLScene(slicer.mrmlScene)
    sliceLogicGreen.FitSliceToAll()
    
    yellowNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeYellow")
    yellowNode.SetSliceVisible(True)  
    yellowWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeYellow")
    yellowWidgetCompNode.SetBackgroundVolumeID(recvol_Reference.GetID())
    yellowNode.SetOrientation("Sagittal")
    sliceLogicYellow = slicer.vtkMRMLSliceLogic()
    sliceLogicYellow.SetName("Yellow")
    sliceLogicYellow.SetMRMLScene(slicer.mrmlScene)
    sliceLogicYellow.FitSliceToAll()
    

  def startAcquisition(self, igtlRemoteLogic, igtlConnectorNode, OutputFilename):
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="StartRecording" OutputFilename="' + OutputFilename + '" CaptureDeviceId="CaptureDevice"></Command>', igtlConnectorNode.GetID())
    return self.commandId
 
    
  def stopAcquisition(self, igtlRemoteLogic, igtlConnectorNode): 
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="StopRecording"  CaptureDeviceId="CaptureDevice"></Command>', igtlConnectorNode.GetID())
    return self.commandId    
        
  def reconstructVolume(self, igtlRemoteLogic, igtlConnectorNode, PreAcquisitionFilename, OutputVolFilename, OutputVolDeviceName):
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="ReconstructVolume" InputSeqFilename="' + PreAcquisitionFilename + '" OutputVolFilename="' + OutputVolFilename + '" OutputVolDeviceName="' + OutputVolDeviceName + '"></Command>', igtlConnectorNode.GetID())       
    return self.commandId   

  def startVolumeReconstruction(self, igtlRemoteLogic, igtlConnectorNode, OutputVolFilename, OutputVolDeviceName):
    self.commandId = igtlRemoteLogic.SendCommand('<Command Name="StartVolumeReconstruction" OutputVolFilename="' + OutputVolFilename + '" OutputVolDeviceName="' + OutputVolDeviceName + '" ChannelId="TrackedVideoStream"></Command>', igtlConnectorNode.GetID()) 
    return self.commandId   

  def suspendVolumeReconstruction(self, igtlRemoteLogic, igtlConnectorNode): 
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="SuspendVolumeReconstruction"></Command>', igtlConnectorNode.GetID())  
    return self.commandId   
    
  def resumeVolumeReconstruction(self, igtlRemoteLogic, igtlConnectorNode): 
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="ResumeVolumeReconstruction"></Command>', igtlConnectorNode.GetID())   
    return self.commandId   
    
  def stopVolumeReconstruction(self, igtlRemoteLogic, igtlConnectorNode): 
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="StopVolumeReconstruction" ReferencedCommandId="' + str(self.commandID) + '"></Command>', igtlConnectorNode.GetID())   
    return self.commandId   
    
  def getVolumeReconstructionSnapshot(self, igtlRemoteLogic, igtlConnectorNode): 
    self.commandId=igtlRemoteLogic.SendCommand('<Command Name="GetVolumeReconstructionSnapshot"></Command>', igtlConnectorNode.GetID())           
    return self.commandId   

  def listenToVolumesAdded(self):
    vl = slicer.modules.volumes
    vl = vl.logic()  
    self.sceneObserver = vl.AddObserver('ModifiedEvent', self.onVolumeAdded)
    
  def isAnUltrasoundVolumeGeneratedWithPLUS(self, volumeNode):
    name = volumeNode.GetName()
    nameArray = name.split("_")
    returnValue = False
    if nameArray.count("Reference") > 0:
      returnValue = True
    return returnValue  
    
  def onVolumeAdded(self, volumeNode):     
    
    if self.isAnUltrasoundVolumeGeneratedWithPLUS(volumeNode) == True:
      print("A volume generated with PLUS was added!!")
      #volumeNode.AddObserver("ModifiedEvent", self.onVolumeModified)      
    
      '''
      It is assumed that the volume was created with respect to the Reference
      The matrix associated with the volume must have the following structure
      sx 0  0  ox
      0  sy 0  oy
      0  0  sz oz
      with sx, sy, and sz >0
      This is checked and if it is not true is modified.
      By default Slicer add a volume with a ijkToRas matrix of the form:
      -1    0    0
       0    -1    0
       0     0    1
      In this case we want a ijkToRAS matrix equal to identity because we want to place the
      volume with respect to the Reference.
      ReferenceToRAS matrix is calculated during registration
      '''
    
      matrix = vtk.vtkMatrix4x4()
      volumeNode.GetIJKToRASMatrix(matrix) 
      #print matrix
      sx = matrix.GetElement(0, 0)
      if (sx < 0):
        ox = matrix.GetElement(0, 3)
        matrix.SetElement(0, 0, -sx)
        matrix.SetElement(0, 3, -ox)
      sy = matrix.GetElement(1, 1)
      if (sy < 0):
        oy = matrix.GetElement(1, 3)
        matrix.SetElement(1, 1, -sy)
        matrix.SetElement(1, 3, -oy)    
      volumeNode.SetIJKToRASMatrix(matrix)
      #print matrix
    
      # Volumes are placed under the Reference coordinate system
      referenceToRASNode = slicer.util.getNode("ReferenceToRAS")
      if referenceToRASNode == None:
        referenceToRASNode = slicer.vtkMRMLLinearTransformNode()
        slicer.mrmlScene.AddNode(referenceToRASNode)
        referenceToRASNode.SetName("ReferenceToRAS")
    
      volumeNode.SetAndObserveTransformNodeID(referenceToRASNode.GetID()) 
      volumeNode.SetDisplayVisibility(True)
      
      colorTransfer= self.createColorTransfer()
      volumePropertyNode=self.createVolumePropertyNode(colorTransfer)
      self.createVolumeRendering(volumeNode, volumePropertyNode)
    
  
  def createColorTransfer(self):
    # the color function is configured
    # zero is associated to the scalar zero and 1 to the scalar 255
    colorTransfer = vtk.vtkColorTransferFunction()
    black = [0., 0., 0.]
    white = [1., 1., 1.]
    colorTransfer.AddRGBPoint(self.scalarRange[0], black[0], black[1], black[2])
    colorTransfer.AddRGBPoint(self.windowLevelMinMax[0], black[0], black[1], black[2])
    colorTransfer.AddRGBPoint(self.windowLevelMinMax[1], white[0], white[1], white[2]);
    colorTransfer.AddRGBPoint(self.scalarRange[1], white[0], white[1], white[2]);
    return colorTransfer
  
  def createVolumePropertyNode(self, colorTransfer):
    volumePropertyNode = slicer.vtkMRMLVolumePropertyNode()
    slicer.mrmlScene.RegisterNodeClass(volumePropertyNode);

    # the scalar opacity mapping function is configured
    # it is a ramp with opacity of 0 equal to zero and opacity of 1 equal to 1. 
    scalarOpacity = vtk.vtkPiecewiseFunction()
    scalarOpacity.AddPoint(self.scalarRange[0], 0.)
    scalarOpacity.AddPoint(self.windowLevelMinMax[0], 0.)
    scalarOpacity.AddPoint(self.windowLevelMinMax[1], 1.)
    scalarOpacity.AddPoint(self.scalarRange[1], 1.)
    volumePropertyNode.SetScalarOpacity(scalarOpacity);
    
    vtkVolumeProperty = volumePropertyNode.GetVolumeProperty()
    
    vtkVolumeProperty.SetInterpolationTypeToNearest();
    vtkVolumeProperty.ShadeOn();
    vtkVolumeProperty.SetAmbient(0.30);
    vtkVolumeProperty.SetDiffuse(0.60);
    vtkVolumeProperty.SetSpecular(0.50);
    vtkVolumeProperty.SetSpecularPower(40);
    
    volumePropertyNode.SetColor(colorTransfer)
   
    slicer.mrmlScene.AddNode(volumePropertyNode)
    
    return volumePropertyNode
  
  
  def createVolumeRendering(self, volumeNode, volumePropertyNode):    
    # The volume rendering display node is created
    vr = slicer.modules.volumerendering
    vrLogic = vr.logic()
    defaultRenderingMethod = vrLogic.GetDefaultRenderingMethod()
    print defaultRenderingMethod
    if defaultRenderingMethod == 'vtkMRMLGPURayCastVolumeRenderingDisplayNode':
      self.vrDisplayNode = slicer.vtkMRMLGPURayCastVolumeRenderingDisplayNode()  
      self.vrDisplayNode.SetName(volumeNode.GetName()+"Rendering")
    elif defaultRenderingMethod == 'vtkMRMLCPURayCastVolumeRenderingDisplayNode':
      self.vrDisplayNode = slicer.vtkMRMLCPURayCastVolumeRenderingDisplayNode()
      self.vrDisplayNode.SetName(volumeNode.GetName()+"Rendering")  
      
    self.vrDisplayNode.SetAndObserveVolumeNodeID(volumeNode.GetID())
    self.vrDisplayNode.SetAndObserveVolumePropertyNodeID(volumePropertyNode.GetID())
    slicer.mrmlScene.AddNode(self.vrDisplayNode)
    self.vrDisplayNode.SetVisibility(False)
    #self.vrDisplayNode.AddObserver("ModifiedEvent", self.onVolumeRenderingModified)  
    
    #vrLogic.Modified()
    #self.vrDisplayNode.Modified()
    self.vrDisplayNode.UpdateScene(slicer.mrmlScene)                 
                        
                                  
  def showVolumeRendering(self, isShown):
    self.vrDisplayNode.SetVisibility(isShown)  
                                           
  def onVolumeModified(self, caller, event):
    print "Volume Modified"
  
  def onVolumeRenderingModified(self, caller, event):
    print "Volume Rendering Modified"      
  






#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------









#
# ScoliosisSlicelet
#

class ScoliosisSliceletTestWidget:
  def __init__(self, parent=None):
    self.chartOptions = ("Count", "Volume mm^3", "Volume cc", "Min", "Max", "Mean", "StdDev")
    if not parent:
      print("There is no parent!")
    else:
      self.parent = parent
      print("There is parent!")



#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------


      

class Slicelet(object):
  """A slicer slicelet is a module widget that comes up in stand alone mode
  implemented as a python class.
  This class provides common wrapper functionality used by all slicer modlets.
  """
  # TODO: put this in a SliceletLib 
  # TODO: parse command line arge


  def __init__(self, widgetClass=None):
      
    self.parent = qt.QFrame()
    self.parent.setLayout(qt.QHBoxLayout())
    
    self.moduleLogic = ScoliosisMonitoringLogic()
    self.moduleLogic.createTransverseProcessesList()
    self.moduleLogic.createRulerList()
    self.moduleLogic.createKeyPointsList()
    
    path = slicer.modules.scoliosismonitoring.path
    self.modulePath = os.path.dirname(path)
    self.moduleName = slicer.modules.scoliosismonitoring.name
    
    self.transverseProcessListIsBeingUpdated = False
    
    self.numberOfKeyPointsAcquired = 0
    self.numberOfLeftSequences=0
    self.numberOfRightSequences=0
    self.leftSideAcquisitionFilename="leftSideAcquisition"+str(self.numberOfLeftSequences)+".mha"
    self.rightSideAcquisitionFilename="rightSideAcquisition"+str(self.numberOfRightSequences)+".mha"
    self.leftSideAcquisitionStarted = False
    self.rightSideAcquisitionStarted = False
    
    self.currentLeftSequenceName = 'None'
    self.currentRightSequenceName = 'None'
    
    self.igtlRemoteLogic = None
    self.reconstructionSuspended = False
    self.reconstructionStarted = False
    self.exploreVolume = False
    self.reconstructedVolumePrefix="recvol"
    #self.outputVolFilename="recvol_Reference.mha"
    self.numberOfGeneratedVolumes=0;
    self.volumeReferenceFrame="Reference"
    self.outputVolFilename=self.reconstructedVolumePrefix+str(self.numberOfGeneratedVolumes)+"_"+self.volumeReferenceFrame+".mha"
    #self.outputVolDeviceName= "recvol_Reference"
    self.outputVolDeviceName= self.reconstructedVolumePrefix+str(self.numberOfGeneratedVolumes)+"_"+self.volumeReferenceFrame
    self.liveReconstruction=True
    self.volumesAddedToTheScene=[] #contains the unique ID of the volumes added to the scene 
    self.scalarRange=[0.,255.]
    self.windowLevelMinMax=[0.1,254.99]
    self.preAcquireVolumeReconstructionSequence=True
    self.preAcquisitionFilename="acquiredFramesForVolumeReconstruction"+str(self.numberOfGeneratedVolumes)+".mha"
    
    self.currentRow = 0
    self.currentColumn = 0
    
    #self.pbarwin = AddProgresWin()
    
    
    igtlRemote=slicer.modules.openigtlinkremote
    self.igtlRemoteLogic=igtlRemote.logic() 
    self.igtlRemoteLogic.SetMRMLScene(slicer.mrmlScene)
    self.igtlConnectorNode = self.moduleLogic.getConnectorNode() 
        
    self.leftFrame = qt.QFrame(self.parent)
    self.leftFrame.setLayout(qt.QVBoxLayout())
    
    self.parent.layout().addWidget(self.leftFrame, 1)
    
    self.sceneManagementFrame = qt.QFrame()
    self.sceneManagementFrame.setLayout( qt.QHBoxLayout() )
    
    self.saveSceneButton = qt.QPushButton("Save Data")
    self.saveSceneButton.connect("clicked()", self.onSaveSceneButtonClicked)
    #self.saveSceneButton.connect("clicked()", slicer.app.ioManager().openSaveDataDialog)
    self.loadSceneButton = qt.QPushButton("Load Scene")
    # self.leftFrame.layout().addWidget(self.loadSceneButton)
    # self.loadSceneButton.connect("clicked()",slicer.app.ioManager().openLoadSceneDialog)
    self.loadSceneButton.connect("clicked()",self.onLoadSceneButtonClicked)
    
    
    self.layoutSelectorFrame2 = qt.QFrame(self.parent)
    self.layoutSelectorFrame2.setLayout(qt.QHBoxLayout())
    

    self.layoutSelectorLabel2 = qt.QLabel("Layout Selector: ", self.layoutSelectorFrame2)
    self.layoutSelectorLabel2.setToolTip("Select the layout ...")
    self.layoutSelectorFrame2.layout().addWidget(self.layoutSelectorLabel2)

    self.layoutSelector2 = qt.QComboBox(self.layoutSelectorFrame2)
    self.layoutSelector2.addItem("FourViews")
    self.layoutSelector2.addItem("3D View")
    self.layoutSelector2.addItem("One view")
    self.layoutSelector2.addItem("Double 3D View")
    self.layoutSelectorFrame2.layout().addWidget(self.layoutSelector2)
    self.layoutSelector2.connect('activated(int)', self.onLayoutSelect)
    
    self.sceneManagementFrame.layout().addWidget(self.layoutSelectorFrame2)
    self.sceneManagementFrame.layout().addWidget(self.loadSceneButton)  
    self.sceneManagementFrame.layout().addWidget(self.saveSceneButton)    
    
    # Connect to Tracker widget
    
    # Status of the connection
    self.connectorCreated=False
    self.estado = "Disconnected"
    self.ConnectedState=False
    self.DisconnectedState=False
    
    self.statusFrame = qt.QFrame()
    self.statusFrame.setLayout( qt.QHBoxLayout() )
    
    self.statusLabel = qt.QLabel("Status: ")
    self.statusLabel.setToolTip( "Status of the connection ...")
    
    self.statusBar = qt.QStatusBar()
    self.statusBar.showMessage("Disconnected")

    # Button to connect
    self.PlusServerConnectionButton = qt.QPushButton("Connect to Tracker")
    # Add to the widget
    self.statusFrame.layout().addWidget(self.PlusServerConnectionButton)
    self.statusFrame.layout().addWidget(self.statusLabel)
    self.statusFrame.layout().addWidget(self.statusBar)
    self.toolsViewer = ToolsViewer()
    self.toolsViewer.setModuleLogic(self.moduleLogic)
    #self.statusFrame.layout().addWidget(self.toolsViewer.getToolsWidget()) 
    
    # Key Points acquisition frame
    self.keyPointsAcquisitionFrame = qt.QFrame()
    self.keyPointsAcquisitionFrame.setLayout(qt.QHBoxLayout()) 
    self.keyPointAcquisitionButton = qt.QPushButton("Add key point") 
    self.keyPointAcquisitionButton.connect('clicked(bool)', self.onKeyPointAcquisitionButtonClicked)
    self.keyPointsAcquisitionFrame.layout().addWidget(self.keyPointAcquisitionButton,1)
    
    # Sequences Acquisition frame
    self.sequencesAcquisitionFrame=qt.QFrame()
    self.sequencesAcquisitionFrame.setLayout(qt.QVBoxLayout())
    
    self.leftSequencesAcquisitionFrame=qt.QFrame()
    self.leftSequencesAcquisitionFrame.setLayout(qt.QHBoxLayout())  
    
    self.rightSequencesAcquisitionFrame=qt.QFrame()
    self.rightSequencesAcquisitionFrame.setLayout(qt.QHBoxLayout()) 
    
    self.leftSequenceAcquisitionLabel = qt.QLabel("Left side: ")
    self.leftSequenceAcquisitionLabel.setToolTip( "Left side sequence acquisition.")
    self.leftSequenceAcquisitionComboBox = qt.QComboBox()  
    self.leftSequenceAcquisitionComboBox.connect('currentIndexChanged(QString)',self.onLeftSequenceChanged) 
    
    self.leftSequenceAcquisitionButton = qt.QPushButton("Start acquisition")
    self.leftSequenceAcquisitionButton.connect('clicked(bool)', self.onStartLeftSideAcquisitionButtonClicked)
    
    self.rightSequenceAcquisitionLabel = qt.QLabel("Right side: ")
    self.rightSequenceAcquisitionLabel.setToolTip( "Right side sequence acquisition.")
    self.rightSequenceAcquisitionComboBox = qt.QComboBox() 
    self.rightSequenceAcquisitionComboBox.connect('currentIndexChanged(QString)',self.onRightSequenceChanged) 
    
    self.rightSequenceAcquisitionButton = qt.QPushButton("Start acquisition") 
    self.rightSequenceAcquisitionButton.connect('clicked(bool)', self.onStartRightSideAcquisitionButtonClicked)
    
    self.leftSequencesAcquisitionFrame.layout().addWidget(self.leftSequenceAcquisitionLabel,1)
    self.leftSequencesAcquisitionFrame.layout().addWidget(self.leftSequenceAcquisitionButton,1)
    self.rightSequencesAcquisitionFrame.layout().addWidget(self.rightSequenceAcquisitionLabel,1)
    self.rightSequencesAcquisitionFrame.layout().addWidget(self.rightSequenceAcquisitionButton,1)
    
    # Sequences display  frame
    #self.sequencesDisplayFrame=qt.QFrame()
    #self.sequencesDisplayFrame.setLayout(qt.QHBoxLayout())
    self.leftSequenceExplorationLabel = qt.QLabel("Left side exploration: ")
    self.leftSequenceExplorationLabel.setToolTip( "Left side sequence exploration.")
    self.rightSequenceExplorationLabel = qt.QLabel("Right side exploration: ")
    self.rightSequenceExplorationLabel.setToolTip( "Right side sequence exploration.")
    
    self.leftSequenceSlider = qt.QSlider()
    self.leftSequenceSlider.setOrientation(1) #horizontal
    self.leftSequenceSlider.setEnabled(0)
    self.leftSequenceSlider.connect('valueChanged(int)', self.onLeftSequenceSliderMoved)
    
    self.rightSequenceSlider = qt.QSlider()
    self.rightSequenceSlider.setOrientation(1) #horizontal
    self.rightSequenceSlider.setEnabled(0)
    self.rightSequenceSlider.connect('valueChanged(int)', self.onRightSequenceSliderMoved)
    
    
    #self.leftSequencesAcquisitionFrame.layout().addWidget(self.leftSequenceExplorationLabel)
    self.leftSequencesAcquisitionFrame.layout().addWidget(self.leftSequenceAcquisitionComboBox,2)
    self.leftSequencesAcquisitionFrame.layout().addWidget(self.leftSequenceSlider,2)
    #self.rightSequencesAcquisitionFrame.layout().addWidget(self.rightSequenceExplorationLabel)
    self.rightSequencesAcquisitionFrame.layout().addWidget(self.rightSequenceAcquisitionComboBox,2)
    self.rightSequencesAcquisitionFrame.layout().addWidget(self.rightSequenceSlider,2)
    
    self.sequencesAcquisitionFrame.layout().addWidget(self.leftSequencesAcquisitionFrame)
    self.sequencesAcquisitionFrame.layout().addWidget(self.rightSequencesAcquisitionFrame)
    # Volume reconstruction
    
    volumeReconstructionFrame=qt.QFrame()
    volumeReconstructionFrame.setLayout(qt.QVBoxLayout())
    
    self.volumeReconstructionLabel = qt.QLabel("Volume reconstruction: ")
    self.volumeReconstructionLabel.setToolTip( "Perform volume reconstruction")
    volumeReconstructionFrame.layout().addWidget(self.volumeReconstructionLabel)
    
    
    self.preAcquisitionFrame=qt.QFrame()
    self.preAcquisitionFrame.setLayout(qt.QHBoxLayout())
    self.preAcquisitionLayout=self.preAcquisitionFrame.layout()
    
    self.isPreAcquisitionLabel = qt.QLabel("Pre Acquisition: ", volumeReconstructionFrame)
    self.isPreAcquisitionLabel.setToolTip( "Enable/Disable the pre acquisition. It is important to pre adquire a sequence to determine the extent of the volume")
    self.preAcquisitionLayout.addWidget(self.isPreAcquisitionLabel)

    self.isPreAcquisitionCheckBox = qt.QCheckBox()
    self.isPreAcquisitionCheckBox.setCheckState(0)
    self.preAcquisitionLayout.addWidget(self.isPreAcquisitionCheckBox)
    self.isPreAcquisitionCheckBox.connect("stateChanged(int)",self.onPreAcquisitionStateChanged)
    
    self.volumeReconstructionButtonsFrame=qt.QFrame()
    self.volumeReconstructionButtonsFrame.setLayout(qt.QHBoxLayout())
    self.volumeReconstructionButtonsLayout=self.volumeReconstructionButtonsFrame.layout()
    
     
    self.startReconstructionButton = qt.QPushButton("Start")
    self.startReconstructionButton.toolTip = "Start/Stop the volume reconstruction"
    self.volumeReconstructionButtonsLayout.addWidget(self.startReconstructionButton)
    self.startReconstructionButton.connect('clicked(bool)', self.onStartReconstructionButtonClicked)
    
    self.suspendReconstructionButton = qt.QPushButton("Suspend")
    self.suspendReconstructionButton.toolTip = "Suspend/Resume the volume reconstruction"
    self.suspendReconstructionButton.setEnabled(False)
    self.volumeReconstructionButtonsLayout.addWidget(self.suspendReconstructionButton)
    self.suspendReconstructionButton.connect('clicked(bool)', self.onSuspendReconstructionButtonClicked)
    
    #volumeReconstructionFrame.layout().addWidget(self.preAcquisitionFrame)
    volumeReconstructionFrame.layout().addWidget(self.volumeReconstructionButtonsFrame)
    
    #self.volumesViewer = VolumesViewer()
    #self.volumesViewer.setModuleLogic(self.moduleLogic)
    #self.volumesViewer.listenToScene()
    #self.volumeRenderingViewer = VolumeRenderingViewer()
    #self.volumeRenderingViewer.setModuleLogic(self.moduleLogic)
    #self.volumeRenderingViewer.listenToScene()
    
    # Connections
    self.PlusServerConnectionButton.connect("clicked()",self.onPlusServerConnectionButtonClicked)
    
    if not self.connectorCreated:
      self.moduleLogic.createAndAssociateConectorNodeWithScene()
      self.connectorNode = self.moduleLogic.getConnectorNode()
      self.connectorNode.AddObserver(self.connectorNode.ConnectedEvent, self.onConnectedEventCaptured)
      self.connectorNode.AddObserver(self.connectorNode.DisconnectedEvent, self.onDisconnectedEventCaptured)
      self.connectorCreated = True    
    
    status = self.connectorNode.GetState()
    # print("Status: " + str(status))
    if status == 2:
      self.estado = "Connected"
      self.PlusServerConnectionButton.setText("Disconnect")
    elif status == 1:
      self.estado = "Waiting"
    elif status == 0:
      self.estado == "Disconnected"
      self.PlusServerConnectionButton.setText("Connect")
    
    self.statusBar.showMessage(self.estado)#print(self.estado)

    
    self.layoutWidget = slicer.qMRMLLayoutWidget()
    self.layoutWidget.setMRMLScene(slicer.mrmlScene)
    self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    
    self.parent.layout().addWidget(self.layoutWidget, 2)
    
    
    # Spine Table Widget
    self.verterbraeNames = []    
#     for cervicalVertebrae in range(1, self.moduleLogic.CONST_NUMBER_OF_CERVICAL_VERTERBRAE + 1):
#       self.verterbraeNames.append("C"+str(cervicalVertebrae))
      
    for thoracicVertebrae in range(1, self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE + 1):
      self.verterbraeNames.append("T"+str(thoracicVertebrae))
      
    for lumbarVertebrae in range(1, self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE + 1):
      self.verterbraeNames.append("L"+str(lumbarVertebrae))
  
    self.superiorAnglesList = [None]*self.moduleLogic.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE
    self.inferiorAnglesList = [None]*self.moduleLogic.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE
    
    horizontalHeaders = ["Vertebrae", "LTP Image", "LTP Point" , "RTP Image", "RTP Point"]

    self.spineTable = qt.QTableWidget(self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE + self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE, 5)
    #self.spineTable.sortingEnabled = False
    #self.spineTable.setEditTriggers(0)
    #self.spineTable.setMinimumHeight(self.spineTable.verticalHeader().length() + 25)
    #self.spineTable.horizontalHeader().setResizeMode(qt.QHeaderView.Stretch)
    #self.spineTable.setSizePolicy (qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.Preferred)
    
    verticalScroll =self.spineTable.verticalScrollBar()
    verticalScroll.setDisabled(False)
    
    self.spineTable.setHorizontalHeaderLabels(horizontalHeaders)
    
    self.spineTable.itemClicked.connect(self.onSpineTableClicked)
    

    self.labelReferences = []
    self.LTPImageItemReferences = []
    self.LTPPointItemReferences = []
    self.RTPImageItemReferences = []
    self.RTPPointReferences = []
    for verterbraeIndex, verterbraeName in enumerate(self.verterbraeNames):
      vertebraeLabel = qt.QTableWidgetItem(verterbraeName)
      self.labelReferences.append(vertebraeLabel)
      self.spineTable.setItem(verterbraeIndex, 0, vertebraeLabel)
      
      LTPImageItem = qt.QTableWidgetItem("")
      self.LTPImageItemReferences.append(LTPImageItem)
      self.spineTable.setItem(verterbraeIndex, 1, LTPImageItem) 
      
      LTPPointItem = qt.QTableWidgetItem("")
      self.LTPPointItemReferences.append(LTPPointItem)
      self.spineTable.setItem(verterbraeIndex, 2, LTPPointItem)   

      RTPImageItem = qt.QTableWidgetItem("")
      self.RTPImageItemReferences.append(RTPImageItem)
      self.spineTable.setItem(verterbraeIndex, 3, RTPImageItem)   
      
      RTPPointItem = qt.QTableWidgetItem("")
      self.RTPPointReferences.append(RTPPointItem)
      self.spineTable.setItem(verterbraeIndex, 4, RTPPointItem)   
    
    # Add the transverse process
    self.addTransverseProcessButton = qt.QPushButton("Add the transverse process position")
    self.addTransverseProcessButton.toolTip = "Select the transverse process position in the image"
    self.addTransverseProcessButton.connect('clicked(bool)', self.onAddTransverseProcessButtonClicked)
    
    #self.acquisitionFrame.layout().addWidget(self.getSnapshotButton)
    #self.acquisitionFrame.layout().addWidget(self.addTransverseProcessButton)
    
    # Compute the TP angle
    self.angleComputationFrame=qt.QFrame()
    self.angleComputationFrame.setLayout(qt.QHBoxLayout())
    
    self.upperVertebraeLabel = qt.QLabel("Upper Vertebrae: ")
    self.upperVertebraeLabel.setToolTip( "Choose the name of the first vertebrae ...")
    self.upperVertebraeComboBox = qt.QComboBox()  
    
    self.downVertebraeLabel = qt.QLabel("Down Vertebrae: ")
    self.downVertebraeLabel.setToolTip( "Choose the name of the second vertebrae ...")
    self.downVertebraeComboBox = qt.QComboBox()
    
    #self.projectionPlaneComboBox = qt.QComboBox()  
    #self.projectionPlaneComboBox.addItems(["Coronal", "Axial", "Sagittal"])
    
    self.computeTransverProcessesAngleButton = qt.QPushButton("Compute TP angle")
    self.computeTransverProcessesAngleButton.toolTip = "Compute the transverse processes angle between two vertebraes"
    self.computeTransverProcessesAngleButton.connect('clicked(bool)', self.onComputeTransverseProcessAngleButtonClicked)
    
    self.computationResultLabel = qt.QLabel()
    
    self.angleComputationFrame.layout().addWidget(self.upperVertebraeLabel)
    self.angleComputationFrame.layout().addWidget(self.upperVertebraeComboBox)
    self.angleComputationFrame.layout().addWidget(self.downVertebraeLabel)
    self.angleComputationFrame.layout().addWidget(self.downVertebraeComboBox)
    #self.angleComputationFrame.layout().addWidget(self.projectionPlaneComboBox)
    self.angleComputationFrame.layout().addWidget(self.computeTransverProcessesAngleButton)
    self.angleComputationFrame.layout().addWidget(self.computationResultLabel)

    
    self.leftFrame.layout().addWidget(self.sceneManagementFrame, 0.5)
    self.leftFrame.layout().addWidget(self.statusFrame, 0.5)
    self.leftFrame.layout().addWidget(self.toolsViewer.getToolsWidget(), 0.5)
    self.leftFrame.layout().addWidget(self.keyPointsAcquisitionFrame, 0.5) 
    self.leftFrame.layout().addWidget(self.sequencesAcquisitionFrame, 0.5)
    #self.leftFrame.layout().addWidget(self.sequencesDisplayFrame, 0.5)
    #self.leftFrame.layout().addWidget(volumeReconstructionFrame, 0.5)
    #self.leftFrame.layout().addWidget(self.volumesViewer.getVolumesViewerWidget(),1)
    #self.leftFrame.layout().addWidget(self.volumeRenderingViewer.getVolumeRenderingViewerWidget(),1)
    #self.leftFrame.layout().addWidget(self.acquisitionFrame, 0.5)
    self.leftFrame.layout().addWidget(self.angleComputationFrame, 0.5)
    self.leftFrame.layout().addWidget(self.spineTable, 5)
    #self.spineTable.show()
    
    
    pixmap=qt.QPixmap("Resources/Icons/AnnotationPointWithArrow.png")
    self.placeFiducialCursor= qt.QCursor(pixmap)
    
    
    if widgetClass:
      self.widget = widgetClass(self.parent)
    self.parent.show()
    
    # self.moduleLogic = slicer.modules.USGuidedProcedure.logic()
    
    
 #---------------------------------------------------------------------------------------------------------------
  
  def onLoadSceneButtonClicked(self):
    from os import listdir
    print "Loading the scene"    
    sceneDirectory = qt.QFileDialog.getExistingDirectory(self.parent,"Select directory")  
    list=listdir(sceneDirectory)
    leftSequences = [];
    rightSequences = [];
    
    # logic used to load the .mha files
    metaFileImporter=slicer.modules.metafileimporter
    logic=metaFileImporter.logic() 
    tr=slicer.modules.transforms
    trLogic=tr.logic()
    
    # logic used to load the models
    modelsModule = slicer.modules.models
    modelsModuleLogic = modelsModule.logic()
    modelsModuleLogic.SetMRMLScene(slicer.mrmlScene)
    
    bar=qt.QProgressBar()
    bar.setMinimum(0)
    bar.setMaximum(0)
    bar.show()
        
    for f in list:
      if ((f.find("left")==0) and (f.find(".mha")!=-1)):
        leftSequences.append(f)  
        self.leftSideAcquisitionFilename = f
        self.acquisitionfilename = os.path.join(sceneDirectory, f)
        logic.Read(self.acquisitionfilename)
        sequenceName=self.leftSideAcquisitionFilename.split(".")
        sequenceName=sequenceName[0]
        self.configureSequenceInTheScene(sequenceName)
      if ((f.find("right")==0) and (f.find(".mha")!=-1)):
        self.rightSideAcquisitionFilename = f
        rightSequences.append(f) 
        self.acquisitionfilename = os.path.join(sceneDirectory, f)
        logic.Read(self.acquisitionfilename)
        sequenceName=self.rightSideAcquisitionFilename.split(".")
        sequenceName=sequenceName[0]
        self.configureSequenceInTheScene(sequenceName) 
      if (f.find("ImageToProbe")==0):
        transformFilename = os.path.join(sceneDirectory, f)
        trLogic.AddTransform(transformFilename, slicer.mrmlScene)
      if (f.find("ReferenceToRAS")==0):
        transformFilename = os.path.join(sceneDirectory, f)
        trLogic.AddTransform(transformFilename, slicer.mrmlScene)  
      if (f.find("TrackerToWall")==0):
        transformFilename = os.path.join(sceneDirectory, f)
        trLogic.AddTransform(transformFilename, slicer.mrmlScene)
        self.moduleLogic.setTrackerToWallTransformationMatrix()  
      if ((f.find("F")==0) and (f.find(".acsv")!=-1)):  
        self.moduleLogic.setActiveAnnotationsList("Key Points List")
        keyPointFilename = os.path.join(sceneDirectory, f) 
        fiducialNode = slicer.vtkMRMLAnnotationFiducialNode()
        slicer.util.loadAnnotationFiducial(keyPointFilename, fiducialNode)   
        
    
    numberOfVertebraes = self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE + self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE 
    for index in range(0,numberOfVertebraes):
      leftFiducialName= "Left" + self.verterbraeNames[index] + "Fiducial"  
      rightFiducialName= "Right" + self.verterbraeNames[index] + "Fiducial"            

      
      leftFiducialFilename = os.path.join(sceneDirectory, leftFiducialName + ".acsv")
      if os.path.isfile(leftFiducialFilename):
        self.moduleLogic.setActiveAnnotationsList("Transverse Processes List")   
        fiducialNode = slicer.vtkMRMLAnnotationFiducialNode()
        slicer.util.loadAnnotationFiducial(leftFiducialFilename, fiducialNode)
        currentItem = self.spineTable.item(index , 2)
        self.spineTable.setCurrentItem(currentItem)
        currentItem.setText("Visible") 
        currentItem.setTextAlignment(4) 
        currentItem.setCheckState(2)
        currentItem.setBackground(qt.QColor(0,255,0))
        self.updateRulerList()

      rightFiducialFilename = os.path.join(sceneDirectory, rightFiducialName + ".acsv")
      if os.path.isfile(rightFiducialFilename):
        self.moduleLogic.setActiveAnnotationsList("Transverse Processes List")    
        fiducialNode = slicer.vtkMRMLAnnotationFiducialNode()
        slicer.util.loadAnnotationFiducial(rightFiducialFilename, fiducialNode)
        currentItem = self.spineTable.item(index , 4)
        self.spineTable.setCurrentItem(currentItem)
        currentItem.setText("Visible") 
        currentItem.setTextAlignment(4) 
        currentItem.setCheckState(2)
        currentItem.setBackground(qt.QColor(0,255,0))
        self.updateRulerList()   
                  
      leftSnapshotName= "Left" + self.verterbraeNames[index] + "Snapshot"  
      leftSnapshotFilename = os.path.join(sceneDirectory, leftSnapshotName + ".vtk")
      if os.path.isfile(leftSnapshotFilename):
        textureFileName=os.path.join(sceneDirectory, leftSnapshotName + "_Texture.nrrd") 
        if os.path.isfile(textureFileName):
          slicer.util.loadVolume(textureFileName)  
          modelsModuleLogic.AddModel(leftSnapshotFilename) 
          snapshotModel=slicer.util.getNode(leftSnapshotName)
          snapshotTexture = slicer.util.getNode(leftSnapshotName + "_Texture")    
          snapshotModelDisplayNode= snapshotModel.GetModelDisplayNode()
          snapshotModelDisplayNode.SetAndObserveTextureImageData( snapshotTexture.GetImageData() )
          snapshotModelDisplayNode.SetBackfaceCulling(False)
          currentItem = self.spineTable.item(index , 1)
          self.spineTable.setCurrentItem(currentItem)
          currentItem.setText("Visible") 
          currentItem.setTextAlignment(4) 
          currentItem.setCheckState(2)
          currentItem.setBackground(qt.QColor(0,255,0))
        leftReferenceToTrackerTransformName= "Left" + self.verterbraeNames[index] + "ReferenceToTrackerTransform.tfm" 
        transformFilename = os.path.join(sceneDirectory, leftReferenceToTrackerTransformName)
        if os.path.isfile(transformFilename):
          trLogic.AddTransform(transformFilename, slicer.mrmlScene)   
      
      rightSnapshotName= "Right" + self.verterbraeNames[index] + "Snapshot"
      rightSnapshotFilename = os.path.join(sceneDirectory, rightSnapshotName + ".vtk")
      if os.path.isfile(rightSnapshotFilename):
        textureFileName=os.path.join(sceneDirectory, rightSnapshotName + "_Texture.nrrd") 
        if os.path.isfile(textureFileName):
          slicer.util.loadVolume(textureFileName)   
          modelsModuleLogic.AddModel(rightSnapshotFilename)
          snapshotModel=slicer.util.getNode(rightSnapshotName)
          snapshotTexture = slicer.util.getNode(rightSnapshotName + "_Texture")
          snapshotModelDisplayNode= snapshotModel.GetModelDisplayNode()
          snapshotModelDisplayNode.SetAndObserveTextureImageData( snapshotTexture.GetImageData() )
          snapshotModelDisplayNode.SetBackfaceCulling(False)
          currentItem = self.spineTable.item(index , 3)
          self.spineTable.setCurrentItem(currentItem)
          currentItem.setText("Visible") 
          currentItem.setTextAlignment(4) 
          currentItem.setCheckState(2)
          currentItem.setBackground(qt.QColor(0,255,0))
        rightReferenceToTrackerTransformName= "Right" + self.verterbraeNames[index] + "ReferenceToTrackerTransform.tfm"
        transformFilename = os.path.join(sceneDirectory, rightReferenceToTrackerTransformName)
        if os.path.isfile(transformFilename):
          trLogic.AddTransform(transformFilename, slicer.mrmlScene)           
    self.listenToFiducialsModification()  
    
    bar.hide()
    
    
#---------------------------------------------------------------------------------------------------------------
   
          
  def saveNode(self, outputDirectory, node, fileExtension):    
    snode = node.GetStorageNode()
    if (snode == None):
      snode = node.CreateDefaultStorageNode()
      # add storage node to scene
      slicer.mrmlScene.AddNode(snode)
      # set storage node to node
      node.SetAndObserveStorageNodeID(snode.GetID())
    # save node
    dstFile = os.path.join(outputDirectory, node.GetName()+ fileExtension)
    slicer.util.saveNode(node,dstFile)       
     
    
 #---------------------------------------------------------------------------------------------------------------

    
  def onSaveSceneButtonClicked(self):
    import shutil  
    outputDirectory = qt.QFileDialog.getExistingDirectory(self.parent,"Select directory")  
    print outputDirectory
    bar=qt.QProgressBar()
    bar.setMinimum(0)
    bar.setMaximum(0)
    bar.show()  
    numberOfLeftSequences=self.leftSequenceAcquisitionComboBox.count
    numberOfRightSequences=self.rightSequenceAcquisitionComboBox.count
    for index in range(0,numberOfLeftSequences):
      leftSideAcquisitionFilename = self.leftSequenceAcquisitionComboBox.itemText(index)+".mha" 
      srcFile = os.path.join(self.modulePath, leftSideAcquisitionFilename)
      dstFile = os.path.join(outputDirectory, leftSideAcquisitionFilename)
      shutil.copy(srcFile, dstFile)
    for index in range(0,numberOfRightSequences):
      rightSideAcquisitionFilename = self.rightSequenceAcquisitionComboBox.itemText(index)+".mha" 
      srcFile = os.path.join(self.modulePath, rightSideAcquisitionFilename)
      dstFile = os.path.join(outputDirectory, rightSideAcquisitionFilename)
      shutil.copy(srcFile, dstFile)  
     
    # Save the ImageToProbe transform 
    node = slicer.util.getNode("ImageToProbe")  
    if node is not None: 
      self.saveNode(outputDirectory, node,".tfm")
    
    # Save the ReferenceToRAS transform 
    node = slicer.util.getNode("ReferenceToRAS") 
    if node is not None: 
      self.saveNode(outputDirectory, node,".tfm")
    
    # Save the TrackerToWall transform 
    # First I ask for a valid transformation that is stored in a variable and then set this 
    # transformation. I do this just to be sure that the transformation that is beeing saved was
    # valid
    trackerToWallTransformationMatrix = self.moduleLogic.getTrackerToWallTransformationMatrix()
    trackerToWallTransformationNode = slicer.util.getNode("TrackerToWall")
    if trackerToWallTransformationNode is not None:
      trackerToWallTransformationNode.SetAndObserveMatrixTransformFromParent(trackerToWallTransformationMatrix)  
      self.saveNode(outputDirectory, trackerToWallTransformationNode,".tfm")
    
    keyPointsListNode = slicer.util.getNode("Key Points List")
    for childrenIndex in xrange(keyPointsListNode.GetNumberOfChildrenNodes()):
      fidHierarchyNode=keyPointsListNode.GetNthChildNode(childrenIndex)
      fidNode=fidHierarchyNode.GetAssociatedNode()
      if fidNode is not None:   
        self.saveNode(outputDirectory, fidNode,".acsv")  
    
    for vertebraeName in self.verterbraeNames:
      leftFiducialName= "Left" + vertebraeName + "Fiducial"  
      rightFiducialName= "Right" + vertebraeName + "Fiducial"  
      leftSnapshotName= "Left" + vertebraeName + "Snapshot"  
      rightSnapshotName= "Right" + vertebraeName + "Snapshot"  
      leftReferenceToTrackerTransformName= "Left" + vertebraeName + "ReferenceToTrackerTransform"  
      rightReferenceToTrackerTransformName= "Right" + vertebraeName + "ReferenceToTrackerTransform"   
      
      rulerNode = slicer.util.getNode(vertebraeName)
      if rulerNode is not None:   
        self.saveNode(outputDirectory, rulerNode,".acsv")  
      
      leftFiducialNode = slicer.util.getNode(leftFiducialName)
      if leftFiducialNode is not None:   
        self.saveNode(outputDirectory, leftFiducialNode,".acsv")  
        
      rightFiducialNode = slicer.util.getNode(rightFiducialName)
      if rightFiducialNode is not None:   
        self.saveNode(outputDirectory, rightFiducialNode,".acsv")   
       
      leftSnapshotNode = slicer.util.getNode(leftSnapshotName)
      if leftSnapshotNode is not None:   
        self.saveNode(outputDirectory, leftSnapshotNode,".vtk")    
      
      leftSnapshotTextureNode = slicer.util.getNode(leftSnapshotName + "_Texture")
      if leftSnapshotTextureNode is not None:   
        self.saveNode(outputDirectory, leftSnapshotTextureNode,".nrrd")    

          
      rightSnapshotNode = slicer.util.getNode(rightSnapshotName)
      if rightSnapshotNode is not None:   
        self.saveNode(outputDirectory, rightSnapshotNode,".vtk")   
        
      rightSnapshotTextureNode = slicer.util.getNode(rightSnapshotName + "_Texture")
      if rightSnapshotTextureNode is not None:   
        self.saveNode(outputDirectory, rightSnapshotTextureNode,".nrrd")     
        
      leftReferenceToTrackerTransformNode = slicer.util.getNode(leftReferenceToTrackerTransformName)
      if leftReferenceToTrackerTransformNode is not None:   
        self.saveNode(outputDirectory, leftReferenceToTrackerTransformNode,".tfm")   
        
      rightReferenceToTrackerTransformNode = slicer.util.getNode(rightReferenceToTrackerTransformName)
      if rightReferenceToTrackerTransformNode is not None:   
        self.saveNode(outputDirectory, rightReferenceToTrackerTransformNode,".tfm")   
    #slicer.app.ioManager().openSaveDataDialog
    bar.hide()    
    
#---------------------------------------------------------------------------------------------------------------
     
        
  def onPlusServerConnectionButtonClicked(self):
    if self.estado=="Disconnected":
       print("Trying to connect...")
       print("Status before Connect(): " + str(self.connectorNode.GetState()))
       self.moduleLogic.connectWithTracker()
       self.moduleLogic.listenToImageSentToTheScene()
       self.toolsViewer.listenToTransformationsSentToTheScene()
       self.estado = "Waiting"
       self.statusBar.showMessage(self.estado)
       print("Status After Connect(): " + str(self.connectorNode.GetState()))
       #self.toolsViewer.startListeningToTransformationsModifications()
    elif (self.estado=="Connected") or (self.estado=="Waiting"):
       self.moduleLogic.stopTracking()  
             
  
  #---------------------------------------------------------------------------------------------------------------

 
  def onConnectedEventCaptured(self, caller,  event):
    #print("Connected event captured!")
    status = self.connectorNode.GetState()
    #print("Status: " + str(status))
    if status==2:
      self.estado = "Connected"
      self.PlusServerConnectionButton.setText("Disconnect")
      self.statusBar.showMessage(self.estado)#print(self.estado)
      self.ConnectedState=True
    else:
      self.ConnectedState=False
          
    if((not self.ConnectedState) and (not self.DisconnectedState)):
      self.estado = "Waiting"
      self.statusBar.showMessage(self.estado)   
          
#---------------------------------------------------------------------------------------------------------------

     
  def onDisconnectedEventCaptured(self, caller,  event):
    #print("Disconnected event captured!")
    status = self.connectorNode.GetState()
    if status==0:
      self.estado = "Disconnected"
      self.statusBar.showMessage(self.estado)
      self.PlusServerConnectionButton.setText("Connect to Tracker")  
      self.DisconnectedState=True
    else:
      self.DisconnectedState=False
    
 #---------------------------------------------------------------------------------------------------------------
    
  def onLayoutSelect(self, argin): 
    print("Layout changed") 
    print(argin)
    if argin == 0:
       self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    elif argin == 1:  
       self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
    elif argin == 2:
       self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTabbedSliceView)
    elif argin == 3:
       self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutDual3DView)
       
#--------------------------------------------------------------------------------------------------------------- 

  def onKeyPointAcquisitionButtonClicked(self):
    print("Key point acquisition button clicked")     
    fidName = "F" + str(self.numberOfKeyPointsAcquired)
    result = self.moduleLogic.captureStylusTipPosition(fidName)  
    if result==True:
      self.numberOfKeyPointsAcquired += 1     


#--------------------------------------------------------------------------------------------------------------- 
   
  def onGetSnapshotButtonClicked(self):
    print("GetSnapshotButton was clicked...")
    currentRow = self.spineTable.currentRow()
    currentColumn = self.spineTable.currentColumn()
    side = ""
    if currentColumn == 1:
      side = "Left"
    elif currentColumn == 3:
      side = "Right"
    else:
      ret=qt.QMessageBox.warning(self.spineTable, 'Spine Table', 'You must select the cell corresponding to the LTP or RTP vertebrae image in order to to get the snapshot.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
      return
    vertebrae = self.spineTable.item(currentRow, 0).text()
    if self.moduleLogic.isValidTransformation("ReferenceToTracker"): 
      referenceToTrackerNode= slicer.util.getNode("ReferenceToTracker")   
      name = side + vertebrae + "Snapshot"
      previousNode=slicer.util.getNode(name)   
      if previousNode is None:
        self.moduleLogic.takeUSSnapshot2(name)    
      else:
        temporalName = name + "-Temporal"      
        self.moduleLogic.takeUSSnapshot2(temporalName)
        replay=qt.QMessageBox.question(self.spineTable, 'Spine Table', 'Are you sure you want to overwrite the current snapshot.', qt.QMessageBox.Ok , qt.QMessageBox.Cancel)  
        temporalNode=slicer.util.getNode(temporalName)
        temporalTexture = slicer.util.getNode(temporalName + "_Texture") 
        temporalDisplayNode =  slicer.util.getNode(temporalName + "_TextureDisplay")  
        if replay==qt.QMessageBox.Cancel:
          slicer.mrmlScene.RemoveNode(temporalNode)  
          slicer.mrmlScene.RemoveNode(temporalTexture)
          slicer.mrmlScene.RemoveNode(temporalDisplayNode) 
          return     
        elif replay==qt.QMessageBox.Ok: 
          slicer.mrmlScene.RemoveNode(previousNode)    
          previousTexture = slicer.util.getNode(name + "_Texture") 
          previousDisplayNode =  slicer.util.getNode(name + "_TextureDisplay") 
          slicer.mrmlScene.RemoveNode(previousTexture)
          slicer.mrmlScene.RemoveNode(previousDisplayNode) 
          temporalNode.SetName(name)           
          temporalTexture.SetName(name + "_Texture")  
          temporalDisplayNode.SetName(name + "_TextureDisplay")
      
      referenceToTrackerTransformName = side + vertebrae + "ReferenceToTrackerTransform"   
      previousReferenceToTrackerTransformNode = slicer.util.getNode(referenceToTrackerTransformName) 
      if previousReferenceToTrackerTransformNode is not None:
        slicer.mrmlScene.RemoveNode(previousReferenceToTrackerTransformNode)       
      transformeNode= slicer.vtkMRMLLinearTransformNode()
      transformeNode.Copy(referenceToTrackerNode)
      transformeNode.SetName(referenceToTrackerTransformName)
      slicer.mrmlScene.AddNode(transformeNode)
      currentItem = self.spineTable.item(currentRow, currentColumn)
      print currentItem
      currentItem.setText("Visible") 
      currentItem.setTextAlignment(4) 
      currentItem.setCheckState(2)
      currentItem.setBackground(qt.QColor(0,255,0))
      if self.spineTable.rowCount > currentRow + 1:
        nextItem = self.spineTable.item(currentRow + 1, currentColumn)
        self.spineTable.setCurrentItem(nextItem)
        self.spineTable.itemClicked(nextItem)
        print "move to the next row"   
      soundFile=os.path.join(self.modulePath,"Resources/Sounds/notify.wav")
      sound=qt.QSound(soundFile)
      sound.play()    
    else:
      soundFile=os.path.join(self.modulePath,"Resources/Sounds/critico.wav") 
      sound=qt.QSound(soundFile)
      sound.play()     
  
#---------------------------------------------------------------------------------------------------------------
     
  def onAddTransverseProcessButtonClicked(self):
    print("AddTransverseProcessButton was clicked...")
    currentRow = self.spineTable.currentRow()
    currentColumn = self.spineTable.currentColumn()
    side = ""
    if currentColumn == 2:
      side = "Left"
    elif currentColumn == 4:
      side = "Right"
    else:
      ret=qt.QMessageBox.warning(self.spineTable, 'Spine Table', 'You must select the cell corresponding to the LTP or RTP vertebrae point in order to to get the snapshot.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
      return
    vertebrae = self.spineTable.item(currentRow, 0).text()
    #snapshotName = side + vertebrae + "Snapshot"      
    #snapshotNode = slicer.util.getNode(snapshotName)
    #if snapshotNode is None:
    #  ret=qt.QMessageBox.warning(self.spineTable, 'Spine Table', 'You must acquire the image corresponding to the LTP or RTP vertebrae point in order to mark the point.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
    #  return
    name = side + vertebrae + "Fiducial"
    node=slicer.util.getNode(name)   
    if node is not None:
      replay=qt.QMessageBox.question(self.spineTable, 'Spine Table', 'Are you sure you want to overwrite the current point.', qt.QMessageBox.Ok , qt.QMessageBox.Cancel)  
      print replay
      if replay==qt.QMessageBox.Cancel:
        return     
      elif replay==qt.QMessageBox.Ok: 
        slicer.mrmlScene.RemoveNode(node) 
    
    self.listenToTransverseProcessesListModifications()   
    self.listenToFiducialsModification()           
    self.moduleLogic.addFiducial(name)
    self.currentFiducialName = name
    #self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpGreenSliceView) 
    
    qt.QApplication.setOverrideCursor(self.placeFiducialCursor) 
    
    
#---------------------------------------------------------------------------------------------------------------
    
  
  def onComputeTransverseProcessAngleButtonClicked(self): 
    print "ComputeTransverseProcessAngleButton was clicked"   
    upperVertebraeName = self.upperVertebraeComboBox.currentText
    downVertebraeName = self.downVertebraeComboBox.currentText
    upperVertebraeNode = slicer.util.getNode(upperVertebraeName)
    downVertebraeNode = slicer.util.getNode(downVertebraeName)
    if (upperVertebraeNode is not None and downVertebraeNode is not None):
      #planeOrientation = self.projectionPlaneComboBox.currentText
      trackerToWallTransformationMatrix = self.moduleLogic.getTrackerToWallTransformationMatrix()
      if trackerToWallTransformationMatrix is None:  
        ret=qt.QMessageBox.warning(self.spineTable, 'Transverse Precess Angle computation', 'The wall marker position was not found.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
        return  
      referenceToTrackerTransform=slicer.util.getNode("*-ReferenceToTrackerTransform [time=*")
      if referenceToTrackerTransform is None:
        # this works for live calculation. It uses the transformation that was saved with the the snapshot.
        referenceToTrackerTransform=slicer.util.getNode("*" + upperVertebraeName + "ReferenceToTrackerTransform")
      referenceToTrackeMatrixTransform=referenceToTrackerTransform.GetMatrixTransformToParent() 
        
      referenceToRASTransform=slicer.util.getNode("ReferenceToRAS")
      RASToReferenceMatrixTransform=referenceToRASTransform.GetMatrixTransformToParent()  
      RASToReferenceMatrixTransform.Invert()
    
      RASToTrackerMatrixTransform = vtk.vtkMatrix4x4()
      vtk.vtkMatrix4x4.Multiply4x4(RASToReferenceMatrixTransform,referenceToTrackeMatrixTransform,RASToTrackerMatrixTransform)
    
      RASToWallMatrixTransform = vtk.vtkMatrix4x4()
      vtk.vtkMatrix4x4.Multiply4x4(RASToTrackerMatrixTransform,trackerToWallTransformationMatrix,RASToWallMatrixTransform)
      transverseProceseAngle = self.moduleLogic.computeTransverseProcessesAngle(upperVertebraeNode, downVertebraeNode, RASToWallMatrixTransform)
      self.computationResultLabel.setText(str("%0.1f" % (transverseProceseAngle))) 
      
      
 #---------------------------------------------------------------------------------------------------------------
      
        
  def onSpineTableClicked(self, item):
    print "Spine table clicked"  
    self.spineTable.removeCellWidget(self.currentRow, self.currentColumn) 
    self.currentRow = self.spineTable.currentRow()
    self.currentColumn = self.spineTable.currentColumn() 
    side = ""
    type=""
    if self.currentColumn == 1:
      side = "Left"
      type="image"
    elif self.currentColumn == 3:
      side = "Right"
      type="image"
    elif self.currentColumn == 2:
      side = "Left"
      type="fiducial"
    elif self.currentColumn == 4:
      side = "Right"
      type="fiducial"        
    vertebrae = self.spineTable.item(self.currentRow, 0).text()
    if type=="image":
      name = side + vertebrae + "Snapshot"
      node=slicer.util.getNode(name)   
      if node is not None:
        self.modifySnapshotButton = qt.QPushButton("Modify")
        self.modifySnapshotButton.toolTip = "Modify the snapshot in which the transverse process is visible"
        self.modifySnapshotButton.connect('clicked(bool)', self.onGetSnapshotButtonClicked)
        self.spineTable.setCellWidget(self.currentRow,self.currentColumn,self.modifySnapshotButton)      
        dispNode = node.GetDisplayNode()  
        if item.checkState() == 2:
          print('"%s" Checked' % str(self.currentRow))
          dispNode.SetVisibility(True) 
        else:
          print('"%s" Clicked' % str(self.currentRow)) 
          dispNode.SetVisibility(False)    
        yellowWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeYellow")
        textureName = side + vertebrae + "Snapshot_Texture"
        imageNode=slicer.util.getNode(textureName)  
        yellowWidgetCompNode.SetBackgroundVolumeID(imageNode.GetID()) 
        yellowNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeYellow")
        vrdl=self.moduleLogic.getVolumeResliceDriverLogic()  
        vrdl.SetDriverForSlice(imageNode.GetID(), yellowNode)
        vrdl.SetModeForSlice(vrdl.MODE_TRANSVERSE180, yellowNode)   
        yellowNode.SetSliceVisible(True) 
        self.moduleLogic.adjustDisplaySettings(imageNode)  
        slicer.util.resetSliceViews()
      else:
        self.getSnapshotButton = qt.QPushButton("Get Snapshot")
        self.getSnapshotButton.toolTip = "Add a snapshot in which the transverse process is visible"
        self.getSnapshotButton.connect('clicked(bool)', self.onGetSnapshotButtonClicked)
        self.spineTable.setCellWidget(self.currentRow,self.currentColumn,self.getSnapshotButton)  
    if type=="fiducial":
      name = side + vertebrae + "Fiducial"
      node=slicer.util.getNode(name)   
      if node is not None: 
        self.modifyTPButton = qt.QPushButton("Modify")
        self.modifyTPButton.toolTip = "Modify the transverse process point"
        self.modifyTPButton.connect('clicked(bool)', self.onAddTransverseProcessButtonClicked)
        self.spineTable.setCellWidget(self.currentRow,self.currentColumn,self.modifyTPButton)      
        if item.checkState() == 2:
          print('"%s" Checked' % str(self.currentRow))
          node.SetDisplayVisibility(True) 
        else:
          print('"%s" Clicked' % str(self.currentRow))   
          node.SetDisplayVisibility(False)  
      else:
        self.addTPButton = qt.QPushButton("Add TP point")
        self.addTPButton.toolTip = "Add the transverse process in the image"
        self.addTPButton.connect('clicked(bool)', self.onAddTransverseProcessButtonClicked) 
        self.spineTable.setCellWidget(self.currentRow,self.currentColumn,self.addTPButton)      

 #---------------------------------------------------------------------------------------------------------------
          
          
  def listenToTransverseProcessesListModifications(self):
    print "Listening to Transverse Processes List modifications"   
    listNode = slicer.util.getNode("Transverse Processes List")   
    self.transversePrecessesListObserver = listNode.AddObserver('ModifiedEvent', self.onTransverseProcessesListModified)
   
#---------------------------------------------------------------------------------------------------------------
   
  def doNotListenToTransverseProcessesListModifications(self): 
    listNode = slicer.util.getNode("Transverse Processes List") 
    listNode.RemoveObserver(self.transversePrecessesListObserver)  
      
 #---------------------------------------------------------------------------------------------------------------

    
  def onTransverseProcessesListModified(self,caller,event):
    print "Tranverse Processes List Modified"  
    self.doNotListenToTransverseProcessesListModifications()
    print "is being updated = " + str(self.transverseProcessListIsBeingUpdated)
    if not self.transverseProcessListIsBeingUpdated:
      self.transverseProcessListIsBeingUpdated = True
      currentRow = self.spineTable.currentRow()
      currentColumn = self.spineTable.currentColumn()
      listNode = slicer.util.getNode("Transverse Processes List")
      numberOfChildrenNodes=listNode.GetNumberOfChildrenNodes()
      childrenNode=listNode.GetNthChildNode(numberOfChildrenNodes-1)
      fidNode = childrenNode.GetAssociatedNode()
      fidNode.SetName(self.currentFiducialName) 
      currentItem = self.spineTable.item(currentRow, currentColumn) 
      currentItem.setText("Visible") 
      currentItem.setTextAlignment(4) 
      currentItem.setCheckState(2)
      currentItem.setBackground(qt.QColor(0,255,0))
      self.updateRulerList()
      if self.spineTable.rowCount > currentRow + 1:
        nextItem = self.spineTable.item(currentRow + 1, currentColumn)
        self.spineTable.setCurrentItem(nextItem)
        self.spineTable.itemClicked(nextItem)
        print "move to the next row"
      self.layoutWidget.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)  
      self.transverseProcessListIsBeingUpdated = False
    qt.QApplication.restoreOverrideCursor()
  
  
#---------------------------------------------------------------------------------------------------------------

  
  def updateRulerList(self):
    numberOfVertebraes = self.moduleLogic.CONST_NUMBER_OF_THORACIC_VERTERBRAE + self.moduleLogic.CONST_NUMBER_OF_LUMBAR_VERTERBRAE  
    for vertebrae in range(0, numberOfVertebraes - 1):
      leftPointItem = self.spineTable.item(vertebrae, 2)
      rightPointItem = self.spineTable.item(vertebrae, 4)
      if (leftPointItem.checkState() == 2 and rightPointItem.checkState() == 2):
        print("Both TP points were marked!")  
        rulerNodeName = self.spineTable.item(vertebrae, 0).text()  
        rulerNode = slicer.util.getNode(rulerNodeName)
        if rulerNode is None:
          rulerListNode = slicer.util.getNode("Ruler List")
          self.moduleLogic.addRulerNodeToTheRulerList(rulerListNode,rulerNodeName)   
          self.upperVertebraeComboBox.addItem(rulerNodeName)     
          self.downVertebraeComboBox.addItem(rulerNodeName)   
  

#---------------------------------------------------------------------------------------------------------------

  
  def onUpdateRulerListDueFiducialModification(self,caller, event):
    # populate the list
    print "onUpdateRulerListDueFiducialModification executed" 
    self.doNotListenToFiducialsModification() 
    rulerListNode=slicer.util.getNode("Ruler List")
    if rulerListNode is not None:   
      print 'updateFiducialsList'
      print rulerListNode.GetNumberOfChildrenNodes()
      for childrenIndex in xrange(rulerListNode.GetNumberOfChildrenNodes()):
        fidHierarchyNode=rulerListNode.GetNthChildNode(childrenIndex)
        rulerNode=fidHierarchyNode.GetAssociatedNode()
        rulerNodeName = rulerNode.GetName()
        leftFiducial = slicer.util.getNode("Left"+rulerNodeName+"Fiducial")
        rightFiducial = slicer.util.getNode("Right"+rulerNodeName+"Fiducial")
        leftPos=[0, 0 ,0]
        rightPos=[0, 0 ,0]
        leftFiducial.GetFiducialCoordinates(leftPos)
        rightFiducial.GetFiducialCoordinates(rightPos)
        rulerNode.SetPosition1(leftPos)
        rulerNode.SetPosition2(rightPos)
    self.listenToFiducialsModification() 

#---------------------------------------------------------------------------------------------------------------

  
  def onStartReconstructionButtonClicked(self):
    if not self.preAcquireVolumeReconstructionSequence:  
      self.reconstructionStarted = not self.reconstructionStarted
      if self.reconstructionStarted == True:
         self.moduleLogic.startVolumeReconstruction(self.igtlRemoteLogic, self.connectorNode,self.outputVolFilename,self.outputVolDeviceName)
         print("volume reconstruction started!")
         self.startReconstructionButton.setText("Stop")
         self.suspendReconstructionButton.setEnabled(True)
      else:
         self.moduleLogic.stopVolumeReconstruction(self.igtlRemoteLogic, self.connectorNode)
         print("volume reconstruction stopped!")
         self.startReconstructionButton.setText("Start")   
         self.startReconstructionButton.setEnabled(False)
         self.suspendReconstructionButton.setEnabled(False)
    else:
      self.reconstructionStarted = not self.reconstructionStarted
      if self.reconstructionStarted == True:
         self.moduleLogic.startAcquisition(self.igtlRemoteLogic, self.connectorNode,self.preAcquisitionFilename)
         print("pre acquisition started!")
         self.startReconstructionButton.setText("Stop")
      else:
         self.moduleLogic.stopAcquisition(self.igtlRemoteLogic, self.connectorNode)
         print("pre acquisition stopped!")
         self.startReconstructionButton.setText("Start")   
         self.startReconstructionButton.setEnabled(False)
         self.moduleLogic.reconstructVolume(self.igtlRemoteLogic, self.connectorNode,self.preAcquisitionFilename,self.outputVolFilename,self.outputVolDeviceName)
         # listen to volumes added
         self.listenToVolumesAdded()
         self.numberOfGeneratedVolumes+=1
         #self.pbarwin.show()
         
 #---------------------------------------------------------------------------------------------------------------
        

  def listenToFiducialsModification(self):
    self.FiducialsObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onUpdateRulerListDueFiducialModification)  

 #---------------------------------------------------------------------------------------------------------------

    
  def doNotListenToFiducialsModification(self):
    slicer.mrmlScene.RemoveObserver(self.FiducialsObserver)

#---------------------------------------------------------------------------------------------------------------

    
  def listenToPlusServerStopAcquisitionReply(self):
    self.PlusServerObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onPlusServerStopAcquisitionReplyReceived)
    

 #---------------------------------------------------------------------------------------------------------------


  def doNotListenToPlusServerReply(self):
    slicer.mrmlScene.RemoveObserver(self.PlusServerObserver)

 #---------------------------------------------------------------------------------------------------------------

  
  def listenToMhaFileReadFromDisk(self):  
    self.MhaFileReaderObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onMhaFileReaded)

 #---------------------------------------------------------------------------------------------------------------
    
  def doNotListenToMhaFileReadFromDisk(self):
    slicer.mrmlScene.RemoveObserver(self.MhaFileReaderObserver)  

 #---------------------------------------------------------------------------------------------------------------
    
  def onMhaFileReaded(self, caller, event):     
      
    if self.acquisitionfilename.find('leftSideAcquisition')!=-1:
      sequenceName = self.leftSideAcquisitionFilename
    else:
      sequenceName = self.rightSideAcquisitionFilename
    sequenceName=sequenceName.split(".")
    sequenceName=sequenceName[0]
      
    self.doNotListenToMhaFileReadFromDisk() 
    self.configureSequenceInTheScene(sequenceName) 
  
 #---------------------------------------------------------------------------------------------------------------
      
      
  def configureSequenceInTheScene(self, sequenceName):
    imageSequenceNode=slicer.util.getNode('*' + sequenceName + "-Image")
    vrdl=self.moduleLogic.getVolumeResliceDriverLogic()
    if imageSequenceNode is not None:      
      # when the sequence is added to the scene the viewer are changed so it is necessary to update them 
      self.updateViewers()
          
      referenceToTrackerSequenceNode=slicer.util.getNode("*" + sequenceName + "-ReferenceToTrackerTransform")
      probeToTrackerSequenceNode=slicer.util.getNode("*" + sequenceName + "-ProbeToTrackerTransform")
      imageVolumeNode=slicer.util.getNode("*" + sequenceName + "-Image [time*")
      trackerToReferenceSequenceNode=slicer.modulelogic.vtkMRMLSequenceNode()  
      name=referenceToTrackerSequenceNode.GetName()  
      newName=name.replace("ReferenceToTracker","TrackerToReference") 
      trackerToReferenceSequenceNode.SetName(newName)
      numberOfDataNodes=referenceToTrackerSequenceNode.GetNumberOfDataNodes()
        
      if sequenceName.find('left')==0:
        currentIndex = self.leftSequenceAcquisitionComboBox.currentIndex  
        self.leftSequenceAcquisitionComboBox.addItem(sequenceName)
        self.leftSequenceAcquisitionComboBox.setCurrentIndex(currentIndex + 1)
        self.configureLeftViewer(sequenceName)
        self.configureLeftSlider(sequenceName)
      else:
        currentIndex = self.rightSequenceAcquisitionComboBox.currentIndex
        self.rightSequenceAcquisitionComboBox.addItem(sequenceName)
        self.rightSequenceAcquisitionComboBox.setCurrentIndex(currentIndex + 1)
        self.configureRightViewer(sequenceName)
        self.configureRightSlider(sequenceName)
        

      for transformationIndex in range(0, numberOfDataNodes-1):
        dn=referenceToTrackerSequenceNode.GetNthDataNode(transformationIndex)  
        newNode=slicer.vtkMRMLLinearTransformNode() 
        newNode.Copy(dn)
        name=newNode.GetName()  
        newName=name.replace("ReferenceToTracker","TrackerToReference") 
        newNode.SetName(newName)
        transform=newNode.GetMatrixTransformToParent()
        transform.Invert()
        newNode.SetAndObserveMatrixTransformToParent(transform)
        iv=referenceToTrackerSequenceNode.GetNthIndexValue(transformationIndex)
        trackerToReferenceSequenceNode.SetDataNodeAtValue(newNode,iv)      
          
      slicer.mrmlScene.AddNode(trackerToReferenceSequenceNode)
      browser=slicer.util.getNode("*" + sequenceName)
      browser.AddSynchronizedRootNode(trackerToReferenceSequenceNode.GetID())
                  
      imageToProbeNode=slicer.util.getNode("ImageToProbe")
      imageToProbeForThisSequence=slicer.vtkMRMLLinearTransformNode()
      imageToProbeForThisSequence.Copy(imageToProbeNode) 
      imageToProbeForThisSequence.SetName(sequenceName + "-ImageToProbe")
      slicer.mrmlScene.AddNode(imageToProbeForThisSequence)
      trackerToReferenceNode=slicer.util.getNode("*" + sequenceName + "-TrackerToReferenceTransform [time*")
      probeToTrackerNode=slicer.util.getNode("*" + sequenceName + "-ProbeToTrackerTransform [time*")
       
      imageVolumeNode.SetAndObserveTransformNodeID(imageToProbeForThisSequence.GetID())
      imageToProbeForThisSequence.SetAndObserveTransformNodeID(probeToTrackerNode.GetID())
      probeToTrackerNode.SetAndObserveTransformNodeID(trackerToReferenceNode.GetID())           
      
      slicer.util.resetSliceViews()  

 #---------------------------------------------------------------------------------------------------------------

      
  def onPlusServerStopAcquisitionReplyReceived(self, caller,  event):
    replyName="ACK_" + str(self.commandId) 
    node = slicer.util.getNode(replyName) 
    if node is not None:
      metaFileImporter=slicer.modules.metafileimporter
      logic=metaFileImporter.logic() 
      self.doNotListenToPlusServerReply() 
      print "Plus server reply received!"
      if os.path.isfile(self.acquisitionfilename):
        logic.Read(self.acquisitionfilename)
        self.listenToMhaFileReadFromDisk()
        print "capture saved in: " + self.acquisitionfilename
      else:
        print "Error: file not found: " + self.acquisitionfilename

#---------------------------------------------------------------------------------------------------------------
      
  def onLeftSequenceSliderMoved(self,currentFrame):
    print("Slider movement captured!")
    activeSequence=self.leftSequenceAcquisitionComboBox.currentText
    activeSequence=activeSequence.split(".")
    activeSequence=activeSequence[0]
    browser=slicer.util.getNode("*"  + activeSequence)
    browser.SetSelectedItemNumber(currentFrame)    

 #---------------------------------------------------------------------------------------------------------------
 
     
  def onRightSequenceSliderMoved(self,currentFrame):
    print("Slider movement captured!")
    activeSequence=self.rightSequenceAcquisitionComboBox.currentText
    activeSequence=activeSequence.split(".")
    activeSequence=activeSequence[0]
    browser=slicer.util.getNode("*"  + activeSequence)
    browser.SetSelectedItemNumber(currentFrame)        
 
 #---------------------------------------------------------------------------------------------------------------
      
  def onLeftSequenceChanged(self, sequenceName):
    self.configureLeftViewer(sequenceName)
    self.configureLeftSlider(sequenceName)

 #---------------------------------------------------------------------------------------------------------------
    
  def onRightSequenceChanged(self, sequenceName):
    self.configureRightViewer(sequenceName)   
    self.configureRightSlider(sequenceName)   

 #---------------------------------------------------------------------------------------------------------------
    
  def configureLeftSlider(self,sequenceName): 
    print sequenceName  
    referenceToTrackerSequenceNode=slicer.util.getNode("*" + sequenceName + "-ReferenceToTrackerTransform")
    if referenceToTrackerSequenceNode is not None:
      numberOfDataNodes=referenceToTrackerSequenceNode.GetNumberOfDataNodes()   
      self.leftSequenceSlider.setMaximum(numberOfDataNodes)
      self.leftSequenceSlider.setMinimum(1)
      self.leftSequenceSlider.setValue(1)
      self.leftSequenceSlider.setEnabled(1)  

#---------------------------------------------------------------------------------------------------------------
    
  def configureRightSlider(self,sequenceName): 
    referenceToTrackerSequenceNode=slicer.util.getNode("*" + sequenceName + "-ReferenceToTrackerTransform")
    if referenceToTrackerSequenceNode is not None:
      numberOfDataNodes=referenceToTrackerSequenceNode.GetNumberOfDataNodes()   
      self.rightSequenceSlider.setMaximum(numberOfDataNodes)
      self.rightSequenceSlider.setMinimum(1)
      self.rightSequenceSlider.setValue(1)
      self.rightSequenceSlider.setEnabled(1)   

#---------------------------------------------------------------------------------------------------------------
      
  def configureLeftViewer(self,sequenceName):
    vrdl=self.moduleLogic.getVolumeResliceDriverLogic()    
    self.currentLeftSequenceName=sequenceName 
    imageVolumeNode=slicer.util.getNode("*" + self.currentLeftSequenceName + "-Image [time*")
    if imageVolumeNode is not None:  
      yellowWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeYellow")
      yellowWidgetCompNode.SetBackgroundVolumeID(imageVolumeNode.GetID()) 
      yellowNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeYellow")
      vrdl.SetDriverForSlice(imageVolumeNode.GetID(), yellowNode)
      vrdl.SetModeForSlice(vrdl.MODE_TRANSVERSE180, yellowNode)   
      yellowNode.SetSliceVisible(True)
      self.moduleLogic.adjustDisplaySettings(imageVolumeNode)

#---------------------------------------------------------------------------------------------------------------    
    
  def configureRightViewer(self,sequenceName):
    vrdl=self.moduleLogic.getVolumeResliceDriverLogic()    
    self.currentRightSequenceName=sequenceName 
    imageVolumeNode=slicer.util.getNode("*" + self.currentRightSequenceName + "-Image [time*")
    if imageVolumeNode is not None:  
      greenWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeGreen")
      greenWidgetCompNode.SetBackgroundVolumeID(imageVolumeNode.GetID()) 
      greenNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeGreen")
      vrdl.SetDriverForSlice(imageVolumeNode.GetID(), greenNode)
      vrdl.SetModeForSlice(vrdl.MODE_TRANSVERSE180, greenNode)  
      greenNode.SetSliceVisible(True)
      self.moduleLogic.adjustDisplaySettings(imageVolumeNode)
#---------------------------------------------------------------------------------------------------------------
        
  def updateViewers(self):
    image_Reference = slicer.util.getNode("Image_Reference")
    if image_Reference is not None:
      redWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeRed")
      redWidgetCompNode.SetBackgroundVolumeID(image_Reference.GetID())
        
    leftVolumeNode=slicer.util.getNode("*" + self.currentLeftSequenceName + "-Image [time*")
    if leftVolumeNode is not None:
      yellowWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeYellow")
      yellowWidgetCompNode.SetBackgroundVolumeID(leftVolumeNode.GetID())
        
    rightVolumeNode=slicer.util.getNode("*" + self.currentRightSequenceName + "-Image [time*")
    if rightVolumeNode is not None:
      greenWidgetCompNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeGreen")
      greenWidgetCompNode.SetBackgroundVolumeID(rightVolumeNode.GetID())   

 #---------------------------------------------------------------------------------------------------------------
      
  def onSuspendReconstructionButtonClicked(self):
    if not self.preAcquireVolumeReconstructionSequence:   
      self.reconstructionSuspended = not self.reconstructionSuspended
      if self.reconstructionSuspended == True:
         self.moduleLogic.suspendVolumeReconstruction(self.igtlRemoteLogic, self.connectorNode)
         self.suspendReconstructionButton.setText("Resume")
      else:
         self.moduleLogic.resumeVolumeReconstruction(self.igtlRemoteLogic, self.connectorNode)
         self.suspendReconstructionButton.setText("Suspend")          

 #---------------------------------------------------------------------------------------------------------------
         
  def onStartLeftSideAcquisitionButtonClicked(self):
      if self.estado is not "Connected":
        ret=qt.QMessageBox.warning(self.spineTable, 'Start Acquisition', 'You must connect to the tracker first in order to acquire the sequences.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
        return    
      self.leftSideAcquisitionStarted = not self.leftSideAcquisitionStarted
      if self.leftSideAcquisitionStarted == True:
         self.leftSideAcquisitionFilename="leftSideAcquisition"+str(self.numberOfLeftSequences)+".mha" 
         filename = os.path.join(self.modulePath, self.leftSideAcquisitionFilename)
         self.moduleLogic.startAcquisition(self.igtlRemoteLogic, self.connectorNode,filename)
         print("left side acquisition started!")
         self.leftSequenceAcquisitionButton.setText("Stop acquisition")
      else:
         self.commandId = self.moduleLogic.stopAcquisition(self.igtlRemoteLogic, self.connectorNode)
         print("left side acquisition stopped!")
         self.leftSequenceAcquisitionButton.setText("Start acquisition")   
         self.leftSideAcquisitionFilename="leftSideAcquisition"+str(self.numberOfLeftSequences)+".mha" 
         self.acquisitionfilename = os.path.join(self.modulePath, self.leftSideAcquisitionFilename)
         #self.leftSequenceAcquisitionButton.setEnabled(False)
         self.numberOfLeftSequences+=1
         self.listenToPlusServerStopAcquisitionReply()
         
 #---------------------------------------------------------------------------------------------------------------
         
  def onStartRightSideAcquisitionButtonClicked(self):
      if self.estado is not "Connected":
        ret=qt.QMessageBox.warning(self.spineTable, 'Start Acquisition', 'You must connect to the tracker first in order to acquire the sequences.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )  
        return  
      self.rightSideAcquisitionStarted = not self.rightSideAcquisitionStarted
      if self.rightSideAcquisitionStarted == True:
         self.rightSideAcquisitionFilename="rightSideAcquisition"+str(self.numberOfRightSequences)+".mha" 
         filename = os.path.join(self.modulePath, self.rightSideAcquisitionFilename)
         self.moduleLogic.startAcquisition(self.igtlRemoteLogic, self.connectorNode,filename)
         print("Right side acquisition started!")
         self.rightSequenceAcquisitionButton.setText("Stop acquisition")
      else:
         self.commandId = self.moduleLogic.stopAcquisition(self.igtlRemoteLogic, self.connectorNode)
         print("right side acquisition stopped!")
         self.rightSequenceAcquisitionButton.setText("Start acquisition")
         self.acquisitionfilename = os.path.join(self.modulePath, self.rightSideAcquisitionFilename)   
         self.numberOfRightSequences+=1
         self.listenToPlusServerStopAcquisitionReply()        

#---------------------------------------------------------------------------------------------------------------
            
  def listenToVolumesAdded(self):
    self.sceneObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onVolumeAdded)

#---------------------------------------------------------------------------------------------------------------

  def doNotListenToVolumesAdded(self):
    slicer.mrmlScene.RemoveObserver(self.sceneObserver)

#---------------------------------------------------------------------------------------------------------------
    
  def onVolumeAdded(self, caller,  event):  
      node = slicer.util.getNode(self.outputVolDeviceName)
      if not node==None:
         self.doNotListenToVolumesAdded()
         #self.pbarwin.hide()
         self.outputVolFilename=self.reconstructedVolumePrefix+str(self.numberOfGeneratedVolumes)+"_"+self.volumeReferenceFrame+".mha"
         self.outputVolDeviceName=self.reconstructedVolumePrefix+str(self.numberOfGeneratedVolumes)+"_"+self.volumeReferenceFrame
         self.preAcquisitionFilename="acquiredFramesForVolumeReconstruction"+str(self.numberOfGeneratedVolumes)+".mha"
         self.startReconstructionButton.setEnabled(True)
         vl=slicer.modules.volumes
         vl=vl.logic()
         vl.SetMRMLScene(slicer.mrmlScene)
         vl.Modified()  
         print "Navigation step: volume added"

#---------------------------------------------------------------------------------------------------------------
         
  def onPreAcquisitionStateChanged(self,status):     
        if status==2:
            self.preAcquireVolumeReconstructionSequence=True
        else:
            self.preAcquireVolumeReconstructionSequence=False           
                          
#---------------------------------------------------------------------------------------------------------------



#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------



 
class ScoliosisSliceletTestSlicelet(Slicelet):
  """ Creates the interface when module is run as a stand alone gui app.
  """

  def __init__(self):
    super(ScoliosisSliceletTestSlicelet, self).__init__(ScoliosisSliceletTestWidget)


if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  import sys
  print(sys.argv)

  slicelet = ScoliosisSliceletTestSlicelet() 

