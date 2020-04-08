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

#ifndef __qSlicerMetafileReader_h
#define __qSlicerMetafileReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerMetafileReaderPrivate;

// Slicer includes
class vtkSlicerMetafileImporterLogic;

//-----------------------------------------------------------------------------
class qSlicerMetafileReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerMetafileReader( vtkSlicerMetafileImporterLogic* newMetafileImporterLogic = 0, QObject* parent = 0 );
  virtual ~qSlicerMetafileReader();

  void setMetafileImporterLogic( vtkSlicerMetafileImporterLogic* newMetafileImporterLogic);
  vtkSlicerMetafileImporterLogic* MetafileImporterLogic() const;

  virtual QString description() const;
  virtual IOFileType fileType() const;
  virtual QStringList extensions() const;

  virtual bool load( const IOProperties& properties );
  
protected:
  QScopedPointer< qSlicerMetafileReaderPrivate > d_ptr;

private:
  Q_DECLARE_PRIVATE( qSlicerMetafileReader );
  Q_DISABLE_COPY( qSlicerMetafileReader );
};

#endif
