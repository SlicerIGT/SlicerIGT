from __main__ import vtk, qt, ctk, slicer

#
# CtToAtlasRegistration
#

class CtToAtlasRegistration:
  def __init__(self, parent):
    parent.title = "CtToAtlasRegistration"
    # Think of more appropriate category
    parent.categories = ["Examples"]
    parent.dependencies = []
    parent.contributors = ["Ben Church (Queen's University)"]
    parent.helpText = """
    This scripted loadable module was initially created for the B-spline (?)
    spinal model registration project.
    """
    parent.acknowledgementText = """ """ 

#
# CtToAtlasRegistrationWidget
#

class CtToAtlasRegistrationWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qtQVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parnet.show()

  def setup(self):
  
    # Set up user interface panel
    moduleInterface = ctk.ctkCollapsibleButton()
    moduleInterface.text = "Model to atlas registration"
    self.layout.addWidget(moduleInterface)
    moduleInterfaceLayout = qt.QGridLayout(moduleInterface)
    
    # Find-list-intersection button
    self.TrimButton = qt.QPushButton("Trim lists")
    self.TrimButton.toolTip = "Finds intersection of atlas and CT scan fiducial sets."
    self.TrimButton.enabled = True
    moduleInterfaceLayout.addWidget(self.TrimButton)
    
    # Reload module button
    self.reloadButton = qt.QPushButton('Reload module')
    moduleInterfaceLayout.addWidget(self.reloadButton)
    self.TrimButton.connect('clicked(bool)', self.onTrimButton)
    self.reloadButton.connect('clicked(bool)', self.onReloadButton)
    
  def onTrimButton(self):
    logic = CtToAtlasRegistrationLogic()
    self.AtlasFids = slicer.util.getNode('UsLandmarks_Atlas')
    self.CtFids = slicer.util.getNode('TrXFiducials')
    self.nCtFids = self.CtFids.GetNumberOfFiducials()
    self.nAtalsFids = self.AtlasFids.GetNumberOfFiducials()
    (self.CtNames, self.NamesIntersection) = logic.run(self.CtFids, self.AtlasFids)
  
  def cleanup(self):
    pass
    
  def onReloadButton(self):
    if(slicer.util.getNode('CommonFiducialsAtlas') != None):
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('CommonFiducialsAtlas'))
    if(slicer.util.getNode('CommonFiducialsCt') != None):  
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('CommonFiducialsCt'))
    slicer.util.reloadScriptedModule(slicer.moduleNames.CtToAtlasRegistration)

#
# CtToAtlasRegistrationLogic
#

