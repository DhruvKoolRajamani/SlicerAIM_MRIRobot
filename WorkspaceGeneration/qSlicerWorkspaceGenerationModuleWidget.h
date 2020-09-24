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

#ifndef __qSlicerWorkspaceGenerationModuleWidget_h
#define __qSlicerWorkspaceGenerationModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerWorkspaceGenerationModuleExport.h"

class qSlicerWorkspaceGenerationModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_WORKSPACEGENERATION_EXPORT qSlicerWorkspaceGenerationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWorkspaceGenerationModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWorkspaceGenerationModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerWorkspaceGenerationModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkspaceGenerationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkspaceGenerationModuleWidget);
};

#endif
