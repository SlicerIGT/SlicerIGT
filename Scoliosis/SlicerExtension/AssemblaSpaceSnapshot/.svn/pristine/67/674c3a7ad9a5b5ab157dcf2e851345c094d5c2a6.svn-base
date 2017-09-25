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
  
    self.leftDeltasList = [None]*(-1 + self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE)
    self.rightDeltasList = [None]*(-1 + self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE*self.CONST_NUMBER_OF_THORACIC_VERTERBRAE*self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE)
    
    # These are useful in many function
    self.fiducialPositionList = []
    self.fiducialLabelList = []
    self.missingFiducials = []
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    self.layout.addWidget(self.reloadButton)
  
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

    # ruler Selector
    self.rulerSelector = slicer.qMRMLNodeComboBox()
    self.rulerSelector.nodeTypes = ( ("vtkMRMLAnnotationHierarchyNode"), "" )
    self.rulerSelector.selectNodeUponCreation = True
    self.rulerSelector.addEnabled = True
    self.rulerSelector.removeEnabled = True
    self.rulerSelector.noneEnabled = False
    self.rulerSelector.showHidden = False
    self.rulerSelector.showChildNodeTypes = False
    self.rulerSelector.setMRMLScene( slicer.mrmlScene )
    self.rulerSelector.setToolTip( "Pick the input rulers to the algorithm." )
    parametersFormLayout.addRow("Input Rulers: ", self.rulerSelector)
    
    # fiducial selector
    self.fiducialSelector = slicer.qMRMLNodeComboBox()
    self.fiducialSelector.nodeTypes = (("vtkMRMLMarkupsFiducialNode"), "")            # Using this tuple structure causes inpu to be treated as list of values, not list of lists ??
    self.fiducialSelector.selectNodeUponCreation = True
    self.fiducialSelector.addEnabled = True
    self.fiducialSelector.removeEnabled = True
    self.fiducialSelector.noneEnabled = True
    self.fiducialSelector.showHidden = False
    self.fiducialSelector.showChildNodeTypes = False
    self.fiducialSelector.setMRMLScene(slicer.mrmlScene)
    self.fiducialSelector.setToolTip("Choose list of fiducials as algorithm input.")
    parametersFormLayout.addRow("Input Fiducials: ", self.fiducialSelector)
    
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
    self.calculateRulers = qt.QPushButton("Calculate with Rulers")
    outputVerticalLayout.addWidget(self.calculateRulers, 0, 0, 1, 3)
    
    # Calculate from fiducials Button
    self.calculateFiducials = qt.QPushButton("Calculate with Fiducials")
    outputVerticalLayout.addWidget(self.calculateFiducials, 0, 3, 1, 3)
    
    # Calculate Deltas
    self.calculateDeltas = qt.QPushButton("Calculate delta vectors")
    outputVerticalLayout.addWidget(self.calculateDeltas, 1, 0, 1, 3)
    
    # Save angle data button
    self.saveAngles = qt.QPushButton("Save angles as CSV")
    outputVerticalLayout.addWidget(self.saveAngles, 2, 0, 1, 3)
    
    # Save fiducial delta vector data
    self.saveDeltas = qt.QPushButton("Save delta vectors as CSV")
    outputVerticalLayout.addWidget(self.saveDeltas, 2, 3, 1, 3)
      
    # Angle Table
    horizontalHeaders = ["Vertebrae", "Superior Angles (degrees)", "Inferior Angles (degrees)"]

    self.angleTable = qt.QTableWidget(self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE + self.CONST_NUMBER_OF_THORACIC_VERTERBRAE + self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE, 3)
    self.angleTable.sortingEnabled = False
    self.angleTable.setEditTriggers(0)
    self.angleTable.setMinimumHeight(self.angleTable.verticalHeader().length() + 25)
    self.angleTable.horizontalHeader().setResizeMode(qt.QHeaderView.Stretch)
    self.angleTable.setSizePolicy (qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.Preferred)
    outputVerticalLayout.addWidget(self.angleTable, 3, 0, 1, 6)
    
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
    self.rulerSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.fiducialSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.calculateRulers.connect('clicked(bool)', self.onCalculateRulers)
    self.calculateFiducials.connect('clicked(bool)', self.onCalculateFiducials)
    self.calculateFiducials.connect('clicked(bool)', self.onCalculateRulers)      # onCalculateFiducials generates the rulers 
    self.calculateDeltas.connect('clicked(bool)', self.onCalculateDeltas)
    self.saveAngles.connect('clicked(bool)', self.onSaveAngles)
    self.saveDeltas.connect('clicked(bool)', self.onSaveDeltas)
    self.reloadButton.connect('clicked(bool)',self.onReloadButton)

    self.layout.addStretch(1)
    
    
  def onSelect(self):
    logic = SpinalCurvatureMeasurementLogic()
  
  def onCalculateDeltas(self):                      # Consider integrating into calculate w/ rulers/fids. button
    logic = SpinalCurvatureMeasurementLogic()
    (self.leftDeltasList, self.rightDeltasList) = logic.calculateDeltas(self.rulerSelector.currentNode())      # Requires calculate w/ rulers/fids. to be run first anyway
  
  
  def onCalculateRulers(self):
    logic = SpinalCurvatureMeasurementLogic()
    (self.superiorAnglesList, self.inferiorAnglesList) = logic.calculateAngles(self.rulerSelector.currentNode(), self.projectionPlaneBox.currentText)
    
    superiorAnglesListCount = 0
    for superiorAngle in self.superiorAnglesList:
      self.angleTable.item(superiorAnglesListCount, 1).setText(str(superiorAngle))
      superiorAnglesListCount += 1

    inferiorAnglesListCount = 0
    for inferiorAngle in self.inferiorAnglesList:
      self.angleTable.item(inferiorAnglesListCount, 2).setText(str(inferiorAngle))
      inferiorAnglesListCount += 1
    
    """
    
    # Refresh observers
    for rulerNode, tag in self.observerTags:
      rulerNode.RemoveObserver(tag)
    
    rulerNodes = vtk.vtkCollection();
    self.rulerSelector.currentNode().GetDirectChildren(rulerNodes)
    
    rulerNodesList = []
    for nodeIndex in range(rulerNodes.GetNumberOfItems()):
      if rulerNodes.GetItemAsObject(nodeIndex).GetClassName() == "vtkMRMLAnnotationRulerNode":
        rulerNodesList.append(rulerNodes.GetItemAsObject(nodeIndex)) 
 
    for rulerNode in rulerNodesList:
      def update(caller, ev):
        import re
        angle = logic.updateAngle(caller, self.projectionPlaneBox.currentText)
        
        rulerName = caller.GetName()
        nameSearch.group(1) - Region (C, T, or L)
        nameSearch.group(2) - Number (1, 2, 3, etc.)
        nameSearch.group(3) - Superior or Inferior (s or i)
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
      
      """
      
  def onCalculateFiducials(self):                                 # Will simply convert fiducials points into rulers, reducing to previously solved problem
    import math
    AnnotationHierarchy = slicer.util.getNode('All Annotations')
    #AnnotationHierarchy.DeleteDirectChildren()
    OldRulerList = slicer.util.getNode('Ruler List')
    fiducialNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLMarkupsFiducialNode')
    fiducialNodes = fiducialNodes.GetItemAsObject(0)
    # Store fiducial info in seperate lists:
    self.fiducialPositionList = []                                      
    self.fiducialLabelList = []                                   # Re-initialize these in case of recalculation
    self.missingFiducials = []                                    
      
    if(OldRulerList != None):
      OldRulerList.DeleteDirectChildren()
      
    # Want to order potentially unordered fiducials
    vertebra = 0                  # Corresponding to T1, 12 to T12, and 17 to L5
    dummyFid = slicer.vtkMRMLMarkupsFiducialNode()
    while(vertebra < 12):         # Search for thoracic spine
      vertebra += 1
      leftMissing = True
      rightMissing = True
      for fiducial in range(0, fiducialNodes.GetNumberOfFiducials()):
        currentLabel = fiducialNodes.GetNthFiducialLabel(fiducial)
        if(leftMissing and rightMissing and currentLabel == "T" + str(vertebra) + "L" and currentLabel not in self.missingFiducials): # We find the left fid. we want to add
          leftMissing = False
          self.fiducialLabelList.append(currentLabel)
          self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(fiducial,0))
          for partner in range(0, fiducialNodes.GetNumberOfFiducials()):
            if(fiducialNodes.GetNthFiducialLabel(partner)[:-1] == currentLabel[:-1] and partner != fiducial):
              rightMissing = False
              self.fiducialLabelList.append(fiducialNodes.GetNthFiducialLabel(partner))
              self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(partner, 0))
          if(rightMissing):
            self.fiducialLabelList.append(currentLabel[:-1] + "R") # Dummy fiducial (no coords) to maintain even-odd order
            self.fiducialPositionList.append(None)
            self.missingFiducials.append(currentLabel[:-1] + "R")
            
        if(leftMissing and (currentLabel == "T" + str(vertebra) + "R") and currentLabel not in self.missingFiducials):  # We find the right before the left
          rightMissing = False
          for partner in range(0, fiducialNodes.GetNumberOfFiducials()):
            if(fiducialNodes.GetNthFiducialLabel(partner)[:-1] == currentLabel[:-1] and fiducialNodes.GetNthFiducialLabel(partner)[-1] == "L"):
              leftMissing = False
              self.fiducialLabelList.append(fiducialNodes.GetNthFiducialLabel(partner))
              self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(partner, 0))
          if(leftMissing):
            self.fiducialLabelList.append(currentLabel[:-1] + "L")
            self.fiducialPositionList.append(None)
            self.missingFiducials.append(currentLabel[:-1] + "L")
          
          self.fiducialLabelList.append(currentLabel)
          self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(fiducial, 0))
      
      if(leftMissing and rightMissing and len(self.fiducialLabelList) > 0):          # We didn't find the either point
        # Dummies are still included here to maintain corret delta vector connectivity
        self.fiducialLabelList.append("T" + str(vertebra) + "L")
        self.fiducialLabelList.append("T" + str(vertebra) + "R")
        self.fiducialPositionList.append(None)
        self.fiducialPositionList.append(None)
        self.missingFiducials.append("T" + str(vertebra) + "L")
        self.missingFiducials.append("T" + str(vertebra) + "R")
        
    while(vertebra-12 < 5):         # Search for lumbar spine  
      vertebra += 1
      leftMissing = True
      rightMissing = True
      for fiducial in range(0, fiducialNodes.GetNumberOfFiducials()):
        currentLabel = fiducialNodes.GetNthFiducialLabel(fiducial)
        if(leftMissing and rightMissing and currentLabel == "L" + str(vertebra-12) + "L" and currentLabel not in self.missingFiducials): # We find the left fid. we want to add
          leftMissing = False
          self.fiducialLabelList.append(currentLabel)
          self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(fiducial,0))
          for partner in range(0, fiducialNodes.GetNumberOfFiducials()):
            if(fiducialNodes.GetNthFiducialLabel(partner)[:-1] == currentLabel[:-1] and fiducialNodes.GetNthFiducialLabel(partner)[-1] == "R"):
              rightMissing = False
              self.fiducialLabelList.append(fiducialNodes.GetNthFiducialLabel(partner))
              self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(partner, 0))
          if(rightMissing):
            self.fiducialLabelList.append(currentLabel[:-1] + "R")
            self.fiducialPositionList.append(None)
            self.missingFiducials.append(currentLabel[:-1] + "R")
            
        if(leftMissing and (currentLabel == "L" + str(vertebra-12) + "R") and currentLabel not in self.missingFiducials):  # We find the right before the left
          rightMissing = False
          for partner in range(0, fiducialNodes.GetNumberOfFiducials()):
            if(fiducialNodes.GetNthFiducialLabel(partner)[:-1] == currentLabel[:-1] and fiducialNodes.GetNthFiducialLabel(partner)[-1] == "L"):
              leftMissing = False
              self.fiducialLabelList.append(fiducialNodes.GetNthFiducialLabel(partner))
              self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(partner, 0))
          if(leftMissing):
            self.fiducialLabelList.append(currentLabel[:-1] + "L")
            self.fiducialPositionList.append(None)
            self.missingFiducials.append(currentLabel[:-1] + "L")
          #else:
          self.fiducialLabelList.append(fiducialNodes.GetNthFiducialLabel(fiducial))
          self.fiducialPositionList.append(fiducialNodes.GetMarkupPointVector(fiducial, 0))
      
      if(leftMissing and rightMissing and len(self.fiducialLabelList) > 0):          # We didn't find the either point
        self.fiducialLabelList.append("L" + str(vertebra-12) + "L")
        self.fiducialLabelList.append("L" + str(vertebra-12) + "R")
        self.fiducialPositionList.append(None)
        self.fiducialPositionList.append(None)
        self.missingFiducials.append("L" + str(vertebra-12) + "L")
        self.missingFiducials.append("L" + str(vertebra-12) + "R")
        
    # This if statement might not behave as intended if, say, T12L and T12R are missing while L1L and L1R are there
    if(len(self.missingFiducials)>1 and self.missingFiducials[-1][:-1] == self.missingFiducials[-2][:-1] and int(self.missingFiducials[-1][1:-1]) > int(self.fiducialLabelList[-1][1:-1])):
      tooLong = True
      while(tooLong):    
        if(self.missingFiducials[-1][:-1] == self.missingFiducials[-2][:-1]):
          self.missingFiducials.pop()
          self.missingFiducials.pop()
          if(len(self.missingFiducials) < 2):
            break
        else:
          tooLong = False
        
    for missing in self.missingFiducials:
      print missing + " fiducial point is missing"    
        
    populatingRulers = True
    ruler = 0
    # Currently can't adapt to left-right even-odd index reversal upon missing nodes,
    # Insert dummy fid. nodes to mark their places and eventually have coords determined?
    while(populatingRulers):
      if(self.fiducialLabelList[2*(ruler)][-1] == "L" and self.fiducialLabelList[2*(ruler)] not in self.missingFiducials):
        if((self.fiducialLabelList[2*(ruler)][:-1]+"R" not in self.missingFiducials)):
          newRuler = slicer.vtkMRMLAnnotationRulerNode()
          newRuler.SetName(self.fiducialLabelList[2*(ruler)][:-1] + "-s")
          newRuler.SetPosition1(self.fiducialPositionList[2*(ruler)])
          newRuler.SetPosition2(self.fiducialPositionList[2*(ruler)+1])
          slicer.mrmlScene.AddNode(newRuler)
      ruler += 1
      if(ruler >= int(len(self.fiducialLabelList)/2)):
        populatingRulers = False
      
    OldRulerList = slicer.util.getNode('Ruler List')
    slicer.modules.SpinalCurvatureMeasurementWidget.rulerSelector.setCurrentNode(OldRulerList)
  
  def onSaveAngles(self):
    import csv
    
    fileName = qt.QFileDialog.getSaveFileName(0, "Save Angle Data", "", "CSV File (*.csv)")
    
    with open(fileName, 'wb') as csvfile:
      writer = csv.writer(csvfile, delimiter=',', quotechar='|')
      writer.writerow(['Vertebrae', 'Superior Angle (degrees)', 'Inferior Angle (degrees)'])
      
      for rowIndex in range(len(self.labelReferences)):
        writer.writerow([self.labelReferences[rowIndex].text(), self.superiorAngleReferences[rowIndex].text(), self.inferiorAngleReferences[rowIndex].text()])
        
  def onSaveDeltas(self):
    import csv, math
    fileName = qt.QFileDialog.getSaveFileName(0, "Save Delta Vector Data", "", "CSV File (*.csv)")
    
    RulerList = slicer.util.getNode('Ruler List')
    
    with open(fileName, 'wb') as csvfile:
      writer = csv.writer(csvfile, 'excel', delimiter = ',')
      writer.writerow(['From ___, to ___', '', 'Left-Side Delta Vector (mm)', '', '', 'Right-Side Delta Vector (mm)', ''])
      writer.writerow(['','deltaX','deltaY','deltaZ','deltaX','deltaY','deltaZ'])
   
      for i, leftDelta in enumerate(self.leftDeltasList):
        row = ["From " + self.fiducialLabelList[((i+1)*2)][:-1] + " to " + self.fiducialLabelList[(i*2)][:-1]]
        for dim in self.leftDeltasList[i]:
          row.append(dim)
        for dim in self.rightDeltasList[i]:
          row.append(dim)
        writer.writerow(row)
        
    """
      for i in range(0, len(self.leftDeltasList)-1):
        if(fiducialNodes.GetNthFiducialLabel(i*2)[:-1]):
          row = [fiducialNodes.GetNthFiducialLabel(i*2)[:-1] + " - " + fiducialNodes.GetNthFiducialLabel(i)]
        for dim in self.leftDeltasList[i]:
          row.append(dim)
        for dim in self.rightDeltasList[i]:
          row.append(dim)
        writer.writerow(row)
    """
  
  def onReloadButton(self):
    for rulerName in self.verterbraeNames:                        # Should prevent fid. input list from being populated w/ old rulers
      #print rulerName + "-s"
      oldRuler = slicer.util.getNode(rulerName + "-s")            # Needs modification for inferior plates
      slicer.mrmlScene.RemoveNode(oldRuler)
    slicer.util.reloadScriptedModule(slicer.moduleNames.SpinalCurvatureMeasurement)


