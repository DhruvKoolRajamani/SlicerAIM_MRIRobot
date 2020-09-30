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

// Qt includes
#include <QtGui>
#include <QDebug>
#include <QButtonGroup>

#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerWorkspaceGenerationModuleWidget.h"
#include "ui_qSlicerWorkspaceGenerationModuleWidget.h"

// Slicer includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLInteractionNode.h"

// module includes
#include "vtkMRMLWorkspaceGenerationNode.h"
#include "vtkSlicerWorkspaceGenerationLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWorkspaceGenerationModuleWidgetPrivate : public Ui_qSlicerWorkspaceGenerationModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationModuleWidget);

protected:
  qSlicerWorkspaceGenerationModuleWidget *const q_ptr;

public:
  qSlicerWorkspaceGenerationModuleWidgetPrivate(qSlicerWorkspaceGenerationModuleWidget &object);
  vtkSlicerWorkspaceGenerationLogic *logic() const;

  // Observed nodes (to keep GUI up-to-date)
  vtkWeakPointer<vtkMRMLWorkspaceGenerationNode> WorkspaceGenerationNode;
  vtkWeakPointer<vtkMRMLMarkupsDisplayNode> MarkupsDisplayNode;
  vtkWeakPointer<vtkMRMLModelDisplayNode> ModelDisplayNode;
};

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidgetPrivate::qSlicerWorkspaceGenerationModuleWidgetPrivate(qSlicerWorkspaceGenerationModuleWidget &object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic *qSlicerWorkspaceGenerationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerWorkspaceGenerationModuleWidget);
  return vtkSlicerWorkspaceGenerationLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::qSlicerWorkspaceGenerationModuleWidget(QWidget *_parent)
    : Superclass(_parent), d_ptr(new qSlicerWorkspaceGenerationModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::~qSlicerWorkspaceGenerationModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setup()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
