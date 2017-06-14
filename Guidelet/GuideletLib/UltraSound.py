import os
from __main__ import vtk, qt, ctk, slicer
import logging
import time

class UltraSound(object):
  DEFAULT_IMAGE_SIZE = [800, 600, 1]

  def __init__(self, guideletParent):
    self.guideletParent = guideletParent
    self.captureDeviceName = self.guideletParent.parameterNode.GetParameter('PLUSCaptureDeviceName')
    self.referenceToRas = None

    from PlusRemote import PlusRemoteLogic
    self.plusRemoteLogic = PlusRemoteLogic()

    fileDir = os.path.dirname(__file__)
    iconPathRecord = os.path.join(fileDir, 'Resources', 'Icons', 'icon_Record.png')
    iconPathStop = os.path.join(fileDir, 'Resources', 'Icons', 'icon_Stop.png')

    if os.path.isfile(iconPathRecord):
      self.recordIcon = qt.QIcon(iconPathRecord)
    if os.path.isfile(iconPathStop):
      self.stopIcon = qt.QIcon(iconPathStop)

  def onConnectorNodeConnected(self):
    self.freezeUltrasoundButton.setText('Freeze')
    self.startStopRecordingButton.setEnabled(True)

  def onConnectorNodeDisconnected(self):
    self.freezeUltrasoundButton.setText('Un-freeze')
    if self.guideletParent.parameterNode.GetParameter('RecordingEnabledWhenConnectorNodeDisconnected') == 'False':
      self.startStopRecordingButton.setEnabled(False)

  def setupPanel(self, parentWidget):
    logging.debug('UltraSound.setupPanel')
    collapsibleButton = ctk.ctkCollapsibleButton()

    collapsibleButton.setProperty('collapsedHeight', 20)
    collapsibleButton.text = "Ultrasound"
    parentWidget.addWidget(collapsibleButton)

    procedureLayout = qt.QVBoxLayout(collapsibleButton)
    procedureLayout.setContentsMargins(12, 4, 4, 12)
    procedureLayout.setSpacing(4)

    ultrasoundLayout = qt.QFormLayout()
    ultrasoundLayout.setContentsMargins(12,4,4,4)
    ultrasoundLayout.setSpacing(4)

    self.startStopRecordingButton = qt.QPushButton("Start Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setIcon(self.recordIcon)
    self.startStopRecordingButton.setToolTip("If clicked, start recording")

    self.freezeUltrasoundButton = qt.QPushButton('Freeze')

    hbox = qt.QHBoxLayout()
    hbox.addWidget(self.startStopRecordingButton)
    hbox.addWidget(self.freezeUltrasoundButton)
    ultrasoundLayout.addRow(hbox)

    self.usFrozen=False

    self.brightnessSliderLabel = qt.QLabel()
    self.brightnessSliderLabel.text = 'Brightness '

    self.brightnessSliderWidget = ctk.ctkDoubleRangeSlider()
    self.brightnessSliderWidget.orientation = 'Horizontal'
    self.brightnessSliderWidget.singleStep = 1
    self.brightnessSliderWidget.minimum = 0
    self.brightnessSliderWidget.maximum = 255
    self.brightnessSliderWidget.minimumValue = 0
    self.brightnessSliderWidget.maximumValue = 255

    self.brigthnessContrastButtonNormal = qt.QPushButton()
    self.brigthnessContrastButtonNormal.text = "Normal"
    self.brigthnessContrastButtonNormal.setEnabled(True)

    self.brigthnessContrastButtonBright = qt.QPushButton()
    self.brigthnessContrastButtonBright.text = "Bright"
    self.brigthnessContrastButtonBright.setEnabled(True)

    self.brigthnessContrastButtonBrighter = qt.QPushButton()
    self.brigthnessContrastButtonBrighter.text = "Brighter"
    self.brigthnessContrastButtonBrighter.setEnabled(True)

    ultrasoundButtonsPresent = False
    ultrasoundSliderPresent = False

    ultrasoundConfiguration = self.guideletParent.parameterNode.GetParameter('UltrasoundBrightnessControl')

    if ultrasoundConfiguration == 'Slider' or ultrasoundConfiguration == 'Dual':
        ultrasoundSliderPresent = True

    if ultrasoundConfiguration == 'Buttons' or ultrasoundConfiguration == 'Dual':
        ultrasoundButtonsPresent = True

    if ultrasoundButtonsPresent == True:
        brightnessContrastBox = qt.QHBoxLayout()
        brightnessContrastBox.addWidget(self.brigthnessContrastButtonNormal)
        brightnessContrastBox.addWidget(self.brigthnessContrastButtonBright)
        brightnessContrastBox.addWidget(self.brigthnessContrastButtonBrighter)
        ultrasoundLayout.addRow(brightnessContrastBox)

    if ultrasoundSliderPresent == True:
      brightnessContrastSliderBox = qt.QHBoxLayout()
      brightnessContrastSliderBox.addWidget(self.brightnessSliderLabel)
      brightnessContrastSliderBox.addWidget(self.brightnessSliderWidget)
      ultrasoundLayout.addRow(brightnessContrastSliderBox)

    procedureLayout.addLayout(ultrasoundLayout)

    return collapsibleButton, ultrasoundLayout, procedureLayout

  def setupScene(self):
    logging.info("UltraSound.setupScene")

    '''
      ReferenceToRas transform is used in almost all IGT applications. Reference is the coordinate system
      of a tool fixed to the patient. Tools are tracked relative to Reference, to compensate for patient
      motion. ReferenceToRas makes sure that everything is displayed in an anatomical coordinate system, i.e.
      R, A, and S (Right, Anterior, and Superior) directions in Slicer are correct relative to any
      images or tracked tools displayed.
      ReferenceToRas is needed for initialization, so we need to set it up before calling Guidelet.setupScene().
    '''

    if self.referenceToRas is None or (self.referenceToRas and slicer.mrmlScene.GetNodeByID(self.referenceToRas.GetID()) is None):
      self.referenceToRas = slicer.util.getNode('ReferenceToRas')
      if self.referenceToRas is None:
        self.referenceToRas = slicer.vtkMRMLLinearTransformNode()
        self.referenceToRas.SetName("ReferenceToRas")
        m = self.guideletParent.logic.readTransformFromSettings('ReferenceToRas', self.guideletParent.configurationName)
        if m is None:
          m = self.guideletParent.logic.createMatrixFromString('1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1')
        self.referenceToRas.SetMatrixTransformToParent(m)
        slicer.mrmlScene.AddNode(self.referenceToRas)

    # live ultrasound
    liveUltrasoundNodeName = self.guideletParent.parameterNode.GetParameter('LiveUltrasoundNodeName')
    self.liveUltrasoundNode_Reference = slicer.util.getNode(liveUltrasoundNodeName)
    if not self.liveUltrasoundNode_Reference:
      imageSpacing=[0.2, 0.2, 0.2]
      # Create an empty image volume
      imageData=vtk.vtkImageData()
      imageData.SetDimensions(self.DEFAULT_IMAGE_SIZE)
      imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
      thresholder=vtk.vtkImageThreshold()
      thresholder.SetInputData(imageData)
      thresholder.SetInValue(0)
      thresholder.SetOutValue(0)
      # Create volume node
      self.liveUltrasoundNode_Reference=slicer.vtkMRMLScalarVolumeNode()
      self.liveUltrasoundNode_Reference.SetName(liveUltrasoundNodeName)
      self.liveUltrasoundNode_Reference.SetSpacing(imageSpacing)
      self.liveUltrasoundNode_Reference.SetImageDataConnection(thresholder.GetOutputPort())
      # Add volume to scene
      slicer.mrmlScene.AddNode(self.liveUltrasoundNode_Reference)
      displayNode=slicer.vtkMRMLScalarVolumeDisplayNode()
      slicer.mrmlScene.AddNode(displayNode)
      colorNode = slicer.util.getNode('Grey')
      displayNode.SetAndObserveColorNodeID(colorNode.GetID())
      self.liveUltrasoundNode_Reference.SetAndObserveDisplayNodeID(displayNode.GetID())

    self.setupResliceDriver()

  def setupResliceDriver(self):
    layoutManager = slicer.app.layoutManager()
    # Show ultrasound in red view.
    redSlice = layoutManager.sliceWidget('Red')
    redSliceLogic = redSlice.sliceLogic()
    redSliceLogic.GetSliceCompositeNode().SetBackgroundVolumeID(self.liveUltrasoundNode_Reference.GetID())
    # Set up volume reslice driver.
    resliceLogic = slicer.modules.volumereslicedriver.logic()
    if resliceLogic:
      redNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
      # Typically the image is zoomed in, therefore it is faster if the original resolution is used
      # on the 3D slice (and also we can show the full image and not the shape and size of the 2D view)
      redNode.SetSliceResolutionMode(slicer.vtkMRMLSliceNode.SliceResolutionMatchVolumes)
      resliceLogic.SetDriverForSlice(self.liveUltrasoundNode_Reference.GetID(), redNode)
      resliceLogic.SetModeForSlice(6, redNode) # Transverse mode, default for PLUS ultrasound.
      resliceLogic.SetFlipForSlice(False, redNode)
      resliceLogic.SetRotationForSlice(180, redNode)
      redSliceLogic.FitSliceToAll()
    else:
      logging.warning('Logic not found for Volume Reslice Driver')

    self.liveUltrasoundNode_Reference.SetAndObserveTransformNodeID(self.referenceToRas.GetID())

  def setupConnections(self):
    self.startStopRecordingButton.connect('clicked(bool)', self.onStartStopRecordingClicked)
    self.freezeUltrasoundButton.connect('clicked()', self.onFreezeUltrasoundClicked)
    self.brightnessSliderWidget.connect('valuesChanged(double, double)', self.onBrightnessSliderChanged)
    self.brigthnessContrastButtonNormal.connect('clicked()', self.onBrightnessContrastNormalClicked)
    self.brigthnessContrastButtonBright.connect('clicked()', self.onBrightnessContrastBrightClicked)
    self.brigthnessContrastButtonBrighter.connect('clicked()', self.onBrightnessContrastBrighterClicked)

  def disconnect(self):
    self.startStopRecordingButton.disconnect('clicked(bool)', self.onStartStopRecordingClicked)
    self.freezeUltrasoundButton.disconnect('clicked()', self.onFreezeUltrasoundClicked)
    self.brigthnessContrastButtonNormal.disconnect('clicked()', self.onBrightnessContrastNormalClicked)
    self.brigthnessContrastButtonBright.disconnect('clicked()', self.onBrightnessContrastBrightClicked)
    self.brigthnessContrastButtonBrighter.disconnect('clicked()', self.onBrightnessContrastBrighterClicked)

  def createPlusConnector(self):
    connectorNode = slicer.util.getNode('PlusConnector')
    if not connectorNode:
      connectorNode = slicer.vtkMRMLIGTLConnectorNode()
      connectorNode.SetLogErrorIfServerConnectionFailed(False)
      slicer.mrmlScene.AddNode(connectorNode)
      connectorNode.SetName('PlusConnector')
      hostNamePort = self.guideletParent.parameterNode.GetParameter('PlusServerHostNamePort') # example: "localhost:18944"
      [hostName, port] = hostNamePort.split(':')
      connectorNode.SetTypeClient(hostName, int(port))
      logging.debug("PlusConnector created")
    return connectorNode

  def recordingCommandCompleted(self, command, q):
    statusText = "Command {0} [{1}]: {2}\n".format(command.GetCommandName(), command.GetID(), command.StatusToString(command.GetStatus()))
    statusTextUser = "{0} {1}\n".format(command.GetCommandName(), command.StatusToString(command.GetStatus()))
    if command.GetResponseMessage():
      statusText = statusText + command.GetResponseMessage()
      statusTextUser = command.GetResponseMessage()
    elif command.GetResponseText():
      statusText = statusText + command.GetResponseText()
      statusTextUser = command.GetResponseText()
    logging.info(statusText)
    self.startStopRecordingButton.setToolTip(statusTextUser)

  def cleanup(self):
    self.disconnect()

  def onStartStopRecordingClicked(self):
    if self.startStopRecordingButton.isChecked():
      self.startStopRecordingButton.setText("  Stop Recording")
      self.startStopRecordingButton.setIcon(self.stopIcon)
      self.startStopRecordingButton.setToolTip("Recording is being started...")
      if self.captureDeviceName  != '':
        # Important to save as .mhd because that does not require lengthy finalization (merging into a single file)
        recordPrefix = self.guideletParent.parameterNode.GetParameter('RecordingFilenamePrefix')
        recordExt = self.guideletParent.parameterNode.GetParameter('RecordingFilenameExtension')
        self.recordingFileName =  recordPrefix + time.strftime("%Y%m%d-%H%M%S") + recordExt

        logging.info("Starting recording to: {0}".format(self.recordingFileName))

        self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('CaptureDeviceId', self.captureDeviceName)
        self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('OutputFilename', self.recordingFileName)
        self.guideletParent.executeCommand(self.plusRemoteLogic.cmdStartRecording, self.recordingCommandCompleted)

    else:
      self.startStopRecordingButton.setText("  Start Recording")
      self.startStopRecordingButton.setIcon(self.recordIcon)
      self.startStopRecordingButton.setToolTip( "Recording is being stopped..." )
      if self.captureDeviceName  != '':  
        logging.info("Stopping recording")  
        self.plusRemoteLogic.cmdStopRecording.SetCommandAttribute('CaptureDeviceId', self.captureDeviceName)
        self.guideletParent.executeCommand(self.plusRemoteLogic.cmdStopRecording, self.recordingCommandCompleted)

  def onFreezeUltrasoundClicked(self):
    logging.debug('onFreezeUltrasoundClicked')
    self.usFrozen = not self.usFrozen
    if self.usFrozen:
      self.guideletParent.connectorNode.Stop()
    else:
      self.guideletParent.connectorNode.Start()

  def setImageMinMaxLevel(self, minLevel, maxLevel):
    liveUsDisplayNode = self.liveUltrasoundNode_Reference.GetDisplayNode()
    if liveUsDisplayNode is None:
      return
    liveUsDisplayNode.SetAutoWindowLevel(0)
    liveUsDisplayNode.SetWindowLevelMinMax(minLevel,maxLevel)

  def onBrightnessContrastNormalClicked(self):
    logging.debug('onBrightnessContrastNormalClicked')
    self.brightnessSliderWidget.setMaximumValue(200)
    self.brightnessSliderWidget.setMinimumValue(0)
    self.setImageMinMaxLevel(0,200)

  def onBrightnessContrastBrightClicked(self):
    logging.debug('onBrightnessContrastBrightClicked')
    self.brightnessSliderWidget.setMaximumValue(120)
    self.brightnessSliderWidget.setMinimumValue(0)
    self.setImageMinMaxLevel(0,120)

  def onBrightnessContrastBrighterClicked(self):
    logging.debug('onBrightnessContrastBrighterClicked')
    self.brightnessSliderWidget.setMaximumValue(60)
    self.brightnessSliderWidget.setMinimumValue(0)
    self.setImageMinMaxLevel(0,60)

  def onBrightnessSliderChanged(self):
    logging.debug('onBrightnessSliderChanged')
    self.setImageMinMaxLevel(self.brightnessSliderWidget.minimumValue, self.brightnessSliderWidget.maximumValue)

