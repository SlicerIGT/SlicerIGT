'''
Created on 28/03/2013

@author: Usuario
'''
import os
from VolumeRenderingPropertiesMenu import *

from __main__ import vtk, qt, ctk, slicer

class VolumeRenderingViewer():
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
        self.renderingLabel=qt.QLabel("Renderings:")
        
        print("Constructor of VolumeRenderingViewer executed")
        #self.listWidget.show()
        self.currentItem=None
        
        self.volumeButtonsFrame=qt.QFrame()
        self.volumeButtonsFrame.setLayout(qt.QVBoxLayout())
        self.volumeButtonsLayout=self.volumeButtonsFrame.layout()
        
        self.showVolumeRenderingButton = qt.QPushButton("Show")
        self.showVolumeRenderingButton.toolTip = "Show the volume rendering"
        self.showVolumeRenderingButton.setEnabled(False)
        self.showVolumeRenderingButton.connect('clicked(bool)', self.onShowVolumeRenderingButtonClicked)
        
        self.hideVolumeRenderingButton = qt.QPushButton("Hide")
        self.hideVolumeRenderingButton.toolTip = "Hide the volume rendering"
        self.hideVolumeRenderingButton.setEnabled(False)
        self.hideVolumeRenderingButton.connect('clicked(bool)', self.onHideVolumeRenderingButtonClicked)
        
        self.modifyVolumeRenderingButton = qt.QPushButton("Modify")
        self.modifyVolumeRenderingButton.toolTip = "Modify the properties of the volume rendering"
        self.modifyVolumeRenderingButton.setEnabled(False)
        self.modifyVolumeRenderingButton.connect('clicked(bool)', self.onModifyVolumeRenderingButtonClicked)
        
        self.volumeButtonsLayout.addWidget(self.renderingLabel )
        self.volumeButtonsLayout.addWidget(self.showVolumeRenderingButton)
        self.volumeButtonsLayout.addWidget(self.hideVolumeRenderingButton)
        self.volumeButtonsLayout.addWidget(self.modifyVolumeRenderingButton)
        
        
        self.volumesFrameLayout.addWidget(self.listWidget)
        self.volumesFrameLayout.addWidget(self.volumeButtonsFrame)
        
        self.volumeRenderingPropertiesMenu = VolumeRenderingPropertiesMenu()
        self.volumeRenderingPropertiesMenu.threshold.connect('valuesChanged(double,double)', self.onThresholdSliderMoved)
        self.volumeRenderingPropertiesMenu.colorRangeSlider.connect('valuesChanged(double,double)', self.onColorSliderMoved)
        self.volumeRenderingPropertiesMenu.checkBoxVisible3D.connect("stateChanged(int)",self.onVisible3DChanged)
        
        self.scalarRange = [0., 255.]
        
    def getVolumeRenderingViewerWidget(self):
        return self.volumesFrame
           
    def setModuleLogic(self,logic):   
        self.logic=logic
           
    def listenToScene(self):
        vr=slicer.modules.volumerendering
        vrl=vr.logic()  
        vrl.SetMRMLScene(slicer.mrmlScene)
        self.sceneObserver = vrl.AddObserver('ModifiedEvent', self.onVolumeRenderingAdded)
        #self.sceneObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onVolumeAdded)
        
    def onVolumeRenderingAdded(self, caller,  event):
        print("A volume rendering was modified")
        self.showVolumeRenderingButton.setEnabled(True)
        self.hideVolumeRenderingButton.setEnabled(True)
        self.modifyVolumeRenderingButton.setEnabled(True)
    
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
                            isAlreadyInList=node.GetName()+"Rendering"==self.listWidget.item(j).text()
                            j=j+1
                        if not isAlreadyInList:
                            print('A volume rendering was added to the scene !')
                            self.listWidget.addItem(node.GetName()+"Rendering")
        
    def addItem(self,item):
        self.listWidget.addItem(item)
    
     
    
    def onShowVolumeRenderingButtonClicked(self):
        item = self.listWidget.currentItem()
        if item==None:
          ret=qt.QMessageBox.warning(self.listWidget, 'Volume Rendering List', 'You must select a volume rendering to show.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )
          return
         
        vrDisplayNode=slicer.util.getNode(item.text())
        if vrDisplayNode.GetVisibility()==True:
           vrDisplayNode.SetVisibility(False)
        vrDisplayNode.SetVisibility(True)  
        
    def onHideVolumeRenderingButtonClicked(self):
        item = self.listWidget.currentItem()
        if item==None:
          ret=qt.QMessageBox.warning(self.listWidget, 'Volume Rendering List', 'You must select a volume rendering to show.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )
          return
         
        vrDisplayNode=slicer.util.getNode(item.text())
        vrDisplayNode.SetVisibility(False)      
        
    def onModifyVolumeRenderingButtonClicked(self):
        item = self.listWidget.currentItem()
        if item==None:
          ret=qt.QMessageBox.warning(self.listWidget, 'Volume Rendering List', 'You must select a volume rendering to show.', qt.QMessageBox.Ok , qt.QMessageBox.Ok )
          return
         
        vrDisplayNode=slicer.util.getNode(item.text())
        self.currentVolumeRendering=vrDisplayNode   
        self.volumeRenderingPropertiesMenu.show()
            
    def onRemoveActionTriggered(self):  
        item = self.listWidget.currentItem()
        node=slicer.util.getNode(item.text())
        node=slicer.vtkMRMLModelNode.SafeDownCast(node)
        currentDisplayNode=node.GetDisplayNode() 
        slicer.mrmlScene.RemoveNode(currentDisplayNode)
        slicer.mrmlScene.RemoveNode(node)
        # Delete the item
        self.listWidget.takeItem(self.listWidget.row(item))
        
        
    def onThresholdSliderMoved(self,minValue,maxValue):
        # Get the current display node
        print("Opacity Slider movement captured!")
        #print opacity
        volumePropertyNode=self.currentVolumeRendering.GetVolumePropertyNode()    
        scalarOpacity=volumePropertyNode.GetScalarOpacity()
        scalarOpacity.RemoveAllPoints()
        scalarOpacity.AddPoint(self.scalarRange[0], 0.)
        scalarOpacity.AddPoint(minValue, 0.)
        scalarOpacity.AddPoint(maxValue, 1.)
        scalarOpacity.AddPoint(self.scalarRange[1], 1.)
        #colorTransfer=volumePropertyNode.GetColor()  
    
    def onColorSliderMoved(self,minValue,maxValue):
        # Get the current display node
        print("Color Slider movement captured!")
        #print opacity
        volumePropertyNode=self.currentVolumeRendering.GetVolumePropertyNode() 
        colorTransfer=volumePropertyNode.GetColor()  
        colorTransfer.RemoveAllPoints()
        black = [0., 0., 0.]
        white = [1., 1., 1.]
        colorTransfer.AddRGBPoint(self.scalarRange[0], black[0], black[1], black[2])
        colorTransfer.AddRGBPoint(minValue, black[0], black[1], black[2])
        colorTransfer.AddRGBPoint(maxValue, white[0], white[1], white[2]);
        colorTransfer.AddRGBPoint(self.scalarRange[1], white[0], white[1], white[2]);   
   
        
        
    def onVisible3DChanged(self,isVisible):
        #print isVisible
        self.currentVolumeRendering.SetVisibility(isVisible)  