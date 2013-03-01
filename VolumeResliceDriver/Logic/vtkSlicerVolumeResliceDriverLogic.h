// .NAME vtkSlicerVolumeResliceDriverLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVolumeResliceDriverLogic_h
#define __vtkSlicerVolumeResliceDriverLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLTransformableNode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerVolumeResliceDriverModuleLogicExport.h"

class vtkMRMLLinearTransformNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLAnnotationRulerNode;
class vtkMRMLSliceNode;

#define VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE "VolumeResliceDriver.Driver"
//#define VOLUMERESLICEDRIVER_METHOD_ATTRIBUTE "VolumeResliceDriver.Method"
//#define VOLUMERESLICEDRIVER_ORIENTATION_ATTRIBUTE "VolumeResliceDriver.Orientation"
#define VOLUMERESLICEDRIVER_MODE_ATTRIBUTE "VolumeResliceDriver.Mode"



/// \ingroup Slicer_QtModules_VolumeResliceDriver
class VTK_SLICER_VOLUMERESLICEDRIVER_MODULE_LOGIC_EXPORT vtkSlicerVolumeResliceDriverLogic
  : public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerVolumeResliceDriverLogic *New();
  vtkTypeMacro(vtkSlicerVolumeResliceDriverLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  enum {
    MODE_NONE,
    MODE_AXIAL,
    MODE_SAGITTAL,
    MODE_CORONAL,
    MODE_INPLANE,
    MODE_INPLANE90,
    MODE_TRANSVERSE,
    MODE_TRANSVERSE180
  };

  /// Set attributes of MRML slice nodes to define reslice driver.
  void SetDriverForSlice( std::string nodeID, vtkMRMLSliceNode* sliceNode );
  void SetModeForSlice( int mode, vtkMRMLSliceNode* sliceNode );
  
protected:
  
  void AddObservedNode( vtkMRMLTransformableNode* node );
  void ClearObservedNodes();
  
  vtkSlicerVolumeResliceDriverLogic();
  virtual ~vtkSlicerVolumeResliceDriverLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void OnMRMLNodeModified( vtkMRMLNode* node );
  
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void * callData);
  
  void UpdateSliceByTransformableNode( vtkMRMLTransformableNode* tnode, vtkMRMLSliceNode* sliceNode );
  void UpdateSliceByTransformNode( vtkMRMLLinearTransformNode* tnode, vtkMRMLSliceNode* sliceNode );
  void UpdateSliceByImageNode( vtkMRMLScalarVolumeNode* inode, vtkMRMLSliceNode* sliceNode );
  void UpdateSliceByRulerNode( vtkMRMLAnnotationRulerNode* rnode, vtkMRMLSliceNode* sliceNode );
  void UpdateSlice( vtkMatrix4x4* driverToRASMatrix, vtkMRMLSliceNode* sliceNode );
  void UpdateSliceIfObserved( vtkMRMLSliceNode* sliceNode );
  
  std::vector< vtkMRMLTransformableNode* > ObservedNodes;
  
private:

  vtkSlicerVolumeResliceDriverLogic(const vtkSlicerVolumeResliceDriverLogic&); // Not implemented
  void operator=(const vtkSlicerVolumeResliceDriverLogic&);               // Not implemented
};

#endif

