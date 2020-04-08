/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerMetafileReader.h"
#include "qSlicerMetafileImporterModule.h"
#include "qSlicerSequencesModule.h"
#include "qSlicerAbstractModuleRepresentation.h"
// Logic includes
#include "vtkSlicerMetafileImporterLogic.h"

// MRML includes
#include "vtkMRMLSequenceBrowserNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// MRML Sequence includes
#include <vtkMRMLLinearTransformSequenceStorageNode.h>

//-----------------------------------------------------------------------------
class qSlicerMetafileReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerMetafileImporterLogic> MetafileImporterLogic;
};

//-----------------------------------------------------------------------------
qSlicerMetafileReader::qSlicerMetafileReader( vtkSlicerMetafileImporterLogic* newMetafileImporterLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMetafileReaderPrivate)
{
  this->setMetafileImporterLogic( newMetafileImporterLogic );
}

//-----------------------------------------------------------------------------
qSlicerMetafileReader::~qSlicerMetafileReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerMetafileReader::setMetafileImporterLogic(vtkSlicerMetafileImporterLogic* newMetafileImporterLogic)
{
  Q_D(qSlicerMetafileReader);
  d->MetafileImporterLogic = newMetafileImporterLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerMetafileImporterLogic* qSlicerMetafileReader::MetafileImporterLogic() const
{
  Q_D(const qSlicerMetafileReader);
  return d->MetafileImporterLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerMetafileReader::description() const
{
  return "Sequence Metafile";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerMetafileReader::fileType() const
{
  return QString("Sequence Metafile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMetafileReader::extensions() const
{
  return QStringList() << "Sequence Metafile (*.seq.mha *.seq.mhd *.mha *.mhd)";
}

//-----------------------------------------------------------------------------
bool qSlicerMetafileReader::load(const IOProperties& properties)
{
  Q_D(qSlicerMetafileReader);
  if (!properties.contains("fileName"))
  {
    qCritical() << "qSlicerMetafileReader::load did not receive fileName property";
  }
  QString fileName = properties["fileName"].toString();

  vtkNew<vtkCollection> loadedSequenceNodes;

  vtkMRMLSequenceBrowserNode* browserNode = d->MetafileImporterLogic->ReadSequenceFile(fileName.toStdString(), loadedSequenceNodes.GetPointer());
  if (browserNode == NULL)
  {
    return false;
  }

  QStringList loadedNodes;
  loadedNodes << QString(browserNode->GetID());
  for (int i = 0; i < loadedSequenceNodes->GetNumberOfItems(); i++)
  {
    vtkMRMLNode* loadedNode = vtkMRMLNode::SafeDownCast(loadedSequenceNodes->GetItemAsObject(i));
    if (loadedNode == NULL)
    {
      continue;
    }
    loadedNodes << QString(loadedNode->GetID());
  }

  this->setLoadedNodes(loadedNodes);

  qSlicerApplication* app = qSlicerApplication::application();
  QString moduleName = app->nodeModule(browserNode); // "Sequences" is expected
  qSlicerAbstractCoreModule* module = app->moduleManager()->module(moduleName);
  qSlicerAbstractModuleRepresentation* widget = module->widgetRepresentation();
  if (widget)
  {
    widget->setEditedNode(browserNode, "toolbar");
  }

  return true;
}
