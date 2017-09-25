import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import numpy as np
#from math import *
#import math

#
# PreProcessLandmarks
#

class PreProcessLandmarks(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Pre-process Landmarks"
    self.parent.categories = ["Scoliosis"]
    self.parent.dependencies = []
    self.parent.contributors = ["Ben Church (PerkLab - Queen's University)"]
    self.parent.helpText = "This module can be used to repair landmark sets having omissions, or to artificially degrade landmark data, simulating errors"
    self.parent.acknowledgementText = """ """ # replace with organization, grant and thanks.

#
# PreProcessLandmarksWidget
#

class PreProcessLandmarksWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    self.reloadCollapsibleButton.collapsed = 1
    
    # Operations performed by calling self.logic.[operation] - self.SpineModel is for storing self.logic.PatientModel
    
    self.logic = None           # Use class variable to store logic to keep performing operations on same nodes
  
    # Instantiate and connect widgets ...
    
    #
    # Data selection area
    #
    
    #*********Initialize*********#
    
    self.DataInterface = ctk.ctkCollapsibleButton()
    self.DataInterface.text = "Select Data"
    
    self.DataInterfaceLayout = qt.QGridLayout(self.DataInterface)
    self.DataInterfaceLayout.setHorizontalSpacing(12)
    self.layout.addWidget(self.DataInterface)
    
    #*********Configuration*********#
    
    # Checkbox to indicate whether to operate on all MarkupsNodes, or to select one
    self.ModifyAllNodesCheckBox = qt.QCheckBox()
    self.ModifyAllNodesCheckBox.checked = 0
    self.ModifyAllNodesCheckBox.toolTip = "Check if you want to operate on all MarkupsNodes in the scene, uncheck if you want to select one"
    self.DataInterfaceLayout.addWidget(qt.QLabel("All Nodes?"), 0, 0, 1, 1)
    self.DataInterfaceLayout.addWidget(self.ModifyAllNodesCheckBox, 0, 1, 1, 1)
    
    #*********DataSelection*********#
    
    # Single node ComboBox selector
    self.SingleNodeSelector = slicer.qMRMLNodeComboBox()
    self.SingleNodeSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.SingleNodeSelector.enabled = True
    self.SingleNodeSelector.selectNodeUponCreation = True
    self.SingleNodeSelector.addEnabled = False
    self.SingleNodeSelector.removeEnabled = False
    self.SingleNodeSelector.noneEnabled = True
    self.SingleNodeSelector.showHidden = False
    self.SingleNodeSelector.showChildNodeTypes = False
    self.SingleNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.SingleNodeSelector.setToolTip( "Pick the input to the algorithm." )
    self.DataInterfaceLayout.addWidget(qt.QLabel("Node selector"), 0, 2, 1, 1)
    self.DataInterfaceLayout.addWidget(self.SingleNodeSelector, 0, 3, 1, 1)
    
    #*********Connections*********#
    
    self.ModifyAllNodesCheckBox.connect('stateChanged(int)', self.OnAllNodesChecked)
    #self.SingleNodeSelector.connect('currentNodeChanged(bool)', self.OnSelectedNodeChanged)
    self.SingleNodeSelector.connect('currentNodeChanged(bool)', self.OnSelectedAnatomyChange)
    
    #
    # User interface for landmark degradation
    #
    
    #*********Initialize*********#
    
    self.DegradationInterface = ctk.ctkCollapsibleButton()
    self.DegradationInterface.text = "Degrade Landmarks"
    self.DegradationInterface.collapsed = True
    self.DegradationInterface.enabled = True   # Wait for valid markups node(s) selection before enabling repair
    self.layout.addWidget(self.DegradationInterface)
    
    self.DegradationInterfaceLayout = qt.QGridLayout(self.DegradationInterface)
    self.DegradationInterfaceLayout.setHorizontalSpacing(12)
    
    #
    # Embedded config panel
    #
    
    #*********Initialize*********#
    
    self.DegradationConfigInterface = ctk.ctkCollapsibleButton()
    self.DegradationConfigInterface.text = "Configuration"
    self.DegradationConfigInterface.collapsed = True
    self.DegradationInterfaceLayout.addWidget(self.DegradationConfigInterface, 0, 0, 1, 4)
    
    self.DegradationConfigInterfaceLayout = qt.QGridLayout(self.DegradationConfigInterface)
    self.DegradationConfigInterfaceLayout.setHorizontalSpacing(12)
    
    #*********ConfigurationWidgets*********#
    
    # Random noise standard deviation slider
    self.StdDevSlider = ctk.ctkSliderWidget()
    self.StdDevSlider.singleStep = 0.01
    self.StdDevSlider.minimum = 0
    self.StdDevSlider.maximum = 10
    self.StdDevSlider.value = 1
    self.StdDevSlider.setToolTip("Set standard deviation (mm) of noise to introduce to all points.")
    self.DegradationConfigInterfaceLayout.addWidget(qt.QLabel("Noise StdDev (mm^2)"), 1, 0, 1, 1)
    self.DegradationConfigInterfaceLayout.addWidget(self.StdDevSlider, 1, 1, 1, 3)
    
    # Deletion fraction slider
    self.DeletionSlider = ctk.ctkSliderWidget()
    self.DeletionSlider.singleStep = 0.01
    self.DeletionSlider.minimum = 0
    self.DeletionSlider.maximum = 1
    self.DeletionSlider.value = 0.5
    self.DeletionSlider.setToolTip("Set fraction of points from original sets to delete.")
    self.DegradationConfigInterfaceLayout.addWidget(qt.QLabel("Deletion fraction"), 2, 0, 1, 1)
    self.DegradationConfigInterfaceLayout.addWidget(self.DeletionSlider, 2, 1, 1, 3)
    
    # Displacement fraction slider
    self.DisplacementSlider = ctk.ctkSliderWidget()
    self.DisplacementSlider.singleStep = 0.01
    self.DisplacementSlider.minimum = 0
    self.DisplacementSlider.maximum = 1
    self.DisplacementSlider.value = 0.5
    self.DisplacementSlider.setToolTip("Set fraction of points from original data set to misplace.")
    self.DegradationConfigInterfaceLayout.addWidget(qt.QLabel("Displacement fraction"), 3, 0, 1, 1)
    self.DegradationConfigInterfaceLayout.addWidget(self.DisplacementSlider, 3, 1, 1, 3)
    
    # Keep boundary landmarks checkbox
    self.KeepBoundaryPointsBox = qt.QCheckBox()
    self.KeepBoundaryPointsBox.toolTip = "If checked, four outer-most points of spine will not be deleted"
    self.KeepBoundaryPointsBox.enabled = True
    self.DegradationConfigInterfaceLayout.addWidget(qt.QLabel("Keep boundaries"), 4, 0, 1, 1)
    self.DegradationConfigInterfaceLayout.addWidget(self.KeepBoundaryPointsBox, 4, 1, 1, 1)
    
    self.DegradationConfigInterfaceLayout.addWidget(qt.QLabel(""), 5, 0, 1, 1)
    
    #*********Operations*********#
    
    # Add random noise
    self.AddNoiseButton = qt.QPushButton("Add Noise")
    self.AddNoiseButton.toolTip = "Add random noise to each coordinate of each landmark point"
    self.AddNoiseButton.enabled = True
    self.AddNoiseButton.checkable = True
    self.DegradationInterfaceLayout.addWidget(self.AddNoiseButton, 1, 0, 1, 4)
    
    # Delete fraction of landmarks
    self.DeleteLandmarksButton = qt.QPushButton("Delete Landmarks")
    self.DeleteLandmarksButton.toolTip = "Delete a fraction of all landmarks in specified sets"
    self.DeleteLandmarksButton.enabled = True
    self.DeleteLandmarksButton.checkable = True
    self.DegradationInterfaceLayout.addWidget(self.DeleteLandmarksButton, 2, 0, 1, 4)
    
    
    # Displace fraction of landmarks
    self.DisplaceLandmarksButton = qt.QPushButton("Displace Landmarks")
    self.DisplaceLandmarksButton.toolTip = "Systematically displace faraction of all landmarks, aiming from TrP to ribs"
    self.DisplaceLandmarksButton.enabled = True
    self.DisplaceLandmarksButton.checkable = True
    self.DegradationInterfaceLayout.addWidget(self.DisplaceLandmarksButton, 3, 0, 1, 4)
    
    # Apply degradations
    self.ApplyDegradationsButton = qt.QPushButton("Apply Degradations")
    self.ApplyDegradationsButton.toolTip = "Apply selected operations to selected nodes"
    self.ApplyDegradationsButton.enabled = True
    self.DegradationInterfaceLayout.addWidget(self.ApplyDegradationsButton, 5, 0, 1, 4)
    
    #*********Connections*********#
    self.ApplyDegradationsButton.connect('clicked(bool)', self.OnApplyDegradationsClicked)
    
    #
    # User interface for incomplete landmark set repair
    #
    
    #*********Initialize*********#
    
    self.RepairInterface = ctk.ctkCollapsibleButton()
    self.RepairInterface.text = "Repair Landmarks"
    self.RepairInterface.collapsed = True
    self.RepairInterface.enabled = True    # Wait for valid markups node selection before enabling repair
    self.layout.addWidget(self.RepairInterface)
    
    self.RepairInterfaceLayout = qt.QGridLayout(self.RepairInterface)
    self.RepairInterfaceLayout.setHorizontalSpacing(12)
    
    #
    # Embeded configuration panel
    #
    
    #*********Initialize*********#
    self.RepairConfigInterface = ctk.ctkCollapsibleButton()
    self.RepairConfigInterface.text = "Configuration"
    self.RepairConfigInterface.collapsed = True
    self.RepairInterfaceLayout.addWidget(self.RepairConfigInterface, 0, 0, 1, 4)
    
    self.RepairConfigInterfaceLayout = qt.QGridLayout(self.RepairConfigInterface)
    self.RepairConfigInterfaceLayout.setHorizontalSpacing(12)
    
    #*********ConfigurationWidgets*********#
    
    # Number of points used in each iteration of right-left kMeans classification
    self.kmWindowSizeSlider = ctk.ctkSliderWidget()
    self.kmWindowSizeSlider.singleStep = 1
    self.kmWindowSizeSlider.setToolTip("Number of points used in each iteration of k-means for right-left categorization")
    self.kmWindowSizeSlider.enabled = True
    self.kmWindowSizeSlider.minimum = 2
    self.kmWindowSizeSlider.maximum = 12
    self.kmWindowSizeSlider.value = 5
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel("kMeans window size"), 1, 0, 1, 1)
    self.RepairConfigInterfaceLayout.addWidget(self.kmWindowSizeSlider, 1, 1, 1, 3)
    
    # Degree of polynomial to fit to landmark coordinates and landmark spacing
    self.PolyFitDegreeSlider = ctk.ctkSliderWidget()
    self.PolyFitDegreeSlider.singleStep = 1
    self.PolyFitDegreeSlider.setToolTip("Number degrees of the polynomials fit to right and left R-L, A-P coordinates, as well as their curve-wise point frequencies")
    self.PolyFitDegreeSlider.enabled = True
    self.PolyFitDegreeSlider.minimum = 1
    self.PolyFitDegreeSlider.maximum = 10
    self.PolyFitDegreeSlider.value = 5
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel("Polyfit degree"), 2, 0, 1, 1)
    self.RepairConfigInterfaceLayout.addWidget(self.PolyFitDegreeSlider, 2, 1, 1, 3)
    
    # Slider to input the number of times to multiply polynomial boundary points, to control deviations at boundaries
    self.BoundaryMultiplicitySlider = ctk.ctkSliderWidget()
    self.BoundaryMultiplicitySlider.singleStep = 1
    self.BoundaryMultiplicitySlider.setToolTip("Number of times to multiply curve boundary points")
    self.BoundaryMultiplicitySlider.enabled = True
    self.BoundaryMultiplicitySlider.minimum = 1
    self.BoundaryMultiplicitySlider.maximum = 10
    self.BoundaryMultiplicitySlider.value = 1
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel("PolyBoundary Multiplicity"), 3, 0, 1, 1)
    self.RepairConfigInterfaceLayout.addWidget(self.BoundaryMultiplicitySlider, 3, 1, 1, 3)
    
    # Scalar multiplier in comparing landmarks/intervals to gloabal/local measures of normality, higher value requires higher abnormality for abnormality detection
    self.SpecificitySlider = ctk.ctkSliderWidget()
    self.SpecificitySlider.singleStep = 1
    self.SpecificitySlider.setToolTip("Scalar multiplier used to compare interval measures to curve statistics")
    self.SpecificitySlider.enabled = True
    self.SpecificitySlider.minimum = -10
    self.SpecificitySlider.maximum = 10
    self.SpecificitySlider.value = 1
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel("AutoGen specificity"), 4, 0, 1, 1)
    self.RepairConfigInterfaceLayout.addWidget(self.SpecificitySlider, 4, 1, 1, 3)
    
    # Slider widget to affect how stringent the program is in adding new points to interval identified as having omission
    self.ImputationSpecificitySlider = ctk.ctkSliderWidget()
    self.ImputationSpecificitySlider.singleStep = 0.1
    self.ImputationSpecificitySlider.setToolTip("Sets a scalar multiplier used to make program more or less likely to add points to patch")
    self.ImputationSpecificitySlider.enabled = True
    self.ImputationSpecificitySlider.minimum = -10
    self.ImputationSpecificitySlider.maximum = 1
    self.ImputationSpecificitySlider.value = -0.1
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel("Imputation specificity"), 5, 0, 1, 1)
    self.RepairConfigInterfaceLayout.addWidget(self.ImputationSpecificitySlider, 5, 1, 1, 3)
    
    self.RepairConfigInterfaceLayout.addWidget(qt.QLabel(""))

    #*********Connections*********#
    
    self.kmWindowSizeSlider.connect('valueChanged(double)', self.OnkmWindowSliderChanged)
    self.PolyFitDegreeSlider.connect('valueChanged(double)', self.OnPolyFitDegreeSliderChanged)
    self.BoundaryMultiplicitySlider.connect('valueChanged(double)', self.OnBoundaryMultiplicitySliderChanged)
    self.SpecificitySlider.connect('valueChanged(double)', self.OnSpecificitySliderChanged)
    self.ImputationSpecificitySlider.connect('valueChanged(double)', self.OnImputationSpecificitySliderChanged)
    
    #*********DataDisplay*********#
    
    self.LeftSideSelector = slicer.qMRMLNodeComboBox()
    self.LeftSideSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.LeftSideSelector.selectNodeUponCreation = False
    self.LeftSideSelector.enabled  = False
    self.LeftSideSelector.addEnabled = False
    #self.LeftSideSelector.editEnabled = False
    self.LeftSideSelector.noneEnabled = True
    self.LeftSideSelector.removeEnabled = False
    self.LeftSideSelector.renameEnabled = False
    self.LeftSideSelector.toolTip = "Stores the left side of the patient's landmarks being repaired"
    self.LeftSideSelector.setMRMLScene(slicer.mrmlScene)

    self.RightSideSelector = slicer.qMRMLNodeComboBox()
    self.RightSideSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.RightSideSelector.selectNodeUponCreation = False
    self.RightSideSelector.enabled  = False
    self.RightSideSelector.addEnabled = False
    self.RightSideSelector.noneEnabled = True
    self.RightSideSelector.removeEnabled = True
    self.RightSideSelector.renameEnabled = False
    self.RightSideSelector.toolTip = "Stores the right side of the patient's landmarks being repaired"
    self.RightSideSelector.setMRMLScene(slicer.mrmlScene)
    
    self.RepairInterfaceLayout.addWidget(qt.QLabel(" "), 3, 1, 1, 3)
    
    self.RepairInterfaceLayout.addWidget(qt.QLabel("Unrepaired"), 4, 1, 1, 1)
    self.RepairInterfaceLayout.addWidget(qt.QLabel("Tentative patch"), 4, 3, 1, 1)
    
    self.LeftPatchSelector = slicer.qMRMLNodeComboBox()
    self.LeftPatchSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.LeftPatchSelector.selectNodeUponCreation = False
    self.LeftPatchSelector.enabled  = False
    self.LeftPatchSelector.addEnabled = False
    self.LeftPatchSelector.noneEnabled = True
    self.LeftPatchSelector.removeEnabled = True
    self.LeftPatchSelector.renameEnabled = False
    self.LeftPatchSelector.toolTip = "Stores the landmarks node with which the module will patch Unrepaired Left" 
    self.LeftPatchSelector.setMRMLScene(slicer.mrmlScene)

    self.RepairInterfaceLayout.addWidget(qt.QLabel("Left"), 5, 0, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.LeftSideSelector, 5, 1, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.LeftPatchSelector, 5, 3, 1, 1)

    self.RightPatchSelector = slicer.qMRMLNodeComboBox()
    self.RightPatchSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.RightPatchSelector.selectNodeUponCreation = False
    self.RightPatchSelector.enabled  = False
    self.RightPatchSelector.addEnabled = False
    self.RightPatchSelector.noneEnabled = True
    self.RightPatchSelector.removeEnabled = True
    self.RightPatchSelector.renameEnabled = False
    self.RightPatchSelector.toolTip = "Stores the landmarks node with which the module will patch Unrepaired Right" 
    self.RightPatchSelector.setMRMLScene(slicer.mrmlScene)
    
    self.RepairInterfaceLayout.addWidget(qt.QLabel("Right"), 6, 0, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.RightSideSelector, 6, 1, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.RightPatchSelector, 6, 3, 1, 1)
  
    self.RepairInterfaceLayout.addWidget(qt.QLabel(" "), 7, 1, 1, 3)
   
    self.RepairSideSelector = slicer.qMRMLNodeComboBox()
    self.RepairSideSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.RepairSideSelector.selectNodeUponCreation = False
    self.RepairSideSelector.enabled  = True
    self.RepairSideSelector.addEnabled = False
    self.RepairSideSelector.noneEnabled = True
    self.RepairSideSelector.removeEnabled = True
    self.RepairSideSelector.renameEnabled = False
    self.RepairSideSelector.toolTip = "Use to select the node containing the spine side to have a patch of points generated"
    self.RepairSideSelector.setMRMLScene(slicer.mrmlScene)
    self.RepairInterfaceLayout.addWidget(qt.QLabel("Make patch for:"), 8, 0, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.RepairSideSelector, 8, 1, 1, 1)
    
    #*********Operations*********#
    
    self.CategorizeLeftRightButton = qt.QPushButton("Categorize left-right landmarks")
    self.CategorizeLeftRightButton.toolTip = "Groups selected nodes landmarks into left and right landmark nodes used for repair operations"
    self.CategorizeLeftRightButton.enabled = True
    self.RepairInterfaceLayout.addWidget(self.CategorizeLeftRightButton, 2, 1, 1, 3)
    
    self.AutoGenPatchButton = qt.QPushButton("AutoGen Patch")
    self.AutoGenPatchButton.toolTip = "Guess missing landmarks"
    self.AutoGenPatchButton.enabled = False
    self.RepairInterfaceLayout.addWidget(self.AutoGenPatchButton, 8, 2, 1, 1)
    
    self.ApplyPatchButton = qt.QPushButton("Apply Patch")
    self.ApplyPatchButton.toolTip = "Replace selected side node with itself merged with its patch"
    self.ApplyPatchButton.enabled = False
    self.RepairInterfaceLayout.addWidget(self.ApplyPatchButton, 8, 3, 1, 1)
    
    self.SubIntervalSelector = qt.QSpinBox()
    self.SubIntervalSelector.minimum = 0
    self.SubIntervalSelector.maximum = 10
    self.SubIntervalSelector.singleStep = 1
    self.RepairInterfaceLayout.addWidget(qt.QLabel("SubPatch:"), 10, 0, 1, 1)
    self.RepairInterfaceLayout.addWidget(self.SubIntervalSelector, 10, 1, 1, 1)
    
    self.AddPointToPatchButton = qt.QPushButton("Add Point")
    self.AddPointToPatchButton.toolTip = "Add one point to SubPatch"
    self.AddPointToPatchButton.enabled = False
    self.RepairInterfaceLayout.addWidget(self.AddPointToPatchButton, 10, 2, 1, 1)

    self.RemovePointButton = qt.QPushButton("Remove Point")
    self.RemovePointButton.toolTip = "Remove one point to SubPatch"
    self.RemovePointButton.enabled = False
    self.RepairInterfaceLayout.addWidget(self.RemovePointButton, 10, 3, 1, 1)    
   
    self.RepairInterfaceLayout.addWidget(qt.QLabel(" "), 11, 1, 1, 3)
   
    """
    self.UndoButton = qt.QPushButton("Undo")
    self.UndoButton.toolTip = "Undo up to" + str(self.MaxUndos) + " operations"
    self.UndoButton.enabled = True
    self.RepairInterfaceLayout.addWidget(self.UndoButton, 12, 0, 1, 4)
    """
   
    self.IdentifyOutliersButton = qt.QPushButton("Identify Outliers")
    self.IdentifyOutliersButton.toolTip = "Highlight apparent outlier points in selected side"
    self.IdentifyOutliersButton.enabled = True
    self.RepairInterfaceLayout.addWidget(self.IdentifyOutliersButton, 12, 0, 1, 2)
    
    self.RemoveOutliersButton = qt.QPushButton("Remove Outliers")
    self.RemoveOutliersButton.toolTip = "Remove points highlighted as outliers by Remove Outliers"
    self.RemoveOutliersButton.enabled = True
    self.RepairInterfaceLayout.addWidget(self.RemoveOutliersButton, 12, 2, 1, 2)
   
    self.RepairInterfaceLayout.addWidget(qt.QLabel(""), 13, 1, 1, 1)
    
    self.RepairNodeButton = qt.QPushButton("Repair landmarks node")
    self.RepairNodeButton.toolTip = "Combine left and right Unrepaired side nodes into RepairedNode"
    self.RepairNodeButton.enabled = True
    self.RepairInterfaceLayout.addWidget(self.RepairNodeButton, 14, 0, 1, 4)
    
    # Dropdown list to store output repaired MarkupsNode
    self.RepairedNodeStorage = slicer.qMRMLNodeComboBox()
    self.RepairedNodeStorage.nodeTypes = ["vtkMRMLMarkupsFiducialNode",]
    self.RepairedNodeStorage.selectNodeUponCreation = False
    self.RepairedNodeStorage.enabled  = True
    self.RepairedNodeStorage.addEnabled = False
    self.RepairedNodeStorage.noneEnabled = True
    self.RepairedNodeStorage.removeEnabled = True
    self.RepairedNodeStorage.renameEnabled = False
    self.RepairedNodeStorage.toolTip = "Stores combined repaired left and right sides - repaired version of input repair node"
    self.RepairInterfaceLayout.addWidget(self.RepairedNodeStorage, 15, 0, 1, 3)
    self.RepairInterfaceLayout.addWidget(qt.QLabel(" <-- Repaired node"), 15, 3, 1, 1)
    self.RepairedNodeStorage.setMRMLScene(slicer.mrmlScene)
    
    #self.layout.addRow(self.reloadButton, 16, 0, 1, 4)
    
    # Button connections
    
    self.CategorizeLeftRightButton.connect('clicked(bool)', self.OnCategorizeLeftRight)
    
    self.AutoGenPatchButton.connect('clicked(bool)', self.OnAutoGenPatchButton)
    self.AutoGenPatchButton.connect('clicked(bool)', self.OnLeftRightBasePatchChange)
    
    self.ApplyPatchButton.connect('clicked(bool)', self.OnApplyPatchButton)
    
    self.AddPointToPatchButton.connect('clicked(bool)', self.OnAddPointToPatchButton)
    self.RemovePointButton.connect('clicked(bool)', self.OnRemovePointButton)
    self.RemovePointButton.connect('clicked(bool)', self.OnLeftRightBasePatchChange)
    
    self.IdentifyOutliersButton.connect('clicked(bool)', self.OnIdentifyOutliersButton)
    self.RemoveOutliersButton.connect('clicked(bool)', self.OnRemoveOutliersButton)
    
    self.RepairNodeButton.connect('clicked(bool)', self.OnRepairButtonClicked)
    
    
    #self.UndoButton.connect('clicked(bool)', self.OnUndoButtonClicked)
    
    # Node interface connections
    #self.AnatomySelector.connect('currentNodeChanged(bool)', self.OnSelectedAnatomyChange)
    #self.LeftSideSelector.connect('currentNodeChanged(bool)', self.OnLeftRightBasePatchChange)
    #self.RightSideSelector.connect('currentNodeChanged(bool)', self.OnLeftRightBasePatchChange)
    #self.LeftPatchSelector.connect('currentNodeChanged(bool)', self.OnLeftRightBasePatchChange)
    #self.RightPatchSelector.connect('currentNodeChanged(bool)', self.OnLeftRightBasePatchChange)
    #self.RepairSideSelector.connect('currentNodeChanged(bool)', self.OnSelectedSideChange)
    self.RepairSideSelector.connect('currentNodeChanged(bool)', self.OnLeftRightBasePatchChange)
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked(bool)', self.OnReloadButton)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
  def cleanup(self):
    AllMarkupsNodes = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for Node in AllMarkupsNodes:
      if Node.GetName().__contains__("_Right") or Node.GetName().__contains__("_Left") or Node.GetName().__contains__("_Patch"):
        slicer.mrmlScene.RemoveNode(Node)

  #
  # Data interface functions
  #
  
  def OnAllNodesChecked(self):
    self.SingleNodeSelector.setMRMLScene(slicer.mrmlScene)
    self.SingleNodeSelector.enabled = not self.ModifyAllNodesCheckBox.isChecked()
    
    self.RepairInterface.collapsed = self.ModifyAllNodesCheckBox.isChecked() or self.RepairInterface.collapsed
    self.RepairInterface.enabled = (not self.ModifyAllNodesCheckBox.isChecked()) and (self.SingleNodeSelector.currentNode() != None)
    
    self.DegradationInterface.collapsed = self.ModifyAllNodesCheckBox.isChecked() or self.DegradationInterface.collapsed or self.SingleNodeSelector.currentNode() == None
    self.DegradationInterface.enabled = self.ModifyAllNodesCheckBox.isChecked() or self.SingleNodeSelector.currentNode() != None
    #self.PerformOperationsButton.enabled = (self.SingleNodeSelector.currentNode() != None) or (self.ModifyAllNodesCheckBox.isChecked())

  #
  # Landmark degradation functions
  #
  
  def OnApplyDegradationsClicked(self):
    # Store one or all nodes, depending on ModifyAllNodesCheckBox
    if self.ModifyAllNodesCheckBox.isChecked():
      Nodes = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    else:
      Nodes = [self.SingleNodeSelector.currentNode()]
      
    logic = DegradeLandmarksLogic(Nodes, self.AddNoiseButton.checked, self.StdDevSlider.value, self.DeleteLandmarksButton.checked, self.DeletionSlider.value, self.DisplaceLandmarksButton.checked, self.DisplacementSlider.value, self.KeepBoundaryPointsBox.isChecked())
    logic.PerformOperations()
    
  # 
  # Landmark repair functions
  #

  # Config interface - functions allow user to update program parameters anytime
  
  def OnkmWindowSliderChanged(self):
    #NewKMeansWindowSize = self.kmWindowSizeSlider.value
    #self.kmWindowTextDisplay.setText(str(NewKMeansWindowSize))
    if self.logic != None:
      #self.logic.PatientModel.kmWindowSize = NewKMeansWindowSize
      self.OnSelectedAnatomyChange()
      if self.LeftPatchSelector.currentNode() != None or self.RightPatchSelector.currentNode() != None:   # Check should prevent unwanted patch generation by name check in OnAutoGenPatchButton
        self.OnAutoGenPatchButton()
    return
  
  
  def OnPolyFitDegreeSliderChanged(self):
    #NewPolyFitDegree = self.PolyFitDegreeSlider.value
    #self.PolyFitDegreeSliderTextDisplay.setText(str(NewPolyFitDegree))
    if self.logic != None:
      self.OnSelectedAnatomyChange()
      if self.LeftPatchSelector.currentNode() != None or self.RightPatchSelector.currentNode() != None:   # Check should prevent unwanted patch generation by name check in OnAutoGenPatchButton
        self.OnAutoGenPatchButton()
      #self.logic.PatientModel.PolyFitDegree = NewPolyFitDegree
      #self.logic.PatientModel.RightSide.PolyFitDegree = NewPolyFitDegree
      #self.logic.PatientModel.LeftSide.PolyFitDegree = NewPolyFitDegree
    return
  
  def OnBoundaryMultiplicitySliderChanged(self):
    #NewBoundaryMultiplicity = self.BoundaryMultiplicitySlider.value
    if self.logic != None:
      self.OnSelectedAnatomyChange()
      if self.LeftPatchSelector.currentNode() != None or self.RightPatchSelector.currentNode() != None:   # Check should prevent unwanted patch generation by name check in OnAutoGenPatchButton
        self.OnAutoGenPatchButton()
      #self.logic.PatientModel.BoundaryMultiplicity = NewBoundaryMultiplicity

  def OnSpecificitySliderChanged(self):
    #NewOmissionSpecificity = self.SpecificitySlider.value
    #self.SpecificityTextDisplay.setText(str(NewOmissionSpecificity))
    if self.logic != None:
      self.OnSelectedAnatomyChange() 
      if self.LeftPatchSelector.currentNode() != None or self.RightPatchSelector.currentNode() != None:   # Check should prevent unwanted patch generation by name check in OnAutoGenPatchButton
        self.OnAutoGenPatchButton()
      #self.logic.PatientModel.OmissionDetectionSpecificity = NewOmissionSpecificity
      #self.logic.PatientModel.RightSide.OmissionDetectionSpecificity = NewOmissionSpecificity
      #self.logic.PatientModel.LeftSide.OmissionDetectionSpecificity = NewOmissionSpecificity
    #print str(self.SpecificitySlider.value)
    return
    
  def OnImputationSpecificitySliderChanged(self):
    #NewImputationSpecificity = self.ImputationSpecificitySlider.value
    #self.SpecificityTextDisplay.setText(str(NewImputationSpecificity))
    if self.logic != None:
      self.OnSelectedAnatomyChange() 
      if self.LeftPatchSelector.currentNode() != None or self.RightPatchSelector.currentNode() != None:   # Check should prevent unwanted patch generation by name check in OnAutoGenPatchButton
        self.OnAutoGenPatchButton()
      #self.logic.PatientModel.ImputationSpecificity = NewImputationSpecificity
      #self.logic.PatientModel.RightSide.ImputationSpecificity = NewImputationSpecificity
      #self.logic.PatientModel.LeftSide.ImputationSpecificity = NewImputationSpecificity
    #print str(self.SpecificitySlider.value)
    return
    
  # Operations
  
  def OnSelectedAnatomyChange(self):
    print "New anatomy selected"
    SpineNode = self.SingleNodeSelector.currentNode()
    #SpineNode =  self.
    if SpineNode != None:
      if self.logic == None:
        self.logic = RepairLandmarksLogic(SpineNode, self.kmWindowSizeSlider.value, self.PolyFitDegreeSlider.value, self.BoundaryMultiplicitySlider.value, self.SpecificitySlider.value, self.ImputationSpecificitySlider.value)
      #self.logic = RepairLandmarksLogic(SpineNode, self.kmWindowSizeSlider.value, self.PolyFitDegreeSlider.value, self.BoundaryMultiplicitySlider.value, self.SpecificitySlider.value, self.ImputationSpecificitySlider.value)
      #self.SpecificitySlider.enabled = True
      #self.SpineModel = self.logic.PatientModel
      #self.CategorizeLeftRightButton.enabled = True
      else:
        self.logic.UpdateParameters(self.kmWindowSizeSlider.value, self.PolyFitDegreeSlider.value, self.BoundaryMultiplicitySlider.value, self.SpecificitySlider.value, self.ImputationSpecificitySlider.value)
      self.OnCategorizeLeftRight()
    else:
      print ""
      print "Selected anatomy not recognized as valid - vtkMRMLMarkupsFiducialNode required"
      print ""
      #self.CategorizeLeftRightButton.enabled = False
      
    self.UpdateColors()
  
  def OnLeftRightBasePatchChange(self):
  
    # Get node selectors' contents, even if they are None
    LeftBaseNode = self.LeftSideSelector.currentNode()
    RightBaseNode = self.RightSideSelector.currentNode()
    LeftPatchNode = self.LeftPatchSelector.currentNode()
    RightPatchNode = self.RightPatchSelector.currentNode()
    NodeForRepair = self.RepairSideSelector.currentNode()
    
    if NodeForRepair == None:   # Prevents other checks which could be None == None
      #self.AutoGenPatchButton.enabled = False
      #self.ApplyPatchButton.enabled = False
      #self.AddPointToPatchButton.enabled = False
      #self.RemovePointButton.enabled = False
      return    
    
    # Having a side node selected for patch generation allows patch generation, and then patch modification
    if NodeForRepair == LeftBaseNode:
      self.AutoGenPatchButton.enabled = True
      if LeftPatchNode!= None and LeftPatchNode.GetName() == LeftBaseNode.GetName() + "_Patch":   # IF we already have a patch for it, make a few more operations availible
        self.ApplyPatchButton.enabled = True
        self.AddPointToPatchButton.enabled = True
        self.RemovePointButton.enabled = True
        if LeftPatchNode.GetNumberOfFiducials() + NodeForRepair.GetNumberOfFiducials() > 17:
          print ""
          print "Too many points in Unrepaired and Patch node union"
          print " Can have 17 points maximum"
          print " Remove some points from the patch or unrepaired node"
          #self.ApplyPatchButton.enabled = False
        
        
    if NodeForRepair == RightBaseNode:
      self.AutoGenPatchButton.enabled = True
      if RightPatchNode != None and RightPatchNode.GetName() == RightBaseNode.GetName() + "_Patch":
        self.ApplyPatchButton.enabled = True
        self.AddPointToPatchButton.enabled = True
        self.RemovePointButton.enabled = True
        if RightPatchNode.GetNumberOfFiducials() + NodeForRepair.GetNumberOfFiducials() > 17:
          print ""
          print "Too many points in Unrepaired and Patch node union"
          print " Can have 17 points maximum"
          print " Remove some points from the patch or unrepaired node"
          #self.ApplyPatchButton.enabled = False
    
    # If we have a left and right side, we can combine them in a RepairedNode
    if LeftBaseNode != None and RightBaseNode != None:
      self.RepairNodeButton.enabled = True
    else:
      self.RepairNodeButton.enabled = False
    
    self.UpdateColors()
    
  def OnCategorizeLeftRight(self):
    
    # Check input data
    if self.SingleNodeSelector.currentNode() == None:
      print "Error - require MarkupsNode to segment into left and right sides"
      return
    
    
    # Following logic check may be un-necessary as it is called when Widget.SingleNodeSelector.currentNode() changes
    if self.logic == None:
      self.logic = RepairLandmarksLogic(self.SingleNodeSelector.currentNode(), self.kmWindowSizeSlider.value, self.PolyFitDegreeSlider.value, self.BoundaryMultiplicitySlider.value, self.SpecificitySlider.value, self.ImputationSpecificitySlider.value)
      #self.SpecificitySlider.enabled = True
    else:
      self.logic.UpdateParameters(self.kmWindowSizeSlider.value, self.PolyFitDegreeSlider.value, self.BoundaryMultiplicitySlider.value, self.SpecificitySlider.value, self.ImputationSpecificitySlider.value)
    # Get Left and Right side markups nodes
    (LeftNode, RightNode) = self.logic.PatientModel.ClassifyLeftRight()
    
    # Update scene
    PriorLeft = slicer.util.getNode(LeftNode.GetName())
    if PriorLeft != None:
      self.LeftSideSelector.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(PriorLeft)
    
    PriorRight = slicer.util.getNode(RightNode.GetName())
    if PriorRight != None:
      self.RightSideSelector.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(PriorRight)    
    
    # Add new nodes to scene
    slicer.mrmlScene.AddNode(LeftNode)
    slicer.mrmlScene.AddNode(RightNode)
    
    LeftNode.CreateDefaultDisplayNodes()
    RightNode.CreateDefaultDisplayNodes()
    
    # Update user interface
    self.LeftSideSelector.setCurrentNode(LeftNode)
    self.RightSideSelector.setCurrentNode(RightNode)
    
    PriorSelectedSideNode = self.RepairSideSelector.currentNode()
    if PriorSelectedSideNode != None:
      self.RepairSideSelector.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(PriorSelectedSideNode)
      
      if PriorSelectedSideNode.GetName().__contains__("_Left"):
        self.RepairSideSelector.setCurrentNode(LeftNode)
      if PriorSelectedSideNode.GetName().__contains__("_Right"):
        self.RepairSideSelector.setCurrentNode(RightNode)
      
    
    self.UpdateColors()
    
  def OnAutoGenPatchButton(self):
    
    # Probably should necessitate continuity of data on the user's end
    #if self.logic == None:
    #  self.logic = PreProcessLandmarksLogic(self.SingleNodeSelector.currentNode(), self.PolynomialDegree)
    #  (LS, RS) = self.logic.CategorizeLeftRight(4)
    
    NodeToPatch = self.RepairSideSelector.currentNode()
    OldPatchNode = slicer.util.getNode(NodeToPatch.GetName() + "_Patch")
    if OldPatchNode != None:
      slicer.mrmlScene.RemoveNode(OldPatchNode)
    
    if NodeToPatch.GetName().__contains__("Left"):
      # Get patch
      PatchNode = self.logic.PatientModel.LeftSide.GeneratePatchNode()
      
      # Update scene
      slicer.mrmlScene.AddNode(PatchNode)
      PatchNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.LeftPatchSelector.setCurrentNode(PatchNode)
      
    elif NodeToPatch.GetName().__contains__("Right"):
      # Get patch
      PatchNode = self.logic.PatientModel.RightSide.GeneratePatchNode()
      
      # Update scene
      slicer.mrmlScene.AddNode(PatchNode)
      PatchNode.CreateDefaultDisplayNodes()
      
      # Update self class variables
      self.RightPatchSelector.setCurrentNode(PatchNode)
    
    # Update side-independent UI 
    self.UpdateColors()
    
  def OnApplyPatchButton(self):
    
    # Get node selectors' contents, even if they are None
    NodeToPatch = self.RepairSideSelector.currentNode()
    #self.StackSaveNode(NodeToPatch)
    LeftNode = self.LeftSideSelector.currentNode()
    LeftPatchNode = self.LeftPatchSelector.currentNode()
    RightNode = self.RightSideSelector.currentNode()
    RightPatchNode = self.RightPatchSelector.currentNode()
    
    if NodeToPatch == LeftNode and LeftPatchNode != None:
      
      # Get side to patch 
      LeftSide = self.logic.PatientModel.LeftSide
      
      # Get side with patch applied
      LeftSide.CombineNodeWithPatch(LeftSide.PatchNode)
      
      self.LeftPatchSelector.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(LeftPatchNode)
      PatchedLeftNode = slicer.vtkMRMLMarkupsFiducialNode()
      PatchedLeftNode.Copy(LeftSide.MarkupsNode)
      
      # Update scene
      slicer.mrmlScene.AddNode(PatchedLeftNode)
      PatchedLeftNode.CreateDefaultDisplayNodes()
      self.LeftSideSelector.setCurrentNode(PatchedLeftNode)
      slicer.mrmlScene.RemoveNode(LeftNode)
      PatchedLeftNode.SetName(LeftNode.GetName())
      #LeftNode = self.LeftSideSelector.currentNode()
      
      
      # SpineSide.CombineNodeWithPatch does not currently update self.MarkupsNode as the function is used iteratively in searching for best subpatches
      # TODO: Fix ^
      #self.logic.PatientModel.LeftSide.MarkupsNode.Copy(PatchedLeftNode)
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(PatchedLeftNode)
    
    if NodeToPatch == RightNode and RightPatchNode != None:
      
      # Get side to patch
      RightSide = self.logic.PatientModel.RightSide
      
      # Get side with patch applied
      RightSide.CombineNodeWithPatch(RightSide.PatchNode)
      
      self.RightPatchSelector.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(RightPatchNode)
      self.logic.PatientModel.RightSide.PatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      
      PatchedRightNode = slicer.vtkMRMLMarkupsFiducialNode()
      PatchedRightNode.Copy(RightSide.MarkupsNode)
      
      # Update scene
      slicer.mrmlScene.AddNode(PatchedRightNode)
      PatchedRightNode.CreateDefaultDisplayNodes()
      self.RightSideSelector.setCurrentNode(PatchedRightNode)
      slicer.mrmlScene.RemoveNode(RightNode)
      PatchedRightNode.SetName(RightNode.GetName())
      
      # SpineSide.CombineNodeWithPatch does not currently update self.MarkupsNode as the function is used iteratively in searching for best subpatches
      # TODO: Fix ^
      #self.logic.PatientModel.RightSide.MarkupsNode.Copy(PatchedRightNode)
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(PatchedRightNode)

    # Disable the button, we used the patch and need a new one
    #self.ApplyPatchButton.enabled = False
      
    self.UpdateColors()
  
  
  def OnAddPointToPatchButton(self):
  
    # Get sub-patch to add point to 
    SubIntervalNumber = self.SubIntervalSelector.value
    
    # Get the patch node having a point added
    NodeUnderRepair = self.RepairSideSelector.currentNode()
    OldPatchName = NodeUnderRepair.GetName() + "_Patch"
    OldPatchNode = slicer.util.getNode(OldPatchName)
    
    if NodeUnderRepair.GetNumberOfFiducials() + OldPatchNode.GetNumberOfFiducials() >= 17:
      print ""
      print "Cannot add any more points to patch"
      print " Patch and unrepaired node together would have over 17 points"
      print " Remove some points from the patch or repair node first"
      print " Adding no points"
      return
    
    if OldPatchName.__contains__("Left"):
      
      # Get patch with point added
      SideUnderRepair = self.logic.PatientModel.LeftSide
      NewPatchNode = SideUnderRepair.AddPointToSubPatch(SubIntervalNumber)
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldPatchNode)
      slicer.mrmlScene.AddNode(NewPatchNode)
      NewPatchNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.LeftPatchSelector.setCurrentNode(NewPatchNode)
      
    else:
      
      # Get patch with point added
      SideUnderRepair = self.logic.PatientModel.RightSide
      NewPatchNode = SideUnderRepair.AddPointToSubPatch(SubIntervalNumber)
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldPatchNode)
      slicer.mrmlScene.AddNode(NewPatchNode)
      NewPatchNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RightPatchSelector.setCurrentNode(NewPatchNode)

    self.UpdateColors() 
    
    return
    
  def OnRemovePointButton(self):
    
    # Get sub-patch to add point to 
    SubIntervalNumber = self.SubIntervalSelector.value
    NodeUnderRepair = self.RepairSideSelector.currentNode()
    OldPatchName = NodeUnderRepair.GetName() + "_Patch"
    OldPatchNode = slicer.util.getNode(OldPatchName)
    
    if OldPatchName.__contains__("Left"):
      
      # Get patch with point added
      SideUnderRepair = self.logic.PatientModel.LeftSide
      NewPatchNode = SideUnderRepair.RemovePointFromSubPatch(SubIntervalNumber)
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldPatchNode)
      slicer.mrmlScene.AddNode(NewPatchNode)
      NewPatchNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.LeftPatchSelector.setCurrentNode(NewPatchNode)
      
    else:
    
      # Get patch with point added
      SideUnderRepair = self.logic.PatientModel.RightSide
      NewPatchNode = SideUnderRepair.RemovePointFromSubPatch(SubIntervalNumber)
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldPatchNode)
      slicer.mrmlScene.AddNode(NewPatchNode)
      NewPatchNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RightPatchSelector.setCurrentNode(NewPatchNode)
     
    self.UpdateColors() 
    
    return
    
  def OnIdentifyOutliersButton(self):
    
    # Get node to be searched for outliers
    OldWorkingNode = self.RepairSideSelector.currentNode()
    
    if OldWorkingNode.GetName().__contains__("Left"):
      # Get node with suspected outliers highlighted (unselected)
      NewWorkingNode = self.logic.PatientModel.LeftSide.IdentifyOutliers()
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldWorkingNode)
      #OldWorkingNode.Copy(NewWorkingNode)
      slicer.mrmlScene.AddNode(NewWorkingNode)
      NewWorkingNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(NewWorkingNode)
      self.LeftSideSelector.setCurrentNode(NewWorkingNode)
      
    else:
      # Get node with suspected outliers highlighted (unselected)
      NewWorkingNode = self.logic.PatientModel.RightSide.IdentifyOutliers()
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldWorkingNode)
      #OldWorkingNode.Copy(NewWorkingNode)
      slicer.mrmlScene.AddNode(NewWorkingNode)
      NewWorkingNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(NewWorkingNode)
      self.RightSideSelector.setCurrentNode(NewWorkingNode)

    self.UpdateColors()
    
    return
    
  def OnRemoveOutliersButton(self):
    # Get node to be searched for outliers
    OldWorkingNode = self.RepairSideSelector.currentNode()
    
    if OldWorkingNode.GetName().__contains__("Left"):
      # Get node with suspected outliers highlighted (unselected)
      NewWorkingNode = self.logic.PatientModel.LeftSide.DeleteIdentifiedOutliers()
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldWorkingNode)
      #OldWorkingNode.Copy(NewWorkingNode)
      slicer.mrmlScene.AddNode(NewWorkingNode)
      NewWorkingNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(NewWorkingNode)
      self.LeftSideSelector.setCurrentNode(NewWorkingNode)
      
    else:
      # Get node with suspected outliers highlighted (unselected)
      NewWorkingNode = self.logic.PatientModel.RightSide.DeleteIdentifiedOutliers()
      
      # Update scene
      slicer.mrmlScene.RemoveNode(OldWorkingNode)
      #OldWorkingNode.Copy(NewWorkingNode)
      slicer.mrmlScene.AddNode(NewWorkingNode)
      NewWorkingNode.CreateDefaultDisplayNodes()
      
      # Update UI
      self.RepairSideSelector.setCurrentNode(NewWorkingNode)
      self.RightSideSelector.setCurrentNode(NewWorkingNode)

    self.UpdateColors()

    return
    
  def OnRepairButtonClicked(self):  # Combines self.LeftSideSelector and self.RightSideSelector nodes into a node stored in self.RepairedNodeStorage
  
    # Get left and right combined into repaired node
    ModelRepairedNode = self.logic.PatientModel.CombineRepairedSides()
    #slicer.mrmlScene.AddNode(RepairedNode)
    
    #RepairedNode = self.logic.ApplyNamingConvention(ModelRepairedNode)
    RepairedNode = slicer.vtkMRMLMarkupsFiducialNode()
    for P in range(ModelRepairedNode.GetNumberOfFiducials()):
      RepairedNode.AddFiducialFromArray(ModelRepairedNode.GetMarkupPointVector(P,0))
      RepairedNode.SetNthFiducialLabel(P, ModelRepairedNode.GetNthFiducialLabel(P))

    slicer.mrmlScene.AddNode(RepairedNode)
    RepairedNode.CreateDefaultDisplayNodes()

    # Update mrmlScene
    OldRepairedNode = self.RepairedNodeStorage.currentNode()
    if OldRepairedNode != None:
      self.RepairedNodeStorage.setCurrentNode(None)
      slicer.mrmlScene.RemoveNode(OldRepairedNode)
    
    # Impose naming convention required by ModelToPatientRegistration extension
    if RepairedNode.GetNumberOfFiducials() == 34:
      # Update UI
      RepairedNode.SetName(ModelRepairedNode.GetName() + "_Repaired")
      self.RepairedNodeStorage.setCurrentNode(RepairedNode)

    else: # Repair button was clicked before all points were present, as part of intermediate operation
      RepairedNode.SetName(ModelRepairedNode.GetName())
      if self.SingleNodeSelector.currentNode() != None:
        self.SingleNodeSelector.setCurrentNode(None)
        slicer.mrmlScene.RemoveNode(self.SingleNodeSelector.currentNode())
      #self.SingleNodeSelector.setCurrentNode(RepairedNode)
      
    # Remove sides and patch nodes
    AllMarkupsNodes = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for Node in AllMarkupsNodes:
      if Node.GetName().__contains__("_Left") or Node.GetName().__contains__("_Right") or Node.GetName().__contains__("_Patch"):
        slicer.mrmlScene.RemoveNode(Node)
    
    self.UpdateColors()
      
    return
    
  def UpdateColors(self):
    # Input set
    MainNode = self.SingleNodeSelector.currentNode()
    if MainNode != None:
      MainNode.GetDisplayNode().SetSelectedColor(1,0,1)
    
    # Both unrepaired sides
    (LeftUnrep, RightUnrep) = (self.LeftSideSelector.currentNode(), self.RightSideSelector.currentNode())
    if LeftUnrep != None:
      LeftUnrep.GetDisplayNode().SetSelectedColor(0,0,1)
    if RightUnrep != None:
      RightUnrep.GetDisplayNode().SetSelectedColor(0,0,1)
      
    # Node selected for patching
    NodeToPatch = self.RepairSideSelector.currentNode()
    if NodeToPatch != None:
      NodeToPatch.GetDisplayNode().SetSelectedColor(1,0,0)
      
    # Selected patches
    (LeftPatchNode, RightPatchNode) = (self.LeftPatchSelector.currentNode(), self.RightPatchSelector.currentNode())
    if LeftPatchNode != None:
      LeftPatchNode.GetDisplayNode().SetSelectedColor(1,1,0)
    if RightPatchNode != None:
      RightPatchNode.GetDisplayNode().SetSelectedColor(1,1,0)    
    
    # Repaired node
    RepairedNode = self.RepairedNodeStorage.currentNode()
    if RepairedNode != None:
      RepairedNode.GetDisplayNode().SetSelectedColor(0,1,0)
    
  def OnReloadButton(self):
    self.logic = None
    #slicer.mrmlScene.Clear(0)
    #self.SpecificitySlider.enabled = False
    print str(slicer.moduleNames.PreProcessLandmarks) + " reloaded"
    self.cleanup()
    slicer.util.reloadScriptedModule(slicer.moduleNames.PreProcessLandmarks)
    
