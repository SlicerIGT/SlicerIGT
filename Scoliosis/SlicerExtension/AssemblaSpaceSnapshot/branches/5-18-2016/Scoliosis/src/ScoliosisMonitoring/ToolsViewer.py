'''
Created on 28/03/2013

@author: Usuario
'''
import os

from __main__ import vtk, qt, ctk, slicer

class ToolsViewer():
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
        self.notVisibleStyle ="background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #d55, stop: 0.1 #e66, stop: 0.49 #c44, stop: 0.5 #b33, stop: 1 #c44);"
        self.visibleStyle =  "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #5d5, stop: 0.1 #6e6, stop: 0.49 #4c4, stop: 0.5 #3b3, stop: 1 #4c4);"
        self.noTrackingStyle = "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ddd, stop: 0.1 #eee, stop: 0.49 #ccc, stop: 0.5 #bbb, stop: 1 #ccc);"
        self.toolsWidget = qt.QFrame() 
        self.toolsWidget.setLayout( qt.QHBoxLayout() )
        
        # stylus semaphore
        self.stylusSemaphore=qt.QPushButton()
        self.stylusSemaphore.setEnabled(False)
        self.stylusSemaphore.setStyleSheet(self.noTrackingStyle)
        self.stylusSemaphore.setText("Stylus") 
        
        # probe semaphore
        self.probeSemaphore=qt.QPushButton()
        self.probeSemaphore.setEnabled(False)
        self.probeSemaphore.setStyleSheet(self.noTrackingStyle)
        self.probeSemaphore.setText("Probe") 
        
        # main reference semaphore
        self.mainReferenceSemaphore=qt.QPushButton()
        self.mainReferenceSemaphore.setEnabled(False)
        self.mainReferenceSemaphore.setStyleSheet(self.noTrackingStyle)
        self.mainReferenceSemaphore.setText("R1") 
        
        # second referencer semaphore
        self.secondReferenceSemaphore=qt.QPushButton()
        self.secondReferenceSemaphore.setEnabled(False)
        self.secondReferenceSemaphore.setStyleSheet(self.noTrackingStyle)
        self.secondReferenceSemaphore.setText("R2")
        
        # third referencer semaphore
        self.thirdReferenceSemaphore=qt.QPushButton()
        self.thirdReferenceSemaphore.setEnabled(False)
        self.thirdReferenceSemaphore.setStyleSheet(self.noTrackingStyle)
        self.thirdReferenceSemaphore.setText("R3")
        
        # wall semaphore
        self.wallSemaphore=qt.QPushButton()
        self.wallSemaphore.setEnabled(False)
        self.wallSemaphore.setStyleSheet(self.noTrackingStyle)
        self.wallSemaphore.setText("Wall")
        
        self.toolsWidget.layout().addWidget(self.stylusSemaphore) 
        self.toolsWidget.layout().addWidget(self.probeSemaphore) 
        self.toolsWidget.layout().addWidget(self.mainReferenceSemaphore) 
        self.toolsWidget.layout().addWidget(self.secondReferenceSemaphore) 
        self.toolsWidget.layout().addWidget(self.thirdReferenceSemaphore) 
        self.toolsWidget.layout().addWidget(self.wallSemaphore) 
         
        print("Constructor of ToolViewer executed")
        self.toolsWidget.show()
        
        
    def getToolsWidget(self):
      return self.toolsWidget
      
    def setModuleLogic(self,logic):   
        self.logic=logic
          
