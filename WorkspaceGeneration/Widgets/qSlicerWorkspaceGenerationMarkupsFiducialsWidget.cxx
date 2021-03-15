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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware
Inc. and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MarkupsFiducials Widgets includes
#include "qSlicerWorkspaceGenerationMarkupsFiducialsWidget.h"
#include "ui_qSlicerWorkspaceGenerationMarkupsFiducialsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceGeneration
class qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate
  : public Ui_qSlicerWorkspaceGenerationMarkupsFiducialsWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationMarkupsFiducialsWidget);

protected:
  qSlicerWorkspaceGenerationMarkupsFiducialsWidget* const q_ptr;

public:
  qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate(
    qSlicerWorkspaceGenerationMarkupsFiducialsWidget& object);
  virtual void setupUi(qSlicerWorkspaceGenerationMarkupsFiducialsWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate ::
  qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate(
    qSlicerWorkspaceGenerationMarkupsFiducialsWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate ::setupUi(
  qSlicerWorkspaceGenerationMarkupsFiducialsWidget* widget)
{
  this->Ui_qSlicerWorkspaceGenerationMarkupsFiducialsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationMarkupsFiducialsWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationMarkupsFiducialsWidget ::
  qSlicerWorkspaceGenerationMarkupsFiducialsWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerWorkspaceGenerationMarkupsFiducialsWidgetPrivate(*this))
{
  Q_D(qSlicerWorkspaceGenerationMarkupsFiducialsWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationMarkupsFiducialsWidget ::
  ~qSlicerWorkspaceGenerationMarkupsFiducialsWidget()
{
}