class CtToAtlasRegistrationLogic():
  def __init__(self):
    self.CtNames = []
    self.NamesIntersection = []
    self.AtlasNames = []
    self.CommonFiducialsAtlas = slicer.vtkMRMLMarkupsFiducialNode()
    self.CommonFiducialsAtlas.SetName('CommonFiducialsAtlas')
    self.CommonFiducialsCt = slicer.vtkMRMLMarkupsFiducialNode()
    self.CommonFiducialsCt.SetName('CommonFiducialsCt')
    self.CtSpineLength = 0         # Will be lengthened with the addition of each common vertebrae as a running average of right and left side lengths
    self.AtlasSpineLength = 0       # Will be computed for the atlas spine as for the Ct spine
    self.AntiSqishOffset = 30       # Distance in anterior direction to duplicate fiducial points for registration accuracy
    self.LeftAtlasCommon = []
    self.RightAtlasCommon = []
    self.LeftCtCommon = []
    self.RightCtCommon = []
    
  def run(self, CtFids, AtlasFids):
    import math
    nCtFids = CtFids.GetNumberOfFiducials()
    nAtlasFids = AtlasFids.GetNumberOfFiducials()
    for i in range(nCtFids):
      name = CtFids.GetNthFiducialLabel(i)
      self.CtNames.append(name)
      
    for i in range(nAtlasFids):
      name = AtlasFids.GetNthFiducialLabel(i)
      self.AtlasNames.append(name)
      
    # Make sure Ct has no points the atlas doesn't  
    for CtPoint in self.CtNames:
      if(int(CtPoint[1:-1])<10):         # If the Ct fiducial name has a "0" as the second char
        if((CtPoint[0]+"0"+CtPoint[1:]) not in self.AtlasNames):
          self.CtNames.remove(CtPoint)
      else:
        if(CtPoint not in self.AtlasNames):
          self.CtNames.remove(CtPoint)
    
    # Still need to make sure the atlas has no more points than CtNames
    BackwardsIt = -1
    for i, AtlasPoint in enumerate(self.AtlasNames):
      if(AtlasPoint[1] == "0"):
        if((AtlasPoint[0]+AtlasPoint[2:]) in self.CtNames):     
          self.NamesIntersection.append(AtlasPoint)
          newAtlasPoint = AtlasFids.GetMarkupPointVector(i,0)
          # This if-structure eliminates need for same-side point searching for spine length computation
          if(AtlasPoint[-1] == "L"):
            self.LeftAtlasCommon.append(newAtlasPoint)
          else:
            self.RightAtlasCommon.append(newAtlasPoint)
            
          self.CommonFiducialsAtlas.AddFiducial(newAtlasPoint[0], newAtlasPoint[1], newAtlasPoint[2])
          self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, AtlasFids.GetNthFiducialLabel(i))
          
      else:
        if(AtlasPoint in self.CtNames):  
          self.NamesIntersection.append(AtlasPoint)
          newAtlasPoint = AtlasFids.GetMarkupPointVector(i,0)
          if(AtlasPoint[-1] == "L"):
            self.LeftAtlasCommon.append(newAtlasPoint)
          else:
            self.RightAtlasCommon.append(newAtlasPoint)
          
          self.CommonFiducialsAtlas.AddFiducial(newAtlasPoint[0], newAtlasPoint[1], newAtlasPoint[2])
          self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, AtlasFids.GetNthFiducialLabel(i))

    CtPointIt = 0
    for i, CommonFid in enumerate(self.NamesIntersection):
      if(int(CommonFid[1]) == 0):
        while((CtFids.GetNthFiducialLabel(CtPointIt) != CommonFid[0] + CommonFid[2:]) and (CtPointIt < nCtFids)):
          CtPointIt = CtPointIt + 1
          if(CtPointIt == nCtFids):
            break

        newCtPoint = CtFids.GetMarkupPointVector(CtPointIt,0)
        if(CommonFid[-1] == "L"):
          self.LeftCtCommon.append(newCtPoint)
        else:
          self.RightCtCommon.append(newCtPoint)
          
        self.CommonFiducialsCt.AddFiducial(newCtPoint[0], newCtPoint[1], newCtPoint[2])
        self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, CtFids.GetNthFiducialLabel(CtPointIt))
      else:
        while((CtFids.GetNthFiducialLabel(CtPointIt) != CommonFid) and (CtPointIt < nCtFids)):
          CtPointIt = CtPointIt + 1
          if(CtPointIt == nCtFids):
            break

        newCtPoint = CtFids.GetMarkupPointVector(CtPointIt,0)
        if(CommonFid[-1] == "L"):
          self.LeftCtCommon.append(newCtPoint)
        else:
          self.RightCtCommon.append(newCtPoint)
          
        self.CommonFiducialsCt.AddFiducial(newCtPoint[0], newCtPoint[1], newCtPoint[2])
        self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, CtFids.GetNthFiducialLabel(CtPointIt))
    
    if(len(self.NamesIntersection) != 0):
      LeftPointsWeight = float(len(self.LeftAtlasCommon))/(len(self.NamesIntersection))
      RightPointsWeight = 1.0 - LeftPointsWeight
      for i, LeftAtlasPoint in enumerate(self.LeftAtlasCommon[1:]):
        self.AtlasSpineLength = self.AtlasSpineLength + (math.sqrt((LeftAtlasPoint[0]-self.LeftAtlasCommon[i][0])**2 + (LeftAtlasPoint[1]-self.LeftAtlasCommon[i][1])**2 + (LeftAtlasPoint[2]-self.LeftAtlasCommon[i][2])**2)) * LeftPointsWeight 
      for i, RightAtlasPoint in enumerate(self.RightAtlasCommon[1:]):
        self.AtlasSpineLength = self.AtlasSpineLength + (math.sqrt((RightAtlasPoint[0]-self.RightAtlasCommon[i][0])**2 + (RightAtlasPoint[1]-self.RightAtlasCommon[i][1])**2 + (RightAtlasPoint[2]-self.RightAtlasCommon[i][2])**2)) * RightPointsWeight
      
      for i, LeftCtPoint in enumerate(self.LeftCtCommon[1:]):
        self.CtSpineLength = self.CtSpineLength + (math.sqrt((LeftCtPoint[0]-self.LeftCtCommon[i][0])**2 + (LeftCtPoint[1]-self.LeftCtCommon[i][1])**2 + (LeftCtPoint[2]-self.LeftCtCommon[i][2])**2)) * LeftPointsWeight
      for i, RightCtPoint in enumerate(self.RightCtCommon[1:]):
        self.CtSpineLength = self.CtSpineLength + (math.sqrt((RightCtPoint[0]-self.RightCtCommon[i][0])**2 + (RightCtPoint[1]-self.RightCtCommon[i][1])**2 + (RightCtPoint[2]-self.RightCtCommon[i][2])**2)) * RightPointsWeight
      
      scale = self.CtSpineLength/self.AtlasSpineLength
      
      for i in range(self.CommonFiducialsCt.GetNumberOfFiducials()):
        OriginalAtlasPoint = self.CommonFiducialsAtlas.GetMarkupPointVector(i,0)
        self.CommonFiducialsAtlas.AddFiducial(OriginalAtlasPoint[0], OriginalAtlasPoint[1] + self.AntiSqishOffset, OriginalAtlasPoint[2])
        self.CommonFiducialsAtlas.SetNthFiducialLabel(self.CommonFiducialsAtlas.GetNumberOfFiducials() - 1, self.CommonFiducialsAtlas.GetNthFiducialLabel(i) + "A")
        OriginalCtPoint = self.CommonFiducialsCt.GetMarkupPointVector(i,0)
        self.CommonFiducialsCt.AddFiducial(OriginalCtPoint[0], OriginalCtPoint[1] + scale*self.AntiSqishOffset, OriginalCtPoint[2])
        self.CommonFiducialsCt.SetNthFiducialLabel(self.CommonFiducialsCt.GetNumberOfFiducials() - 1, self.CommonFiducialsCt.GetNthFiducialLabel(i) + "A")
      
      slicer.mrmlScene.AddNode(self.CommonFiducialsCt)
      slicer.mrmlScene.AddNode(self.CommonFiducialsAtlas)
      print "\n Atlas model curve length(mm) = " + str(self.AtlasSpineLength) 
      print "Ct model curve length(mm) = " + str(self.CtSpineLength)
      print "\n Intersection of CT and atlas landmarks:"
      for i, name in enumerate(self.NamesIntersection[:-1]):
        if((name[:-1] == self.NamesIntersection[i+1][:-1]) and (name[-1] != self.NamesIntersection[i+1][-1])):
          print name + "  " + self.NamesIntersection[i+1]
            
        else:
          if(i>0 and ((name[:-1] + "L" not in self.NamesIntersection) or (name[:-1] + "R" not in self.NamesIntersection))):
            print name

      # This should fix bug where last landmark is not printed if it has no partner
      if(self.NamesIntersection[-1][:-1] != self.NamesIntersection[-2][:-1]): # and (self.NamesIntersection[-1][-1] != self.NamesIntersection))
        print self.NamesIntersection[-1]
      return (self.CtNames, self.NamesIntersection)
    
    else: # For whatever reason, no points in common could be found between the CT and atlas
      print "Warning - intersection of CT and atlas points is the empty set."
      print "   Maybe points are named incorrectly."
      print "   Returning empty lists."
      return ([], [])
    


class CtToAtlasRegistrationTest():
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
    self.test_CtToAtlasRegistration1()

  def test_CtToAtlasRegistration1(self):
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
    logic = CtToAtlasRegistrationLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
