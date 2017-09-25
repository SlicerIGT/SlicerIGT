'''
Created on 28/03/2013

@author: Usuario
'''
import os

from __main__ import vtk, qt, ctk, slicer

class VolumesViewer():
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
        """
        path=slicer.modules.usguidedprocedure.path
        modulePath=os.path.dirname(path)
        loadedDataGUIfile=os.path.join(modulePath,"USGuidedWizard/loadedData.ui")
        f = qt.QFile(loadedDataGUIfile)
        #f = qt.QFile('C:/Users/Usuario/devel/slicelets/USGuidedProcedure/USGuidedWizard/fiducials.ui')
        f.open(qt.QFile.ReadOnly)
        loader = qt.QUiLoader()
        self.loadedDataWidget = loader.load(f)
        f.close()
        """
        self.volumesFrame= qt.QFrame()
        self.volumesFrame.setLayout(qt.QHBoxLayout())
        self.volumesFrameLayout=self.volumesFrame.layout()
        self.listWidget = qt.QListWidget() 
       
        self.volumesLabel=qt.QLabel("Volumes:")
        print("Constructor of VolumeRenderingViewer executed")
        #self.listWidget.show()
        self.currentItem=None
        
        self.volumeButtonsFrame=qt.QFrame()
        self.volumeButtonsFrame.setLayout(qt.QVBoxLayout())
        self.volumeButtonsLayout=self.volumeButtonsFrame.layout()
        
        self.exploreVolumeButton = qt.QPushButton("Explore")
        self.exploreVolumeButton.toolTip = "Show the selected volume"
        self.exploreVolumeButton.setEnabled(False)
        self.exploreVolumeButton.connect('clicked(bool)', self.onExploreVolumeClicked)
        
        self.hideVolumeButton = qt.QPushButton("Hide")
        self.hideVolumeButton.toolTip = "Hide the volume"
        self.hideVolumeButton.setEnabled(False)
        self.hideVolumeButton.connect('clicked(bool)', self.onHideVolumeClicked)
        
        self.resliceVolumeButton = qt.QPushButton("Reslice")
        self.resliceVolumeButton.toolTip = "Reslice the volume with a driver"
        self.resliceVolumeButton.setEnabled(False)
        self.resliceVolumeButton.connect('clicked(bool)', self.onResliceVolumeClicked)
        
     
        self.volumeButtonsLayout.addWidget(self.volumesLabel)
        self.volumeButtonsLayout.addWidget(self.exploreVolumeButton)
        self.volumeButtonsLayout.addWidget(self.hideVolumeButton)
        self.volumeButtonsLayout.addWidget(self.resliceVolumeButton)
        
        
        self.volumesFrameLayout.addWidget(self.listWidget)
        self.volumesFrameLayout.addWidget(self.volumeButtonsFrame)
        
    def getVolumesViewerWidget(self):
        return self.volumesFrame
           
    def setModuleLogic(self,logic):   
        self.logic=logic
           
    def listenToScene(self):
        vl=slicer.modules.volumes
        vl=vl.logic()  
        vl.SetMRMLScene(slicer.mrmlScene)
        self.sceneObserver = vl.AddObserver('ModifiedEvent', self.onVolumeAdded)
        #self.sceneObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onVolumeAdded)
        
    def onVolumeAdded(self, caller,  event):
        print("Volumes module was modified")
        
        numVolumes=slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScalarVolumeNode") 
        print "Number of volumes: " + str(numVolumes)
        #print('A model was added to the scene !')
        if numVolumes>0:
            for i in xrange(0,numVolumes):
                node = slicer.mrmlScene.GetNthNodeByClass(i, "vtkMRMLScalarVolumeNode")
                if not (node==None): 
                    if (not (node.GetName() == "Image_Reference")):     
                        print(node.GetName())  
                        isAlreadyInList=False 
                        j=0
                        while ((j < self.listWidget.count) and (not isAlreadyInList)):
                            isAlreadyInList=node.GetName()==self.listWidget.item(j).text()
                            j=j+1
                        if not isAlreadyInList:
                            print('A volume was added to the scene !')
                            self.listWidget.addItem(node.GetName())
                            node=slicer.util.getNode(node.GetName())
                            node=slicer.vtkMRMLScalarVolumeNode.SafeDownCast(node)
                            self.logic.onVolumeAdded(node)
                            self.exploreVolumeButton.setEnabled(True)
                            self.hideVolumeButton.setEnabled(True)
                            self.resliceVolumeButton.setEnabled(True)
                            #print("Info of added node:")
    
    
        
    def addItem(self,item):
        self.listWidget.addItem(item)
    
        
    def onExploreVolumeClicked(self):
        print("Explore volume button clicked")
        item = self.listWidget.currentItem()
        if item==None:
             ret=qt.QMessageBox.warning(self.listWidget, 'Volumes List', 'You must select a volume to explore.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )
             return
         
        node=slicer.util.getNode(item.text())
        self.logic.disconnectDriverForSlice()
        self.logic.showReconstructedVolume(node.GetName())

   
    def onHideVolumeClicked(self):  
        self.logic.showRedSliceIn3D(True)
        greenWidgetCompNode=slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeGreen")
        greenWidgetCompNode.SetBackgroundVolumeID(None)
        yellowWidgetCompNode=slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeYellow")
        yellowWidgetCompNode.SetBackgroundVolumeID(None)    
        
        
    def onResliceVolumeClicked(self):
        print("Reslice volume button clicked")
        item = self.listWidget.currentItem()
        if item==None:
             ret=qt.QMessageBox.warning(self.listWidget, 'Volumes List', 'You must select a volume to reslice.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )
             return
         
        node=slicer.util.getNode(item.text())
        self.logic.disconnectDriverForSlice()
        self.logic.showRedSliceIn3D(True)
        self.logic.resliceVolumeWithDriver(node)     