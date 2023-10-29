/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyPathExplorerPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

#include <vtkSlicerPathExplorerLogic.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyPathExplorerPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyPathExplorerPlugin);
protected:
  qSlicerSubjectHierarchyPathExplorerPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyPathExplorerPluginPrivate(qSlicerSubjectHierarchyPathExplorerPlugin& object);
  ~qSlicerSubjectHierarchyPathExplorerPluginPrivate() override;
  void init();
public:
  QIcon TrajectoryIcon;

  vtkWeakPointer<vtkSlicerPathExplorerLogic> PathExplorerLogic;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPathExplorerPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPathExplorerPluginPrivate::qSlicerSubjectHierarchyPathExplorerPluginPrivate(qSlicerSubjectHierarchyPathExplorerPlugin& object)
: q_ptr(&object)
{
  this->TrajectoryIcon = QIcon(":Icons/Trajectory.png");
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPathExplorerPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPathExplorerPluginPrivate::~qSlicerSubjectHierarchyPathExplorerPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPathExplorerPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPathExplorerPlugin::qSlicerSubjectHierarchyPathExplorerPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyPathExplorerPluginPrivate(*this) )
{
  Q_D(qSlicerSubjectHierarchyPathExplorerPlugin);
  this->m_Name = QString("PathExplorer");
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPathExplorerPlugin::~qSlicerSubjectHierarchyPathExplorerPlugin() = default;

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPathExplorerPlugin::setPathExplorerLogic(vtkSlicerPathExplorerLogic* PathExplorerLogic)
{
  Q_D(qSlicerSubjectHierarchyPathExplorerPlugin);
  d->PathExplorerLogic = PathExplorerLogic;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyPathExplorerPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLPathPlannerTrajectoryNode"))
    {
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyPathExplorerPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLPathPlannerTrajectoryNode"))
    {
    return 0.5; // There may be other plugins that can handle special PlotChart better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyPathExplorerPlugin::roleForPlugin()const
{
  return tr("Path Explorer trajectories");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyPathExplorerPlugin::icon(vtkIdType itemID)
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyPathExplorerPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QIcon();
    }

  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLPathPlannerTrajectoryNode"))
    {
    return d->TrajectoryIcon;
    }

  // Item unknown by plugin
  return QIcon();
}
