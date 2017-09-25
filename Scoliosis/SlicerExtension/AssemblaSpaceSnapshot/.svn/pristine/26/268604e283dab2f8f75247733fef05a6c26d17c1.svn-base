'''
Created on 29/03/2013

@author: Usuario
'''
import os

from __main__ import vtk, qt, ctk, slicer


class VolumeRenderingPropertiesMenu(qt.QWidget):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
        qt.QWidget.__init__(self)
        self.propertiesMenuWidget=qt.QFrame()
        self.propertiesMenuWidget.setLayout(qt.QVBoxLayout())
        #self.checkBoxVisible2D = qt.QCheckBox()
        #self.checkBoxVisible2D.setTristate(False)
        self.checkBoxVisible3D = qt.QCheckBox()
        self.checkBoxVisible3D.setTristate(False)
        
        #self.propertiesMenuWidget.setCellWidget(0,0,qt.QLabel("Visible2D"))
        #self.propertiesMenuWidget.setCellWidget(0,1,self.checkBoxVisible2D)
        self.visible3DFrame=qt.QFrame()
        self.visible3DFrame.setLayout(qt.QHBoxLayout())
        self.visible3DFrame.layout().addWidget(qt.QLabel("Visible in 3D"))
        self.visible3DFrame.layout().addWidget(self.checkBoxVisible3D)
    
        
        self.thresholdFrame=qt.QFrame()
        self.thresholdFrame.setLayout(qt.QHBoxLayout())
        self.thresholdLabel = qt.QLabel("Opacity Range:")
        self.thresholdLabel.setToolTip("Set the opacity range.")
        self.thresholdFrame.layout().addWidget(self.thresholdLabel)
   
        self.threshold = ctk.ctkRangeWidget()
        self.threshold.spinBoxAlignment = 0xff # put enties on top

        self.threshold.minimum = 0.
        self.threshold.maximum = 255.
        self.threshold.singleStep = 1.
        # set min/max based on current range
        self.thresholdFrame.layout().addWidget(self.threshold)
        
        self.colorFrame=qt.QFrame()
        self.colorFrame.setLayout(qt.QHBoxLayout())
        self.colorLabel = qt.QLabel("Color Range:")
        self.colorLabel.setToolTip("Set the color range.")
        self.colorFrame.layout().addWidget(self.colorLabel)
   
        self.colorRangeSlider = ctk.ctkRangeWidget()
        self.colorRangeSlider.spinBoxAlignment = 0xff # put enties on top
        self.colorRangeSlider.singleStep = 0.01
        self.colorRangeSlider.minimum = 0.
        self.colorRangeSlider.maximum = 255.
        # set min/max based on current range
        self.colorFrame.layout().addWidget(self.colorRangeSlider)
        
        #success, lo, hi = self.getBackgroundScalarRange()
        #if success:
        #  self.threshold.minimum, self.threshold.maximum = lo, hi
        #  self.threshold.singleStep = (hi - lo) / 1000.
        
        #self.thresholdFrame.layout().addWidget(self.threshold)

        self.propertiesMenuWidget.layout().addWidget(self.visible3DFrame)
        self.propertiesMenuWidget.layout().addWidget(self.thresholdFrame )
        self.propertiesMenuWidget.layout().addWidget(self.colorFrame )
       
        
        #self.timer = qt.QTimer()


        #self.connections.append( (self.timer, 'timeout()', self.preview) )
        #self.connections.append( (self.threshold, 'valuesChanged(double,double)', self.onThresholdValuesChanged) )
        
        
    def show(self):
        self.propertiesMenuWidget.show()    
        
        
    def contextMenuEvent(self, event):
         menu = qt.QMenu(self)
         quitAction = menu.addAction("Quit")