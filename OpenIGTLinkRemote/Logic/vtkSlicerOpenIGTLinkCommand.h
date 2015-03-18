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

// .NAME vtkSlicerOpenIGTLinkCommand - class for storing an OpenIGTLink command message and response
// .SECTION Description
// This class is used by vtkSlicerOpenIGTLinkRemoteLogic for storing OpenIGTLink command and response data.


#ifndef __vtkSlicerOpenIGTLinkCommand_h
#define __vtkSlicerOpenIGTLinkCommand_h

#include "vtkSlicerOpenIGTLinkRemoteModuleLogicExport.h"

#include "vtkCommand.h"

class vtkXMLDataElement;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_LOGIC_EXPORT vtkSlicerOpenIGTLinkCommand :
  public vtkObject
{
public:
  
  enum StatusValues
  {
    // Command initialization:
    CommandUnknown,
    // Command in progress:
    CommandWaiting,
    // Command completed:
    CommandSuccess,
    CommandFail,
    CommandExpired, // timeout elapsed before command response was received
    CommandCancelled // cancel command was requested before command response was received
  };
  
  enum Events
  {
    // vtkCommand::UserEvent + 123 is just a random value that is very unlikely to be used for anything else in this class
    CommandCompletedEvent = vtkCommand::UserEvent + 123
  };
  
  static vtkSlicerOpenIGTLinkCommand *New();
  vtkTypeMacro(vtkSlicerOpenIGTLinkCommand, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Common command information (both command and response)

  /// Unique identifier of the command. It is used for generating the OpenIGTLink STRING command ID (CMD_[ID])
  vtkGetStringMacro(ID);
  vtkSetStringMacro(ID);

  /// Status of the command request (Unknown->Waiting->Success/Fail/Expired/Cancelled->Unknown)
  vtkGetMacro(Status, int);
  vtkSetMacro(Status, int);
  /// Convert status enum to human-readable string
  static const char* StatusToString(int status);
  
  // Command information

  /// Set command name (required)
  const char* GetCommandName();
  void SetCommandName(const char* name);
  
  /// Set optional command attributes
  const char* GetCommandAttribute(const char* attName);
  void SetCommandAttribute(const char* attName, const char* attValue);

  /// Generate command XML string from ID, name, and attributes
  virtual const char* GetCommandText();

  /// Set command name and attributes from an XML string. Returns with true on success.
  virtual bool SetCommandText(const char* text);

  /// If >0 then commands expires after the specified timeout (state changes from Waiting to Expired).
  // Default timeout is 10 seconds, as most commands should return with a result immediately.
  vtkGetMacro(CommandTimeoutSec, double);
  vtkSetMacro(CommandTimeoutSec, double);

  // Response information

  /// Get the message string from the response (stored in Message attribute)
  const char* GetResponseMessage();

  /// Get custom response attributes
  const char* GetResponseAttribute(const char* attName);
  
  /// Get the raw command response text that was set using SetResponseText.
  /// It contains the response text as it received it, so it is valid even if XML parsing of the text failed.
  const char* GetResponseText();
  /// Set the command response from XML. Updates response message, status, and attributes.
  /// In case of XML parsing error, the state is set to CommandFail.
  virtual void SetResponseText(const char* text);
  
  /// Get the response as an XML element. Returns NULL if the response text was not set or was invalid.
  vtkGetMacro(ResponseXML, vtkXMLDataElement*);

  /// Returns true if command execution is in progress
  bool IsInProgress();

  /// Returns true if command execution is completed (with either success or failure)
  bool IsCompleted();

  /// Returns true if command execution is completed with success
  bool IsSucceeded();

  /// Returns true if command execution is completed but not successfully
  bool IsFailed();
    
protected:
  vtkSlicerOpenIGTLinkCommand();
  virtual ~vtkSlicerOpenIGTLinkCommand();

  /// Helper method for storing the returned command text.
  /// The exported command text is stored as a member variable to allow returning it
  /// as a simple const char pointer
  vtkSetStringMacro(ExportedCommandText);

  /// Helper functions for storing the raw response text.
  vtkGetStringMacro(ResponseTextInternal);
  vtkSetStringMacro(ResponseTextInternal);


private:
  vtkSlicerOpenIGTLinkCommand(const vtkSlicerOpenIGTLinkCommand&); // Not implemented
  void operator=(const vtkSlicerOpenIGTLinkCommand&);               // Not implemented
  
  char* ID;
  int Status;
  vtkXMLDataElement* CommandXML;
  double CommandTimeoutSec;
  char* ExportedCommandText;
  vtkXMLDataElement* ResponseXML;
  char* ResponseTextInternal;
};

#endif
