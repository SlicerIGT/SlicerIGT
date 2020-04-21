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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLWatchdogDisplayNode.h"

//#include "vtkMRMLScene.h"

/*
#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkMRMLProceduralColorNode.h>
*/

#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWatchdogDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLWatchdogDisplayNode::vtkMRMLWatchdogDisplayNode()
  :vtkMRMLDisplayNode()
{
  this->Position=POSITION_BOTTOM_LEFT;
  this->FontSize=18;
  this->Color[0]=1;
  this->Color[1]=1;
  this->Color[2]=1;
  this->EdgeColor[0]=1;
  this->EdgeColor[1]=0;
  this->EdgeColor[2]=0;
}

//----------------------------------------------------------------------------
vtkMRMLWatchdogDisplayNode::~vtkMRMLWatchdogDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " position=\""<< ConvertPositionToString(this->Position) << "\"";
  of << indent << " fontSize=\""<< this->FontSize << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName,"position"))
      {
      this->Position = ConvertPositionFromString(attValue);
      continue;
      }
    else if (!strcmp(attName,"fontSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FontSize;
      continue;
      }
    }

  this->Modified();
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLWatchdogDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();
  Superclass::Copy(anode);

  vtkMRMLWatchdogDisplayNode *node = vtkMRMLWatchdogDisplayNode::SafeDownCast(anode);

  this->SetPosition(node->Position);
  this->SetFontSize(node->FontSize);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Position = "<< ConvertPositionToString(this->Position) << "\n";
  os << indent << "FontSize = "<< this->FontSize << "\n";
}

//----------------------------------------------------------------------------
const char* vtkMRMLWatchdogDisplayNode::ConvertPositionToString(int positionIndex)
{
  switch (positionIndex)
    {
    case POSITION_TOP_LEFT: return "top-left";
    case POSITION_BOTTOM_LEFT: return "bottom-left";
    case POSITION_TOP_RIGHT: return "top-right";
    case POSITION_BOTTOM_RIGHT: return "bottom-right";
    default: return "";
    }
}

//----------------------------------------------------------------------------
int vtkMRMLWatchdogDisplayNode::ConvertPositionFromString(const char* positionString)
{
  if (positionString==NULL)
    {
    return -1;
    }
  for (int positionIndex=0; positionIndex<POSITION_LAST; positionIndex++)
    {
    if (strcmp(positionString, vtkMRMLWatchdogDisplayNode::ConvertPositionToString(positionIndex))==0)
      {
      return positionIndex;
      }
    }
  return -1;
}