#
# DegradeLandmarksLogic
#
    
class DegradeLandmarksLogic(ScriptedLoadableModuleLogic):
  def __init__(self, Nodes, DoAddNoise=False, NoiseStdDev=0.0, DoDeletePoints=False, DeletionFraction=0.0, DoDisplacePoints=False, DisplacementFraction=0.0, DoKeepBoundaries=False):
    self.MarkupsNodes = Nodes     # Expecting a list of vtkMRMLMarkupsFiducialNode for Nodes, apply operations to each node
    
    # Parameters from user interface
    self.DoAddNoise = DoAddNoise
    self.NoiseStdDev = NoiseStdDev
    
    self.DoDeletePoints = DoDeletePoints
    self.DeletionFraction = DeletionFraction
    
    self.DoDisplacePoints = DoDisplacePoints
    self.DisplacementFraction = DisplacementFraction
    
    self.DoKeepBoundaries = DoKeepBoundaries
    
  # Add noise to point locations 
  #
  # MAKE SURE YOU CORRECTLY DESCRIBE THIS NOISE'S STATISTICS
  #
  def AddNoise(self): 
    for MarkupsNode in self.MarkupsNodes:     # For each markup node present
      print "Adding noise to ", MarkupsNode.GetName(), " landmarks"
      LabelsCoords = [[MarkupsNode.GetNthFiducialLabel(i), MarkupsNode.GetMarkupPointVector(i,0)] for i in range(MarkupsNode.GetNumberOfFiducials())]
            
      # Add noise to coords
      for i, (Label, Coords) in enumerate(LabelsCoords):    # For each point of this markup node
        RandomNoise = np.random.normal([0.0,0.0,0.0], [self.NoiseStdDev,self.NoiseStdDev,self.NoiseStdDev])
        LabelsCoords[i][1] = [(Coords[dim] + RandomNoise[dim]) for dim in range(3)]
   
      # Update MarkupsNode landmarks
      MarkupsNode.RemoveAllMarkups()
      for i, (Label, Coords) in enumerate(LabelsCoords):
        MarkupsNode.AddFiducialFromArray(Coords)
        MarkupsNode.SetNthFiducialLabel(i, Label)
        
    return True
    

  def DisplacePoints(self):   # Assumed to be run before DeletePoints, so all info is available to compute displacement vectors
    self.PointsJustDisplaced = len(self.MarkupsNodes) * [0]
    for j, MarkupsNode in enumerate(self.MarkupsNodes):     # For each markup node present
      self.PointsJustDisplaced[j] = MarkupsNode.GetNumberOfFiducials() * [0] #np.zeros(MarkupsNode.GetNumberOfFiducials())
      print "Displacing ", MarkupsNode.GetName(), " landmarks"
      
      LabelsCoords = [[MarkupsNode.GetNthFiducialLabel(i), MarkupsNode.GetMarkupPointVector(i,0)] for i in range(MarkupsNode.GetNumberOfFiducials())]
      DisplacementAmount = int(self.DisplacementFraction * len(LabelsCoords))
      
      # Select points for displacement and compute their new locations
      while DisplacementAmount > 0:
        RandomIndex = int(np.random.uniform(0, len(LabelsCoords)))
        
        # Make sure point at RandomIndex wasn't already displaced
        if not self.PointsJustDisplaced[j][RandomIndex] == 1:
          self.PointsJustDisplaced[j][RandomIndex] = 1
          DisplacementVector = self.GetDisplacementVector(MarkupsNode, RandomIndex)
          LabelsCoords[RandomIndex][1] = [(LabelsCoords[RandomIndex][1][dim] + DisplacementVector[dim]) for dim in range(3)]
          DisplacementAmount -= 1
      
      # Update MarkupsNode
      MarkupsNode.RemoveAllMarkups()
      for i, (Label, Coords) in enumerate(LabelsCoords):
        MarkupsNode.AddFiducialFromArray(Coords)
        MarkupsNode.SetNthFiducialLabel(i, Label)
        
    return True
  
  def GetDisplacementVector(self, MarkupsNode, PointIndex):         # Returns OffsetVector, pointing laterally out and forward from landmark, aiming from TrP to rib; Assuming that the landmark set has no omissions...
    
    LabelsCoords = [[MarkupsNode.GetNthFiducialLabel(i), MarkupsNode.GetMarkupPointVector(i,0)] for i in range(MarkupsNode.GetNumberOfFiducials())]
    CurrentLabelCoords = LabelsCoords[PointIndex]
    
    # Get lateral vector
    
    if PointIndex < 2:    # Deal with top-of-spine boundary
      AxialVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex+2][1][dim]) for dim in range(3)]
      
      # Use AxialVector to estimate Displacement magnitudes
      LateralOffsetMagnitude = (np.linalg.norm(AxialVector))
      AnteriorOffsetMagnitude = (np.linalg.norm(AxialVector))
      
      # LateralVector requires additional boundary consideration
      if PointIndex < 1:
        LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[1][1][dim]) for dim in range(3)]
      else:   # LateralNeighbor could be above or below CurrentPoint in MarkupsNode list
        if LabelsCoords[PointIndex-1][0][:-1] == CurrentLabelCoords[0][:-1]:    # If we find CurrentPoint neighbor above it
          LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex-1][1][dim]) for dim in range(3)]
        else:       # Otherwise, we know the lateral negihbor is below CurrentPoint
          LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex+1][1][dim]) for dim in range(3)]        
      
    if PointIndex > len(LabelsCoords)-3:    # Bottom-of-spine boundary
      AxialVector = [(LabelsCoords[PointIndex-2][1][dim] - CurrentLabelCoords[1][dim]) for dim in range(3)]
      
      # Use AxialVector to estimate Displacement magnitudes
      LateralOffsetMagnitude = (np.linalg.norm(AxialVector))
      AnteriorOffsetMagnitude = (np.linalg.norm(AxialVector))
      
      # LateralVector requires additional boundary consideration
      if PointIndex > len(LabelsCoords)-2:
        LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[-2][1][dim]) for dim in range(3)]
      else:   # LateralNeighbor could be above or below CurrentPoint in MarkupsNode list
        if LabelsCoords[PointIndex-1][0][:-1] == CurrentLabelCoords[0][:-1]:    # If we find CurrentPoint neighbor above it
          LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex-1][1][dim]) for dim in range(3)]
        else:       # Otherwise, we know the lateral negihbor is below CurrentPoint
          LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex+1][1][dim]) for dim in range(3)] 
    
    if PointIndex < 2 or PointIndex > len(LabelsCoords)-3:  # If we went into either boundary condition above, we have the info we need
      # Scale LateralVector
      LateralVectorNorm = np.linalg.norm(LateralVector)
      LateralVector = [(LateralVector[dim] * (np.sqrt(LateralOffsetMagnitude)/LateralVectorNorm)) for dim in range(3)]
      
      # Get AnteriorVector from LateralVector and AxialVector cross product
      AnteriorVector = np.cross(AxialVector, LateralVector)
      
      # Scale AnteriorVector
      AnteriorVectorNorm = np.linalg.norm(AnteriorVector)
      if AnteriorVector[1] < 0: # IF AnteriorVector points in posterior direction
        AnteriorVector = [((-1*AnteriorVector[dim])*(np.sqrt(AnteriorOffsetMagnitude)/AnteriorVectorNorm)) for dim in range(3)] # THEN reverse and scale it
      else:
        AnteriorVector = [(AnteriorVector[dim]*(np.sqrt(AnteriorOffsetMagnitude)/AnteriorVectorNorm)) for dim in range(3)] # Just scale it
    
      # DisplacementVector is vector-sum of LateralVector and AnteriorVector
      DisplacementVector = [(LateralVector[dim] + AnteriorVector[dim]) for dim in range(3)]
      return DisplacementVector
    
    #else: # Not at a boundary condition
    
    # Average of above and below vectors provide better estimate of axial direction at their midpoint than either alone
    AxialVector = [((LabelsCoords[PointIndex-2][1][dim] + LabelsCoords[PointIndex+2][1][dim])/2.0) for dim in range(3)]
    
    # Use AxialVector to estimate Displacement magnitudes
    LateralOffsetMagnitude = (np.linalg.norm(AxialVector))
    AnteriorOffsetMagnitude = (np.linalg.norm(AxialVector))
    
    # Lateral vector direction depends on whether PointIndex corresponds to a left or right sided point
    if LabelsCoords[PointIndex-1][0][:-1] == CurrentLabelCoords[0][:-1]:
      LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex-1][1][dim]) for dim in range(3)]
    else:
      LateralVector = [(CurrentLabelCoords[1][dim] - LabelsCoords[PointIndex+1][1][dim]) for dim in range(3)]
    
    # Scale LateralVector
    LateralVectorNorm = np.linalg.norm(LateralVector)
    LateralVector = [(LateralVector[dim])*(np.sqrt(LateralOffsetMagnitude)/LateralVectorNorm) for dim in range(3)]
    
    # Get AnteriorVector
    AnteriorVector = np.cross(AxialVector, LateralVector)
    
    # Scale and reflect AnteriorVector as necessary
    AnteriorVectorNorm = np.linalg.norm(AnteriorVector)
    if AnteriorVector[1] < 0:
      AnteriorVector = [(-1*(AnteriorVector[dim])*(np.sqrt(AnteriorOffsetMagnitude)/AnteriorVectorNorm)) for dim in range(3)]
    else:
      AnteriorVector = [((AnteriorVector[dim])*(np.sqrt(AnteriorOffsetMagnitude)/AnteriorVectorNorm)) for dim in range(3)]

    # DisplacementVector is vector sum of AnteriorVector and LateralVector
    DisplacementVector = [(AnteriorVector[dim] + LateralVector[dim]) for dim in range(3)]
    
    return DisplacementVector
    
  def DeletePoints(self, DoKeepBoundaries=False):     # MUST check to make sure points to be deleted weren't displaced by DisplacePoints
    for j, MarkupsNode in enumerate(self.MarkupsNodes):     # For each markup node present
      print "Deleting ", str(self.DeletionFraction) + "/1.0 of ", MarkupsNode.GetName(), " landmarks"
      LabelsCoords = [(MarkupsNode.GetNthFiducialLabel(i), MarkupsNode.GetMarkupPointVector(i,0)) for i in range(MarkupsNode.GetNumberOfFiducials())]
      DeletionAmount = int(self.DeletionFraction*len(LabelsCoords))
      
      # Select points for deletion
      while DeletionAmount > 0:
        # Select random index
        if DoKeepBoundaries:
          RandomIndex = int(np.random.uniform(2, len(LabelsCoords)-2))
        else:
          RandomIndex = int(np.random.uniform(0, len(LabelsCoords)))
        
        # Make sure we aren't deleting a point that was displaced
        if self.PointsJustDisplaced[j][RandomIndex] != 1:
          LabelsCoords.__delitem__(RandomIndex)
          self.PointsJustDisplaced[j].__delitem__(RandomIndex)
          DeletionAmount -= 1
      
      # Update MarkupsNode
      MarkupsNode.RemoveAllMarkups()
      for i, (Label, Coords) in enumerate(LabelsCoords):
        MarkupsNode.AddFiducialFromArray(Coords)
        MarkupsNode.SetNthFiducialLabel(i, Label)
    
      # Reinitialize self.PointsJustDisplaced
      self.PointsJustDisplaced[j] = MarkupsNode.GetNumberOfFiducials() * [0] #np.zeros(MarkupsNode.GetNumberOfFiducials())
    
    return True
    
  def PerformOperations(self):
    
    if self.DoAddNoise:
      self.AddNoise()
      
    if self.DoDisplacePoints:
      self.DisplacePoints()
    else: # We must initialize self.PointsJustDisplaced, even though no points were displaced
      self.PointsJustDisplaced = len(self.MarkupsNodes) * [0]
      for i, Node in enumerate(self.MarkupsNodes):
        self.PointsJustDisplaced[i] = Node.GetNumberOfFiducials() * [0]
      
    if self.DoDeletePoints:
      self.DeletePoints(self.DoKeepBoundaries)
      
    return True
    