#
# SpinalCurvatureMeasurementLogic
#
class SpinalCurvatureMeasurementLogic:
  def __init__(self):
    self.CONST_NUMBER_OF_CERVICAL_VERTERBRAE = 7
    self.CONST_NUMBER_OF_THORACIC_VERTERBRAE = 12
    self.CONST_NUMBER_OF_LUMBAR_VERTERBRAE = 5

  def calculateDeltas(self, annotationHierarchy):
    import math
    widgetClass = slicer.modules.SpinalCurvatureMeasurementWidget
    leftDeltas = []
    rightDeltas = []
    
    rulerNodes = vtk.vtkCollection();
    annotationHierarchy.GetDirectChildren(rulerNodes)
    
    for i, fidPoint in enumerate(widgetClass.fiducialLabelList[:-2]):
      leftDelta = []
      rightDelta = []
      
      # If a pair of points is missing, we must include dummy vectors to maintain connectivity
      if(fidPoint in widgetClass.missingFiducials and widgetClass.fiducialLabelList[i+1] in widgetClass.missingFiducials and fidPoint[1:-1] == widgetClass.fiducialLabelList[i+1][1:-1]):
        if(i % 2 == 0):
          leftDeltas.append([None,None,None])
        else:
          rightDeltas.append([None,None,None])
        
      # Special case for thoracic to lumbar transition
      elif(fidPoint[1:-1] == "12" and fidPoint not in widgetClass.missingFiducials and "L1" + fidPoint[-1] not in widgetClass.missingFiducials):  
        if(i % 2 == 0):
          for j in range(0,3):
            leftDelta.append(widgetClass.fiducialPositionList[i][j] - widgetClass.fiducialPositionList[i+2][j])
          leftDeltas.append(leftDelta)
        else:
          for j in range(0,3):
            rightDelta.append(widgetClass.fiducialPositionList[i][j] - widgetClass.fiducialPositionList[i+2][j])
          rightDeltas.append(rightDelta)
      
      elif((fidPoint[1:-1] != "12") and (fidPoint not in widgetClass.missingFiducials) and (fidPoint[0] + str(int(fidPoint[1:-1])+1) + fidPoint[-1] not in widgetClass.missingFiducials)):
       # Assert the point below the current is not missing
        if(i % 2 == 0):
          for j in range(0,3):
            leftDelta.append(widgetClass.fiducialPositionList[i][j] - widgetClass.fiducialPositionList[i+2][j])
          leftDeltas.append(leftDelta)
        else:
          for j in range(0,3):
            rightDelta.append(widgetClass.fiducialPositionList[i][j] - widgetClass.fiducialPositionList[i+2][j])
          rightDeltas.append(rightDelta)
        
      # Append None to vector list as placeholder to avoid index out-of-bounds in file IO
      else:
        if(i % 2 == 0): 
          leftDeltas.append([None,None,None])
        else:
          rightDeltas.append([None,None,None])
      
    return (leftDeltas, rightDeltas)    

       
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

