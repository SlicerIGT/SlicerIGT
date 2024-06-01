import logging
import os
from typing import Annotated, Optional

import vtk
import qt
from qt import QObject

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin
from slicer.parameterNodeWrapper import (
    parameterNodeWrapper,
    WithinRange,
)

from slicer import vtkMRMLScalarVolumeNode


#
# SequenceReplay
#


class SequenceReplay(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Sequence Replay")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "IGT")]
        self.parent.dependencies = []
        self.parent.contributors = ["Tamas Ungi (Queen's University)"]
        # TODO: update with short description of the module and a link to online module documentation
        # _() function marks text as translatable to other languages
        self.parent.helpText = _("""
This is an example of scripted loadable module bundled in an extension.
See more information in <a href="https://github.com/organization/projectname#SequenceReplay">module documentation</a>.
""")
        # TODO: replace with organization, grant and thanks
        self.parent.acknowledgementText = _("""
This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc., Andras Lasso, PerkLab,
and Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
""")

        # Additional initialization step after application startup is complete
        slicer.app.connect("startupCompleted()", registerSampleData)


#
# Register sample data sets in Sample Data module
#


def registerSampleData():
    """Add data sets to Sample Data module."""
    # It is always recommended to provide sample data for users to make it easy to try the module,
    # but if no sample data is available then this method (and associated startupCompeted signal connection) can be removed.

    import SampleData

    iconsPath = os.path.join(os.path.dirname(__file__), "Resources/Icons")

    # To ensure that the source code repository remains small (can be downloaded and installed quickly)
    # it is recommended to store data sets that are larger than a few MB in a Github release.

    # SequenceReplay1
    SampleData.SampleDataLogic.registerCustomSampleDataSource(
        # Category and sample name displayed in Sample Data module
        category="SequenceReplay",
        sampleName="SequenceReplay1",
        # Thumbnail should have size of approximately 260x280 pixels and stored in Resources/Icons folder.
        # It can be created by Screen Capture module, "Capture all views" option enabled, "Number of images" set to "Single".
        thumbnailFileName=os.path.join(iconsPath, "SequenceReplay1.png"),
        # Download URL and target file name
        uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95",
        fileNames="SequenceReplay1.nrrd",
        # Checksum to ensure file integrity. Can be computed by this command:
        #  import hashlib; print(hashlib.sha256(open(filename, "rb").read()).hexdigest())
        checksums="SHA256:998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95",
        # This node name will be used when the data set is loaded
        nodeNames="SequenceReplay1",
    )

    # SequenceReplay2
    SampleData.SampleDataLogic.registerCustomSampleDataSource(
        # Category and sample name displayed in Sample Data module
        category="SequenceReplay",
        sampleName="SequenceReplay2",
        thumbnailFileName=os.path.join(iconsPath, "SequenceReplay2.png"),
        # Download URL and target file name
        uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97",
        fileNames="SequenceReplay2.nrrd",
        checksums="SHA256:1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97",
        # This node name will be used when the data set is loaded
        nodeNames="SequenceReplay2",
    )


#
# SequenceReplayParameterNode
#


@parameterNodeWrapper
class SequenceReplayParameterNode:
    """
    The parameters needed by module.
    """
    imageThreshold: Annotated[float, WithinRange(-100, 500)] = 100
    invertThreshold: bool = False
    thresholdedVolume: vtkMRMLScalarVolumeNode
    invertedVolume: vtkMRMLScalarVolumeNode


#
# SequenceReplayWidget
#


class SequenceReplayWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None) -> None:
        """Called when the user opens the module the first time and the widget is initialized."""
        ScriptedLoadableModuleWidget.__init__(self, parent)
        VTKObservationMixin.__init__(self)  # needed for parameter node observation
        self.logic = None
        self._parameterNode = None
        self._parameterNodeGuiTag = None
        
        self._addingNodes = False
        self._importingScene = False
        self.currentTransformableNodeIds = []  # List of transformable node IDs that currently have a checkbox on the GUI

    def setup(self) -> None:
        """Called when the user opens the module the first time and the widget is initialized."""
        ScriptedLoadableModuleWidget.setup(self)
        
        # Load widget from .ui file (created by Qt Designer).
        # Additional widgets can be instantiated manually and added to self.layout.
        uiWidget = slicer.util.loadUI(self.resourcePath("UI/SequenceReplay.ui"))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)

        # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
        # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
        # "setMRMLScene(vtkMRMLScene*)" slot.
        uiWidget.setMRMLScene(slicer.mrmlScene)

        # Create logic class. Logic implements all computations that should be possible to run
        # in batch mode, without a graphical user interface.
        self.logic = SequenceReplayLogic()

        # Connections
        
        # These connections ensure that we update parameter node when scene is closed
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)
        
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.NodeAddedEvent, self._updateGui)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.NodeRemovedEvent, self._updateGui)
        
        # Observe scene load events to be able to update the GUI after a scene is loaded
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartImportEvent, self.onStartImport)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndImportEvent, self.onEndImport)

        # Buttons
        self.ui.addButton.connect("clicked(bool)", self.onAddButton)

        # Make sure parameter node is initialized (needed for module reload)
        self.initializeParameterNode()

    def cleanup(self) -> None:
        """Called when the application closes and the module widget is destroyed."""
        self.removeObservers()

    def enter(self) -> None:
        """Called each time the user opens this module."""
        # Make sure parameter node exists and observed
        self.initializeParameterNode()
        
        # Collapse dataprobe collapsible button
        dataProbeCollapsibleWidget = slicer.util.findChildren(name='DataProbeCollapsibleWidget')[0]
        dataProbeCollapsibleWidget.collapsed = True
        
        self._updateGui()

    def exit(self) -> None:
        """Called each time the user opens a different module."""
        # Do not react to parameter node changes (GUI will be updated when the user enters into the module)
        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self._parameterNodeGuiTag = None
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._updateGui)
    
    def onStartImport(self, caller, event) -> None:
        """Called just before a scene is imported."""
        # Pause rendering to make scene import faster
        slicer.app.pauseRender()
        self._importingScene = True
        
    def onEndImport(self, caller, event) -> None:
        """Called just after a scene is imported."""
        # Resume rendering
        self._importingScene = False
        slicer.app.resumeRender()
        
        # If this module is shown while the scene is imported then recreate a new parameter node immediately
        if self.parent.isEntered:
            self.initializeParameterNode()
        
    def onSceneStartClose(self, caller, event) -> None:
        """Called just before the scene is closed."""
        # Parameter node will be reset, do not use it anymore
        self.setParameterNode(None)

    def onSceneEndClose(self, caller, event) -> None:
        """Called just after the scene is closed."""
        # If this module is shown while the scene is closed then recreate a new parameter node immediately
        if self.parent.isEntered:
            self.initializeParameterNode()

    def initializeParameterNode(self) -> None:
        """Ensure parameter node exists and observed."""
        # Parameter node stores all user choices in parameter values, node selections, etc.
        # so that when the scene is saved and reloaded, these settings are restored.

        self.setParameterNode(self.logic.getParameterNode())


    def setParameterNode(self, inputParameterNode: Optional[SequenceReplayParameterNode]) -> None:
        """
        Set and observe parameter node.
        Observation is needed because when the parameter node is changed then the GUI must be updated immediately.
        """

        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._updateGui)
        self._parameterNode = inputParameterNode
        if self._parameterNode:
            # Note: in the .ui file, a Qt dynamic property called "SlicerParameterName" is set on each
            # ui element that needs connection.
            self._parameterNodeGuiTag = self._parameterNode.connectGui(self.ui)
            self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._updateGui)
            self._updateGui()

    def onAddButton(self) -> None:
        """Add a new sequence browser to the scene using the selected nodes as proxy nodes."""
        self._addingNodes = True
        
        # Create a sequence browser node
        sequenceBrowserNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode")
        sequenceBrowserNode.SetPlaybackRateFps(30)
        sequencesLogic = slicer.modules.sequences.logic()
        
        # Iterate through all selected nodes with their node IDs and add them to the sequence browser
        for i in range(self.ui.nodesCollapsibleButton.layout().count()):
            checkBox = self.ui.nodesCollapsibleButton.layout().itemAt(i).widget()
            if checkBox.isChecked():
                node = slicer.mrmlScene.GetNodeByID(self.currentTransformableNodeIds[i])
                sequenceNode = sequencesLogic.AddSynchronizedNode(None, node, sequenceBrowserNode)
                sequenceBrowserNode.SetRecording(sequenceNode, True)
                sequenceBrowserNode.SetPlayback(sequenceNode, True)                
        
        # Make sure the last added sequence browser is selected in Slicer's seqeuence browser module
        slicer.modules.sequences.toolBar().setActiveBrowserNode(sequenceBrowserNode)
        self.addObserver(sequenceBrowserNode, vtk.vtkCommand.ModifiedEvent, self._updateGui)
        
        self._addingNodes = False
        self._updateGui()
    
    def onSequenceBrowserButtonClicked(self, sequenceBrowserNodeId: str) -> None:
        # Make sure the clicked sequence browser is selected in Slicer's seqeuence browser module
        sequenceBrowserNode = slicer.mrmlScene.GetNodeByID(sequenceBrowserNodeId)
        slicer.modules.sequences.toolBar().setActiveBrowserNode(sequenceBrowserNode)
        
        # Iterate through all sequence browser nodes. Stop replay in all except the clicked one. Start play in the clicked one.
        for node in slicer.util.getNodesByClass("vtkMRMLSequenceBrowserNode"):
            if node.GetID() == sequenceBrowserNodeId:
                node.SetPlaybackActive(True)
            else:
                node.SetPlaybackActive(False)
        
    def _updateGui(self, caller=None, event=None) -> None:
        """Update GUI based on parameter node and mrml scene state."""
        if self._addingNodes:
            return
        
        if self._importingScene:
            return
        
        # Find all sequence browser nodes and create a button for each of them
        
        sequenceBrowserNodes = slicer.util.getNodesByClass("vtkMRMLSequenceBrowserNode")
        
        sequenceBrowsersLayout = self.ui.sequenceBrowsersCollapsibleButton.layout()
        
        # Remove all buttons from the layout
        for i in reversed(range(sequenceBrowsersLayout.count())):
            widget = sequenceBrowsersLayout.itemAt(i).widget()
            self.buttonMapper.removeMappings(widget)
            widget.setParent(None)
            widget.deleteLater()
            
        # Create a button for each node
        for node in sequenceBrowserNodes:
            button = qt.QPushButton(node.GetName())
            button.setCheckable(False)
            button.clicked.connect(lambda checked, buttonId=node.GetID(): self.onSequenceBrowserButtonClicked(buttonId))
            sequenceBrowsersLayout.addWidget(button)
        
        # Find all transformable nodes in the scene and create a checkbox for each of them
        
        nodeTypes = ["vtkMRMLTransformNode", "vtkMRMLVolumeNode", "vtkMRMLMarkupsNode"]
        
        transformableNodeIds = []
        
        for nodeType in nodeTypes:
            nodeList = slicer.util.getNodesByClass(nodeType)
            for node in nodeList:
                if not node.GetHideFromEditors():
                    transformableNodeIds.append(node.GetID())
                    
        # Check if transformableNodeIds is different from currentTransformableNodeIds
        if transformableNodeIds != self.currentTransformableNodeIds:
            self.currentTransformableNodeIds = transformableNodeIds
            
            checkboxesLayout = self.ui.nodesCollapsibleButton.layout()
            
            # Remove all checkboxes from the layout
            for i in reversed(range(checkboxesLayout.count())): 
                checkboxesLayout.itemAt(i).widget().setParent(None)
                checkboxesLayout.itemAt(i).widget().deleteLater()
            
            # Create a checkbox for each node
            for nodeId in transformableNodeIds:
                node = slicer.mrmlScene.GetNodeByID(nodeId)
                checkBox = qt.QCheckBox(node.GetName())
                checkboxesLayout.addWidget(checkBox)


#
# SequenceReplayLogic
#


class SequenceReplayLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self) -> None:
        """Called when the logic class is instantiated. Can be used for initializing member variables."""
        ScriptedLoadableModuleLogic.__init__(self)

    def getParameterNode(self):
        return SequenceReplayParameterNode(super().getParameterNode())

#
# SequenceReplayTest
#


class SequenceReplayTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear()

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_SequenceReplay1()

    def test_SequenceReplay1(self):
        """Ideally you should have several levels of tests.  At the lowest level
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

        # Get/create input data

        import SampleData

        registerSampleData()
        inputVolume = SampleData.downloadSample("SequenceReplay1")
        self.delayDisplay("Loaded test data set")

        inputScalarRange = inputVolume.GetImageData().GetScalarRange()
        self.assertEqual(inputScalarRange[0], 0)
        self.assertEqual(inputScalarRange[1], 695)

        outputVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
        threshold = 100

        # Test the module logic

        logic = SequenceReplayLogic()


        self.delayDisplay("Test passed")