#
# RepairLandmarksLogic
#

class RepairLandmarksLogic(ScriptedLoadableModuleLogic):
  class PatientTransverseProcesses:
    def __init__(self, parent, Node, kmWindowSize, PolyFitDegree, BoundaryMultiplicity, OmissionDetectionSpecificity, ImputationSpecificity):
      self.ParentLogic = parent
    
      # Markups node
      self.MarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.MarkupsNode.SetName(Node.GetName())
      for P in range(Node.GetNumberOfFiducials()):
        self.MarkupsNode.AddFiducialFromArray(Node.GetMarkupPointVector(P,0))
        self.MarkupsNode.SetNthFiducialLabel(P, Node.GetNthFiducialLabel(P))

      # Operation parameters
      self.kmWindowSize = int(kmWindowSize)
      self.PolyFitDegree = int(PolyFitDegree)
      self.BoundaryMultiplicity = int(BoundaryMultiplicity)
      self.OmissionDetectionSpecificity = OmissionDetectionSpecificity
      self.ImputationSpecificity = ImputationSpecificity
      
      # Initialize Left-Right sided nodes
      (L,R) = self.ClassifyLeftRight()
      
    def SortPointsVertically(self):
      # Get points with their labels, sort in descending position
      UnsortedMarkups = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      LabelsCoords = sorted(UnsortedMarkups, key=lambda Tup: -1*Tup[1][2])
      
      # Update self.MarkupsNode
      self.MarkupsNode.RemoveAllMarkups()
      for i, Markup in enumerate(LabelsCoords):
        self.MarkupsNode.AddFiducialFromArray(Markup[1])
        self.MarkupsNode.SetNthFiducialLabel(i, Markup[0])
      
    def ClassifyLeftRight(self):  

      self.SortPointsVertically()
      LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      SortedPointsLeftVotes = self.MarkupsNode.GetNumberOfFiducials() * [0]
      SortedPointsRightVotes = self.MarkupsNode.GetNumberOfFiducials() * [0]
      
      for i in range(0, self.MarkupsNode.GetNumberOfFiducials()-self.kmWindowSize+1):
        MarkupsWindow = LabelsCoords[i:i+self.kmWindowSize]
        #print MarkupsWindow
        NormalizedWindow = self.AnisotrpoicNormalization(MarkupsWindow)
        (KmLabels, KmCentroids) = self.KMeans(NormalizedWindow, 2)

        # If KmLabel == 0 indicates a left-side point
        if KmCentroids[0][0] < KmCentroids[1][0]:
          for j, Label in enumerate(KmLabels):
            if Label == 0:
              SortedPointsLeftVotes[i + j] = SortedPointsLeftVotes[i + j] + 1
            else:
              SortedPointsRightVotes[i + j] = SortedPointsRightVotes[i + j] + 1
        else: # If KmLabel == 0 indicates a right-side point
          for j, Label in enumerate(KmLabels):
            #print i, j
            if Label == 0:
              SortedPointsRightVotes[i + j] = SortedPointsRightVotes[i + j] + 1
            else:
              SortedPointsLeftVotes[i + j] = SortedPointsLeftVotes[i + j] + 1
      
      NewLeftSide = slicer.vtkMRMLMarkupsFiducialNode()
      NewLeftSide.SetName(self.MarkupsNode.GetName() + "_Left")
      
      NewRightSide = slicer.vtkMRMLMarkupsFiducialNode()
      NewRightSide.SetName(self.MarkupsNode.GetName() + "_Right")
      
      for i, UnclassifiedPoint in enumerate(LabelsCoords):
        #OriginalPointIndex = self.LabelsCoords.index(UnclassifiedPoint)
        #OriginalPoint = self.LabelsCoords[OriginalPointIndex]
        if SortedPointsLeftVotes[i] > SortedPointsRightVotes[i]:
          NewLeftSide.AddFiducialFromArray(UnclassifiedPoint[1])
          NewLeftSide.SetNthFiducialLabel(NewLeftSide.GetNumberOfFiducials()-1, UnclassifiedPoint[0] + '_Left')
        else:
          NewRightSide.AddFiducialFromArray(UnclassifiedPoint[1])
          NewRightSide.SetNthFiducialLabel(NewRightSide.GetNumberOfFiducials()-1, UnclassifiedPoint[0] + '_Right')

      self.LeftSide = self.ParentLogic.SpineSide(self, NewLeftSide, self.PolyFitDegree, self.BoundaryMultiplicity, self.OmissionDetectionSpecificity, self.ImputationSpecificity)
      self.RightSide = self.ParentLogic.SpineSide(self, NewRightSide, self.PolyFitDegree, self.BoundaryMultiplicity, self.OmissionDetectionSpecificity, self.ImputationSpecificity)
      
      return (self.LeftSide.MarkupsNode, self.RightSide.MarkupsNode)
      
    def AnisotrpoicNormalization(self, PointSet):
      # Start by finding top and bottom points of spine for verticle normalization
      SetRight = -10000
      SetLeft = 10000
      SetFront = -10000
      SetBack = 10000
      SetTop = -10000
      SetBottom = 10000

      for Point in PointSet:
        Coords  = Point[1]
        if Coords[0] < SetLeft:
          SetLeft = Coords[0]
        if Coords[0] > SetRight:
          SetRight = Coords[0]
        if Coords[1] < SetBack:
          SetBack = Coords[1]
        if Coords[1] > SetFront:
          SetFront = Coords[1]
        if Coords[2] > SetTop:
          SetTop = Coords[2]
        if Coords[2] < SetBottom:
          SetBottom = Coords[2]
        
      SetHeight = SetTop - SetBottom
      SetWidth = SetRight - SetLeft
      SetDepth = SetFront - SetBack
      
      # (Re) initialize normalized point list
      NormalizedPoints = len(PointSet) * [0]
      
      # Normalize S-I dimension to R-L scale
      for i, Point in enumerate(PointSet):
        Coords = Point[1]
        NormalizedPoints[i] = np.array([Coords[0], (Coords[1]) * (SetWidth / (SetDepth * 3.0)), (Coords[2]) * (SetWidth / (SetHeight * 2.0))])
      return NormalizedPoints
    
    def ShouldStopKMeans(self, oldCentroids, Centroids, iterations):
      MaxIterations = 500
      StoppingDelta = 0.05
      #print oldCentroids, Centroids
      if iterations > MaxIterations: return True
      if iterations == 0:
        return False
      for C in range(len(oldCentroids)):
        #print oldCentroids[C], Centroids[C]
        if np.linalg.norm(oldCentroids[C] - Centroids[C]) < StoppingDelta:
          return True
      return False

    def GetKMeansLabels(self, DataSet, Centroids, Labels):
      for i, Coords in enumerate(DataSet):
        #PointCentroidDistance = np.linalg.norm(Coords - Centroids[0])
        minDist = 1000000
        Labels[i] = 0
        for j, CentroidVector in enumerate(Centroids):
          #print Coords, CentroidVector#, np.linalg.norm(Coords - CentroidVector)
          PointCentroidDistance = np.linalg.norm(Coords - CentroidVector)
          if PointCentroidDistance < minDist:
            minDist = PointCentroidDistance
            Labels[i] = j
      return Labels

    def GetCentroids(self, DataSet, Labels, k):
      Centroids = []
      #print Labels
      for C in range(k):    # For each centroid
        Centroid = np.random.uniform() * np.ones(len(DataSet[0]))    # Each centroid with as many dimensions as the data
        
        for i, Coords in enumerate(DataSet): # Take each data point contributing to the centroid into consideration
          if Labels[i] == C:                    # if it belongs to the current centroid
            for dim in range(len(Centroid)):
              Centroid[dim] += Coords[dim]
              
        for dim in range(len(Centroid)):
          Centroid[dim] = Centroid[dim] / np.count_nonzero(Labels == C)
        Centroids.append(Centroid)
      return Centroids

    def KMeans(self, DataSet, k=2):   # Expects DataSet as list of (Label, np.array[R,A,S]) tuples
      DataSetLabels = np.zeros(len(DataSet))
      for i in range(len(DataSetLabels)):
        DataSetLabels[i] = int(round(np.random.uniform()))
      # Initialize centroids
      numFeatures = len(DataSet[0])
      Centroids = k * [0]
      # Try initializing one centroid on each side
      Centroids[0] = np.array([min([i[0] for i in DataSet]),0,0])
      Centroids[1] = np.array([max([i[0] for i in DataSet]),0,0])
      
    # Initialize book keeping variables
      iterations = 0
      oldCentroids = k * [0]
      for Cent in range(k):
        oldCentroids[Cent] = np.array(numFeatures * [np.random.uniform()])

      # Run the k-means algorithm
      while not self.ShouldStopKMeans(oldCentroids, Centroids, iterations):
        oldCentroids = Centroids
        iterations += 1
        
        DataSetLabels = self.GetKMeansLabels(DataSet, Centroids, DataSetLabels)
        Centroids = self.GetCentroids(DataSet, DataSetLabels, k)
        #print Centroids
      return (DataSetLabels, Centroids)

    def CombineRepairedSides(self):
      
      # Retrieve and sort LabelCoords of Left and Right sides
      LeftLabelCoords = [(self.LeftSide.MarkupsNode.GetNthFiducialLabel(i), self.LeftSide.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.LeftSide.MarkupsNode.GetNumberOfFiducials())]
      SortedLeftLabelCoords = sorted(LeftLabelCoords, key=lambda Tup: -1*Tup[1][2])
      RightLabelsCoords = [(self.RightSide.MarkupsNode.GetNthFiducialLabel(j), self.RightSide.MarkupsNode.GetMarkupPointVector(j,0)) for j in range(self.RightSide.MarkupsNode.GetNumberOfFiducials())]
      SortedRightLabelCoords = sorted(RightLabelsCoords, key=lambda Tup: -1*Tup[1][2])
      
      # Consolidate Left,Right LabelCoords - in that order
      # Assumes that left and right sides start at the same vertebra and each point corresponds to one on the other side
      CombinedLabelCoords = []
      for i, (LeftPoint, RightPoint) in enumerate(zip(SortedLeftLabelCoords, SortedRightLabelCoords)):
        CombinedLabelCoords.append(LeftPoint)
        CombinedLabelCoords.append(RightPoint)
      #CombinedLabelCoords = LeftLabelCoords + RightLabelsCoords
      
      # IF this is an intermediate repair for logic update, append any remaining points
      if not len(SortedLeftLabelCoords) + len(SortedRightLabelCoords) == 34:
        if len(SortedLeftLabelCoords) > len(SortedRightLabelCoords):
          PointsRemaining = len(SortedLeftLabelCoords) - len(SortedRightLabelCoords)
          while PointsRemaining > 0:
            Coords = SortedLeftLabelCoords[len(SortedLeftLabelCoords) - PointsRemaining][1]
            Label = SortedLeftLabelCoords[len(SortedLeftLabelCoords) - PointsRemaining][0]
            CombinedLabelCoords.append((Label, Coords))
            PointsRemaining -= 1
        else:
          PointsRemaining = len(SortedRightLabelCoords) - len(SortedLeftLabelCoords)
          while PointsRemaining > 0:
            Coords = SortedRightLabelCoords[len(SortedRightLabelCoords) - PointsRemaining][1]
            Label = SortedRightLabelCoords[len(SortedRightLabelCoords) - PointsRemaining][0]
            CombinedLabelCoords.append((Label, Coords))
            PointsRemaining -= 1
      
      
      # Create node to populate with markups using CombinedLabelCoordsLabelCoords
      RepairedNode = slicer.vtkMRMLMarkupsFiducialNode()
      if RepairedNode.GetNumberOfFiducials() == 34:
        RepairedNode.SetName(self.MarkupsNode.GetName() + "_Repaired")
      else:
        RepairedNode.SetName(self.MarkupsNode.GetName())
      
      
      # Populate RepairedNode with markups points
      self.MarkupsNode.RemoveAllMarkups()
      for i, LabelCoord in enumerate(CombinedLabelCoords):
        self.MarkupsNode.AddFiducialFromArray(LabelCoord[1])
        self.MarkupsNode.SetNthFiducialLabel(i, LabelCoord[0])
        #self.MarkupsNode.Add(i, LabelCoord[1])
        
      self.ParentLogic.ApplyNamingConvention(self.MarkupsNode)
      #self.MarkupsNode.Copy(RepairedNode)
        
      return self.MarkupsNode
      
  class SpineSide:
    def __init__(self, parent, Node, PolyFitDegree, BoundaryMultiplicity, OmissionDetectionSpecificity, ImputationSpecificity):
      self.ParentPatient = parent
      self.PointsPerPolynomialCurve = 500
      
      self.MarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.MarkupsNode.SetName(Node.GetName())
      for P in range(Node.GetNumberOfFiducials()):
        self.MarkupsNode.AddFiducialFromArray(Node.GetMarkupPointVector(P,0))
        self.MarkupsNode.SetNthFiducialLabel(P, Node.GetNthFiducialLabel(P))
      #self.MarkupsNode.Copy(Node)
      
      self.PatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.PatchNode.SetName(self.MarkupsNode.GetName() + "_Patch")
      
      # Each element corresponds to an interval of self.MarkupsNode identified as having omissions,
      self.SubPatchesPointCounts = []      # Values denote number of omissions of self.MarkupsNode's intervals, to be stored in self.PatchNode
      
      self.PolyFitDegree = PolyFitDegree
      self.BoundaryMultiplicity = BoundaryMultiplicity
      self.OmissionDetectionSpecificity = OmissionDetectionSpecificity
      
      self.WeightedBoundaryNode = self.GetWeightedBoundaryNode(self.MarkupsNode, BoundaryMultiplicity)
      
      self.ImputationSpecificity = ImputationSpecificity
      self.OutlierIdentificationVotes = np.zeros(self.MarkupsNode.GetNumberOfFiducials())
      #self.ImputationErrorImprovementThreshold = 0.05
     
    def GetWeightedBoundaryNode(self, Node, Multiplicity):
      WeightedBoundaryNode = slicer.vtkMRMLMarkupsFiducialNode()
      Multiplicity = int(Multiplicity)
      
      # Add upper boundary point, multiple times, with noise for non-coincidence
      UpperBoundaryPoint = Node.GetMarkupPointVector(0,0)
      for i in range(Multiplicity-1):
        RandomNoise = np.random.uniform(-1,1,3)
        NoisifiedBoundaryPoint = [UpperBoundaryPoint[dim] + RandomNoise[dim] for dim in range(3)]
        WeightedBoundaryNode.AddFiducialFromArray(NoisifiedBoundaryPoint)
      
      # Add non-boundary points normally throughout
      for i in range(0, Node.GetNumberOfFiducials()):
        Point = Node.GetMarkupPointVector(i,0)
        WeightedBoundaryNode.AddFiducialFromArray(Point)
      
      # Add upper boundary point, multiple times, with noise for non-coincidence
      LowerBoundaryPoint = Node.GetMarkupPointVector(Node.GetNumberOfFiducials()-1,0)
      for i in range(Multiplicity-1):
        RandomNoise = np.random.uniform(-1,1,3)
        NoisifiedBoundaryPoint = [LowerBoundaryPoint[dim] + RandomNoise[dim] for dim in range(3)]
        WeightedBoundaryNode.AddFiducialFromArray(NoisifiedBoundaryPoint)
        
      self.OrderPointsSuperiorToInferior(WeightedBoundaryNode)
      return WeightedBoundaryNode
     
    def OrderPointsSuperiorToInferior(self, Node):
      #print  "Sorting node " + Node.GetName() + " landmarks"
      LabelsCoords = [(Node.GetNthFiducialLabel(i), Node.GetMarkupPointVector(i,0)) for i in range(Node.GetNumberOfFiducials())]
      Node.RemoveAllMarkups()
      LabelsCoords = sorted(LabelsCoords, key=lambda Tup: -1*Tup[1][2])
      for i, (Label, Coord) in enumerate(LabelsCoords):
        Node.AddFiducialFromArray(Coord)
        Node.SetNthFiducialLabel(i, Label)
     
    #def CombineNodeWithPatch(self, BaseNode, PatchNode):
    def CombineNodeWithPatch(self, PatchNode):      # Combines (appends and sorts) PatchNode onto self.MarkupsNode and updates self.WeightedBoundaryNode
      # Returns a node containing the original markups points, and those from the patch - used to check patch fit improvements
      
      #TentativeNode = slicer.vtkMRMLMarkupsFiducialNode()
      OriginalLabelPoints = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      PatchLabelPoints = [(PatchNode.GetNthFiducialLabel(i), PatchNode.GetMarkupPointVector(i,0)) for i in range(PatchNode.GetNumberOfFiducials())]
      AllLabelPoints = OriginalLabelPoints + PatchLabelPoints
      
      # Also sort points Sup to Inf
      AllLabelPoints = sorted(AllLabelPoints, key=lambda LabelPoint: -1*LabelPoint[1][2])
      
      self.MarkupsNode.RemoveAllMarkups()
      for i, (Label, Coords) in enumerate(AllLabelPoints):
        self.MarkupsNode.AddFiducialFromArray(Coords)
        self.MarkupsNode.SetNthFiducialLabel(self.MarkupsNode.GetNumberOfFiducials()-1, Label)
        
      self.UpdatePatchedSidePointNames(self.MarkupsNode)
      
      self.WeightedBoundaryNode = self.GetWeightedBoundaryNode(self.MarkupsNode, self.BoundaryMultiplicity)
      
      # Update self variables      
      self.SubPatchesPointCounts = []
      self.OutlierIdentificationVotes = np.zeros(self.MarkupsNode.GetNumberOfFiducials())
      
      #self.MarkupsNode = TentativeNode
      return True 
      #return TentativeNode
     
    def CoordsPolyFit(self):    # Always run on self.WeightedBoundaryNode
      #Coords = [self.MarkupsNode.GetMarkupPointVector(i,0) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      #print Coords
      #LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      Coords = [self.WeightedBoundaryNode.GetMarkupPointVector(i,0) for i in range(self.WeightedBoundaryNode.GetNumberOfFiducials())]
      R = [Coords[i][0] for i in range(len(Coords))]
      A = [Coords[i][1] for i in range(len(Coords))]
      S = [Coords[i][2] for i in range(len(Coords))]

      # The degrees of these polynomials should not be hard coded
      S_R_FitCoefs = np.polyfit(S, R, self.PolyFitDegree)
      S_A_FitCoefs = np.polyfit(S, A, self.PolyFitDegree)
      
      SrPolynomial = np.poly1d(S_R_FitCoefs)
      SaPolynomial = np.poly1d(S_A_FitCoefs)
  
      return (SrPolynomial, SaPolynomial)
      
    def GetCurvewiseInterpointDistances(self):  # Always run on self.MarkupsNode
      #Coords = [self.MarkupsNode.GetMarkupPointVector(i,0) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      (SrPolynomial, SaPolynomial) = self.CoordsPolyFit()
      LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      R = [LabelsCoords[i][1][0] for i in range(len(LabelsCoords))]
      A = [LabelsCoords[i][1][1] for i in range(len(LabelsCoords))]
      S = [LabelsCoords[i][1][2] for i in range(len(LabelsCoords))]
      sSpace = np.linspace(S[0], S[-1], self.PointsPerPolynomialCurve)
     
      # Distances along the polynomial to each landmark in the given dimension
      PointDistances = []
      #priorS = sSpace[0]
      sIndex = 1
      
      # Find points in sSpace corresponding to landmarks in both R and A dimension
      # Using "sSpace" for each polynomial's independent variable allows the use of the S coordinate to recognize when we've reached a point along the polynomials
      CurveDistance = 0
      for i, Landmark in enumerate(zip(R[:-1],A[:-1])):
        CurrentIntervalLength = 0
        while sIndex < self.PointsPerPolynomialCurve and sSpace[sIndex] > S[i+1]:
          PriorS = sSpace[sIndex-1]
          CurrentS = sSpace[sIndex]
          PriorR = SrPolynomial(PriorS)
          CurrentR = SrPolynomial(CurrentS)
          PriorA = SaPolynomial(PriorS)
          CurrentA = SaPolynomial(CurrentS)
          CurveIncrementDistance = np.sqrt(((CurrentS - PriorS)**2) + ((CurrentR - PriorR)**2) + ((CurrentA - PriorA)**2)) 
          CurrentIntervalLength += CurveIncrementDistance
          CurveDistance += CurveIncrementDistance
          sIndex += 1
        PointDistances.append((CurveDistance, CurrentIntervalLength))
          
      return PointDistances
      
    def FrequencyPolyFit(self, IntervalData):
      IntervalDistances = [IntervalData[i][1] for i in range(len(IntervalData))]
      
      # Try fitting polynomial to relationship between total distance travelled down the spine to the inter-landmark distances
      CumulativeDistance = 0
      CumulativeCurveDistances = []
      for i, Interval in enumerate(IntervalDistances):
        CumulativeCurveDistances.append(CumulativeDistance + (Interval/2.0))
        CumulativeDistance += Interval
      
      CurveSpace = np.linspace(0, CumulativeDistance, self.PointsPerPolynomialCurve)
      FrequencyCoeffs = np.polyfit(CumulativeCurveDistances, IntervalDistances, self.PolyFitDegree)
      FrequencyPolynomial = np.poly1d(FrequencyCoeffs)
      
      return FrequencyPolynomial
      
    def GetPolyfitErrors(self, Data, Polynomial):
      FitErrors = []
      for Datum in Data:
        PolyPrediction = Polynomial(Datum[0])
        PredictionError = Datum[1] - PolyPrediction
        FitErrors.append(PredictionError)
      return FitErrors
  
    def GetPolyfitRMS(self, FitErrors):
      if len(FitErrors) == 0:
        print "Computing RMS error of empty error set - returning 0"
        return 0
      else:
        SumSquaredError = sum([(Error)**2 for Error in FitErrors])
        RMS = np.sqrt(SumSquaredError)
        #print RMS
        return RMS
       
    def GetNonparametricSkewness(self, Data):   # 
      Mean = np.mean(Data)
      Median = np.median(Data)
      StdDev = np.std(Data)
      Skewness = (Mean - Median) / StdDev
      return Skewness
       
    def IdentifyOmissionsFromLocalDistances(self, IntervalIndex, IntervalData):  # Not currently used, other Identifiers work better so far
      IntervalLength = IntervalData[IntervalIndex][1]
      
      # The first interval is a boundary, having only one neighboring interval
      if IntervalIndex == 0:
        NextIntervalLength = IntervalData[IntervalIndex+1][1]
        if NextIntervalLength < 0.75*IntervalLength:
          return True
        else:
          return False
          
      # The last interval is also a boundary condiditon
      if IntervalIndex == len(IntervalData)-1:
        PriorIntervalLength = IntervalData[IntervalIndex-1][1]
        if PriorIntervalLength < 0.75*IntervalLength:
          return True
        else:
          return False
          
      #else:
      NextIntervalLength = IntervalData[IntervalIndex+1][1]
      PriorIntervalLength = IntervalData[IntervalIndex-1][1]
      if NextIntervalLength < 0.75*IntervalLength or PriorIntervalLength < 0.75*IntervalLength:
        return True
      else:
        return False
   
    def IdentifyOmissionsFromIntervalLengthFit(self, IntervalIndex, FitErrors):
      AbsErrors = [abs(E) for E in FitErrors]
      IntervalError = FitErrors[IntervalIndex]
      MeanAbsError = np.mean(AbsErrors)
      ErrorStdDev = np.std(FitErrors)
      
      # Try skewness instead of standard deviation - 
      IntervalLengthSkewness = self.GetNonparametricSkewness(FitErrors)
      
      # The first interval is a boundary, having only one neighboring interval below
      if IntervalIndex == 0:
        #if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*ErrorStdDev):
          return True
        else:
          return False
          
      # The last interval is also a boundary condiditon
      elif IntervalIndex == len(FitErrors)-1:
        #if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*ErrorStdDev):
          return True
        else:
          return False
          
      else:
        PriorIntervalError = FitErrors[IntervalIndex+1]
        NextIntervalError = FitErrors[IntervalIndex-1]
        if (np.sign(IntervalError) != np.sign(PriorIntervalError) and abs(PriorIntervalError)!=max(AbsErrors)) and (np.sign(IntervalError) != np.sign(NextIntervalError) and abs(NextIntervalError)!=max(AbsErrors)) and (abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*ErrorStdDev)):
          return True
        #if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if abs(IntervalError) > MeanAbsError + (self.OmissionDetectionSpecificity*ErrorStdDev):
          return True
        else:
          return False
    
    def IdentifyOmissionsFromLocalIntervalLengths(self, IntervalIndex, IntervalData):
      IntervalLengths = [IntervalData[i][1] for i in range(len(IntervalData))]
      IntervalLengthMean = np.mean(IntervalLengths)
      IntervalLengthStd = np.std(IntervalLengths)
      
      # Try skewness instead of standard deviation - 
      IntervalLengthSkewness = self.GetNonparametricSkewness(IntervalLengths)
      
      CurrentIntervalLength = IntervalLengths[IntervalIndex]
      
      # Boundary condition checks leave the possibility that two intervals with omissions neighbor
      
      # Inferior-most boundary condition - only one neighbor, above
      if IntervalIndex == len(IntervalData)-1:
        NextIntervalLength = IntervalLengths[IntervalIndex-1]
        #if CurrentIntervalLength > NextIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if CurrentIntervalLength > NextIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthStd):
          return True
        else:
          return False
        
      # Superior-most boundary condition - only one neighbor, below
      if IntervalIndex == 0:
        PriorIntervalLength = IntervalLengths[IntervalIndex+1]
        #if CurrentIntervalLength > PriorIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if CurrentIntervalLength > PriorIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthStd):
          return True
        else:
          return False
      
      else:         # Interval is somewhere in the middle, with two neighbors
        NextIntervalLength = IntervalLengths[IntervalIndex-1]
        PriorIntervalLength = IntervalLengths[IntervalIndex+1]
        #if CurrentIntervalLength > NextIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthSkewness) or CurrentIntervalLength > PriorIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthSkewness):
        if CurrentIntervalLength > NextIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthStd) or CurrentIntervalLength > PriorIntervalLength + (self.OmissionDetectionSpecificity*IntervalLengthStd):
          return True
        else:
          return False
     
    def EstimateOmissions(self, Node, IntervalIndex, IntervalData):   # Finds and adds the optimum number of points to a given interval to minimize the entire curve's frequency fit RMS
      # Returns CountEstimate - If CountEstimate == 0 and this method was called, an infinite loop may result from omissions being identified and never fixed
      LabelsCoords = [(Node.GetNthFiducialLabel(i), Node.GetMarkupPointVector(i,0)) for i in range(Node.GetNumberOfFiducials())]
      S = [LabelsCoords[i][1][2] for i in range(len(LabelsCoords))]
      
      # Initialize original measures for comparison
      (OriginalSr, OriginalSa) = self.CoordsPolyFit()
      OriginalFreqPoly = self.FrequencyPolyFit(IntervalData)
      OriginalFitErrors = self.GetPolyfitErrors(IntervalData, OriginalFreqPoly)
      
      BestRMS = self.GetPolyfitRMS(OriginalFitErrors)
      BestIntervalFitError = abs(OriginalFitErrors[IntervalIndex])
      #print OriginalFitErrors
      
      print ""
      print " About to test imputation - ", self.MarkupsNode.GetName(), " - Interval ", IntervalIndex
      
      # Try one imputation, as boundary, to get initial tentative improvement
      CountEstimate = 1
      print "Try ", CountEstimate, " imputation"

      SubPatchNode = slicer.vtkMRMLMarkupsFiducialNode()

      # Create SubPatchNode and add points by interpolating Node characteristic polynomials
      NewSLocations = np.linspace(S[IntervalIndex], S[IntervalIndex+1], CountEstimate+2)[1:-1]
      for x in NewSLocations: 
        SubPatchNode.AddFiducialFromArray([OriginalSr(x), OriginalSa(x), x])
        SubPatchNode.SetNthFiducialLabel(0,"SubPatch-0_Point-0")
      
      # Create TentativeSide object for operations needed to reasses fit error
      TentativeSide = self.ParentPatient.ParentLogic.SpineSide(self.ParentPatient, self.MarkupsNode, self.PolyFitDegree, self.BoundaryMultiplicity, self.OmissionDetectionSpecificity, self.ImputationSpecificity)
      TentativeSide.CombineNodeWithPatch(self.PatchNode)
      TentativeSide.CombineNodeWithPatch(SubPatchNode)
      
      # Use said operations to update measures
      TentInterpointData = TentativeSide.GetCurvewiseInterpointDistances()
      TentFreqPolynomial = TentativeSide.FrequencyPolyFit(TentInterpointData)
      TentFitErrors = TentativeSide.GetPolyfitErrors(TentInterpointData, TentFreqPolynomial)
      #TentFitErrors = TentativeSide.GetPolyfitErrors(TentInterpointData, OriginalFreqPoly)
      TentRMS = TentativeSide.GetPolyfitRMS(TentFitErrors)
      TentIntervalFitError = np.mean([abs(Error) for Error in TentFitErrors[IntervalIndex:IntervalIndex+CountEstimate+1]])
      
      AverageIntervalFitErrorChange = (TentIntervalFitError - BestIntervalFitError) / float(Node.GetNumberOfFiducials() - 1 + CountEstimate)
      print "Additional AverageIntervalFitErrorChange with " + str(CountEstimate) + " imputations: " + str(AverageIntervalFitErrorChange)
      print " (Unaveraged error: " + str(TentIntervalFitError - BestIntervalFitError) + ")"
      
      # Try 2 or more imputations in the interval, see if the freq. fit improves
      
      while AverageIntervalFitErrorChange < self.ImputationSpecificity and TentativeSide.MarkupsNode.GetNumberOfFiducials() < 34:    # While the latest addition improved the frequency fit over the original polynomial
        #print ""
        #print TentIntervalFitError - BestIntervalFitError
        #print TentFitErrors
        #print ""
        CountEstimate += 1            # Try adding one more point
        print "Try ", CountEstimate, " imputations"
        BestRMS = TentRMS
        BestIntervalFitError = TentIntervalFitError

        SubPatchNode = slicer.vtkMRMLMarkupsFiducialNode()

        # Create SubPatchNode and add points by interpolating Node characteristic polynomials
        NewSLocations = np.linspace(S[IntervalIndex], S[IntervalIndex+1], CountEstimate+2)[1:-1]
        for i, x in enumerate(NewSLocations, 1): 
          NewLabel = str("SubPatch-" + str(len(self.SubPatchesPointCounts)) + "_Point-" + str(i))
          SubPatchNode.AddFiducialFromArray([OriginalSr(x), OriginalSa(x), x])
          SubPatchNode.SetNthFiducialLabel(i, NewLabel)
          # No need to set point labels until final instantiation
      
        TentativeSide = self.ParentPatient.ParentLogic.SpineSide(self.ParentPatient, self.MarkupsNode, self.PolyFitDegree, self.BoundaryMultiplicity, self.OmissionDetectionSpecificity, self.ImputationSpecificity)
        TentativeSide.CombineNodeWithPatch(self.PatchNode)
        TentativeSide.CombineNodeWithPatch(SubPatchNode)

        TentInterpointData = TentativeSide.GetCurvewiseInterpointDistances()
        TentFreqPolynomial = TentativeSide.FrequencyPolyFit(TentInterpointData)
        TentFitErrors = TentativeSide.GetPolyfitErrors(TentInterpointData, TentFreqPolynomial)
        
        TentRMS = TentativeSide.GetPolyfitRMS(TentFitErrors)
        TentIntervalFitError = np.mean([abs(Error) for Error in TentFitErrors[IntervalIndex:IntervalIndex+CountEstimate+1]])
        
        # Normalize error for number of intervals present - helps ensure imputing omission won't increase error by increasing number of intervals
        AverageIntervalFitErrorChange = (TentIntervalFitError - BestIntervalFitError) / float(Node.GetNumberOfFiducials() - 1 + CountEstimate)
        print "Additional AverageIntervalFitErrorChange with " + str(CountEstimate) + " imputations: " + str(AverageIntervalFitErrorChange)
        print " (Unaveraged error: " + str(TentIntervalFitError - BestIntervalFitError) + ")"
        
      # ASSERT TentRMS > BestRMS, because CountEstimate is one too high
      CountEstimate -= 1
      
      print " Done testing imputation - ", self.MarkupsNode.GetName(), " - Interval ", IntervalIndex
      print ""
      if TentativeSide.MarkupsNode.GetNumberOfFiducials() == 34:
        print "Frequency fit optimization stopped by landmark count limit"
      else:
        print "Frequency fit optimized by imputing ", CountEstimate, " points in interval ", IntervalIndex, " - ", self.MarkupsNode.GetName()
      #print CountEstimate
      
      # Create node with optimum number of imputations

      SubPatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      SubPatchNode.SetName(self.MarkupsNode.GetName() + "-SubPatch-" + str(IntervalIndex))
      
      NewSLocations = np.linspace(S[IntervalIndex], S[IntervalIndex+1], CountEstimate+2)[1:-1]
      for i, x in enumerate(NewSLocations): 
        SubPatchNode.AddFiducialFromArray([OriginalSr(x), OriginalSa(x), x])
        NewLabel = str("SubPatch-" + str(len(self.SubPatchesPointCounts)) + "_Point-" + str(i))
        print ""
        SubPatchNode.SetNthFiducialLabel(i, NewLabel)

      #self.OrderPointsSuperiorToInferior(SubPatchNode)

      return SubPatchNode
      
    def GeneratePatchNode(self):
      self.SubPatchesPointCounts = []  # Reinitialize
      
      # SkipIntervals only used when looping over the side a number of times, currently not implemented
      SkipIntervals = np.zeros(self.MarkupsNode.GetNumberOfFiducials()-1)   # Used to keep track of which intervals were tested for omissions - IdentifyOmissionsFromIntervalLengthFit may indicate omissions while EstimateOmissions might impute none
      
      #SubPatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      #PatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.PatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      self.PatchNode.SetName(self.MarkupsNode.GetName() + "_Patch")
      
      TentativeRepairNode = slicer.vtkMRMLMarkupsFiducialNode()
      TentativeRepairNode.Copy(self.MarkupsNode)
      
      # Use node with boundary point multiplication as per BoundaryMultiplicitySlider.value
      WeightedBoundaryNode = self.GetWeightedBoundaryNode(self.MarkupsNode, self.BoundaryMultiplicity)
      (SrPolynomial, SaPolynomial) = self.CoordsPolyFit()
      
      #(SrPolynomial, SaPolynomial) = self.CoordsPolyFit(self.MarkupsNode)
      CurvewiseInterpointData = self.GetCurvewiseInterpointDistances()
      CurvewiseFrequencyPolynomial = self.FrequencyPolyFit(CurvewiseInterpointData)
      FrequencyFitErrors = self.GetPolyfitErrors(CurvewiseInterpointData, CurvewiseFrequencyPolynomial)

      print ""
      print "About to look for intervals with omissions  - ", self.MarkupsNode.GetName()
      ImputingPoints = True     # NOT IMPLEMENTED - ImputingPoints used to loop through curve iteratively finding omissions and updating errors/stats
      #while ImputingPoints:
      ImputingPoints = False
        
      #for TentativeInterval, (IntervalDatum, FitError) in enumerate(zip(CurvewiseInterpointData, FrequencyFitErrors), start=sum(self.SubPatchesPointCounts)):
      for TentativeInterval, (IntervalDatum, FitError) in enumerate(zip(CurvewiseInterpointData, FrequencyFitErrors)):
        # For each interval in the TentativeRepairNode
        #CorrespondingOriginalInterval = TentativeInterval - sum(self.SubPatchesPointCounts)
        #CorrespondingOriginalInterval = TentativeInterval
        if not SkipIntervals[TentativeInterval] and (self.IdentifyOmissionsFromIntervalLengthFit(TentativeInterval, FrequencyFitErrors) or self.IdentifyOmissionsFromLocalIntervalLengths(TentativeInterval, CurvewiseInterpointData)):
          print ""
          print "Interval " + str(TentativeInterval) + " apparently missing points - " + self.MarkupsNode.GetName()
          ImputingPoints = True
          #print CorrespondingOriginalInterval, TentativeInterval
          
          # Find optimum number of points to impute in interval i
          SubPatchNode = self.EstimateOmissions(TentativeRepairNode, TentativeInterval, CurvewiseInterpointData)
          
          # Keep track of interval changes
          NumImputations = SubPatchNode.GetNumberOfFiducials()
          if NumImputations > 0:
            self.SubPatchesPointCounts.append(NumImputations)
          
            # Add each point of this SubPatch interval to the cumulative PatchNode
            for i in range(SubPatchNode.GetNumberOfFiducials()):
              SubPatchPointCoords = SubPatchNode.GetMarkupPointVector(i,0)
              SubPatchPointLabel = SubPatchNode.GetNthFiducialLabel(i)
              self.PatchNode.AddFiducialFromArray(SubPatchPointCoords)
              self.PatchNode.SetNthFiducialLabel(self.PatchNode.GetNumberOfFiducials()-1, SubPatchPointLabel)
          
          SkipIntervals = np.insert(SkipIntervals, TentativeInterval, np.ones(NumImputations+1))      # Don't check intervals we've already identified as incomplete - infinite loop
          
          """
          # Update node tracking overall patch improvement
          TentativeRepairNode = self.CombineNodeWithPatch(TentativeRepairNode, SubPatchNode)
          
          # Update node characteristics
          (SrPolynomial, SaPolynomial) = self.CoordsPolyFit(TentativeRepairNode)
          CurvewiseInterpointData = self.GetCurvewiseInterpointDistances(TentativeRepairNode, SrPolynomial, SaPolynomial)
          CurvewiseFrequencyPolynomial = self.FrequencyPolyFit(CurvewiseInterpointData)
          FrequencyFitErrors = self.GetPolyfitErrors(CurvewiseInterpointData, CurvewiseFrequencyPolynomial)
          #break               # Start again from the top of the newly shaped (frequency fit) spine
          """
          #PatchNode = self.CombineNodeWithPatch(PatchNode, SubPatchNode)

          # Reinitialize SubPatch for new interval
          #SubPatchNode.RemoveAllMarkups()
      print "Done looking for intervals with omissions - ", self.MarkupsNode.GetName()
            
      return self.PatchNode
   
    def AddPointToSubPatch(self, SubPatchIndex):    # Updates self.PatchNode to include on more point in interval corresponding to SubPatchIndex
      if SubPatchIndex > len(self.SubPatchesPointCounts):
        print "Non-existent patch interval - cannot add point"
        return self.PatchNode
      
      # Identify top and bottom points of SubPatch so they and the points they bound can be modified
      SubPatchIntervalStartIndex = sum(self.SubPatchesPointCounts[:SubPatchIndex])
      SubPatchIntervalEndIndex = sum(self.SubPatchesPointCounts[:SubPatchIndex]) + self.SubPatchesPointCounts[SubPatchIndex] - 1
      
      TopSubPatchPoint = (self.PatchNode.GetNthFiducialLabel(SubPatchIntervalStartIndex), self.PatchNode.GetMarkupPointVector(SubPatchIntervalStartIndex,0))
      BottomSubPatchPoint = (self.PatchNode.GetNthFiducialLabel(SubPatchIntervalEndIndex), self.PatchNode.GetMarkupPointVector(SubPatchIntervalEndIndex,0))
      
      # Find last point before, and first point after SubPatch's interval
      LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      for i, LabelCoord in enumerate(LabelsCoords[:-1]):
        if LabelsCoords[i+1][1][2] < BottomSubPatchPoint[1][2]:
          BoundingPointBelow = LabelCoord
          break
        
      LabelsCoords.reverse()
      for i, LabelCoord in enumerate(LabelsCoords[:-1]):
        if LabelsCoords[i+1][1][2] > TopSubPatchPoint[1][2]:
          BoundingPointAbove = LabelCoord
          break
      
      OriginalPatchPoints = [(self.PatchNode.GetNthFiducialLabel(i), self.PatchNode.GetMarkupPointVector(i,0)) for i in range(self.PatchNode.GetNumberOfFiducials())]
      NewPatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      NewPatchNode.SetName(self.PatchNode.GetName())
      
      # Copy points above interval having addition, if any
      for OriginalPoint in OriginalPatchPoints[:SubPatchIntervalStartIndex]:
        NewPatchNode.AddFiducialFromArray(OriginalPoint[1])
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, OriginalPoint[0])      
      
      
      # Populate interval with one more point than before
      (SrPolynomial, SaPolynomial) = self.CoordsPolyFit()
      NewSiLocations = np.linspace(BoundingPointAbove[1][2], BoundingPointBelow[1][2], SubPatchIntervalEndIndex - SubPatchIntervalStartIndex + 4)[1:-1]
      NewPointCoords = [[SrPolynomial(S), SaPolynomial(S), S] for S in NewSiLocations]
      for i, NewPoint in enumerate(NewPointCoords):
        NewPatchNode.AddFiducialFromArray(NewPoint)
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, "SubPatch-"+str(SubPatchIndex)+"_Point-"+str(i))
      
      # Copy points inferior to interval having addition, if any
      for OriginalPoint in OriginalPatchPoints[SubPatchIntervalEndIndex+1:]:
        NewPatchNode.AddFiducialFromArray(OriginalPoint[1])
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, OriginalPoint[0])
      
      # Update self attributes
      self.SubPatchesPointCounts[SubPatchIndex] += 1
      self.PatchNode.Copy(NewPatchNode)
      
      return self.PatchNode
      
    def RemovePointFromSubPatch(self, SubPatchIndex):
      if SubPatchIndex > len(self.SubPatchesPointCounts):
        print "Non-existent patch interval - no points to remove"
        return self.PatchNode
      
      # Identify top and bottom points of SubPatch so they and the points they bound can be modified
      SubPatchIntervalStartIndex = sum(self.SubPatchesPointCounts[:SubPatchIndex])
      SubPatchIntervalEndIndex = sum(self.SubPatchesPointCounts[:SubPatchIndex]) + self.SubPatchesPointCounts[SubPatchIndex] - 1
      
      TopSubPatchPoint = (self.PatchNode.GetNthFiducialLabel(SubPatchIntervalStartIndex), self.PatchNode.GetMarkupPointVector(SubPatchIntervalStartIndex,0))
      BottomSubPatchPoint = (self.PatchNode.GetNthFiducialLabel(SubPatchIntervalEndIndex), self.PatchNode.GetMarkupPointVector(SubPatchIntervalEndIndex,0))
      
      # Find last point before, and first point after SubPatch's interval
      LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      for i, LabelCoord in enumerate(LabelsCoords[:-1]):
        if LabelsCoords[i+1][1][2] < BottomSubPatchPoint[1][2]:
          BoundingPointBelow = LabelCoord
          break
        
      LabelsCoords.reverse()
      for i, LabelCoord in enumerate(LabelsCoords[:-1]):
        if LabelsCoords[i+1][1][2] > TopSubPatchPoint[1][2]:
          BoundingPointAbove = LabelCoord
          break
      
      OriginalPatchPoints = [(self.PatchNode.GetNthFiducialLabel(i), self.PatchNode.GetMarkupPointVector(i,0)) for i in range(self.PatchNode.GetNumberOfFiducials())]
      NewPatchNode = slicer.vtkMRMLMarkupsFiducialNode()
      NewPatchNode.SetName(self.PatchNode.GetName())
      
      # Copy points superior to the interval having deletions, if any
      for OriginalPoint in OriginalPatchPoints[:SubPatchIntervalStartIndex]:
        NewPatchNode.AddFiducialFromArray(OriginalPoint[1])
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, OriginalPoint[0])      
      
      # Populate interval with one point fewer than before
      NewSiLocations = np.linspace(BoundingPointAbove[1][2], BoundingPointBelow[1][2], SubPatchIntervalEndIndex - SubPatchIntervalStartIndex + 2)[1:-1]
      (SrPolynomial, SaPolynomial) = self.CoordsPolyFit()
      NewPointCoords = [[SrPolynomial(S), SaPolynomial(S), S] for S in NewSiLocations]
      for i, NewPoint in enumerate(NewPointCoords):
        NewPatchNode.AddFiducialFromArray(NewPoint)
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, "SubPatch-"+str(SubPatchIndex)+"_Point-"+str(i))
      
      # Keep track of sub patches with self.SubPatchesPointCounts
      self.SubPatchesPointCounts[SubPatchIndex] -= 1
          
      # Copy points inferior to the interval having deletions, if any
      for OriginalPoint in OriginalPatchPoints[SubPatchIntervalEndIndex+1:]:
        NewPatchNode.AddFiducialFromArray(OriginalPoint[1])
        NewPatchNode.SetNthFiducialLabel(NewPatchNode.GetNumberOfFiducials()-1, OriginalPoint[0])
      
      if self.SubPatchesPointCounts[SubPatchIndex] == 0:
        self.SubPatchesPointCounts.__delitem__(SubPatchIndex)
        PatchPointsBeforeIndexShift = sum(self.SubPatchesPointCounts[:SubPatchIndex])
        # Adjust the subsequent Subpatch indices
        for j, SubPatch in enumerate(self.SubPatchesPointCounts[SubPatchIndex:]):
          for i, SubPatchPointNumber in enumerate(range(SubPatch)):
            PointsAlreadyRelabelled = sum(self.SubPatchesPointCounts[SubPatchIndex:j]) + i
            NewPatchNode.SetNthFiducialLabel(PatchPointsBeforeIndexShift+SubPatchPointNumber+PointsAlreadyRelabelled, "SubPatch-"+str(SubPatchIndex+j)+"_Point-"+str(i))
      
      self.PatchNode.Copy(NewPatchNode)
      
      return self.PatchNode
   
    def IdentifyOutliers(self):     # self.MarkupsNode remains the same EXCEPT that points identified as outliers are UNSELECTED
      #from np import math
      LabelsCoords = [(self.MarkupsNode.GetNthFiducialLabel(i), self.MarkupsNode.GetMarkupPointVector(i,0)) for i in range(self.MarkupsNode.GetNumberOfFiducials())]
      
      # Polynomial curve fit to see which points deviate the most
      (SrPolynomial, SaPolynomial) = self.CoordsPolyFit()
      
      RlFitSqErrors = []
      ApFitSqErrors = []
      print " R-L  - A-P"
      print "" 
      
      print "Polynomial fit errors"
      for PointIndex in range(self.MarkupsNode.GetNumberOfFiducials()):
        CurrentPointCoords = self.MarkupsNode.GetMarkupPointVector(PointIndex,0)
        RlFitSqErrors.append((CurrentPointCoords[0] - SrPolynomial(CurrentPointCoords[2]))**2)
        ApFitSqErrors.append((CurrentPointCoords[1] - SaPolynomial(CurrentPointCoords[2]))**2)
        print RlFitSqErrors[-1], "  ", ApFitSqErrors[-1]
   
      print "Mean: ", np.mean(RlFitSqErrors), " - ", np.mean(ApFitSqErrors) 
      print "StdDev: ", np.std(RlFitSqErrors), "  - ", np.std(ApFitSqErrors)
      print ""
   
      # Try local inter-landmark vector direction changes
      RsAngles = []
      AsAngles = []
      sVector = [0.0,0.0,1.0]    # For angle measurment with cross-product
      
      print "Inter-landmark angles (relative to vertical)"
      for i, LabelCoords in enumerate(LabelsCoords[1:], start=1):
        #InterLandmarkVector = [0.0,0.0,0.0]
        RsVector = [(LabelsCoords[i-1][1][0] - LabelCoords[1][0]), 0.0, (LabelsCoords[i-1][1][2] - LabelCoords[1][2])]
        AsVector = [0.0, (LabelsCoords[i-1][1][1] - LabelCoords[1][1]), (LabelsCoords[i-1][1][2] - LabelCoords[1][2])]
        RsDot = np.dot(RsVector, sVector) / np.linalg.norm(RsVector)
        AsDot = np.dot(AsVector, sVector) / np.linalg.norm(AsVector)
        RsAngle = np.math.acos(RsDot) * np.sign(RsVector[0])
        AsAngle = np.math.acos(AsDot) * np.sign(AsVector[1])
        RsAngles.append(RsAngle)
        AsAngles.append(AsAngle)
        print RsAngle, "  ", AsAngle
        
      print "Mean (of abs): ", np.mean([abs(Angle) for Angle in RsAngles]), "  - ", np.mean([abs(Angle) for Angle in AsAngles])
      print "StdDev: ", np.std(RsAngles), " - ", np.std(AsAngles)
      
      # Check angle/fit criteria for outlier identification
      for i, ((RlFitSqError, ApFitSqError),(RsAngle, AsAngle)) in enumerate(zip(zip(RlFitSqErrors, ApFitSqErrors), zip(RsAngles, AsAngles))[1:-1], start=1):
        CurrentRsAngle = RsAngles[i]
        if np.sign(CurrentRsAngle) != np.sign(RsAngles[i-1]) and np.sign(CurrentRsAngle) != np.sign(RsAngles[i+1]):
          # Outlier detected
          print "Outlier detected at ", self.MarkupsNode.GetNthFiducialLabel(i-1), " of ", self.MarkupsNode.GetName()
          self.OutlierIdentificationVotes[i] += 1
          self.MarkupsNode.SetNthFiducialSelected(i,0)    # Currently, unselect outliers
      
      print self.OutlierIdentificationVotes
      print ""
      
      return self.MarkupsNode
   
    def DeleteIdentifiedOutliers(self):
      NumOutliersDeleted = 0
      OrigNumMarkups = self.MarkupsNode.GetNumberOfFiducials()
      
      # Landmarks un-selected to identify suspected outliers, delete them
      for P in range(OrigNumMarkups):
        if not self.MarkupsNode.GetNthFiducialSelected(P-NumOutliersDeleted):
          self.MarkupsNode.RemoveMarkup(P-NumOutliersDeleted)
          NumOutliersDeleted += 1
          
      # Update self class variables
      self.OutlierIdentificationVotes = np.zeros(self.MarkupsNode.GetNumberOfFiducials())
   
      return self.MarkupsNode
      
    def UpdatePatchedSidePointNames(self, Node):
      CurrentSubPatchIndex = 0
      SubPatchPointIndex = 0
      CurrentPointIndex = 0
      
      while CurrentPointIndex < Node.GetNumberOfFiducials():
        CurrentPointName = Node.GetNthFiducialLabel(CurrentPointIndex)
        while CurrentPointName.__contains__("SubPatch-"):
          Node.SetNthFiducialLabel(CurrentPointIndex, "SubPatch-"+str(CurrentSubPatchIndex)+"_Point-"+str(SubPatchPointIndex))
          SubPatchPointIndex += 1
          CurrentPointIndex += 1
          CurrentPointName = Node.GetNthFiducialLabel(CurrentPointIndex)
        if SubPatchPointIndex != 0:
          SubPatchPointIndex = 0
          CurrentSubPatchIndex += 1
        else:
          CurrentPointIndex += 1
  
  def __init__(self, Markups, kmWindowSize, PolyFitDegree, BoundaryMultiplicity, OmissionIdentificationSpecificity, ImputationSpecificity):
    self.PatientModel = self.PatientTransverseProcesses(self, Markups, kmWindowSize, PolyFitDegree, BoundaryMultiplicity, OmissionIdentificationSpecificity, ImputationSpecificity)
   
  def UpdateParameters(self, kmWindowSize, PolyFitDegree, BoundaryMultiplicity, OmissionIdentificationSpecificity, ImputationSpecificity):
    self.PatientModel.kmWindowSize = int(kmWindowSize)
    self.PatientModel.PolyFitDegree = int(PolyFitDegree)
    self.PatientModel.BoundaryMultiplicity = int(BoundaryMultiplicity)
    self.PatientModel.OmissionIdentificationSpecificity = OmissionIdentificationSpecificity
    self.PatientModel.ImputationSpecificity = ImputationSpecificity
    
    LS = self.PatientModel.LeftSide
    if LS != None:
      LS.PolyFitDegree = int(PolyFitDegree)
      LS.BoundaryMultiplicity = int(BoundaryMultiplicity)
      LS.OmissionIdentificationSpecificity = OmissionIdentificationSpecificity
      LS.ImputationSpecificity = ImputationSpecificity
      
    RS = self.PatientModel.RightSide
    if RS != None:
      RS.PolyFitDegree = int(PolyFitDegree)
      RS.BoundaryMultiplicity = int(BoundaryMultiplicity)
      RS.OmissionIdentificationSpecificity = OmissionIdentificationSpecificity
      RS.ImputationSpecificity = ImputationSpecificity
   
  def ApplyNamingConvention(self, MarkupsNode):   # Renames points in MarkupsNode according to convention used by ModelToPatientRegistration extension
    # List names expected by ModelToPatientRegistration
    ExpectedLabels = ['T1L','T1R','T2L','T2R','T3L','T3R','T4L','T4R','T5L','T5R','T6L','T6R','T7L','T7R','T8L','T8R','T9L','T9R',\
      'T10L','T10R','T11L','T11R','T12L','T12R','L1L','L1R','L2L','L2R','L3L','L3R','L4L','L4R','L5L','L5R']
      
    # Get names currently used by MarkupsNode
    #IndicesLabels = [(i, MarkupsNode.GetNthFiducialLabel(i)) for i in range(MarkupsNode.GetNumberOfFiducials())]
    Labels = [MarkupsNode.GetNthFiducialLabel(i) for i in range(MarkupsNode.GetNumberOfFiducials())]
    
    """
    NewNode = slicer.vtkMRMLMarkupsFiducialNode()
    NewNode.SetName(MarkupsNode.GetName())
    for Coord in [MarkupsNode.GetMarkupPointVector(i,0) for i in range(MarkupsNode.GetNumberOfFiducials())]:
      NewNode.AddFiducialFromArray(Coord)
    """
    
    # Find some label in MarkupsNode which contains corresponds to some ExpectedLabel
    for i, ExpectedLabel in enumerate(ExpectedLabels):
      # Check each label of MarkupsNode against all ExpectedLabels
      for j, Label in enumerate(Labels):
        if Label.__contains__(ExpectedLabel):
          if j > i: # Check compatability of point set with naming convention
            print "Error - Tried to name more vertebrae than currently recognized"
            print " Does scan include cervical spine?"
            print " Maybe point sets were over-extrapolated."
            print " Leaving names unmodified."
            return
          else:
            #CurrentLabel = ExpectedLabels[i-j]
            for k, ExpectedLabel in enumerate(ExpectedLabels[i:MarkupsNode.GetNumberOfFiducials()+2+i]):
              #print ExpectedLabel
              MarkupsNode.SetNthFiducialLabel(k, ExpectedLabel)
            #return NewNode
    return True
   
class RepairLandmarksTest(ScriptedLoadableModuleTest):
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
    self.test_PreProcessLandmarks1()

  def test_PreProcessLandmarks1(self):
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
    logic = PreProcessLandmarksLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
