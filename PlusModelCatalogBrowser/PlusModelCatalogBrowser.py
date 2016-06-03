import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# PlusModelCatalogBrowser
#

class PlusModelCatalogBrowser(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Plus model catalog browser" # TODO make this more human readable by adding spaces
    self.parent.categories = ["IGT"]
    self.parent.dependencies = []
    self.parent.contributors = ["Andras Lasso (PerkLab)"]
    self.parent.helpText = """This module allows convenient download of 3D models from Plus toolkit's 3D model catalog."""
    self.parent.acknowledgementText = """The module is based on Thingiverse browser module, developed by Nigel Goh, UWA, as part of a final year project.
      The assistance of Steve Pieper, Isomics, Inc., and Jean-Christophe, KitWare, in the development process is greatly appreciated.
      """

#
# PlusModelCatalogBrowserWidget
#

class PlusModelCatalogBrowserWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    self.showButton = qt.QPushButton("Show/hide catalog")
    self.showButton.toolTip = "Show/hide Plus toolkit's 3D model catalog"
    self.showButton.setCheckable(True)
    self.showButton.setAutoFillBackground(True)
    self.showButton.setStyleSheet("background-color: rgb(150, 255, 150); color: rgb(0, 0, 0); height: 40px")
    self.layout.addWidget(self.showButton)
    self.showButton.connect('toggled(bool)', self.showButtonClicked)
 
    modelsButton = qt.QPushButton("Go to models module")
    modelsButton.toolTip = "Switch to Models module"
    self.layout.addWidget(modelsButton)
    modelsButton.connect('clicked(bool)',self.modelsButtonClicked)

    plusToolkitLabel = qt.QLabel('<a href="https://www.assembla.com/spaces/plus/wiki">Plus toolkit (www.plustoolkit.org)</a>')
    plusToolkitLabel.setOpenExternalLinks(True)
    plusToolkitLabel.setAlignment(qt.Qt.AlignCenter)
    self.layout.addWidget(plusToolkitLabel)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    self.logic = PlusModelCatalogBrowserLogic()
    
    self.showButton.setChecked(True)

  def cleanup(self):
    self.logic.hideBrowser()

  def showButtonClicked(self, show):
    if show:
      self.logic.showBrowser()
    else:
      if not self.logic.browser.visible:
        # user closed the browser, so probably it should be shown now
        # regardless of the button state
        self.logic.showBrowser()
        wasBlocked = self.showButton.blockSignals(True)
        self.showButton.setChecked(True)
        self.showButton.blockSignals(wasBlocked)
      else:
        self.logic.hideBrowser()

  def modelsButtonClicked(self):
    self.logic.hideBrowser()
    wasBlocked = self.showButton.blockSignals(True)
    self.showButton.setChecked(False)
    self.showButton.blockSignals(wasBlocked)
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('Models')

#
# PlusModelCatalogBrowserLogic
#

class PlusModelCatalogBrowserLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self):
    self.catalogMainUrl = qt.QUrl('http://perk-software.cs.queensu.ca/plus/doc/nightly/modelcatalog/')
    self.catalogName = 'Plus toolkit 3D model catalog'
  
    self.browser = qt.QWebView()
    # Change QWebView such that link clicks are not automatically acted on
    # When links are clicked, run the handleClick() function
    self.browser.page().setLinkDelegationPolicy(qt.QWebPage.DelegateAllLinks)        
    self.browser.linkClicked.connect(self.handleClick)
    
    self.downloadProgressBar = qt.QProgressDialog()
    self.downloadProgressBar.setLabelText('Downloading File')
    
  def showBrowser(self):
    self.browser.setWindowTitle(self.catalogName)
    self.browser.setUrl(self.catalogMainUrl)
    self.browser.resize(1050,800)
    self.browser.show()
    
  def hideBrowser(self):
    self.browser.hide() 
    
  def downloadFile(self, dlUrl):
    import urllib

    # Set download directory to a folder in Slicer cache
    destinationDir = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()

    fileInfo = qt.QFileInfo(dlUrl.path())        
    destinationFilePath = destinationDir + "/" + fileInfo.fileName()        
    logging.info("Downloading model file: {0}".format(destinationFilePath))

    self.downloadProgressBar.show()
    urllib.urlretrieve(dlUrl.toString(), destinationFilePath, self.reportProgress)
    self.downloadProgressBar.close()       
      
    return destinationFilePath
  
  def extract(self, ZipSource):
    """Extract all files from a zip"""
    zip = zipfile.ZipFile(ZipSource)
    
    # Extract name of the folder inside the zip
    zipList = zip.namelist()
    folderName = zipList[0]
    
    #Returns final directory including the folder that was in the zip
    extractDir = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()
    zip.extractall(path = extractDir)
    finalDir = extractDir + '/' + folderName
    return finalDir
  
  def loadModel(self, filePath):
    """Load model into Slicer"""
    return slicer.util.loadModel(filePath)
     
  def loadFiles(self, folderToLoad):
    """Loads all the models inside a folder into Slicer"""
    fileList = dircache.listdir(folderToLoad) # lists all the files inside a folder
    for filename in fileList:
      self.loadModel(folderToLoad + "/" + filename)

  def reportProgress(self, blocks, blockSize, totalSize):
    """Changes value of ProgressBar based on download progress.
    ProgressBar is hidden by default and only shown when downloads occur"""
    percent = (blocks * blockSize * 100)/totalSize
    self.downloadProgressBar.setValue(percent)

  def handleClick(self, url):
    """Main function that runs whenever a link is clicked"""
    urlString = url.toString()
    if ".stl" in urlString or ".STL" in urlString:
      # STL file is clicked
      filePath = self.downloadFile(url)
      self.loadModel(filePath)
    elif ".zip" in urlString or ".ZIP" in urlString:
      # ZIP file is clicked
      filePath = self.downloadFile(url)
      finalDir = self.extract(filePath)
      self.loadFiles(finalDir)
    else:
      # other link is clicked, navigate to new link
      self.browser.setUrl(url)

class PlusModelCatalogBrowserTest(ScriptedLoadableModuleTest):
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
    self.test_PlusModelCatalogBrowser1()

  def test_PlusModelCatalogBrowser1(self):

    self.delayDisplay("Starting the test")
    
    logic = PlusModelCatalogBrowserLogic()
    logic.showBrowser()
    logic.handleClick(qt.QUrl('http://perk-software.cs.queensu.ca/plus/doc/nightly/modelcatalog/'))
    logic.handleClick(qt.QUrl('http://perk-software.cs.queensu.ca/plus/doc/nightly/modelcatalog/printable/CauteryGrabber.STL'))
    logic.hideBrowser()

    self.assertIsNotNone( slicer.util.getNode('CauteryGrabber') )
    self.delayDisplay('Test passed!')