#     def listenToScene(self):     
#         igtl=slicer.modules.openigtlinkif
#         logic=igtl.logic()
#         logic.SetMRMLScene(slicer.mrmlScene)
#         #self.connectorNode=self.logic.getConnectorNode()
#         logic.AddObserver('ModifiedEvent',self.onConnectedEventCaptured)

     
#     def onConnectedEventCaptured(self, caller,  event):
#         print "An OpenIGTLink connection was captured!"
#         igtlConnectorNode = slicer.util.getNode("Plus Server Connection") 
#         if igtlConnectorNode is not None:
#             self.startListeningToTransformationsModifications()
            
    def startListeningToTransformationsModifications(self):    
        print "Tools viewer is listening to the scene"
        
        referenceToTrackerNode = slicer.util.getNode("ReferenceToTracker")
        referenceToTrackerNode.RemoveAllObservers()
        #if referenceToTrackerNode is not None:
        #    self.onReferenceTransformationModified()
        referenceToTrackerNode.AddObserver('ModifiedEvent', self.onReferenceTransformationModified)
        
        r2ToTrackerNode = slicer.util.getNode("R2ToTracker")
        r2ToTrackerNode.RemoveAllObservers()
        r2ToTrackerNode.AddObserver('ModifiedEvent', self.onR2TransformationModified)
        
        r3ToTrackerNode = slicer.util.getNode("R3ToTracker")
        r3ToTrackerNode.RemoveAllObservers()
        r3ToTrackerNode.AddObserver('ModifiedEvent', self.onR3TransformationModified)
       
        probeToReference = slicer.util.getNode("ProbeToReference")
        probeToReference.RemoveAllObservers()
        #if probeToReference is not None:
        #    self.onProbeTransformationModified()
        probeToReference.AddObserver('ModifiedEvent', self.onProbeTransformationModified)
       
        stylusToReference = slicer.util.getNode("StylusTipToReference")
        stylusToReference.RemoveAllObservers()
        #if stylusToReference is not None:
        #    self.onStylusTransformationModified()
        stylusToReference.AddObserver('ModifiedEvent', self.onStylusTransformationModified)
        
        wallToTracker = slicer.util.getNode("WallToTracker")
        wallToTracker.RemoveAllObservers()
        wallToTracker.AddObserver('ModifiedEvent', self.onWallTransformationModified)
       
        stylusModelNode=self.logic.getStylusModel()
        self.stylusModelDisplayNode=stylusModelNode.GetDisplayNode()
        self.stylusModelDisplayNode.SetVisibility(False)    
        
        
    def onReferenceTransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("ReferenceToTracker"):
          self.mainReferenceSemaphore.setStyleSheet(self.visibleStyle) 
        else:
          self.mainReferenceSemaphore.setStyleSheet(self.notVisibleStyle)  

    def onR2TransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("R2ToTracker"):
          self.secondReferenceSemaphore.setStyleSheet(self.visibleStyle) 
        else:
          self.secondReferenceSemaphore.setStyleSheet(self.notVisibleStyle)   
          
    def onR3TransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("R3ToTracker"):
          self.thirdReferenceSemaphore.setStyleSheet(self.visibleStyle) 
        else:
          self.thirdReferenceSemaphore.setStyleSheet(self.notVisibleStyle)                  
   
    
    def onWallTransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("WallToTracker"):
          self.wallSemaphore.setStyleSheet(self.visibleStyle) 
          self.logic.setTrackerToWallTransformationMatrix()
        else:
          self.wallSemaphore.setStyleSheet(self.notVisibleStyle)  
        
    def onProbeTransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("ProbeToReference"):
          self.probeSemaphore.setStyleSheet(self.visibleStyle) 
          self.logic.showRedSliceIn3D(True)
          #print "Probe Transformation is valid!!"
        else:
          self.probeSemaphore.setStyleSheet(self.notVisibleStyle) 
          #slicer.util.resetSliceViews()  
          self.logic.showRedSliceIn3D(False)
          #print "Probe Transformation is invalid!!" 
          
    def onStylusTransformationModified(self, caller,  event):
        if self.logic.isValidTransformation("StylusTipToReference"):
          self.stylusSemaphore.setStyleSheet(self.visibleStyle) 
          self.stylusModelDisplayNode.SetVisibility(True)  
        else:
          self.stylusSemaphore.setStyleSheet(self.notVisibleStyle)
          self.stylusModelDisplayNode.SetVisibility(False)    
                    
    def listenToTransformationsSentToTheScene(self):
      self.sceneObserver = slicer.mrmlScene.AddObserver('ModifiedEvent', self.onTransformationsSentToTheScene)  
      
    def doNotListenToTransformationsSentToTheScene(self):
      slicer.mrmlScene.RemoveObserver(self.sceneObserver)

    def onTransformationsSentToTheScene(self, caller, event):
      image_Reference = slicer.util.getNode("Image_Reference")   
      if image_Reference is not None: 
        self.doNotListenToTransformationsSentToTheScene() 
        self.logic.associateTransformations()  
        self.startListeningToTransformationsModifications()
        slicer.util.resetSliceViews() 
        slicer.util.resetThreeDViews() 
         
        