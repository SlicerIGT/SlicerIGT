import os
from __main__ import vtk, qt, ctk, slicer
import logging
import time

class UltraSound(object):

  DEFAULT_IMAGE_SIZE = [800, 600, 1]

  def __init__(self, guideletParent):
    self.guideletParent = guideletParent
    self.captureDeviceName='CaptureDevice'

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
    self.startStopRecordingButton.setEnabled(False)

  def setupPanel(self, parentWidget):
    logging.debug('UltraSound.setupPanel')
    collapsibleButton = ctk.ctkCollapsibleButton()

    collapsibleButton.setProperty('collapsedHeight', 20)
    collapsibleButton.text = "Ultrasound"
    parentWidget.addWidget(collapsibleButton)

    ultrasoundLayout = qt.QFormLayout(collapsibleButton)
    ultrasoundLayout.setContentsMargins(12,4,4,4)
    ultrasoundLayout.setSpacing(4)

    self.startStopRecordingButton = qt.QPushButton("  Start Recording")
    self.startStopRecordingButton.setCheckable(True)
    self.startStopRecordingButton.setIcon(self.recordIcon)
    self.startStopRecordingButton.setToolTip("If clicked, start recording")

    self.freezeUltrasoundButton = qt.QPushButton('Freeze')

    hbox = qt.QHBoxLayout()
    hbox.addWidget(self.startStopRecordingButton)
    hbox.addWidget(self.freezeUltrasoundButton)
    ultrasoundLayout.addRow(hbox)

    self.usFrozen=False

    self.brigthnessContrastButtonNormal = qt.QPushButton()
    self.brigthnessContrastButtonNormal.text = "Normal"
    self.brigthnessContrastButtonNormal.setEnabled(True)

    self.brigthnessContrastButtonBright = qt.QPushButton()
    self.brigthnessContrastButtonBright.text = "Bright"
    self.brigthnessContrastButtonBright.setEnabled(True)

    self.brigthnessContrastButtonBrighter = qt.QPushButton()
    self.brigthnessContrastButtonBrighter.text = "Brighter"
    self.brigthnessContrastButtonBrighter.setEnabled(True)

    brightnessContrastBox = qt.QHBoxLayout()
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonNormal)
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonBright)
    brightnessContrastBox.addWidget(self.brigthnessContrastButtonBrighter)
    ultrasoundLayout.addRow(brightnessContrastBox)

    return collapsibleButton, ultrasoundLayout

  def setupScene(self):
    logging.info("UltraSound.setupScene")

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
      #self.liveUltrasoundNode_Reference.CreateDefaultStorageNode()

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

    self.liveUltrasoundNode_Reference.SetAndObserveTransformNodeID(self.guideletParent.referenceToRas.GetID())

  def setupConnections(self):
    self.startStopRecordingButton.connect('clicked(bool)', self.onStartStopRecordingClicked)
    self.freezeUltrasoundButton.connect('clicked()', self.onFreezeUltrasoundClicked)
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

      # Important to save as .mhd because that does not require lengthy finalization (merging into a single file)
      recordPrefix = self.guideletParent.parameterNode.GetParameter('RecordingFilenamePrefix')
      recordExt = self.guideletParent.parameterNode.GetParameter('RecordingFilenameExtension')
      self.recordingFileName =  recordPrefix + time.strftime("%Y%m%d-%H%M%S") + recordExt

      logging.info("Starting recording to: {0}".format(self.recordingFileName))

      self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('CaptureDeviceId', self.captureDeviceName)
      self.plusRemoteLogic.cmdStartRecording.SetCommandAttribute('OutputFilename', self.recordingFileName)
      self.guideletParent.executeCommand(self.plusRemoteLogic.cmdStartRecording, self.recordingCommandCompleted)

    else:
      logging.info("Stopping recording")
      self.startStopRecordingButton.setText("  Start Recording")
      self.startStopRecordingButton.setIcon(self.recordIcon)
      self.startStopRecordingButton.setToolTip( "Recording is being stopped..." )
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
    self.liveUltrasoundNode_Reference.GetDisplayNode().SetAutoWindowLevel(0)
    self.liveUltrasoundNode_Reference.GetDisplayNode().SetWindowLevelMinMax(minLevel,maxLevel)

  def onBrightnessContrastNormalClicked(self):
    logging.debug('onBrightnessContrastNormalClicked')
    self.setImageMinMaxLevel(0,200)

  def onBrightnessContrastBrightClicked(self):
    logging.debug('onBrightnessContrastBrightClicked')
    self.setImageMinMaxLevel(0,120)

  def onBrightnessContrastBrighterClicked(self):
    logging.debug('onBrightnessContrastBrighterClicked')
    self.setImageMinMaxLevel(0,60)