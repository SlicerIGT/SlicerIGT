from __main__ import vtk, qt, ctk, slicer
#
# SpinalCurvatureMeasurement
#

class SpinalCurvatureMeasurement:
  def __init__(self, parent):
    parent.title = "Spinal Curvature Measurement"
    parent.categories = ["Quantification"]
    parent.contributors = ["Franklin King (Queen's University), Tamas Ungi (Queen's University)"]
    parent.helpText = """
    This is a module for measuring spinal curvature by measuring angles specified by ruler annotations. See <a>https://www.assembla.com/spaces/Scoliosis/wiki/Spinal_Curvature_Measurement</a> for more information.
    """
    parent.acknowledgementText = """
    This work was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)
"""
    #parent.icon = qt.QIcon(":Icons/SpinalCurvatureMeasurement.png")
    self.parent = parent

    
#
# qSpinalCurvatureMeasurementWidget
#
class SpinalCurvatureMeasurementWidget:
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
    self.layout.addWidget(instructionsCollapsibleButton)    
    
    instructionsLayout = qt.QVBoxLayout(instructionsCollapsibleButton)    
    
    self.namingInstructions = ctk.ctkFittedTextBrowser()
    self.namingInstructions.setReadOnly(True)
    self.namingInstructions.setPlainText("C - Cervical, T - Thoracic, L - Lumbar\ns - Superior, i - Inferior\nName ruler annotations like so: T1-s, T1-i, T2-s, etc.")
    instructionsLayout.addWidget(self.namingInstructions)
    
    
    # Input Area
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Input"
    self.layout.addWidget(parametersCollapsibleButton)

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
    self.layout.addWidget(outputCollapsibleButton)
    
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
    
    
  def onSelect(self):
    logic = SpinalCurvatureMeasurementLogic()
  
  
  def onCalculateButton(self):
    logic = SpinalCurvatureMeasurementLogic()
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
# SpinalCurvatureMeasurementLogic
#
class SpinalCurvatureMeasurementLogic:
  def __init__(self):
    self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE = 7
    self.CONST_NUMBER_OF_THORACIC_VERTERBRAE = 12
    self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE = 5

   
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

