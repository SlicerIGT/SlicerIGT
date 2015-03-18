#include "vtkSlicerOpenIGTLinkCommand.h"

#include <sstream>

#include <vtkObjectFactory.h>
#include <vtkXMLDataElement.h>
#include <vtkSmartPointer.h>
#include <vtkXMLUtilities.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerOpenIGTLinkCommand);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkCommand::vtkSlicerOpenIGTLinkCommand()
: ID(NULL)
, Status(CommandUnknown)
, CommandXML(NULL)
, CommandTimeoutSec(10)
, ExportedCommandText(NULL)
, ResponseXML(NULL)
, ResponseTextInternal(NULL)
{
  this->CommandXML = vtkXMLDataElement::New();
  this->CommandXML->SetName("Command");
}

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkCommand::~vtkSlicerOpenIGTLinkCommand()
{
  this->SetID(NULL);
  if (this->CommandXML)
    {
    this->CommandXML->Delete();
    this->CommandXML=NULL;
    }
  this->SetExportedCommandText(NULL);
  if (this->ResponseXML)
    {
    this->ResponseXML->Delete();
    this->ResponseXML=NULL;
    }
  this->SetResponseTextInternal(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkCommand::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << "ID: " << ( (this->GetID()) ? this->GetID() : "None" ) << "\n";
  os << "Status: " << vtkSlicerOpenIGTLinkCommand::StatusToString(this->GetStatus()) << "\n";
  os << "CommandText: " << ( (this->GetCommandText()) ? this->GetCommandText() : "None" ) << "\n";
  os << "CommandXML: ";
  if (this->CommandXML)
    {
    this->CommandXML->PrintXML(os, indent.GetNextIndent());
    }
  else
    {
    os << "None" << "\n";
    }
  os << "ResponseText: " << ( (this->GetResponseText()) ? this->GetResponseText() : "None" ) << "\n";
  os << "ResponseXML: ";
  if (this->ResponseXML)
    {
    this->ResponseXML->PrintXML(os, indent.GetNextIndent());
    }
  else
    {
    os << "None" << "\n";
    }
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetCommandName()
{
  return this->GetCommandAttribute("Name");
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkCommand::SetCommandName(const char* name)
{
  this->SetCommandAttribute("Name", name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetCommandAttribute(const char* attName)
{
  return this->CommandXML->GetAttribute(attName);
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkCommand::SetCommandAttribute(const char* attName, const char* attValue)
{
  this->CommandXML->SetAttribute(attName, attValue);
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetCommandText()
{
  std::ostringstream os;
  this->CommandXML->PrintXML(os, vtkIndent(0));
  SetExportedCommandText(os.str().c_str());
  return this->ExportedCommandText;
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkCommand::SetCommandText(const char* text)
{
  if (text==NULL)
    {
    vtkErrorMacro("Failed to set OpenIGTLink command from text: empty input");
    return false;
    }
 
  vtkSmartPointer<vtkXMLDataElement> parsedElem = vtkSmartPointer<vtkXMLDataElement>::Take(vtkXMLUtilities::ReadElementFromString(text));
  if (parsedElem == NULL)
  {
    vtkErrorMacro("Failed to set OpenIGTLink command from text: not an XML string: "<<text);
    return false;
  }

  this->CommandXML->DeepCopy(parsedElem);
  return true;
}

  
//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetResponseMessage()
{
  return GetResponseAttribute("Message");
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetResponseAttribute(const char* attName)
{
  if (this->ResponseXML==NULL)
    {
    return NULL;
    }
  return this->ResponseXML->GetAttribute(attName);
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::GetResponseText()
{
  return this->ResponseTextInternal;
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkCommand::SetResponseText(const char* text)
{
  SetResponseTextInternal(text);
  
  if (this->ResponseXML)
    {
    this->ResponseXML->Delete();
    this->ResponseXML=NULL;
    }
    
  if (text==NULL)
  {
    SetStatus(CommandFail);
    return;
  }

  this->ResponseXML = vtkXMLUtilities::ReadElementFromString(text);
  if (this->ResponseXML == NULL)
  {
    // The response is not XML
    vtkWarningMacro("OpenIGTLink command response is not XML: "<<text);
    SetStatus(CommandFail);
    return;
  }
  
  // Retrieve status from XML string
  if (this->ResponseXML->GetAttribute("Status")==NULL)
  {
    vtkWarningMacro("OpenIGTLink command response: missing Status attribute: "<<text);
  }
  else
  {
    if (strcmp(this->ResponseXML->GetAttribute("Status"),"SUCCESS")==0)
    {
      SetStatus(CommandSuccess);
    }
    else if (strcmp(this->ResponseXML->GetAttribute("Status"),"FAIL")==0)
    {
      SetStatus(CommandFail);
    }
    else
    {
      vtkErrorMacro("OpenIGTLink command response: invalid Status attribute value: "<<this->ResponseXML->GetAttribute("Status"));
      SetStatus(CommandFail);
    }
  }
}

//----------------------------------------------------------------------------
const char* vtkSlicerOpenIGTLinkCommand::StatusToString(int status)
{
  switch (status)
  {
  case CommandUnknown: return "Unknown";
  case CommandWaiting: return "Waiting";
  case CommandSuccess: return "Success";
  case CommandFail: return "Fail";
  case CommandExpired: return "Expired";
  case CommandCancelled: return "Cancelled";
  default:
    return "Invalid";
  }
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkCommand::IsInProgress()
{
  return this->Status == CommandWaiting;
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkCommand::IsCompleted()
{
  return
    this->Status == CommandSuccess ||
    this->Status == CommandFail ||
    this->Status == CommandExpired ||
    this->Status == CommandCancelled;
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkCommand::IsSucceeded()
{
  return this->Status == CommandSuccess;
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkCommand::IsFailed()
{
  return
    this->Status == CommandFail ||
    this->Status == CommandExpired ||
    this->Status == CommandCancelled;
}
