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
#include "qSlicerWorkspaceVisualizationMarkupsFiducialsWidget.h"
#include "ui_qSlicerWorkspaceVisualizationMarkupsFiducialsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceVisualization
class qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate
  : public Ui_qSlicerWorkspaceVisualizationMarkupsFiducialsWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceVisualizationMarkupsFiducialsWidget);

protected:
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidget* const q_ptr;

public:
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate(
    qSlicerWorkspaceVisualizationMarkupsFiducialsWidget& object);
  virtual void setupUi(qSlicerWorkspaceVisualizationMarkupsFiducialsWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate ::
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate(
    qSlicerWorkspaceVisualizationMarkupsFiducialsWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate ::setupUi(
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidget* widget)
{
  this->Ui_qSlicerWorkspaceVisualizationMarkupsFiducialsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceVisualizationMarkupsFiducialsWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationMarkupsFiducialsWidget ::
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate(*this))
{
  Q_D(qSlicerWorkspaceVisualizationMarkupsFiducialsWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationMarkupsFiducialsWidget ::
  ~qSlicerWorkspaceVisualizationMarkupsFiducialsWidget()
{
}
